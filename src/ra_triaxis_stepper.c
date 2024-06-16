#include "ra_triaxis_stepper.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "common_data.h"
#include "fsp_common_api.h"
#include "hal_data.h"
#include "queue.h"
#include "r_elc.h"
#include "r_elc_api.h"
#include "r_gpt.h"
#include "r_ioport.h"
#include "r_ioport_api.h"
#include "r_timer_api.h"
#include "task.h"
#include "triaxis_table.h"

typedef struct {
    void (*callback)(void*, const table_3d_event_t*);
    void* cb_ctx;
    table_mm_per_count_t mm_per_count;
    struct axis_sign axis_dirction;
    int x_in_motion;
    int y_in_motion;
    int z_in_motion;
} ra_triaxis_table_driver_t;

ra_triaxis_table_driver_t g_ra_triaxis_driver;

static void getpos(void* instance, float* x, float* y, float* z) {
    ra_triaxis_table_driver_t* d = (ra_triaxis_table_driver_t*)instance;
    timer_status_t s;
    R_GPT_StatusGet(&g_timer_xpos_ctrl, &s);
    *x = (float)((int32_t)s.counter) * d->mm_per_count.x;
    R_GPT_StatusGet(&g_timer_ypos_ctrl, &s);
    *y = (float)((int32_t)s.counter) * d->mm_per_count.y;
    R_GPT_StatusGet(&g_timer_zpos_ctrl, &s);
    *z = (float)((int32_t)s.counter) * d->mm_per_count.z;
}
static void start_motion() {
    R_ELC_SoftwareEventGenerate(&g_elc_ctrl, ELC_SOFTWARE_EVENT_0);
}
static void cancel(void* inctance) {
    R_GPT_Stop(&g_timer_xclock_ctrl);
    R_GPT_Stop(&g_timer_yclock_ctrl);
    R_GPT_Stop(&g_timer_zclock_ctrl);
}
static void setzero(void* instance) {
    R_GPT_Reset(&g_timer_xpos_ctrl);
    R_GPT_Reset(&g_timer_ypos_ctrl);
    R_GPT_Reset(&g_timer_xpos_ctrl);
}
uint32_t get_period_counts(float mm_ps, float mm_per_count, uint32_t counter_hz) {
    uint32_t pps_hz = (uint32_t)(mm_ps / mm_per_count);
    if (pps_hz == 0) {
        pps_hz = 1;
    }
    return counter_hz / pps_hz;
}
void timer_clock_set(gpt_instance_ctrl_t* t, float v, float mm_per_count) {
    timer_info_t s;
    R_GPT_InfoGet(t, &s);
    uint32_t period = get_period_counts(v, mm_per_count, s.clock_frequency);
    R_GPT_PeriodSet(t, get_period_counts(v, mm_per_count, s.clock_frequency));
}

void set_count_comparematch(gpt_instance_ctrl_t* t, float dist, float currnt, float mm_per_count) {
    const gpt_extended_cfg_t* cfg_ext = t->p_cfg->p_extend;
    int min = 0;
    if (dist > currnt) {
        // 軸を動かす方向によってカウント要因を入れ替え
        t->p_reg->GTUPSR = cfg_ext->count_up_source;
        t->p_reg->GTDNSR = GPT_SOURCE_NONE;
        min = 1;
    } else {
        t->p_reg->GTDNSR = cfg_ext->count_up_source;
        t->p_reg->GTUPSR = GPT_SOURCE_NONE;
        min = -1;
    }
    int sig = 1;
    if (dist < 0) {
        sig = -1;
        dist = -dist;
    }
    uint32_t gtccr = sig * (int32_t)(dist / mm_per_count);
    uint32_t current_gtccr = t->p_reg->GTCCR[0];
    if (current_gtccr == gtccr) {  // 指示された動作幅が最小分解能より小さい場合は最小分解能で動く
        if (min > 0) {
            gtccr++;
        }
        if (min < 0) {
            gtccr--;
        }
    }
    t->p_reg->GTCCR[0] = gtccr;  // set gtccra
    t->p_reg->GTCCR[2] = gtccr;  // バッファが有効になっているので，0をまたぐときにGTCCRCの値がGTCCRAに書き込まれる．
}
void set_direction_pin(ioport_ctrl_t* io, bsp_io_port_pin_t pin, float dist, float current, int invert) {
    int dir = ((dist > current) ? 1 : -1) * invert;
    if (dir > 0) {
        R_IOPORT_PinWrite(io, pin, BSP_IO_LEVEL_LOW);
    } else {
        R_IOPORT_PinWrite(io, pin, BSP_IO_LEVEL_HIGH);
    }
}

void moveto(void* instance, float x, float y, float z, float vx, float vy, float vz, void (*callback)(void*, const table_3d_event_t*), void* ctx) {
    ra_triaxis_table_driver_t* d = (ra_triaxis_table_driver_t*)instance;
    float current_x, current_y, current_z;
    getpos(instance, &current_x, &current_y, &current_z);
    vx = (vx < 0) ? -vx : vx;
    vy = (vy < 0) ? -vy : vy;
    vz = (vz < 0) ? -vz : vz;
    if (vx > 0) {
        timer_clock_set(&g_timer_xclock_ctrl, vx, d->mm_per_count.x);
        R_GPT_Enable(&g_timer_xclock_ctrl);
        R_IOPORT_PinWrite(&g_ioport_ctrl, X_MOTOR_TQ, BSP_IO_LEVEL_HIGH);
        d->x_in_motion = 1;
    } else {
        R_GPT_Disable(&g_timer_xclock_ctrl);
        R_IOPORT_PinWrite(&g_ioport_ctrl, X_MOTOR_TQ, BSP_IO_LEVEL_LOW);
        d->x_in_motion = 0;
    }
    if (vy > 0) {
        timer_clock_set(&g_timer_yclock_ctrl, vy, d->mm_per_count.y);
        R_GPT_Enable(&g_timer_yclock_ctrl);
        R_IOPORT_PinWrite(&g_ioport_ctrl, Y_MOTOR_TQ, BSP_IO_LEVEL_HIGH);
        d->y_in_motion = 1;
    } else {
        R_GPT_Disable(&g_timer_yclock_ctrl);
        R_IOPORT_PinWrite(&g_ioport_ctrl, Y_MOTOR_TQ, BSP_IO_LEVEL_LOW);
        d->y_in_motion = 0;
    }
    if (vz > 0) {
        timer_clock_set(&g_timer_zclock_ctrl, vz, d->mm_per_count.z);
        R_GPT_Enable(&g_timer_zclock_ctrl);
        R_IOPORT_PinWrite(&g_ioport_ctrl, Z_MOTOR_TQ, BSP_IO_LEVEL_HIGH);
        d->z_in_motion = 1;
    } else {
        R_GPT_Disable(&g_timer_zclock_ctrl);
        R_IOPORT_PinWrite(&g_ioport_ctrl, Z_MOTOR_TQ, BSP_IO_LEVEL_LOW);
        d->z_in_motion = 0;
    }
    if (d->x_in_motion) {
        set_count_comparematch(&g_timer_xpos_ctrl, x, current_x, d->mm_per_count.x);
    }
    if (d->y_in_motion) {
        set_count_comparematch(&g_timer_ypos_ctrl, y, current_y, d->mm_per_count.y);
    }
    if (d->z_in_motion) {
        set_count_comparematch(&g_timer_zpos_ctrl, z, current_z, d->mm_per_count.z);
    }
    set_direction_pin(&g_ioport_ctrl, X_MOTOR_DIR, x, current_x, d->axis_dirction.x);
    set_direction_pin(&g_ioport_ctrl, Y_MOTOR_DIR, y, current_y, d->axis_dirction.y);
    set_direction_pin(&g_ioport_ctrl, Z_MOTOR_DIR, z, current_z, d->axis_dirction.z);
    d->callback = callback;
    d->cb_ctx = ctx;
    start_motion();
}
static void movedelta(void* instance, float x, float y, float z, float vx, float vy, float vz, void (*callback)(void*, const table_3d_event_t*), void* ctx) {
    float current_x, current_y, current_z;
    getpos(instance, &current_x, &current_y, &current_z);
    moveto(instance, x + current_x, y + current_y, z + current_z, vx, vy, vz, callback, ctx);
}
static void motor_enable(void* instance, int enable) {
    if (enable == 1) {
        R_IOPORT_PinWrite(&g_ioport_ctrl, STEPPER_ENABLE, BSP_IO_LEVEL_HIGH);
        R_IOPORT_PinWrite(&g_ioport_ctrl, STEPPER_RESET, BSP_IO_LEVEL_HIGH);
    } else {
        R_IOPORT_PinWrite(&g_ioport_ctrl, STEPPER_ENABLE, BSP_IO_LEVEL_LOW);
        R_IOPORT_PinWrite(&g_ioport_ctrl, STEPPER_RESET, BSP_IO_LEVEL_LOW);
    }
}
void xpos_counter_isr(timer_callback_args_t* p_args) {
    if ((p_args->event == TIMER_EVENT_CAPTURE_A) || (p_args->event == TIMER_EVENT_CAPTURE_B)) {
        ra_triaxis_table_driver_t* d = (ra_triaxis_table_driver_t*)p_args->p_context;
        float x, y, z;
        getpos(d, &x, &y, &z);
        table_3d_event_t evt = {.id = X_AXIS_MOTION_COMPLETE, .pos = {.x = x, .y = y, .z = z}, .inmotion = {.x = d->x_in_motion, .y = d->y_in_motion, .z = d->z_in_motion}};
        if (d->callback) {
            d->callback(d->cb_ctx, &evt);
        }
    }
}

void ypos_counter_isr(timer_callback_args_t* p_args) {
    if ((p_args->event == TIMER_EVENT_CAPTURE_A) || (p_args->event == TIMER_EVENT_CAPTURE_B)) {
        ra_triaxis_table_driver_t* d = (ra_triaxis_table_driver_t*)p_args->p_context;
        float x, y, z;
        getpos(d, &x, &y, &z);
        table_3d_event_t evt = {.id = Y_AXIS_MOTION_COMPLETE, .pos = {.x = x, .y = y, .z = z}, .inmotion = {.x = d->x_in_motion, .y = d->y_in_motion, .z = d->z_in_motion}};
        if (d->callback) {
            d->callback(d->cb_ctx, &evt);
        }
    }
}

void zpos_counter_isr(timer_callback_args_t* p_args) {
    if ((p_args->event == TIMER_EVENT_CAPTURE_A) || (p_args->event == TIMER_EVENT_CAPTURE_B)) {
        ra_triaxis_table_driver_t* d = (ra_triaxis_table_driver_t*)p_args->p_context;
        float x, y, z;
        getpos(d, &x, &y, &z);
        table_3d_event_t evt = {.id = Z_AXIS_MOTION_COMPLETE, .pos = {.x = x, .y = y, .z = z}, .inmotion = {.x = d->x_in_motion, .y = d->y_in_motion, .z = d->z_in_motion}};
        if (d->callback) {
            d->callback(d->cb_ctx, &evt);
        }
    }
}

void ra_triaxis_stepper_init(table_3d_driver_t* d, const table_mm_per_count_t* mmpc, const table_axis_sign_t* sign) {
    R_ELC_Open(&g_elc_ctrl, &g_elc_cfg);
    R_GPT_Open(&g_timer_xclock_ctrl, &g_timer_xclock_cfg);
    R_GPT_Open(&g_timer_yclock_ctrl, &g_timer_yclock_cfg);
    R_GPT_Open(&g_timer_zclock_ctrl, &g_timer_zclock_cfg);
    R_GPT_Open(&g_timer_xpos_ctrl, &g_timer_xpos_cfg);
    R_GPT_Open(&g_timer_ypos_ctrl, &g_timer_ypos_cfg);
    R_GPT_Open(&g_timer_zpos_ctrl, &g_timer_zpos_cfg);
    R_ELC_Enable(&g_elc_ctrl);
    R_GPT_Enable(&g_timer_xclock_ctrl);
    R_GPT_Enable(&g_timer_yclock_ctrl);
    R_GPT_Enable(&g_timer_zclock_ctrl);
    R_GPT_Enable(&g_timer_xpos_ctrl);
    R_GPT_Enable(&g_timer_ypos_ctrl);
    R_GPT_Enable(&g_timer_zpos_ctrl);
    R_GPT_CallbackSet(&g_timer_xpos_ctrl, xpos_counter_isr, &g_ra_triaxis_driver, NULL);
    R_GPT_CallbackSet(&g_timer_ypos_ctrl, ypos_counter_isr, &g_ra_triaxis_driver, NULL);
    R_GPT_CallbackSet(&g_timer_zpos_ctrl, zpos_counter_isr, &g_ra_triaxis_driver, NULL);
    d->cancel = cancel;
    d->getpos = getpos;
    d->movedelta = movedelta;
    d->moveto = moveto;
    d->setzero = setzero;
    d->enable = motor_enable;
    d->hw_driver_instance = &g_ra_triaxis_driver;
    g_ra_triaxis_driver.axis_dirction = *sign;
    g_ra_triaxis_driver.mm_per_count = *mmpc;
    R_IOPORT_PinWrite(&g_ioport_ctrl, X_MOTOR_TQ, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, Y_MOTOR_TQ, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, Z_MOTOR_TQ, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, STEPPER_ENABLE, BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, STEPPER_RESET, BSP_IO_LEVEL_LOW);
}