use crate::osc_relay::OscRelay;
use crate::protocol::{handshake_reply, read_be16, read_be32, read_be_f32, PKT_CONTROL, PKT_FRAME, PKT_HANDSHAKE, PKT_MAGIC};
use crate::qoi_decoder::decode_qoi_to_rgba;
use crate::spout_sender::SpoutSender;
use std::net::UdpSocket;
use std::time::{Duration, Instant};

pub struct UdpReceiver {
    socket: UdpSocket,
    buf: Vec<u8>,
    frame_acc: Vec<u8>,
    expected_frame_size: usize,
    expecting_frame: bool,
    last_seq: u32,
    fps_counter: u32,
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
            frame_acc: Vec::new(),
            expected_frame_size: 0,
            expecting_frame: false,
            last_seq: 0,
            fps_counter: 0,
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
                    self.handle_datagram(&self.buf[..n], spout, osc);
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
        if self.expecting_frame {
            if data.len() >= 2 && data[0] == PKT_MAGIC && data[1] == PKT_FRAME {
                self.expecting_frame = false;
                self.frame_acc.clear();
                self.handle_packet(data, spout, osc);
                return;
            }
            self.frame_acc.extend_from_slice(data);
            if self.frame_acc.len() >= self.expected_frame_size {
                let frame = self.frame_acc[..self.expected_frame_size].to_vec();
                self.expecting_frame = false;
                self.process_qoi(&frame, spout);
            }
            return;
        }

        if data.len() < 2 || data[0] != PKT_MAGIC {
            return;
        }

        self.handle_packet(data, spout, osc);
    }

    fn handle_packet(
        &mut self,
        data: &[u8],
        spout: &mut Option<SpoutSender>,
        osc: &OscRelay,
    ) {
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
            PKT_FRAME if data.len() >= 14 => {
                let seq = read_be32(&data[2..6]);
                let _ts = read_be32(&data[6..10]);
                let size = read_be32(&data[10..14]) as usize;

                if seq <= self.last_seq && self.last_seq != 0 && seq.wrapping_sub(self.last_seq) < 0x8000_0000 {
                    return;
                }
                self.last_seq = seq;

                if data.len() >= 14 + size {
                    self.process_qoi(&data[14..14 + size], spout);
                } else {
                    self.expecting_frame = true;
                    self.expected_frame_size = size;
                    self.frame_acc.clear();
                    self.frame_acc.extend_from_slice(&data[14..]);
                }
            }
            PKT_CONTROL if data.len() >= 7 => {
                let param_id = data[2];
                let value = read_be_f32(&data[3..7]);
                osc.relay_param(param_id, value);
            }
            _ => {}
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
                "FPS: {} | {}x{} | Spout: {}",
                self.fps_counter,
                w,
                h,
                if spout.is_some() { "on" } else { "off" }
            );
            self.fps_counter = 0;
            self.fps_timer = Instant::now();
        }
    }

    pub fn connected(&self) -> bool {
        self.connected_addr.is_some()
    }
}
