#include "touch_ui.h"
#include "ui_draw.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

static bool ipv4_valid(const char* s)
{
    struct in_addr a;
    return s && s[0] && inet_aton(s, &a) != 0;
}

static void edit_append(TouchUI* ui, char c)
{
    size_t n = strlen(ui->edit_ip);
    if (n >= sizeof(ui->edit_ip) - 1) return;
    if (c == '.') {
        if (n == 0 || ui->edit_ip[n - 1] == '.') return;
        int dots = 0;
        for (size_t i = 0; i < n; i++)
            if (ui->edit_ip[i] == '.') dots++;
        if (dots >= 3) return;
    }
    ui->edit_ip[n] = c;
    ui->edit_ip[n + 1] = '\0';
    ui->dirty = true;
}

static void edit_backspace(TouchUI* ui)
{
    size_t n = strlen(ui->edit_ip);
    if (n == 0) return;
    ui->edit_ip[n - 1] = '\0';
    ui->dirty = true;
}

static void config_enter(TouchUI* ui, const AppState* state)
{
    ui->config_mode = true;
    strncpy(ui->edit_ip, state->pc_ip, sizeof(ui->edit_ip) - 1);
    ui->edit_ip[sizeof(ui->edit_ip) - 1] = '\0';
    ui->last_edit_ip[0] = '\0';
    ui->dirty = true;
}

static void config_cancel(TouchUI* ui)
{
    ui->config_mode = false;
    ui->dirty = true;
}

static bool config_save(TouchUI* ui, AppState* state, NetworkContext* net)
{
    if (!ipv4_valid(ui->edit_ip)) {
        return false;
    }
    strncpy(state->pc_ip, ui->edit_ip, sizeof(state->pc_ip) - 1);
    state->pc_ip[sizeof(state->pc_ip) - 1] = '\0';
    app_save_config(state);
    network_set_pc_target(net, state);
    if (state->streaming) {
        network_send_handshake(net, STREAM_WIDTH, STREAM_HEIGHT);
    }
    ui->config_mode = false;
    ui->dirty = true;
    return true;
}

void touch_ui_init(TouchUI* ui)
{
    memset(ui, 0, sizeof(*ui));
    ui->dirty = true;
    ui->last_cam = -1;
    gfxSetDoubleBuffering(GFX_BOTTOM, false);
}

void touch_ui_draw(TouchUI* ui, const AppState* state, const char* ip_text)
{
    u8* fb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
    if (!fb) return;

    if (ui->config_mode) {
        if (!ui->dirty && strncmp(ui->edit_ip, ui->last_edit_ip, sizeof(ui->last_edit_ip)) == 0)
            return;
        ui_render_config(fb, ui->edit_ip, state->lang);
        strncpy(ui->last_edit_ip, ui->edit_ip, sizeof(ui->last_edit_ip) - 1);
        ui->dirty = false;
        return;
    }

    int rec = state->streaming ? 1 : 0;
    int conn = state->pc_connected ? 1 : 0;
    int cam = (state->camera == CAM_FRONT) ? 1 : 0;
    int sw = state->cam_switching ? 1 : 0;
    int lowres = state->low_res ? 1 : 0;

    bool changed = ui->dirty
        || ui->last_rec != rec
        || ui->last_conn != conn
        || ui->last_cam != cam
        || ui->last_switching != sw
        || ui->last_lowres != lowres
        || ui->last_lang != state->lang
        || (rec && conn && ui->last_fps != state->stream_fps)
        || strncmp(state->pc_ip, ui->last_edit_ip, sizeof(ui->last_edit_ip)) != 0;
    if (!changed) {
        return;
    }

    char ip1[40], ip2[40];
    snprintf(ip1, sizeof(ip1), "3DS %s", ip_text ? ip_text : "?");
    snprintf(ip2, sizeof(ip2), "PC  %s:%u", state->pc_ip, (unsigned)state->pc_port);

    UiInfo info = {
        .rec_on = rec,
        .pc_connected = conn,
        .fps = state->stream_fps,
        .cam_front = cam,
        .cam_switching = sw,
        .low_res = lowres,
        .lang = state->lang,
        .line_ip1 = ip1,
        .line_ip2 = ip2,
    };

    ui_render(fb, &info);

    ui->last_rec = rec;
    ui->last_conn = conn;
    ui->last_cam = cam;
    ui->last_switching = sw;
    ui->last_lowres = lowres;
    ui->last_lang = state->lang;
    ui->last_fps = state->stream_fps;
    strncpy(ui->last_edit_ip, state->pc_ip, sizeof(ui->last_edit_ip) - 1);
    ui->last_edit_ip[sizeof(ui->last_edit_ip) - 1] = '\0';
    ui->dirty = false;
    ui->rec_active = state->streaming;
}

static void toggle_rec(TouchUI* ui, AppState* state, NetworkContext* net)
{
    ui->rec_active = !ui->rec_active;
    state->streaming = ui->rec_active;
    ui->dirty = true;
    if (state->streaming) {
        network_send_handshake(net, STREAM_WIDTH, STREAM_HEIGHT);
    } else {
        net->connected = false;
        state->pc_connected = false;
        state->stream_fps = 0;
    }
}

static void select_camera(AppState* state, CameraMode mode)
{
    if (state->camera != mode && !state->cam_switching) {
        state->camera = mode;
        state->cam_switching = true;
        /* dirty flag set by caller path via touch */
    }
}

static void config_handle_touch(TouchUI* ui, AppState* state, NetworkContext* net, int hit)
{
    switch (hit) {
    case UI_CFG_0: edit_append(ui, '0'); break;
    case UI_CFG_1: edit_append(ui, '1'); break;
    case UI_CFG_2: edit_append(ui, '2'); break;
    case UI_CFG_3: edit_append(ui, '3'); break;
    case UI_CFG_4: edit_append(ui, '4'); break;
    case UI_CFG_5: edit_append(ui, '5'); break;
    case UI_CFG_6: edit_append(ui, '6'); break;
    case UI_CFG_7: edit_append(ui, '7'); break;
    case UI_CFG_8: edit_append(ui, '8'); break;
    case UI_CFG_9: edit_append(ui, '9'); break;
    case UI_CFG_DOT: edit_append(ui, '.'); break;
    case UI_CFG_DEL: edit_backspace(ui); break;
    case UI_CFG_SAVE:
        if (!config_save(ui, state, net)) {
            /* IP invalide : garde l'ecran ouvert */
            ui->dirty = true;
        }
        break;
    case UI_CFG_CANCEL:
        config_cancel(ui);
        break;
    case UI_CFG_LANG:
        state->lang = (state->lang == LANG_FR) ? LANG_EN : LANG_FR;
        app_save_config(state);
        ui->dirty = true;
        break;
    default:
        break;
    }
}

void touch_ui_update(TouchUI* ui, AppState* state, NetworkContext* net)
{
    hidScanInput();
    u32 kDown = hidKeysDown();

    if (ui->config_mode) {
        if (kDown & KEY_B) {
            config_cancel(ui);
            return;
        }
        if (!(kDown & KEY_TOUCH)) return;
        touchPosition touch;
        hidTouchRead(&touch);
        config_handle_touch(ui, state, net, ui_config_hit_test(touch.px, touch.py));
        return;
    }

    if (kDown & KEY_Y) {
        select_camera(state, CAM_REAR);
        ui->dirty = true;
    }
    if (kDown & KEY_X) {
        select_camera(state, CAM_FRONT);
        ui->dirty = true;
    }
    if (kDown & KEY_A) toggle_rec(ui, state, net);

    if (!(kDown & KEY_TOUCH)) {
        return;
    }

    touchPosition touch;
    hidTouchRead(&touch);

    switch (ui_hit_test(touch.px, touch.py)) {
    case UI_HIT_REC:
        toggle_rec(ui, state, net);
        break;
    case UI_HIT_CAM_EXT:
        select_camera(state, CAM_REAR);
        ui->dirty = true;
        break;
    case UI_HIT_CAM_INT:
        select_camera(state, CAM_FRONT);
        ui->dirty = true;
        break;
    case UI_HIT_QUALITY:
        state->low_res = !state->low_res;
        ui->dirty = true;
        break;
    case UI_HIT_IP_CFG:
        config_enter(ui, state);
        break;
    default:
        break;
    }
}
