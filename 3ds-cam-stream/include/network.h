#pragma once

#include "common.h"
#include "camera.h"
#include "encoder.h"
#include <3ds.h>
#include <netinet/in.h>

typedef struct {
    int sock;
    struct sockaddr_in pc_addr;
    struct sockaddr_in local_addr;
    u32 seq;
    bool connected;
    LightLock lock;
    Handle stop_event;
} NetworkContext;

bool network_init(NetworkContext* net, const AppState* state);
void network_shutdown(NetworkContext* net);
bool network_send_handshake(NetworkContext* net, u16 width, u16 height);
bool network_send_frame(NetworkContext* net, const u8* qoi, u32 qoi_size, u32 timestamp_ms);
bool network_send_control(NetworkContext* net, u8 param_id, float value);
bool network_poll_handshake_ack(NetworkContext* net);
Thread network_start_thread(NetworkContext* net, AppState* state,
                            CameraContext* cam, EncoderContext* enc);
void network_stop_thread(Thread thread, NetworkContext* net);

bool app_load_config(AppState* state);
bool app_save_config(const AppState* state);
void network_set_pc_target(NetworkContext* net, const AppState* state);
