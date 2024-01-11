#include "ra_gpt_encoder.h"

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

typedef struct {
    timer_ctrl_t* p_ctrl;
    uint32_t prev_count;
    uint32_t count;
} ra_gpt_encoder_t;

static void set_encoder_val_raw(void* device, int val) {
    ra_gpt_encoder_t* enc=(ra_gpt_encoder_t*)device;
    timer_ctrl_t* p_ctrl = enc->p_ctrl;
    R_GPT_Stop(p_ctrl);
    R_GPT_CounterSet(p_ctrl, val);
    R_GPT_Start(p_ctrl);
    enc->count=val;
    enc->prev_count=val;
}
static int get_encoder_val_raw(void* device) {
    ra_gpt_encoder_t* enc=(ra_gpt_encoder_t*)device;
    return (int)enc->count;
}
static int get_encoder_diff_raw(void* device) {
    ra_gpt_encoder_t* enc=(ra_gpt_encoder_t*)device;
    return (int)(enc->count-enc->prev_count);
}
static void update_encoder_raw(void* device) {
    ra_gpt_encoder_t* enc=(ra_gpt_encoder_t*)device;
    timer_ctrl_t* p_ctrl = enc->p_ctrl;
    timer_status_t stat;
    R_GPT_StatusGet(p_ctrl, &stat);
    enc->prev_count=enc->count;
    enc->count=stat.counter;    
}
static ra_gpt_encoder_t g_ra_gpt_encoder;

void ra_gpt_encoder_init(ENCODER_t* enc) {
    R_GPT_Open(&g_timer_uienc_ctrl, &g_timer_uienc_cfg);
    R_GPT_Enable(&g_timer_uienc_ctrl);
    R_GPT_Start(&g_timer_uienc_ctrl);
    g_ra_gpt_encoder.p_ctrl = &g_timer_uienc_ctrl;
    g_ra_gpt_encoder.prev_count = 0;
    enc->get_encoder_diff_raw = get_encoder_diff_raw;
    enc->get_encoder_val_raw = get_encoder_val_raw;
    enc->update_encoder_raw = update_encoder_raw;
    enc->set_encoder_val_raw = set_encoder_val_raw;
    enc->device = &g_ra_gpt_encoder;
}