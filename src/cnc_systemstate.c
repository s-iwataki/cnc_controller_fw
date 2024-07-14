#include "cnc_systemstate.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "fsp_common_api.h"
#include "hal_data.h"
#include "portmacro.h"
#include "rm_vee_api.h"
#include "task.h"

static cnc_system_state_t state;

void cnc_system_state_init(cnc_nonvolatile_config_t* cfg) {
    state.is_g_code_exec = 0;
    if (cfg == NULL) {
        state.g_code_state.abs_incr = G91_INCR;
        state.g_code_state.feed = 60.0f;  // 60mm/min=1mm/sec
        state.g_code_state.interpolation_mode = G00_FAST_FEED;
        state.g_code_state.spindle_rpm_ref = 2000.0f;
        state.g_code_state.working_plane = G17_XY_PLANE;
    }else{
        state.g_code_state=cfg->gcode_cfg;
    }
}
int cnc_system_state_is_gcode_exec() {
    return state.is_g_code_exec;
}

g_code_state_t* cnc_system_state_set_gcode_exec() {
    state.is_g_code_exec = 1;
    return &state.g_code_state;
}
void cnc_system_state_unset_gcode_exec() {
    state.is_g_code_exec = 0;
}

typedef struct {
    TaskHandle_t h;
} vee_command_ctx_t;

void vee_callback(rm_vee_callback_args_t* p_args) {
    vee_command_ctx_t* ctx = p_args->p_context;
    TaskHandle_t h = ctx->h;
    uint32_t w = 0;
    xTaskNotifyFromISR(h, 1, eSetBits, &w);
    if (w) {
        portYIELD_FROM_ISR(w);
    }
}

static void vee_command_wait(void) {
    uint32_t v = 0;
    xTaskNotifyWait(1, 1, &v, portMAX_DELAY);
}
int cnc_system_state_config_load(cnc_nonvolatile_config_t** cfg) {
    if (cfg == NULL) {
        return FSP_ERR_INVALID_POINTER;
    }
    fsp_err_t r = g_vee0.p_api->open(g_vee0.p_ctrl, g_vee0.p_cfg);
    vee_command_ctx_t ctx;
    ctx.h = xTaskGetCurrentTaskHandle();
    g_vee0.p_api->callbackSet(g_vee0.p_ctrl, vee_callback, &ctx, NULL);
    switch (r) {
        case FSP_ERR_NOT_INITIALIZED:
            r = g_vee0.p_api->refresh(g_vee0.p_ctrl);
            vee_command_wait();
            break;
        case FSP_SUCCESS:
        case FSP_ERR_ALREADY_OPEN:
            r = FSP_SUCCESS;
        default:
            break;
    }
    if (r != FSP_SUCCESS) {
        *cfg = NULL;
        return r;
    }
    uint32_t len = 0;
    r = g_vee0.p_api->recordPtrGet(g_vee0.p_ctrl, 0, (uint8_t**)cfg, &len);
    if (r != FSP_SUCCESS) {
        *cfg = 0;
        return r;
    }
    g_vee0.p_api->close(g_vee0.p_ctrl);
    return r;
}
int cnc_system_state_config_save(cnc_nonvolatile_config_t* cfg) {
    if (cfg == NULL) {
        return FSP_ERR_INVALID_POINTER;
    }
    fsp_err_t r = g_vee0.p_api->open(g_vee0.p_ctrl, g_vee0.p_cfg);
    vee_command_ctx_t ctx;
    ctx.h = xTaskGetCurrentTaskHandle();
    g_vee0.p_api->callbackSet(g_vee0.p_ctrl, vee_callback, &ctx, NULL);
    if (r != FSP_SUCCESS) {
        return r;
    }
    uint32_t size = sizeof(cnc_nonvolatile_config_t);
    r = g_vee0.p_api->recordWrite(g_vee0.p_ctrl, 0, (uint8_t*)cfg, size);
    if (r != FSP_SUCCESS) {
        return r;
    }
    vee_command_wait();
    return r;
}