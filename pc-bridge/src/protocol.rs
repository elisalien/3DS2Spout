pub const PKT_MAGIC: u8 = 0xFC;
pub const PKT_HANDSHAKE: u8 = 0x00;
pub const PKT_FRAME: u8 = 0x01;
pub const PKT_CONTROL: u8 = 0x02;

/// Frame chunk layout (must match FRAME_CHUNK_* in 3ds-cam-stream/source/network.c):
/// FC 01 | seq be32 | total_size be32 | chunk_idx be16 | chunk_count be16 |
/// payload_len be16 | payload (<= FRAME_CHUNK_PAYLOAD bytes).
pub const FRAME_CHUNK_HEADER: usize = 16;
pub const FRAME_CHUNK_PAYLOAD: usize = 1280;

/// Upper bound on a QOI frame (400x240 RGB worst case is ~384 KB); anything
/// bigger is a corrupt/hostile header and must not trigger a huge allocation.
pub const MAX_FRAME_SIZE: usize = 1024 * 1024;

/// True when `a` is a more recent sequence number than `b` (wrap-around safe).
pub fn seq_newer(a: u32, b: u32) -> bool {
    a != b && a.wrapping_sub(b) < 0x8000_0000
}

pub fn read_be32(buf: &[u8]) -> u32 {
    ((buf[0] as u32) << 24)
        | ((buf[1] as u32) << 16)
        | ((buf[2] as u32) << 8)
        | (buf[3] as u32)
}

pub fn read_be16(buf: &[u8]) -> u16 {
    ((buf[0] as u16) << 8) | (buf[1] as u16)
}

pub fn read_be_f32(buf: &[u8]) -> f32 {
    f32::from_bits(read_be32(buf))
}

pub fn write_be32(buf: &mut [u8], v: u32) {
    buf[0] = ((v >> 24) & 0xFF) as u8;
    buf[1] = ((v >> 16) & 0xFF) as u8;
    buf[2] = ((v >> 8) & 0xFF) as u8;
    buf[3] = (v & 0xFF) as u8;
}

pub fn handshake_reply() -> [u8; 3] {
    [PKT_MAGIC, PKT_HANDSHAKE, 1]
}
