#include "test_cmd.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "command_list.h"
#include "common_data.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "r_ioport.h"
#include "task.h"
#include "triaxis_table.h"

static void print_available_subcmd(const command_entry_t* cmds);
static int apply_sub_cmd(int argc, char** argv, const command_entry_t* cmds);

static int led_cmd(int argc, char** argv) {
    if (argc == 3) {
        bsp_io_level_t level = BSP_IO_LEVEL_LOW;
        if (strcmp(argv[2], "on") == 0) {
            level = BSP_IO_LEVEL_HIGH;
        } else if (strcmp(argv[2], "off") == 0) {
            level = BSP_IO_LEVEL_LOW;
        } else {
            printf("invalid config\r\n");
            return -1;
        }
        if (strcmp(argv[1], "0") == 0) {
            R_IOPORT_PinWrite(&g_ioport_ctrl, LED_GREEN, level);
        } else if (strcmp(argv[1], "1") == 0) {
            R_IOPORT_PinWrite(&g_ioport_ctrl, LED_RED, level);
        } else {
            printf("invalid led number\r\n");
            return -1;
        }
    }
    return 0;
}
static int test_table_enable_cmd(int argc, char** argv) {
    table_3d_driver_t* tbl_drv = table_get_driver();
    table_enable(tbl_drv, 1);
    printf("table motor enabled\r\n");
    return 0;
}
static int test_table_disable_cmd(int argc, char** argv) {
    table_3d_driver_t* tbl_drv = table_get_driver();
    table_enable(tbl_drv, 0);
    printf("table motor disabled\r\n");
    return 0;
}
/**
 * @brief 文字列を浮動小数点にする
 *
 * @param s 文字列
 * @param result 変換された浮動少数点数
 * @return int 変換成功:0,変換失敗:-1
 */
static int my_atof(const char* s, float* result) {
    *result = 0;
    int len = strlen(s);
    int sign = 1;
    int i = 0;
    int int_part = 0;
    for (i = 0; i < len; i++) {
        int_part = int_part * 10;
        if (s[i] == '-' && i == 0) {
            sign = -1;
        } else if (s[i] == '+' && i == 0) {
            sign = 1;
        } else if ((s[i] >= '0') && (s[i] <= '9')) {
            int_part += (s[i] - '0');
        } else if (s[i] == '.') {
            break;
        } else {
            return -1;
        }
    }
    float float_part = 0;
    float digit = 1.0f;
    for (i++; i < len; i++) {
        digit = digit * 0.1f;
        if ((s[i] >= '0') && (s[i] <= '9')) {
            float_part += digit * (s[i] - '0');
        } else {
            return -1;
        }
    }
    *result = (float)sign * ((float)int_part + float_part);
    return 0;
}
typedef struct {
    TaskHandle_t handle;
} table_test_cmd_status_t;
static void table_callback(void* ctx, const table_3d_event_t* evt) {
    table_test_cmd_status_t* s = ctx;
    uint32_t x_complted = ((evt->id == X_AXIS_MOTION_COMPLETE) || (evt->inmotion.x == 0)) ? X_AXIS_MOTION_COMPLETE : 0;
    uint32_t y_complted = ((evt->id == Y_AXIS_MOTION_COMPLETE) || (evt->inmotion.y == 0)) ? Y_AXIS_MOTION_COMPLETE : 0;
    uint32_t z_complted = ((evt->id == Z_AXIS_MOTION_COMPLETE) || (evt->inmotion.z == 0)) ? Z_AXIS_MOTION_COMPLETE : 0;
    BaseType_t other_task_woken;
    xTaskNotifyFromISR(s->handle, x_complted | y_complted | z_complted, eSetBits, &other_task_woken);
    portYIELD_FROM_ISR(other_task_woken);
}

static int test_table_move_cmd(int argc, char** argv) {
    table_3d_driver_t* tbl_drv = table_get_driver();
    if (argc != 5) {
        printf("usage: move dx dy dz sec\r\n");
        return -1;
    }
    float dx, dy, dz, t;
    if (my_atof(argv[1], &dx) < 0) {
        printf("parse error\r\n");
        return -1;
    }
    if (my_atof(argv[2], &dy) < 0) {
        printf("parse error\r\n");
        return -1;
    }

    if (my_atof(argv[3], &dz) < 0) {
        printf("parse error\r\n");
        return -1;
    }
    if (my_atof(argv[4], &t) < 0) {
        printf("parse error\r\n");
        return -1;
    }
    if (t < 0) {
        printf("move dulation must be positive value\r\n");
        return -1;
    }
    table_test_cmd_status_t status = {.handle = xTaskGetCurrentTaskHandle()};
    table_movedelta(tbl_drv, dx, dy, dz, dx / t, dy / t, dz / t, table_callback, &status);
    int wait_ms = 100;
    int nwait = (int)(t * 1000 / wait_ms) + 1;
    uint32_t notification;
    uint32_t evt_mask = X_AXIS_MOTION_COMPLETE | Y_AXIS_MOTION_COMPLETE | Z_AXIS_MOTION_COMPLETE;
    int retval = xTaskNotifyWait(evt_mask, 0, &notification, pdMS_TO_TICKS(wait_ms));
    if ((retval == pdTRUE) &&
        ((notification & evt_mask) == evt_mask)) {
        printf("motion complted\r\n");
        return 0;
    }
    for (int i = 0; i < nwait; i++) {
        retval = xTaskNotifyWait(0, 0, &notification, pdMS_TO_TICKS(wait_ms));
        if ((retval == pdTRUE) &&
            ((notification & evt_mask) == evt_mask)) {
            printf("motion complted\r\n");
            return 0;
        } else {
            float x, y, z;
            table_getpos(tbl_drv, &x, &y, &z);
            printf("x=%d,y=%d,z=%d\r\n", (int32_t)x, (int32_t)y, (int32_t)z);
        }
    }
    if ((notification & evt_mask) != evt_mask) {
        printf("motion is not completed within %d second.\r\n bugs?\r\n", (int32_t)t);
        for (;;) {
            retval = xTaskNotifyWait(0, 0, &notification, pdMS_TO_TICKS(wait_ms));
            if ((retval == pdTRUE) &&
                ((notification & evt_mask) == evt_mask)) {
                printf("motion complted\r\n");
                return 0;
            } else {
                float x, y, z;
                table_getpos(tbl_drv, &x, &y, &z);
                printf("x=%d,y=%d,z=%d\r\n", (int32_t)x, (int32_t)y, (int32_t)z);
            }
        }
    }
    return 0;
}
static int test_table_getpos_cmd(int argc, char** argv) {
    table_3d_driver_t* tbl_drv = table_get_driver();
    float x, y, z;
    table_getpos(tbl_drv, &x, &y, &z);
    printf("x=%d,y=%d,z=%d\r\n", (int32_t)x, (int32_t)y, (int32_t)z);
    return 0;
}
static int test_table_setzero_cmd(int argc, char** argv) {
    table_3d_driver_t* tbl_drv = table_get_driver();
    table_setzero(tbl_drv);
    return 0;
}
static command_entry_t table_subcmd[] = {
    {"enable", test_table_enable_cmd, "table motor drive enable"},
    {"disable", test_table_disable_cmd, "table motor drive disable"},
    {"move", test_table_move_cmd, "tabel move table move dx dy dz second"},
    {"getpos", test_table_getpos_cmd, "get table position"},
    {"setzero", test_table_setzero_cmd, "set current position as origin"},
    {NULL, NULL, NULL}};
static int table_cmd(int argc, char** argv) {
    return apply_sub_cmd(argc, argv, table_subcmd);
}

static command_entry_t subcmd[] = {
    {"led", led_cmd, "led function test: test led <led_number> [on|off]"},
    {"table", table_cmd, "table function test."},
    {NULL, NULL, NULL}};

static void print_available_subcmd(const command_entry_t* cmds) {
    printf("available sub commands:\r\n");
    for (int i = 0; cmds[i].cmd_name != NULL; i++) {
        printf("%s: %s\r\n", cmds[i].cmd_name, cmds[i].help_text);
    }
}

static int apply_sub_cmd(int argc, char** argv, const command_entry_t* cmds) {
    if (argc < 2) {
        print_available_subcmd(cmds);
        return 0;
    }
    for (int i = 0; cmds[i].cmd_name != NULL; i++) {
        if (strcmp(argv[1], cmds[i].cmd_name) == 0) {
            return cmds[i].cmd_func(argc - 1, &argv[1]);
        }
    }
    printf("unknown subcommand:%s\r\n", argv[1]);
    print_available_subcmd(cmds);
    return 0;
}

int test_cmd(int argc, char** argv) {
    return apply_sub_cmd(argc, argv, subcmd);
}