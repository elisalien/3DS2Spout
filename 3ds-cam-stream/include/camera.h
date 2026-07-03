#pragma once

#include "common.h"
#include <3ds.h>

typedef struct {
    u16* buffer;     /* frame publiee (toujours complete, protegee par lock) */
    u16* cap_buffer; /* cible DMA du capteur — jamais lue pendant un transfert */
    u32 size_bytes;
    u32 transfer_unit;
    bool ready;
    u32 frame_id;   /* incremente a chaque capture — dedup cote reseau */
    LightLock lock;
    CameraMode mode;
    Handle stop_event;
} CameraContext;

bool camera_init(CameraContext* ctx, CameraMode mode);
void camera_shutdown(CameraContext* ctx);
void camera_set_mode(CameraContext* ctx, CameraMode mode);
Thread camera_start_thread(CameraContext* ctx, AppState* state);
void camera_stop_thread(Thread thread, CameraContext* ctx);
void camera_draw_preview(const u16* rgb565, u16 width, u16 height);
