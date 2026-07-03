#define QOI_IMPLEMENTATION
#include "qoi_mod.h"

#include "encoder.h"
#include <stdlib.h>
#include <string.h>

void rgb565_to_rgb888(u8* dst, const u16* src, u32 count)
{
    u32 i;
    for (i = 0; i < count; i++) {
        u16 px = src[i];
        dst[i * 3 + 0] = (u8)(((px >> 11) & 0x1F) << 3);
        dst[i * 3 + 1] = (u8)(((px >> 5) & 0x3F) << 2);
        dst[i * 3 + 2] = (u8)((px & 0x1F) << 3);
    }
}

bool encoder_init(EncoderContext* enc, u16 width, u16 height)
{
    enc->desc.width = width;
    enc->desc.height = height;
    enc->desc.channels = QOI_RGB_CHANNELS;
    enc->desc.colorspace = QOI_LINEAR;
    enc->capacity = qoi_max_size(&enc->desc);
    enc->buffer = (unsigned char*)malloc(enc->capacity);
    return enc->buffer != NULL;
}

void encoder_shutdown(EncoderContext* enc)
{
    free(enc->buffer);
    enc->buffer = NULL;
    enc->capacity = 0;
}

int encoder_rgb565_to_qoi(EncoderContext* enc, const u16* rgb565, u16 width, u16 height)
{
    static u8* rgb_work = NULL;
    static u32 rgb_work_cap = 0;
    u32 needed = (u32)width * height * 3;

    if (needed > rgb_work_cap) {
        free(rgb_work);
        rgb_work = (u8*)malloc(needed);
        if (!rgb_work) return -1;
        rgb_work_cap = needed;
    }

    rgb565_to_rgb888(rgb_work, rgb565, (u32)width * height);

    enc->desc.width = width;
    enc->desc.height = height;
    int out_len = 0;
    if (!qoi_encode(enc->buffer, rgb_work, &enc->desc, &out_len)) {
        return -1;
    }
    return out_len;
}

const unsigned char* encoder_data(const EncoderContext* enc)
{
    return enc->buffer;
}

u32 encoder_size(const EncoderContext* enc)
{
    (void)enc;
    return 0;
}
