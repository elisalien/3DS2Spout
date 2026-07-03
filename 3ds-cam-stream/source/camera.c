#include "camera.h"
#include <3ds.h>
#include <malloc.h>
#include <string.h>

#define WAIT_TIMEOUT 1000000000ULL

typedef struct {
    CameraContext* ctx;
    AppState* state;
} CameraThreadArg;

static u32 cam_select_for_mode(CameraMode mode)
{
    switch (mode) {
    case CAM_FRONT: return SELECT_IN1;
    case CAM_STEREO: return SELECT_OUT1_OUT2;
    default: return SELECT_OUT1;
    }
}

/* Routage materiel (verifie via l'implementation cam:u de Citra) :
 *   PORT_CAM1 <- OUT1 (externe) OU IN1 (interne)
 *   PORT_CAM2 <- OUT2 uniquement (oeil gauche, mode stereo)
 * La camera interne capture donc sur PORT_CAM1, PAS sur PORT_CAM2.
 * Utiliser un port non active met le service cam dans un etat indefini. */
static u32 cam_port_primary(CameraMode mode)
{
    (void)mode;
    return PORT_CAM1;
}

/* Framebuffer GSP_BGR8_OES : column-major (offset = (x*height + y)*3),
 * origine en bas de l'ecran. On parcourt donc x en externe et les lignes
 * image en descendant : le pointeur destination avance sequentiellement,
 * ce qui remplace 96 000 multiplications par des increments et rend les
 * ecritures lineaires (cache/bus beaucoup plus efficaces que l'acces
 * disperse de l'ancienne version ligne par ligne). */
static void write_rgb565_to_top_fb(void* fb, const u16* img, u16 width, u16 height)
{
    u8* dst = (u8*)fb;
    u16 x, y;
    for (x = 0; x < width; x++) {
        const u16* col = img + (u32)(height - 1) * width + x;
        for (y = 0; y < height; y++) {
            u16 data = *col;
            col -= width;
            /* ordre memoire B,G,R */
            dst[0] = (u8)((data & 0x1F) << 3);
            dst[1] = (u8)(((data >> 5) & 0x3F) << 2);
            dst[2] = (u8)(((data >> 11) & 0x1F) << 3);
            dst += 3;
        }
    }
}

void camera_draw_preview(const u16* rgb565, u16 width, u16 height)
{
    u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    write_rgb565_to_top_fb(fb, rgb565, width, height);
}

static bool camera_setup(CameraContext* ctx, CameraMode mode)
{
    u32 select = cam_select_for_mode(mode);
    u32 bufSize;

    CAMU_SetSize(select, SIZE_CTR_TOP_LCD, CONTEXT_A);
    CAMU_SetOutputFormat(select, OUTPUT_RGB_565, CONTEXT_A);
    /* Adaptatif : en basse lumiere le capteur reduit sa cadence au lieu de
     * sortir des frames sombres et bruitees (qui compressent tres mal). */
    CAMU_SetFrameRate(select, FRAME_RATE_30_TO_10);
    /* Flip explicite : la camera interne est mirroree par defaut par le
     * driver, on normalise pour un rendu identique des deux cameras. */
    CAMU_FlipImage(select, FLIP_NONE, CONTEXT_A);
    CAMU_SetNoiseFilter(select, true);
    CAMU_SetAutoExposure(select, true);
    CAMU_SetAutoWhiteBalance(select, true);
    CAMU_SetTrimming(PORT_CAM1, false);
    CAMU_SetTrimming(PORT_CAM2, false);

    if (R_FAILED(CAMU_GetMaxBytes(&bufSize, STREAM_WIDTH, STREAM_HEIGHT))) {
        return false;
    }
    ctx->transfer_unit = bufSize;
    ctx->size_bytes = STREAM_WIDTH * STREAM_HEIGHT * (u32)sizeof(u16);

    if (mode == CAM_STEREO) {
        CAMU_SetTransferBytes(PORT_BOTH, bufSize, STREAM_WIDTH, STREAM_HEIGHT);
        if (R_FAILED(CAMU_Activate(SELECT_OUT1_OUT2))) return false;
        CAMU_ClearBuffer(PORT_BOTH);
        CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2);
        if (R_FAILED(CAMU_StartCapture(PORT_BOTH))) return false;
    } else {
        u32 port = cam_port_primary(mode);
        CAMU_SetTransferBytes(port, bufSize, STREAM_WIDTH, STREAM_HEIGHT);
        if (R_FAILED(CAMU_Activate(select))) return false;
        CAMU_ClearBuffer(port);
        if (R_FAILED(CAMU_StartCapture(port))) return false;
    }

    ctx->mode = mode;
    return true;
}

bool camera_init(CameraContext* ctx, CameraMode mode)
{
    memset(ctx, 0, sizeof(*ctx));
    LightLock_Init(&ctx->lock);
    svcCreateEvent(&ctx->stop_event, RESET_STICKY);

    ctx->size_bytes = STREAM_WIDTH * STREAM_HEIGHT * sizeof(u16);
    ctx->buffer = (u16*)malloc(ctx->size_bytes);
    ctx->cap_buffer = (u16*)malloc(ctx->size_bytes);
    if (!ctx->buffer || !ctx->cap_buffer) {
        return false;
    }

    if (R_FAILED(camInit())) {
        return false;
    }
    return camera_setup(ctx, mode);
}

void camera_shutdown(CameraContext* ctx)
{
    CAMU_StopCapture(PORT_BOTH);
    CAMU_Activate(SELECT_NONE);
    camExit();
    svcCloseHandle(ctx->stop_event);
    free(ctx->buffer);
    free(ctx->cap_buffer);
    ctx->buffer = NULL;
    ctx->cap_buffer = NULL;
}

void camera_set_mode(CameraContext* ctx, CameraMode mode)
{
    if (ctx->mode == mode) return;

    u32 old_port = cam_port_primary(ctx->mode);

    /* Sequence sure : stop -> attendre la fin du DMA -> clear -> deactivate.
     * Desactiver pendant un transfert en cours = data abort (crash). */
    CAMU_StopCapture(old_port);
    bool busy = true;
    int guard = 0;
    while (busy && guard++ < 50) {
        if (R_FAILED(CAMU_IsBusy(&busy, old_port))) break;
        if (busy) svcSleepThread(2 * 1000 * 1000); /* 2 ms */
    }
    CAMU_ClearBuffer(old_port);
    CAMU_Activate(SELECT_NONE);
    svcSleepThread(20 * 1000 * 1000); /* laisse le service cam se poser */

    if (!camera_setup(ctx, mode)) {
        /* Echec : on fige quand meme le mode pour eviter une tempete de
         * retries a chaque iteration du thread camera. */
        ctx->mode = mode;
    }
}

static void camera_thread(void* arg)
{
    CameraThreadArg* a = (CameraThreadArg*)arg;
    CameraContext* ctx = a->ctx;
    AppState* state = a->state;

    bool running = true;
    while (running) {
        if (state->camera != ctx->mode) {
            state->cam_switching = true;
            camera_set_mode(ctx, state->camera);
            state->cam_switching = false;
        }

        u32 port = cam_port_primary(ctx->mode);
        Handle recv = 0;
        /* DMA vers cap_buffer dedie : ecrire directement dans le buffer
         * partage provoquait des frames dechirees (2 images superposees)
         * car le DMA ignore le lock logiciel. */
        Result rres = CAMU_SetReceiving(&recv, ctx->cap_buffer, port,
                                        ctx->size_bytes, (s16)ctx->transfer_unit);
        if (R_FAILED(rres) || !recv) {
            /* camera pas prete (ex: juste apres un switch) : on respire */
            svcSleepThread(20 * 1000 * 1000);
            continue;
        }

        Handle waits[2] = { ctx->stop_event, recv };
        s32 index = -1;
        Result res = svcWaitSynchronizationN(&index, waits, 2, false, WAIT_TIMEOUT);

        if (R_SUCCEEDED(res) && index == 0) {
            running = false;
        } else if (R_SUCCEEDED(res) && index == 1) {
            /* transfert termine : publication de la frame complete */
            LightLock_Lock(&ctx->lock);
            memcpy(ctx->buffer, ctx->cap_buffer, ctx->size_bytes);
            ctx->ready = true;
            ctx->frame_id++;
            LightLock_Unlock(&ctx->lock);
        }

        if (recv) {
            svcCloseHandle(recv);
        }
    }

    free(a);
}

Thread camera_start_thread(CameraContext* ctx, AppState* state)
{
    CameraThreadArg* arg = (CameraThreadArg*)malloc(sizeof(CameraThreadArg));
    arg->ctx = ctx;
    arg->state = state;
    return threadCreate(camera_thread, arg, 64 * 1024, 0x2F, 0, false);
}

void camera_stop_thread(Thread thread, CameraContext* ctx)
{
    svcSignalEvent(ctx->stop_event);
    threadJoin(thread, U64_MAX);
    threadFree(thread);
}
