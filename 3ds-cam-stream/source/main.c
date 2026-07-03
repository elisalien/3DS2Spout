#include "common.h"
#include "camera.h"
#include "encoder.h"
#include "network.h"
#include "touch_ui.h"

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

static AppState g_state;
static CameraContext g_cam;
static EncoderContext g_enc;
static NetworkContext g_net;
static TouchUI g_ui;

static char g_ip_text[32];

/* L'UI n'utilise plus la console : en cas d'echec d'init on l'affiche
 * explicitement et on attend START. */
static void fatal_error(const char* msg)
{
    consoleInit(GFX_BOTTOM, NULL);
    printf("\n ERREUR: %s\n\n START = quitter\n", msg);
    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & KEY_START) break;
        gspWaitForVBlank();
    }
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    gfxInitDefault();
    osSetSpeedupEnable(true); /* 804 MHz sur New3DS — encode QOI bien plus vite */
    gfxSetDoubleBuffering(GFX_TOP, true);
    gfxSetDoubleBuffering(GFX_BOTTOM, false);
    hidScanInput();
    app_state_defaults(&g_state);
    app_load_config(&g_state);
    touch_ui_init(&g_ui);
    g_ui.rec_active = g_state.streaming;

    if (!camera_init(&g_cam, g_state.camera)) {
        fatal_error("camera init");
        goto cleanup;
    }
    if (!encoder_init(&g_enc, STREAM_WIDTH, STREAM_HEIGHT)) {
        fatal_error("encoder init");
        goto cleanup;
    }
    if (!network_init(&g_net, &g_state)) {
        fatal_error("network init (WiFi actif ?)");
        goto cleanup;
    }

    snprintf(g_ip_text, sizeof(g_ip_text), "%s", inet_ntoa((struct in_addr){ .s_addr = gethostid() }));

    Thread cam_th = camera_start_thread(&g_cam, &g_state);
    Thread net_th = network_start_thread(&g_net, &g_state, &g_cam, &g_enc);

    /* Preview : on ne redessine que lorsqu'une NOUVELLE frame camera arrive,
     * et deux fois (une par framebuffer, ecran du haut double-bufferise) pour
     * que les deux buffers restent identiques. Redessiner a chaque VBlank
     * copiait 96 000 pixels pour rien et volait du CPU a l'encodage QOI. */
    u32 last_preview_id = 0;
    int preview_draws_pending = 0;

    while (aptMainLoop()) {
        touch_ui_update(&g_ui, &g_state, &g_net);

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) {
            break;
        }

        {
            u16* preview = NULL;
            LightLock_Lock(&g_cam.lock);
            if (g_cam.ready) {
                preview = g_cam.buffer;
                if (g_cam.frame_id != last_preview_id) {
                    last_preview_id = g_cam.frame_id;
                    preview_draws_pending = 2;
                }
            }
            LightLock_Unlock(&g_cam.lock);
            if (preview && preview_draws_pending > 0) {
                camera_draw_preview(preview, STREAM_WIDTH, STREAM_HEIGHT);
                preview_draws_pending--;
            }
        }

        touch_ui_draw(&g_ui, &g_state, g_ip_text);

        gfxFlushBuffers();
        gspWaitForVBlank();
        gfxSwapBuffers();
    }

    network_stop_thread(net_th, &g_net);
    camera_stop_thread(cam_th, &g_cam);

cleanup:
    network_shutdown(&g_net);
    encoder_shutdown(&g_enc);
    camera_shutdown(&g_cam);
    gfxExit();
    return 0;
}
