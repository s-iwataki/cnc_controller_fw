#include "ui_view.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "cnc_systemstate.h"
#include "common_data.h"
#include "encoder.h"
#include "graphic.h"
#include "gui.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "r_adc_api.h"
#include "r_ioport.h"
#include "spindle.h"
#include "task.h"
#include "textbox.h"
#include "triaxis_table.h"

typedef struct {
    TEXTBOX_t x_pos;
    TEXTBOX_t y_pos;
    TEXTBOX_t z_pos;
    TEXTBOX_t spindle_rpm;
    TEXTBOX_t autofeed_speed;
} ui_view_items_t;

typedef enum {
    UI_AXIS_SEL_NONE,
    UI_X_AXIS_SEL,
    UI_Y_AXIS_SEL,
    UI_Z_AXIS_SEL
} ui_xyz_axis_selection_t;
typedef enum {
    UI_AUTOFEED_PLUS,
    UI_AUTOFEED_MINUS
} ui_autofeed_dir_selection_t;

typedef struct {
    int autofeed_enabled;
    int spindle_enabled;
    int spindle_speed;
    float autofeed_speed;
    int uienc_diff;
    int setzero_button_pressed;
    ui_xyz_axis_selection_t xyz_sel;
    ui_autofeed_dir_selection_t autofeed_dir;

} userinput_state_t;

typedef struct {
    ui_xyz_axis_selection_t xyz_sel;
    float xpos;
    float ypos;
    float zpos;
    int spindle_rpm;
    float autofeed_speed;
} ui_view_state_t;

static bsp_io_level_t get_level(bsp_io_port_pin_t pin) {
    bsp_io_level_t level;
    g_ioport.p_api->pinRead(g_ioport.p_ctrl, pin, &level);
    return level;
}

typedef struct {
    TaskHandle_t h;
    uint32_t evt_code;
} adc_cb_ctx_t;

static void adc_cb(adc_callback_args_t* arg) {
    adc_cb_ctx_t* ctx = (adc_cb_ctx_t*)arg->p_context;
    BaseType_t wakeup = 0;
    xTaskNotifyFromISR(ctx->h, ctx->evt_code, eSetBits, &wakeup);
    portYIELD_FROM_ISR(wakeup);
}

static uint16_t adc_get(const adc_instance_t* adc) {
    uint16_t val;
    TaskHandle_t h = xTaskGetCurrentTaskHandle();
    adc_cb_ctx_t ctx = {.h = h, .evt_code = 1};
    adc->p_api->callbackSet(adc->p_ctrl, adc_cb, &ctx, NULL);
    adc->p_api->scanCfg(adc->p_ctrl, adc->p_channel_cfg);
    adc->p_api->scanStart(adc->p_ctrl);
    uint32_t notification_val = 0;
    while (notification_val != ctx.evt_code) {
        xTaskNotifyWait(ctx.evt_code, ctx.evt_code, &notification_val, portMAX_DELAY);
    }
    adc->p_api->read(adc->p_ctrl, ADC_CHANNEL_17, &val);
    return val;
}

static void update_ui_input(userinput_state_t* s) {
    s->autofeed_dir = (get_level(UI_AUTOFEED_DIR) == BSP_IO_LEVEL_LOW) ? UI_AUTOFEED_MINUS : UI_AUTOFEED_PLUS;
    s->autofeed_enabled = (get_level(UI_AUTOFEED_ON) == BSP_IO_LEVEL_LOW);
    s->setzero_button_pressed = (get_level(UI_SET_ORIG) == BSP_IO_LEVEL_LOW);
    s->spindle_enabled = (get_level(UI_SPINDLE_ON) == BSP_IO_LEVEL_LOW);
    if (get_level(UI_XSEL) == BSP_IO_LEVEL_LOW) {
        s->xyz_sel = UI_X_AXIS_SEL;
    } else if (get_level(UI_YSEL) == BSP_IO_LEVEL_LOW) {
        s->xyz_sel = UI_Y_AXIS_SEL;
    } else {
        s->xyz_sel = UI_Z_AXIS_SEL;
    }
    s->spindle_speed = adc_get(&g_adc0) * MAX_SPINDLE_RPM / 4096;
    s->autofeed_speed = (float)adc_get(&g_adc1) * MAX_FEEDSPEED_MM_PER_SEC / 4096;
    update_encoder(&g_ui_encoder);
    s->uienc_diff = get_encoder_diff(&g_ui_encoder);
}

static int axis_sel_feed_changed(userinput_state_t* prev, userinput_state_t* current) {
    if (prev->autofeed_dir != current->autofeed_dir) {
        return pdTRUE;
    }
    if (prev->autofeed_enabled != current->autofeed_enabled) {
        return pdTRUE;
    }
    if (prev->xyz_sel != current->xyz_sel) {
        return pdTRUE;
    }
    if (fabsf(prev->autofeed_speed - current->autofeed_speed) > 1.0f) {
        return pdTRUE;
    }
    return pdFALSE;
}

static float calc_autofeed_pos(float v, float x) {
    if (v > 0) {
        return 1000.0f;
    }
    if (v < 0) {
        return -1000.0f;
    }
    return x;
}

static void process_table_motion(userinput_state_t* s, userinput_state_t* s_prev) {
    table_3d_driver_t* table = table_get_driver();
    if (axis_sel_feed_changed(s_prev, s)) {
        table_move_cancel(table);
        if (s->autofeed_enabled) {
            float xpos, ypos, zpos;
            float sign = (s->autofeed_dir == UI_AUTOFEED_PLUS) ? 1.0f : -1.0f;
            float v = s->autofeed_speed;
            float vx = (s->xyz_sel == UI_X_AXIS_SEL) ? (sign * v) : 0;
            float vy = (s->xyz_sel == UI_Y_AXIS_SEL) ? (sign * v) : 0;
            float vz = (s->xyz_sel == UI_Z_AXIS_SEL) ? (sign * v) : 0;
            table_getpos(table, &xpos, &ypos, &zpos);
            xpos = calc_autofeed_pos(vx, xpos);
            ypos = calc_autofeed_pos(vy, ypos);
            zpos = calc_autofeed_pos(vz, zpos);
            table_moveto(table, xpos, ypos, zpos, vx, vy, vz, NULL, NULL);
        }
    }
    if (s->autofeed_enabled == pdFALSE) {
        if (s->setzero_button_pressed == pdTRUE) {
            table_setzero(table);
        }
        if (s->uienc_diff != 0) {
            float dpos = (float)s->uienc_diff * UI_POS_INCR_PER_ENC;
            float speed = dpos * 1000.0f / UI_VIEW_UPDATE_PERIOD_MS;
            float dx = (s->xyz_sel == UI_X_AXIS_SEL) ? dpos : 0;
            float dy = (s->xyz_sel == UI_Y_AXIS_SEL) ? dpos : 0;
            float dz = (s->xyz_sel == UI_Z_AXIS_SEL) ? dpos : 0;
            float vx = (s->xyz_sel == UI_X_AXIS_SEL) ? speed : 0;
            float vy = (s->xyz_sel == UI_Y_AXIS_SEL) ? speed : 0;
            float vz = (s->xyz_sel == UI_Z_AXIS_SEL) ? speed : 0;
            table_movedelta(table, dx, dy, dz, vx, vy, vz, NULL, NULL);
        }
    }
}

static void process_spindle(userinput_state_t* s, userinput_state_t* s_prev) {
    if (s->spindle_enabled != s_prev->spindle_enabled) {
        if (s->spindle_enabled) {
            spindle_control_mode_set(&g_spindle_motor, SPINDLE_DIRECT_DUTY_CONTROL);
            spindle_enable(&g_spindle_motor, pdTRUE);
        } else {
            spindle_enable(&g_spindle_motor, pdFALSE);
        }
    }
    uint32_t duty = s->spindle_speed * 100 / MAX_SPINDLE_RPM;
    spindle_set_duty(&g_spindle_motor, duty);
}

static void process_user_input(userinput_state_t* s) {
    static userinput_state_t s_prev;
    process_table_motion(s, &s_prev);
    process_spindle(s, &s_prev);
    s_prev = *s;
}

static void print_mm_disp(TEXTBOX_t* t, float val) {
    int sign = (val >= 0.0f) ? 1 : -1;
    int mm = (int)((float)sign * val * 1000.0f);
    int mm_part = mm / 1000;
    int micron_part = mm % 1000;
    textbox_printf(t, "%c% 3d.%03d", (sign > 0) ? ' ' : '-', mm_part, micron_part);
}

static void update_position_disp(TEXTBOX_t* t, float prev, float current) {
    int micron_prev = (int)(prev * 1000.0f);
    int micron_current = (int)(current * 1000.0f);
    if (micron_current != micron_prev) {
        print_mm_disp(t, current);
    }
}

static void update_display(userinput_state_t* s, ui_view_items_t* items) {
    static ui_view_state_t prev;
    static int count = 0;
    ui_view_state_t current;
    table_3d_driver_t* table = table_get_driver();
    table_getpos(table, &current.xpos, &current.ypos, &current.zpos);
    current.autofeed_speed = s->autofeed_speed;
    current.xyz_sel = s->xyz_sel;
    current.spindle_rpm = spindle_get_speed(&g_spindle_motor);
    if (current.xyz_sel != prev.xyz_sel) {
        uint16_t xpos_color, ypos_color, zpos_color;
        switch (current.xyz_sel) {
            case UI_X_AXIS_SEL:
                xpos_color = GUI_DEFAULT_FOCUSED_BG_COLOR;
                ypos_color = GUI_DEFAULT_BACKGROUND_COLOR;
                zpos_color = GUI_DEFAULT_BACKGROUND_COLOR;
                break;
            case UI_Y_AXIS_SEL:
                xpos_color = GUI_DEFAULT_BACKGROUND_COLOR;
                ypos_color = GUI_DEFAULT_FOCUSED_BG_COLOR;
                zpos_color = GUI_DEFAULT_BACKGROUND_COLOR;
                break;
            case UI_Z_AXIS_SEL:
                xpos_color = GUI_DEFAULT_BACKGROUND_COLOR;
                ypos_color = GUI_DEFAULT_BACKGROUND_COLOR;
                zpos_color = GUI_DEFAULT_FOCUSED_BG_COLOR;
                break;
        }
        textbox_setcolor(&items->x_pos, GUI_DEFAULT_TEXT_COLOR, xpos_color, 1);
        textbox_setcolor(&items->y_pos, GUI_DEFAULT_TEXT_COLOR, ypos_color, 1);
        textbox_setcolor(&items->z_pos, GUI_DEFAULT_TEXT_COLOR, zpos_color, 1);
    }
    update_position_disp(&items->x_pos, prev.xpos, current.xpos);
    update_position_disp(&items->y_pos, prev.ypos, current.ypos);
    update_position_disp(&items->z_pos, prev.zpos, current.zpos);

    count++;
    if (count > UI_SPINDLE_SPEED_DISP_UPDATE_PERIOD_MS / UI_VIEW_UPDATE_PERIOD_MS) {
        count = 0;
        textbox_printf(&items->autofeed_speed, "% 4d mm/s", (int)current.autofeed_speed);
        textbox_printf(&items->spindle_rpm, "% 4d rpm", current.spindle_rpm);
    }
    prev = current;
}
static ui_view_items_t g_view_items;

void ui_view_init(void) {
    const graphic_apis_t* api = gui_get_graphic_driver();
    int w = api->get_width(api);
    textbox_create(&g_view_items.x_pos, 0, 5, w, 3);
    textbox_create(&g_view_items.y_pos, 0, 30, w, 3);
    textbox_create(&g_view_items.z_pos, 0, 55, w, 3);
    textbox_create(&g_view_items.autofeed_speed, 10, 80, w, 2);
    textbox_create(&g_view_items.spindle_rpm, 10, 100, w, 2);
    print_mm_disp(&g_view_items.x_pos, 0);
    print_mm_disp(&g_view_items.y_pos, 0);
    print_mm_disp(&g_view_items.z_pos, 0);
    textbox_printf(&g_view_items.autofeed_speed, "% 4d mm/s", 0);
    textbox_printf(&g_view_items.spindle_rpm, "% 4d rpm", 0);
    table_3d_driver_t* tbl = table_get_driver();
    table_enable(tbl, pdTRUE);
}
void ui_view_process(void) {
    userinput_state_t s;
    update_ui_input(&s);
    if (cnc_system_state_is_gcode_exec() == pdFALSE) {
        process_user_input(&s);
    }
    update_display(&s, &g_view_items);
}