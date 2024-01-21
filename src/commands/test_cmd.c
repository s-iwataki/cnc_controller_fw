#include "test_cmd.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "command_list.h"
#include "commands/test_encoder_commands.h"
#include "commands/test_switch_commands.h"
#include "common_data.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "r_ioport.h"
#include "task.h"
#include "triaxis_table.h"
#include "utils.h"
#include "table_test_commands.h"
#include "subcmd_helper.h"



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

static command_entry_t subcmd[] = {
    {"led", led_cmd, "led function test: test led <led_number> [on|off]"},
    {"table", table_test_cmd, "table function test."},
    {"enc", test_encoder_cmd, "encoder function test."},
    {"switch", test_switch_cmd, "switch function test."},
    {NULL, NULL, NULL}};



int test_cmd(int argc, char** argv) {
    return apply_sub_cmd(argc, argv, subcmd);
}