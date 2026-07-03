#include "ui_draw.h"
#include <string.h>
#include <stdio.h>

/* ---------------------------------------------------------------- layout */

typedef struct { int id, x, y, w, h; } UiRect;

#define RECT_REC      8,   8, 304, 72
#define RECT_CAM_EXT  8,   92, 148, 60
#define RECT_CAM_INT  164, 92, 148, 60
#define RECT_QUALITY  8,   164, 148, 44
#define RECT_IP_CFG   8,   212, 96,  26

static const UiRect k_zones[] = {
    { UI_HIT_REC,     RECT_REC },
    { UI_HIT_CAM_EXT, RECT_CAM_EXT },
    { UI_HIT_CAM_INT, RECT_CAM_INT },
    { UI_HIT_QUALITY, RECT_QUALITY },
    { UI_HIT_IP_CFG,  RECT_IP_CFG },
};

int ui_hit_test(int x, int y)
{
    unsigned i;
    for (i = 0; i < sizeof(k_zones) / sizeof(k_zones[0]); i++) {
        const UiRect* r = &k_zones[i];
        if (x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h) {
            return r->id;
        }
    }
    return UI_HIT_NONE;
}

/* ---------------------------------------------------------------- pixels */

static void px(uint8_t* fb, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x < 0 || x >= UI_SCREEN_W || y < 0 || y >= UI_SCREEN_H) return;
    int o = (x * UI_SCREEN_H + (UI_SCREEN_H - 1 - y)) * 3;
    fb[o] = b;
    fb[o + 1] = g;
    fb[o + 2] = r;
}

static void fill_rect(uint8_t* fb, int x, int y, int w, int h,
                      uint8_t r, uint8_t g, uint8_t b)
{
    int i, j;
    for (i = 0; i < h; i++)
        for (j = 0; j < w; j++)
            px(fb, x + j, y + i, r, g, b);
}

static void border_rect(uint8_t* fb, int x, int y, int w, int h,
                        uint8_t r, uint8_t g, uint8_t b)
{
    int i;
    for (i = 0; i < w; i++) {
        px(fb, x + i, y, r, g, b);
        px(fb, x + i, y + h - 1, r, g, b);
    }
    for (i = 0; i < h; i++) {
        px(fb, x, y + i, r, g, b);
        px(fb, x + w - 1, y + i, r, g, b);
    }
}

/* ---------------------------------------------------------------- police */
/* 5x7, 5 bits de poids faible par ligne */

static const uint8_t F_SP[7]  = { 0, 0, 0, 0, 0, 0, 0 };
static const uint8_t F_NUM[10][7] = {
    { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E }, /* 0 */
    { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E }, /* 1 */
    { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F }, /* 2 */
    { 0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E }, /* 3 */
    { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 }, /* 4 */
    { 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E }, /* 5 */
    { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E }, /* 6 */
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 }, /* 7 */
    { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E }, /* 8 */
    { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C }, /* 9 */
};
static const uint8_t F_ALPHA[26][7] = {
    { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 }, /* A */
    { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E }, /* B */
    { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E }, /* C */
    { 0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C }, /* D */
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F }, /* E */
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 }, /* F */
    { 0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F }, /* G */
    { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 }, /* H */
    { 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E }, /* I */
    { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C }, /* J */
    { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 }, /* K */
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F }, /* L */
    { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 }, /* M */
    { 0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11 }, /* N */
    { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E }, /* O */
    { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 }, /* P */
    { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D }, /* Q */
    { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 }, /* R */
    { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E }, /* S */
    { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }, /* T */
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E }, /* U */
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 }, /* V */
    { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A }, /* W */
    { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 }, /* X */
    { 0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04 }, /* Y */
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F }, /* Z */
};
static const uint8_t F_COLON[7] = { 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00 };
static const uint8_t F_DOT[7]   = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C };
static const uint8_t F_DASH[7]  = { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 };
static const uint8_t F_LT[7]    = { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02 };
static const uint8_t F_GT[7]    = { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 };
static const uint8_t F_SLASH[7] = { 0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10 };
static const uint8_t F_QM[7]    = { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 };
static const uint8_t F_EQ[7]    = { 0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00 };

static const uint8_t* glyph(char c)
{
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    if (c >= 'A' && c <= 'Z') return F_ALPHA[c - 'A'];
    if (c >= '0' && c <= '9') return F_NUM[c - '0'];
    switch (c) {
    case ':': return F_COLON;
    case '.': return F_DOT;
    case '-': return F_DASH;
    case '<': return F_LT;
    case '>': return F_GT;
    case '/': return F_SLASH;
    case '?': return F_QM;
    case '=': return F_EQ;
    default:  return F_SP;
    }
}

static void draw_text(uint8_t* fb, int x, int y, int scale,
                      uint8_t r, uint8_t g, uint8_t b, const char* s)
{
    int cx = x;
    for (; *s; s++) {
        const uint8_t* gl = glyph(*s);
        int row, col, sy, sx;
        for (row = 0; row < 7; row++) {
            for (col = 0; col < 5; col++) {
                if (gl[row] & (1 << (4 - col))) {
                    for (sy = 0; sy < scale; sy++)
                        for (sx = 0; sx < scale; sx++)
                            px(fb, cx + col * scale + sx, y + row * scale + sy, r, g, b);
                }
            }
        }
        cx += 6 * scale;
    }
}

static int text_w(const char* s, int scale)
{
    return (int)strlen(s) * 6 * scale - scale;
}

static void draw_text_centered(uint8_t* fb, int x, int y, int w, int h, int scale,
                               uint8_t r, uint8_t g, uint8_t b, const char* s)
{
    draw_text(fb, x + (w - text_w(s, scale)) / 2, y + (h - 7 * scale) / 2,
              scale, r, g, b, s);
}

/* ---------------------------------------------------------------- i18n */

enum {
    S_WAITING, S_PC_DOTS, S_CAM_EXT, S_CAM_INT,
    S_Q_SMOOTH, S_Q_HD, S_IP_BTN, S_HELP,
    S_CFG_TITLE, S_CFG_HINT, S_CANCEL, S_LANG_BTN,
    S__COUNT
};

static const char* k_str[2][S__COUNT] = {
    /* FR */
    { "ATTENTE", "PC...", "EXTERNE", "INTERNE",
      "FLUIDE 200P", "HD 400P", "IP PC", "A=REC  Y/X=CAM  START=QUIT",
      "IP DU PC", "Tape l'IP affichee sur le PC", "ANNUL", "LANGUE FR" },
    /* EN */
    { "WAITING", "PC...", "OUTER", "INNER",
      "SMOOTH 200P", "HD 400P", "PC IP", "A=REC  Y/X=CAM  START=QUIT",
      "PC IP ADDRESS", "Enter the IP shown on the PC", "CANCEL", "LANG EN" },
};

static const char* tr(int lang, int id)
{
    return k_str[lang == 1 ? 1 : 0][id];
}

/* ---------------------------------------------------------------- theme */

#define C_BG      14, 14, 22
#define C_PANEL   40, 42, 58
#define C_BORDER  70, 74, 96
#define C_TEXT    235, 235, 240
#define C_MUTED   150, 152, 168
#define C_REC_ON  190, 40, 45
#define C_REC_HI  255, 90, 90
#define C_ACT     20, 110, 120
#define C_ACT_HI  60, 200, 210

static void button(uint8_t* fb, int x, int y, int w, int h, int active)
{
    if (active) {
        fill_rect(fb, x, y, w, h, C_ACT);
        border_rect(fb, x, y, w, h, C_ACT_HI);
    } else {
        fill_rect(fb, x, y, w, h, C_PANEL);
        border_rect(fb, x, y, w, h, C_BORDER);
    }
}

/* ---------------------------------------------------------------- rendu */

void ui_render(uint8_t* fb, const UiInfo* info)
{
    char buf[48];

    fill_rect(fb, 0, 0, UI_SCREEN_W, UI_SCREEN_H, C_BG);

    /* --- REC --- */
    if (info->rec_on) {
        fill_rect(fb, RECT_REC, C_REC_ON);
        border_rect(fb, RECT_REC, C_REC_HI);
    } else {
        fill_rect(fb, RECT_REC, C_PANEL);
        border_rect(fb, RECT_REC, C_BORDER);
    }
    /* pastille + label */
    fill_rect(fb, 28, 32, 18, 18, info->rec_on ? 255 : 90, 90, 90);
    draw_text(fb, 62, 32, 3, C_TEXT, "REC");
    if (info->rec_on) {
        if (info->pc_connected) {
            draw_text(fb, 192, 22, 2, C_TEXT, "PC OK");
            snprintf(buf, sizeof(buf), "%u FPS", info->fps);
            draw_text(fb, 192, 46, 2, C_TEXT, buf);
        } else {
            draw_text(fb, 192, 22, 2, C_TEXT, tr(info->lang, S_WAITING));
            draw_text(fb, 192, 46, 2, C_TEXT, tr(info->lang, S_PC_DOTS));
        }
    } else {
        draw_text(fb, 192, 34, 2, 200, 202, 214, "OFF");
    }

    /* --- CAMERA --- */
    button(fb, RECT_CAM_EXT, !info->cam_front && !info->cam_switching);
    button(fb, RECT_CAM_INT, info->cam_front && !info->cam_switching);
    if (info->cam_switching) {
        /* sablier sur le bouton cible, l'autre reste nomme */
        if (info->cam_front) {
            draw_text_centered(fb, RECT_CAM_EXT, 2, C_MUTED, tr(info->lang, S_CAM_EXT));
            draw_text_centered(fb, RECT_CAM_INT, 3, C_ACT_HI, "...");
        } else {
            draw_text_centered(fb, RECT_CAM_EXT, 3, C_ACT_HI, "...");
            draw_text_centered(fb, RECT_CAM_INT, 2, C_MUTED, tr(info->lang, S_CAM_INT));
        }
    } else {
        draw_text_centered(fb, RECT_CAM_EXT, 2, C_TEXT, tr(info->lang, S_CAM_EXT));
        draw_text_centered(fb, RECT_CAM_INT, 2, C_TEXT, tr(info->lang, S_CAM_INT));
    }

    /* --- QUALITE --- */
    button(fb, RECT_QUALITY, info->low_res);
    draw_text_centered(fb, RECT_QUALITY, 1, C_TEXT,
                       tr(info->lang, info->low_res ? S_Q_SMOOTH : S_Q_HD));

    /* --- infos reseau (a droite du bloc qualite, colonne INTERNE) --- */
    if (info->line_ip1) draw_text(fb, 164, 175, 1, C_MUTED, info->line_ip1);
    if (info->line_ip2) draw_text(fb, 164, 189, 1, C_MUTED, info->line_ip2);

    /* --- config IP PC --- */
    button(fb, RECT_IP_CFG, 0);
    draw_text_centered(fb, RECT_IP_CFG, 1, C_ACT_HI, tr(info->lang, S_IP_BTN));

    /* --- aide boutons --- */
    draw_text_centered(fb, 110, 218, 210, 14, 1, C_MUTED,
                       tr(info->lang, S_HELP));
}

/* ---------------------------------------------------------------- config IP */

typedef struct { int id, x, y, w, h; const char* label; } UiKey;

static const UiKey k_cfg_keys[] = {
    { UI_CFG_1, 16,  68, 88, 34, "1" },
    { UI_CFG_2, 116, 68, 88, 34, "2" },
    { UI_CFG_3, 216, 68, 88, 34, "3" },
    { UI_CFG_4, 16,  106, 88, 34, "4" },
    { UI_CFG_5, 116, 106, 88, 34, "5" },
    { UI_CFG_6, 216, 106, 88, 34, "6" },
    { UI_CFG_7, 16,  144, 88, 34, "7" },
    { UI_CFG_8, 116, 144, 88, 34, "8" },
    { UI_CFG_9, 216, 144, 88, 34, "9" },
    { UI_CFG_DEL, 16,  182, 88, 34, "DEL" },
    { UI_CFG_0, 116, 182, 88, 34, "0" },
    { UI_CFG_DOT, 216, 182, 88, 34, "." },
    { UI_CFG_SAVE, 16,  216, 140, 22, "OK" },
    { UI_CFG_CANCEL, 164, 216, 140, 22, "ANNUL" },
    { UI_CFG_LANG, 216, 4, 88, 26, "" }, /* label dynamique (FR/EN) */
};

int ui_config_hit_test(int x, int y)
{
    unsigned i;
    for (i = 0; i < sizeof(k_cfg_keys) / sizeof(k_cfg_keys[0]); i++) {
        const UiKey* k = &k_cfg_keys[i];
        if (x >= k->x && x < k->x + k->w && y >= k->y && y < k->y + k->h)
            return k->id;
    }
    return UI_CFG_NONE;
}

void ui_render_config(uint8_t* fb, const char* edit_ip, int lang)
{
    fill_rect(fb, 0, 0, UI_SCREEN_W, UI_SCREEN_H, C_BG);
    draw_text(fb, 16, 8, 2, C_ACT_HI, tr(lang, S_CFG_TITLE));
    draw_text(fb, 16, 30, 1, C_MUTED, tr(lang, S_CFG_HINT));

    fill_rect(fb, 12, 44, 296, 22, C_PANEL);
    border_rect(fb, 12, 44, 296, 22, C_BORDER);
    if (edit_ip && edit_ip[0])
        draw_text(fb, 20, 48, 2, C_TEXT, edit_ip);
    else
        draw_text(fb, 20, 48, 2, C_MUTED, "192.168.0.1");

    unsigned i;
    for (i = 0; i < sizeof(k_cfg_keys) / sizeof(k_cfg_keys[0]); i++) {
        const UiKey* k = &k_cfg_keys[i];
        const char* label = k->label;
        int scale = 2;
        if (k->id == UI_CFG_CANCEL) label = tr(lang, S_CANCEL);
        if (k->id == UI_CFG_LANG) { label = tr(lang, S_LANG_BTN); scale = 1; }
        int accent = (k->id == UI_CFG_SAVE);
        if (accent)
            fill_rect(fb, k->x, k->y, k->w, k->h, C_ACT);
        else
            button(fb, k->x, k->y, k->w, k->h, 0);
        draw_text_centered(fb, k->x, k->y, k->w, k->h, scale, C_TEXT, label);
    }
}
