#pragma once

#include "common.h"
#include "qoi_mod.h"

typedef struct {
    unsigned char* buffer;
    u32 capacity;
    qoi_desc desc;
} EncoderContext;

bool encoder_init(EncoderContext* enc, u16 width, u16 height);
void encoder_shutdown(EncoderContext* enc);
int encoder_rgb565_to_qoi(EncoderContext* enc, const u16* rgb565, u16 width, u16 height);
const unsigned char* encoder_data(const EncoderContext* enc);
void rgb565_to_rgb888(u8* dst, const u16* src, u32 count);
