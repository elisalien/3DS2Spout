#ifndef QOI_H
#define QOI_H

#ifdef __cplusplus
extern "C" {
#endif

#define QOI_SRGB   0
#define QOI_LINEAR 1

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned char channels;
    unsigned char colorspace;
} qoi_desc;

void* qoi_encode(void* bytes, const void* data, const qoi_desc* desc, int* out_len);
unsigned int qoi_max_size(const qoi_desc* desc);

#ifdef __cplusplus
}
#endif
#endif

#ifdef QOI_IMPLEMENTATION
#ifndef QOI_IMPL_INCLUDED
#define QOI_IMPL_INCLUDED
#include <stdlib.h>
#include <string.h>

#ifndef QOI_MALLOC
#define QOI_MALLOC(sz) malloc(sz)
#define QOI_FREE(p)    free(p)
#endif
#ifndef QOI_ZEROARR
#define QOI_ZEROARR(a) memset((a), 0, sizeof(a))
#endif

#define QOI_OP_INDEX 0x00
#define QOI_OP_DIFF  0x40
#define QOI_OP_LUMA  0x80
#define QOI_OP_RUN   0xc0
#define QOI_OP_RGB   0xfe
#define QOI_OP_RGBA  0xff
#define QOI_MASK_2   0xc0
#define QOI_COLOR_HASH(C) (C.rgba.r * 3 + C.rgba.g * 5 + C.rgba.b * 7 + C.rgba.a * 11)
#define QOI_MAGIC \
    (((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | ((unsigned int)'i') << 8 | ((unsigned int)'f'))
#define QOI_HEADER_SIZE 14
#define QOI_PIXELS_MAX ((unsigned int)400000000)

typedef union {
    struct { unsigned char r, g, b, a; } rgba;
    unsigned int v;
} qoi_rgba_t;

static const unsigned char qoi_padding[8] = {0, 0, 0, 0, 0, 0, 0, 1};

unsigned int qoi_max_size(const qoi_desc* desc)
{
    return desc->width * desc->height * (desc->channels + 1) + QOI_HEADER_SIZE + sizeof(qoi_padding);
}

static void qoi_write_32(unsigned char* bytes, int* p, unsigned int v)
{
    bytes[(*p)++] = (0xff000000 & v) >> 24;
    bytes[(*p)++] = (0x00ff0000 & v) >> 16;
    bytes[(*p)++] = (0x0000ff00 & v) >> 8;
    bytes[(*p)++] = (0x000000ff & v);
}

void* qoi_encode(void* bytes, const void* data, const qoi_desc* desc, int* out_len)
{
    unsigned char* out = (unsigned char*)bytes;
    int i, p, run;
    int px_len, px_end, px_pos, channels;
    const unsigned char* pixels;
    qoi_rgba_t index[64];
    qoi_rgba_t px, px_prev;

    if (!data || !out_len || !desc || !bytes ||
        desc->width == 0 || desc->height == 0 ||
        desc->channels < 3 || desc->channels > 4 ||
        desc->colorspace > 1 ||
        desc->height >= QOI_PIXELS_MAX / desc->width) {
        return NULL;
    }

    p = 0;
    qoi_write_32(out, &p, QOI_MAGIC);
    qoi_write_32(out, &p, desc->width);
    qoi_write_32(out, &p, desc->height);
    out[p++] = desc->channels;
    out[p++] = desc->colorspace;

    pixels = (const unsigned char*)data;
    QOI_ZEROARR(index);
    run = 0;
    px_prev.rgba.r = 0;
    px_prev.rgba.g = 0;
    px_prev.rgba.b = 0;
    px_prev.rgba.a = 255;
    px = px_prev;

    px_len = desc->width * desc->height * desc->channels;
    px_end = px_len - desc->channels;
    channels = desc->channels;

    for (px_pos = 0; px_pos < px_len; px_pos += channels) {
        px.rgba.r = pixels[px_pos + 0];
        px.rgba.g = pixels[px_pos + 1];
        px.rgba.b = pixels[px_pos + 2];
        if (channels == 4) {
            px.rgba.a = pixels[px_pos + 3];
        }

        if (px.v == px_prev.v) {
            run++;
            if (run == 62 || px_pos == px_end) {
                out[p++] = QOI_OP_RUN | (run - 1);
                run = 0;
            }
        } else {
            int index_pos;
            if (run > 0) {
                out[p++] = QOI_OP_RUN | (run - 1);
                run = 0;
            }
            index_pos = QOI_COLOR_HASH(px) % 64;
            if (index[index_pos].v == px.v) {
                out[p++] = QOI_OP_INDEX | index_pos;
            } else {
                index[index_pos] = px;
                if (px.rgba.a == px_prev.rgba.a) {
                    signed char vr = px.rgba.r - px_prev.rgba.r;
                    signed char vg = px.rgba.g - px_prev.rgba.g;
                    signed char vb = px.rgba.b - px_prev.rgba.b;
                    signed char vg_r = vr - vg;
                    signed char vg_b = vb - vg;
                    if (vr > -3 && vr < 2 && vg > -3 && vg < 2 && vb > -3 && vb < 2) {
                        out[p++] = QOI_OP_DIFF | ((vr + 2) << 4) | ((vg + 2) << 2) | (vb + 2);
                    } else if (vg_r > -9 && vg_r < 8 && vg > -33 && vg < 32 && vg_b > -9 && vg_b < 8) {
                        out[p++] = QOI_OP_LUMA | (vg + 32);
                        out[p++] = ((vg_r + 8) << 4) | (vg_b + 8);
                    } else {
                        out[p++] = QOI_OP_RGB;
                        out[p++] = px.rgba.r;
                        out[p++] = px.rgba.g;
                        out[p++] = px.rgba.b;
                    }
                } else {
                    out[p++] = QOI_OP_RGBA;
                    out[p++] = px.rgba.r;
                    out[p++] = px.rgba.g;
                    out[p++] = px.rgba.b;
                    out[p++] = px.rgba.a;
                }
            }
        }
        px_prev = px;
    }

    for (i = 0; i < (int)sizeof(qoi_padding); i++) {
        out[p++] = qoi_padding[i];
    }
    *out_len = p;
    return out;
}

#endif /* QOI_IMPL_INCLUDED */
#endif /* QOI_IMPLEMENTATION */
