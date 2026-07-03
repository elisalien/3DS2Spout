#define QOI_IMPLEMENTATION
#include "qoi_mod.h"

#include "encoder.h"
#include <stdlib.h>
#include <string.h>

/* Expansion 5/6 bits -> 8 bits par table avec replication des bits de poids
 * fort ((i<<3)|(i>>2)) : 0x1F donne 255 et non 248, donc les blancs sont
 * vraiment blancs cote Spout, sans cout supplementaire par pixel. Tables
 * remplies au premier appel (appele uniquement depuis le thread reseau). */
void rgb565_to_rgb888(u8* dst, const u16* src, u32 count)
{
    static u8 lut5[32];
    static u8 lut6[64];
    static bool lut_ready = false;
    u32 i;

    if (!lut_ready) {
        for (i = 0; i < 32; i++) lut5[i] = (u8)((i << 3) | (i >> 2));
        for (i = 0; i < 64; i++) lut6[i] = (u8)((i << 2) | (i >> 4));
        lut_ready = true;
    }

    for (i = 0; i < count; i++) {
        u16 px = src[i];
        dst[0] = lut5[px >> 11];
        dst[1] = lut6[(px >> 5) & 0x3F];
        dst[2] = lut5[px & 0x1F];
        dst += 3;
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
