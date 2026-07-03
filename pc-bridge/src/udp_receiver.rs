use crate::osc_relay::OscRelay;
use crate::protocol::{
    handshake_reply, read_be16, read_be32, read_be_f32, seq_newer, FRAME_CHUNK_HEADER,
    FRAME_CHUNK_PAYLOAD, MAX_FRAME_SIZE, PKT_CONTROL, PKT_FRAME, PKT_HANDSHAKE, PKT_MAGIC,
};
use crate::qoi_decoder::decode_qoi_to_rgba;
use crate::spout_sender::SpoutSender;
use std::net::UdpSocket;
use std::time::{Duration, Instant};

/// One frame being reassembled from its UDP chunks.
///
/// UDP over the 3DS WiFi loses and reorders packets routinely, so the
/// assembly tracks every chunk individually: duplicates are ignored,
/// out-of-order chunks land at their right offset, and an incomplete frame
/// is dropped as soon as a chunk from a newer frame arrives (never displayed
/// half-filled).
struct FrameAssembly {
    seq: u32,
    total_size: usize,
    chunk_count: u16,
    received: Vec<bool>,
    received_count: u16,
    data: Vec<u8>,
}

impl FrameAssembly {
    fn new(seq: u32, total_size: usize, chunk_count: u16) -> Self {
        Self {
            seq,
            total_size,
            chunk_count,
            received: vec![false; chunk_count as usize],
            received_count: 0,
            data: vec![0u8; total_size],
        }
    }
}

pub struct UdpReceiver {
    socket: UdpSocket,
    buf: Vec<u8>,
    assembly: Option<FrameAssembly>,
    /// Seq of the last fully decoded frame; late chunks for it are ignored.
    last_completed_seq: Option<u32>,
    fps_counter: u32,
    dropped_counter: u32,
    fps_timer: Instant,
    connected_addr: Option<std::net::SocketAddr>,
}

impl UdpReceiver {
    pub fn bind(port: u16) -> std::io::Result<Self> {
        let socket = UdpSocket::bind(format!("0.0.0.0:{port}"))?;
        socket.set_read_timeout(Some(Duration::from_millis(50)))?;
        Ok(Self {
            socket,
            buf: vec![0u8; 65536],
            assembly: None,
            last_completed_seq: None,
            fps_counter: 0,
            dropped_counter: 0,
            fps_timer: Instant::now(),
            connected_addr: None,
        })
    }

    pub fn poll(
        &mut self,
        spout: &mut Option<SpoutSender>,
        osc: &OscRelay,
    ) -> std::io::Result<()> {
        loop {
            match self.socket.recv_from(&mut self.buf) {
                Ok((n, addr)) => {
                    self.connected_addr = Some(addr);
                    let datagram = std::mem::take(&mut self.buf);
                    self.handle_datagram(&datagram[..n], spout, osc);
                    self.buf = datagram;
                }
                Err(e) if e.kind() == std::io::ErrorKind::WouldBlock => break,
                Err(e) if e.kind() == std::io::ErrorKind::TimedOut => break,
                Err(e) => return Err(e),
            }
        }
        Ok(())
    }

    fn handle_datagram(
        &mut self,
        data: &[u8],
        spout: &mut Option<SpoutSender>,
        osc: &OscRelay,
    ) {
        if data.len() < 2 || data[0] != PKT_MAGIC {
            return;
        }

        match data[1] {
            PKT_HANDSHAKE if data.len() >= 10 => {
                let _width = read_be16(&data[6..8]);
                let _height = read_be16(&data[8..10]);
                let reply = handshake_reply();
                if let Some(addr) = self.connected_addr {
                    let _ = self.socket.send_to(&reply, addr);
                }
                println!("Handshake from 3DS ({addr:?})", addr = self.connected_addr);
            }
            PKT_FRAME => self.handle_frame_chunk(data, spout),
            PKT_CONTROL if data.len() >= 7 => {
                let param_id = data[2];
                let value = read_be_f32(&data[3..7]);
                osc.relay_param(param_id, value);
            }
            _ => {}
        }
    }

    /// Chunk header (16 bytes, see protocol.rs / network.c):
    /// FC 01 | seq be32 | total_size be32 | chunk_idx be16 | chunk_count be16 |
    /// payload_len be16 | payload.
    fn handle_frame_chunk(&mut self, data: &[u8], spout: &mut Option<SpoutSender>) {
        if data.len() < FRAME_CHUNK_HEADER {
            return;
        }
        let seq = read_be32(&data[2..6]);
        let total_size = read_be32(&data[6..10]) as usize;
        let chunk_idx = read_be16(&data[10..12]);
        let chunk_count = read_be16(&data[12..14]);
        let payload_len = read_be16(&data[14..16]) as usize;

        // Sanity: reject anything inconsistent instead of allocating blindly.
        if total_size == 0
            || total_size > MAX_FRAME_SIZE
            || chunk_count == 0
            || chunk_idx >= chunk_count
            || payload_len == 0
            || payload_len > FRAME_CHUNK_PAYLOAD
            || data.len() < FRAME_CHUNK_HEADER + payload_len
        {
            return;
        }
        // The sender uses a fixed stride of FRAME_CHUNK_PAYLOAD bytes.
        let expected_count = (total_size + FRAME_CHUNK_PAYLOAD - 1) / FRAME_CHUNK_PAYLOAD;
        if chunk_count as usize != expected_count {
            return;
        }
        // Every chunk is exactly FRAME_CHUNK_PAYLOAD bytes except the last,
        // which carries the remainder. Anything else is a corrupt datagram.
        let offset = chunk_idx as usize * FRAME_CHUNK_PAYLOAD;
        let expected_len = (total_size - offset).min(FRAME_CHUNK_PAYLOAD);
        if payload_len != expected_len {
            return;
        }

        // Resync: if the seq jumps far backwards, the 3DS app was restarted
        // (its counter restarts at 0). Without this, the stream would stay
        // frozen until the new counter catches up with the old one. A LAN
        // never reorders packets by anywhere near RESYNC_GAP frames, so a
        // small gap is a late chunk and a large one is a restart.
        const RESYNC_GAP: u32 = 90; // ~3 s of frames at 30 fps
        let latest = self
            .assembly
            .as_ref()
            .map(|a| a.seq)
            .or(self.last_completed_seq);
        if let Some(latest) = latest {
            if seq != latest && !seq_newer(seq, latest) && latest.wrapping_sub(seq) > RESYNC_GAP {
                println!("Seq reset detected (3DS restart) — resyncing");
                self.assembly = None;
                self.last_completed_seq = None;
            }
        }

        // Late chunk of a frame we already decoded (or older): ignore.
        if let Some(done) = self.last_completed_seq {
            if !seq_newer(seq, done) {
                return;
            }
        }

        // Decide whether to keep filling the current assembly or start fresh.
        // Decisions are computed from copies first, mutations happen after.
        enum Action {
            Continue,
            Restart { drop_current: bool },
            Corrupt,
            IgnoreLate,
        }
        let action = match &self.assembly {
            None => Action::Restart { drop_current: false },
            Some(a) if a.seq == seq => {
                if a.total_size == total_size && a.chunk_count == chunk_count {
                    Action::Continue
                } else {
                    // Same seq but different geometry: corrupt, drop everything.
                    Action::Corrupt
                }
            }
            // A newer frame started before the old one completed: the old
            // one is lost for good (UDP), count it and move on.
            Some(a) if seq_newer(seq, a.seq) => Action::Restart { drop_current: true },
            // Late chunk of an older frame than the one in flight: ignore.
            Some(_) => Action::IgnoreLate,
        };
        match action {
            Action::Continue => {}
            Action::Corrupt => {
                self.assembly = None;
                return;
            }
            Action::IgnoreLate => return,
            Action::Restart { drop_current } => {
                if drop_current {
                    self.dropped_counter += 1;
                }
                self.assembly = Some(FrameAssembly::new(seq, total_size, chunk_count));
            }
        }

        let assembly = self.assembly.as_mut().expect("assembly set above");
        if assembly.received[chunk_idx as usize] {
            return; // duplicate chunk
        }
        assembly.data[offset..offset + payload_len]
            .copy_from_slice(&data[FRAME_CHUNK_HEADER..FRAME_CHUNK_HEADER + payload_len]);
        assembly.received[chunk_idx as usize] = true;
        assembly.received_count += 1;

        if assembly.received_count == assembly.chunk_count {
            let done = self.assembly.take().expect("assembly set above");
            self.last_completed_seq = Some(done.seq);
            self.process_qoi(&done.data, spout);
        }
    }

    fn process_qoi(&mut self, qoi_data: &[u8], spout: &mut Option<SpoutSender>) {
        let Some((rgba, w, h)) = decode_qoi_to_rgba(qoi_data) else {
            return;
        };

        if let Some(sender) = spout {
            if !sender.send_rgba(&rgba, w, h) {
                eprintln!("Spout send failed");
            }
        }

        self.fps_counter += 1;
        if self.fps_timer.elapsed() >= Duration::from_secs(1) {
            println!(
                "FPS: {} | {}x{} | dropped: {} | Spout: {}",
                self.fps_counter,
                w,
                h,
                self.dropped_counter,
                if spout.is_some() { "on" } else { "off" }
            );
            self.fps_counter = 0;
            self.dropped_counter = 0;
            self.fps_timer = Instant::now();
        }
    }

    #[allow(dead_code)]
    pub fn connected(&self) -> bool {
        self.connected_addr.is_some()
    }
}
