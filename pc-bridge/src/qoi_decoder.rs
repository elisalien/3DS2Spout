use qoi::{Channels, Decoder};

pub fn decode_qoi_to_rgba(data: &[u8]) -> Option<(Vec<u8>, u32, u32)> {
    let mut decoder = Decoder::new(data).ok()?.with_channels(Channels::Rgba);
    let width = decoder.header().width;
    let height = decoder.header().height;
    let pixels = decoder.decode_to_vec().ok()?;
    Some((pixels, width, height))
}
