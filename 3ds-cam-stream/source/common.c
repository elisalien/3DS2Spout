#include "common.h"
#include <string.h>
#include <stdio.h>

#ifndef DEFAULT_PC_IP
#define DEFAULT_PC_IP "192.168.0.1"
#endif

#ifndef DEFAULT_PC_PORT
#define DEFAULT_PC_PORT UDP_PORT_DEFAULT
#endif

void app_state_defaults(AppState* state)
{
    memset(state, 0, sizeof(*state));
    state->camera = CAM_REAR;
    state->streaming = false;
    state->low_res = true; /* mode fluide par defaut, toggle QUALITE dans l'UI */
    state->lang = LANG_FR;
    strncpy(state->pc_ip, DEFAULT_PC_IP, sizeof(state->pc_ip) - 1);
    state->pc_port = (u16)DEFAULT_PC_PORT;
}
