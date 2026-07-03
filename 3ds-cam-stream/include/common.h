#pragma once

#include <3ds/types.h>
#include <stdbool.h>

#define STREAM_WIDTH  400
#define STREAM_HEIGHT 240
#define PIXEL_COUNT   (STREAM_WIDTH * STREAM_HEIGHT)

#define SOC_ALIGN     0x1000
#define SOC_BUFFERSIZE 0x100000

#define UDP_PORT_DEFAULT 5000

#define PKT_MAGIC     0xFC
#define PKT_HANDSHAKE 0x00
#define PKT_FRAME     0x01
#define PKT_CONTROL   0x02

#define QOI_RGB_CHANNELS 3

typedef enum {
    CAM_REAR = 0,
    CAM_FRONT = 1,
    CAM_STEREO = 2
} CameraMode;

typedef enum {
    LANG_FR = 0,
    LANG_EN = 1
} Language;

typedef struct {
    CameraMode camera;
    bool streaming;
    bool low_res;       /* true = stream 200x120 (fluide), false = 400x240 (HD) */
    bool cam_switching; /* switch camera en cours (reinit capteur ~300ms) */
    char pc_ip[16];
    u16 pc_port;
    int lang;           /* Language : LANG_FR ou LANG_EN, persiste dans le .cfg */
    u32 stream_fps;
    bool pc_connected;
} AppState;

void app_state_defaults(AppState* state);
