#include "bsp_pin_cfg.h"
#include "common_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "r_ioport.h"
#include "safety_obsavation_task.h"
#include "spindle.h"
#include "triaxis_table.h"
/* safety_obsavation entry function */
/* pvParameters contains TaskHandle_t */
void safety_obsavation_task_entry(void* pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);

    /* TODO: add your own code here */
    TickType_t last_wake_time = xTaskGetTickCount();
    while (1) {
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));
        table_3d_driver_t* table = table_get_driver();
        if (spindle_get_status(&g_spindle_motor) == SPINDLE_ERROR) {
            spindle_enable(&g_spindle_motor, pdFALSE);

            table_move_cancel(table);
        }
        bsp_io_level_t stepper_alert_status;
        R_IOPORT_PinRead(&g_ioport_ctrl, STEPPER_ALERT, &stepper_alert_status);
        if (stepper_alert_status == pdFALSE) {  // alert状態
            table_move_cancel(table);//table停止
        }
        table_state_t table_state;
        table_get_status(table, &table_state);
        bsp_io_level_t xmax_hit, xmin_hit, ymax_hit, ymin_hit, zmax_hit, zmin_hit;
        R_IOPORT_PinRead(&g_ioport_ctrl, XP_LIM, &xmax_hit);
        R_IOPORT_PinRead(&g_ioport_ctrl, XM_LIM, &xmin_hit);
        R_IOPORT_PinRead(&g_ioport_ctrl, YP_LIM, &ymax_hit);
        R_IOPORT_PinRead(&g_ioport_ctrl, YM_LIM, &ymin_hit);
        R_IOPORT_PinRead(&g_ioport_ctrl, ZP_LIM, &zmax_hit);
        R_IOPORT_PinRead(&g_ioport_ctrl, ZM_LIM, &zmin_hit);
        if (((table_state.x_speed < 0) && (xmin_hit == pdTRUE)) ||
            ((table_state.x_speed > 0) && (xmax_hit == pdTRUE)) ||
            ((table_state.y_speed < 0) && (ymin_hit == pdTRUE)) ||
            ((table_state.y_speed > 0) && (ymax_hit == pdTRUE)) ||
            ((table_state.z_speed < 0) && (zmin_hit == pdTRUE)) ||
            ((table_state.z_speed > 0) && (zmax_hit == pdTRUE))) {
            table_move_cancel(table);
        }
    }
}
