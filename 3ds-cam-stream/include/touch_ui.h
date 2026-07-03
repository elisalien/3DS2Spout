#pragma once

#include "common.h"
#include "network.h"

typedef struct {
    bool rec_active;
    bool dirty;          /* force un redraw au prochain touch_ui_draw */
    bool config_mode;    /* ecran saisie IP PC */
    char edit_ip[16];    /* buffer edition tactile */
    /* snapshot du dernier rendu — on ne redessine que si ca change */
    int last_rec;
    int last_conn;
    int last_cam;
    int last_switching;
    int last_lowres;
    int last_lang;
    u32 last_fps;
    char last_edit_ip[16];
} TouchUI;

void touch_ui_init(TouchUI* ui);
void touch_ui_draw(TouchUI* ui, const AppState* state, const char* ip_text);
void touch_ui_update(TouchUI* ui, AppState* state, NetworkContext* net);
