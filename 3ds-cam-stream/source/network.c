#include "network.h"
#include "encoder.h"
#include "camera.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static u32* g_soc_buffer = NULL;

static void write_be32(u8* p, u32 v)
{
    p[0] = (u8)((v >> 24) & 0xFF);
    p[1] = (u8)((v >> 16) & 0xFF);
    p[2] = (u8)((v >> 8) & 0xFF);
    p[3] = (u8)(v & 0xFF);
}

static void write_be16(u8* p, u16 v)
{
    p[0] = (u8)((v >> 8) & 0xFF);
    p[1] = (u8)(v & 0xFF);
}

static void write_be_f32(u8* p, float v)
{
    union { float f; u32 i; } u;
    u.f = v;
    write_be32(p, u.i);
}

static bool parse_cfg_line(const char* line, char* key, size_t key_sz, char* val, size_t val_sz)
{
    const char* eq = strchr(line, '=');
    if (!eq) return false;
    size_t klen = (size_t)(eq - line);
    while (klen > 0 && (line[klen - 1] == ' ' || line[klen - 1] == '\t')) klen--;
    size_t i = 0;
    while (i < klen && line[i] == ' ') i++;
    if (klen - i >= key_sz) return false;
    memcpy(key, line + i, klen - i);
    key[klen - i] = '\0';

    const char* v = eq + 1;
    while (*v == ' ' || *v == '\t') v++;
    snprintf(val, val_sz, "%s", v);
    size_t vlen = strlen(val);
    while (vlen > 0 && (val[vlen - 1] == '\r' || val[vlen - 1] == '\n' || val[vlen - 1] == ' '))
        val[--vlen] = '\0';
    return key[0] != '\0' && val[0] != '\0';
}

bool app_load_config(AppState* state)
{
    FILE* f = fopen("sdmc:/3ds-cam-stream.cfg", "r");
    if (!f) return false;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        char key[32], val[16];
        if (!parse_cfg_line(line, key, sizeof(key), val, sizeof(val))) continue;
        if (strcmp(key, "pc_ip") == 0) {
            snprintf(state->pc_ip, sizeof(state->pc_ip), "%s", val);
        } else if (strcmp(key, "pc_port") == 0) {
            state->pc_port = (u16)atoi(val);
        } else if (strcmp(key, "lang") == 0) {
            state->lang = (val[0] == 'e' || val[0] == 'E') ? LANG_EN : LANG_FR;
        }
    }
    fclose(f);
    return true;
}

bool app_save_config(const AppState* state)
{
    FILE* f = fopen("sdmc:/3ds-cam-stream.cfg", "w");
    if (!f) return false;
    fprintf(f, "pc_ip=%s\npc_port=%u\nlang=%s\n",
            state->pc_ip, (unsigned)state->pc_port,
            state->lang == LANG_EN ? "en" : "fr");
    fclose(f);
    return true;
}

void network_set_pc_target(NetworkContext* net, const AppState* state)
{
    LightLock_Lock(&net->lock);
    net->pc_addr.sin_port = htons(state->pc_port);
    inet_aton(state->pc_ip, &net->pc_addr.sin_addr);
    net->connected = false;
    LightLock_Unlock(&net->lock);
}

bool network_init(NetworkContext* net, const AppState* state)
{
    memset(net, 0, sizeof(*net));
    LightLock_Init(&net->lock);
    svcCreateEvent(&net->stop_event, RESET_STICKY);

    g_soc_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if (!g_soc_buffer) return false;

    acInit();
    if (R_FAILED(socInit(g_soc_buffer, SOC_BUFFERSIZE))) {
        free(g_soc_buffer);
        g_soc_buffer = NULL;
        return false;
    }

    net->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (net->sock < 0) return false;

    memset(&net->pc_addr, 0, sizeof(net->pc_addr));
    net->pc_addr.sin_family = AF_INET;
    net->pc_addr.sin_port = htons(state->pc_port);
    inet_aton(state->pc_ip, &net->pc_addr.sin_addr);

    memset(&net->local_addr, 0, sizeof(net->local_addr));
    net->local_addr.sin_family = AF_INET;
    net->local_addr.sin_port = htons(state->pc_port);
    net->local_addr.sin_addr.s_addr = gethostid();

    int flags = fcntl(net->sock, F_GETFL, 0);
    fcntl(net->sock, F_SETFL, flags | O_NONBLOCK);

    return true;
}

void network_shutdown(NetworkContext* net)
{
    if (net->sock >= 0) {
        close(net->sock);
        net->sock = -1;
    }
    if (g_soc_buffer) {
        socExit();
        acExit();
        free(g_soc_buffer);
        g_soc_buffer = NULL;
    }
    svcCloseHandle(net->stop_event);
}

bool network_send_handshake(NetworkContext* net, u16 width, u16 height)
{
    u8 pkt[10];
    pkt[0] = PKT_MAGIC;
    pkt[1] = PKT_HANDSHAKE;
    write_be32(pkt + 2, gethostid());
    write_be16(pkt + 6, width);
    write_be16(pkt + 8, height);
    int sent = sendto(net->sock, pkt, sizeof(pkt), 0,
                      (struct sockaddr*)&net->pc_addr, sizeof(net->pc_addr));
    return sent == (int)sizeof(pkt);
}

bool network_poll_handshake_ack(NetworkContext* net)
{
    u8 buf[8];
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    int n = recvfrom(net->sock, buf, sizeof(buf), 0, (struct sockaddr*)&from, &fromlen);
    if (n >= 3 && buf[0] == PKT_MAGIC && buf[1] == PKT_HANDSHAKE && buf[2] == 1) {
        net->connected = true;
        return true;
    }
    return false;
}

/* Frame chunk: FC 01 | seq be32 | total_size be32 | chunk_idx be16 |
 * chunk_count be16 | payload_len be16 | payload.
 * 16-byte header + <=1280 payload = 1296 bytes: under the WiFi MTU,
 * so no IP fragmentation and no >64KB sendto on the 3DS. */
#define FRAME_CHUNK_PAYLOAD 1280
#define FRAME_CHUNK_HEADER  16

bool network_send_frame(NetworkContext* net, const u8* qoi, u32 qoi_size, u32 timestamp_ms)
{
    (void)timestamp_ms;
    static u8 pkt[FRAME_CHUNK_HEADER + FRAME_CHUNK_PAYLOAD];

    u16 chunk_count = (u16)((qoi_size + FRAME_CHUNK_PAYLOAD - 1) / FRAME_CHUNK_PAYLOAD);
    if (chunk_count == 0) return false;

    LightLock_Lock(&net->lock);
    u32 seq = net->seq++;
    bool ok = true;

    for (u16 idx = 0; idx < chunk_count && ok; idx++) {
        u32 off = (u32)idx * FRAME_CHUNK_PAYLOAD;
        u16 payload_len = (u16)((qoi_size - off > FRAME_CHUNK_PAYLOAD)
                                    ? FRAME_CHUNK_PAYLOAD
                                    : (qoi_size - off));
        pkt[0] = PKT_MAGIC;
        pkt[1] = PKT_FRAME;
        write_be32(pkt + 2, seq);
        write_be32(pkt + 6, qoi_size);
        write_be16(pkt + 10, idx);
        write_be16(pkt + 12, chunk_count);
        write_be16(pkt + 14, payload_len);
        memcpy(pkt + FRAME_CHUNK_HEADER, qoi + off, payload_len);

        int want = FRAME_CHUNK_HEADER + payload_len;
        int sent = sendto(net->sock, pkt, want, 0,
                          (struct sockaddr*)&net->pc_addr, sizeof(net->pc_addr));
        /* Socket non bloquant : buffer plein -> retries courts, sinon la frame
         * part a moitie et la bande passante est gaspillee. */
        int tries = 0;
        while (sent != want && sent < 0 && errno == EWOULDBLOCK && tries < 10) {
            svcSleepThread(1 * 1000 * 1000); /* 1 ms */
            sent = sendto(net->sock, pkt, want, 0,
                          (struct sockaddr*)&net->pc_addr, sizeof(net->pc_addr));
            tries++;
        }
        if (sent != want) ok = false;
    }
    LightLock_Unlock(&net->lock);
    return ok;
}

bool network_send_control(NetworkContext* net, u8 param_id, float value)
{
    u8 pkt[7];
    pkt[0] = PKT_MAGIC;
    pkt[1] = PKT_CONTROL;
    pkt[2] = param_id;
    write_be_f32(pkt + 3, value);
    int sent = sendto(net->sock, pkt, sizeof(pkt), 0,
                      (struct sockaddr*)&net->pc_addr, sizeof(net->pc_addr));
    return sent == (int)sizeof(pkt);
}

typedef struct {
    NetworkContext* net;
    AppState* state;
    CameraContext* cam;
    EncoderContext* enc;
} NetworkThreadArg;

/* Downscale 2x2 box average RGB565 : denoise le signal camera, ce qui
 * multiplie le taux de compression QOI, et divise les pixels par 4.
 * ~5-10x moins d'octets par frame -> le WiFi 3DS suit. */
static void downscale2x2_rgb565(const u16* src, u16* dst, u16 sw, u16 sh)
{
    u16 dw = sw / 2, dh = sh / 2;
    for (u16 y = 0; y < dh; y++) {
        const u16* r0 = src + (u32)(y * 2) * sw;
        const u16* r1 = r0 + sw;
        for (u16 x = 0; x < dw; x++) {
            u16 a = r0[x * 2], b = r0[x * 2 + 1];
            u16 c = r1[x * 2], d = r1[x * 2 + 1];
            u16 r = (u16)((((a >> 11) & 0x1F) + ((b >> 11) & 0x1F) +
                           ((c >> 11) & 0x1F) + ((d >> 11) & 0x1F)) >> 2);
            u16 g = (u16)((((a >> 5) & 0x3F) + ((b >> 5) & 0x3F) +
                           ((c >> 5) & 0x3F) + ((d >> 5) & 0x3F)) >> 2);
            u16 bl = (u16)(((a & 0x1F) + (b & 0x1F) +
                            (c & 0x1F) + (d & 0x1F)) >> 2);
            dst[y * dw + x] = (u16)((r << 11) | (g << 5) | bl);
        }
    }
}

static void network_thread(void* arg)
{
    NetworkThreadArg* a = (NetworkThreadArg*)arg;
    NetworkContext* net = a->net;
    AppState* state = a->state;
    CameraContext* cam = a->cam;
    EncoderContext* enc = a->enc;

    u16* frame_copy = (u16*)malloc(cam->size_bytes);
    u16* small_copy = (u16*)malloc(cam->size_bytes / 4);
    if (!frame_copy || !small_copy) {
        free(frame_copy);
        free(small_copy);
        free(a);
        return;
    }

    u64 last_handshake = 0;
    u32 frame_ts = 0;
    u32 fps_counter = 0;
    u64 fps_last = 0;
    u32 last_frame_id = 0;
    bool running = true;

    while (running) {
        if (svcWaitSynchronization(net->stop_event, 0) == 0) {
            running = false;
            break;
        }

        if (!state->streaming) {
            net->connected = false;
            state->pc_connected = false;
            state->stream_fps = 0;
            svcSleepThread(50 * 1000 * 1000);
            continue;
        }

        u64 now = osGetTime();
        if (!net->connected || (now - last_handshake) > 3000) {
            network_send_handshake(net, STREAM_WIDTH, STREAM_HEIGHT);
            network_poll_handshake_ack(net);
            last_handshake = now;
        }

        /* N'encode que les NOUVELLES frames : sans ce test, la meme frame
         * etait re-envoyee en boucle et saturait le WiFi de doublons. */
        bool have_frame = false;
        LightLock_Lock(&cam->lock);
        if (cam->ready && cam->frame_id != last_frame_id) {
            memcpy(frame_copy, cam->buffer, cam->size_bytes);
            last_frame_id = cam->frame_id;
            have_frame = true;
        }
        LightLock_Unlock(&cam->lock);

        if (!have_frame) {
            svcSleepThread(2 * 1000 * 1000);
            continue;
        }

        int qoi_len;
        if (state->low_res) {
            downscale2x2_rgb565(frame_copy, small_copy, STREAM_WIDTH, STREAM_HEIGHT);
            qoi_len = encoder_rgb565_to_qoi(enc, small_copy,
                                            STREAM_WIDTH / 2, STREAM_HEIGHT / 2);
        } else {
            qoi_len = encoder_rgb565_to_qoi(enc, frame_copy, STREAM_WIDTH, STREAM_HEIGHT);
        }
        if (qoi_len > 0) {
            if (network_send_frame(net, encoder_data(enc), (u32)qoi_len, frame_ts++)) {
                fps_counter++;
            }
        }

        {
            u64 fps_now = osGetTime();
            if (fps_now - fps_last >= 1000) {
                state->stream_fps = fps_counter;
                fps_counter = 0;
                fps_last = fps_now;
            }
            state->pc_connected = net->connected;
        }
    }

    free(frame_copy);
    free(small_copy);
    free(a);
}

Thread network_start_thread(NetworkContext* net, AppState* state,
                            CameraContext* cam, EncoderContext* enc)
{
    NetworkThreadArg* arg = (NetworkThreadArg*)malloc(sizeof(NetworkThreadArg));
    arg->net = net;
    arg->state = state;
    arg->cam = cam;
    arg->enc = enc;
    return threadCreate(network_thread, arg, 96 * 1024, 0x30, 0, false);
}

void network_stop_thread(Thread thread, NetworkContext* net)
{
    svcSignalEvent(net->stop_event);
    threadJoin(thread, U64_MAX);
    threadFree(thread);
}
