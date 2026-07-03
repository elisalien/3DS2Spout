#pragma once

/* Rendu UI ecran bas — aucune dependance libctru, testable sur PC.
 * Le framebuffer bottom 3DS fait 240x320 (tourne 90 degres) :
 * pixel ecran (x,y) avec 0<=x<320, 0<=y<240 -> offset ((x*240)+(239-y))*3,
 * ordre memoire B,G,R (GSP_BGR8_OES). */

#include <stdint.h>

#define UI_SCREEN_W 320
#define UI_SCREEN_H 240
#define UI_FB_BYTES (UI_SCREEN_W * UI_SCREEN_H * 3)

typedef struct {
    int rec_on;
    int pc_connected;
    unsigned fps;
    int cam_front;        /* 0 = externe, 1 = interne */
    int cam_switching;    /* 1 = reinit capteur en cours -> sablier */
    int low_res;          /* 1 = 200x120 fluide, 0 = 400x240 HD */
    int lang;             /* 0 = FR, 1 = EN */
    const char* line_ip1; /* ex: "3DS 192.168.1.45" */
    const char* line_ip2; /* ex: "PC  192.168.1.42:5000" */
} UiInfo;

/* Zones tactiles (ecran principal) */
enum {
    UI_HIT_NONE = 0,
    UI_HIT_REC,
    UI_HIT_CAM_EXT,
    UI_HIT_CAM_INT,
    UI_HIT_QUALITY,
    UI_HIT_IP_CFG
};

/* Zones saisie IP (ecran config) */
enum {
    UI_CFG_NONE = 0,
    UI_CFG_0, UI_CFG_1, UI_CFG_2, UI_CFG_3, UI_CFG_4,
    UI_CFG_5, UI_CFG_6, UI_CFG_7, UI_CFG_8, UI_CFG_9,
    UI_CFG_DOT,
    UI_CFG_DEL,
    UI_CFG_SAVE,
    UI_CFG_CANCEL,
    UI_CFG_LANG
};

void ui_render(uint8_t* fb, const UiInfo* info);
void ui_render_config(uint8_t* fb, const char* edit_ip, int lang);
int ui_hit_test(int x, int y);
int ui_config_hit_test(int x, int y);
