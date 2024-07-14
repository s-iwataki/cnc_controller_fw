#include "config_cmd.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "cnc_systemstate.h"
#include "command_list.h"
#include "common_data.h"
#include "fsp_common_api.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "r_ioport.h"
#include "spindle.h"
#include "subcmd_helper.h"
#include "table_test_commands.h"
#include "task.h"
#include "triaxis_table.h"
#include "utils.h"

static int table_cfg_cmd(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: [x|y|z] mm/step (if <0, invert axis direction.)\r\n");
        printf("example: config table x 0.0125 y 0.0125 z -0.0125\r\n");
        return -1;
    }
    cnc_nonvolatile_config_t* cfg;
    cnc_nonvolatile_config_t initial = {0};
    cnc_system_state_config_load(&cfg);
    if (cfg == NULL) {
        printf("[WARN]: initial config. spindle param is not valid.\r\n");
        cfg = &initial;
    }
    for (int i = 0; i < (argc - 1) / 2; i++) {
        char axis = argv[1 + 2 * i][0];
        float val = 0;
        if (my_atof(argv[1 + 2 * i + 1], &val) < 0) {
            printf("invalid paramter.\r\n");
            return -1;
        }
        int sign = 1;
        if (val < 0) {
            val = -val;
            sign = -1;
        }

        switch (axis) {
            case 'x':
                cfg->table_cfg.table_mm_per_step.x = val;
                cfg->table_cfg.table_axis_direction.x = sign;
                break;
            case 'y':
                cfg->table_cfg.table_mm_per_step.y = val;
                cfg->table_cfg.table_axis_direction.y = sign;
                break;
            case 'z':
                cfg->table_cfg.table_mm_per_step.z = val;
                cfg->table_cfg.table_axis_direction.z = sign;
                break;
            default:
                printf("invalid axis.\r\n");
                break;
        }
    }
    int r = cnc_system_state_config_save(cfg);
    if (r == FSP_SUCCESS) {
        printf("Table config was saved. To apply the config, please reboot.\r\n");
    }
    return r;
}

static int spindle_cfg_cmd(int argc, char** argv) {
    if (argc < 3) {
        printf("example: config spindle p 0.0125 i 0.0125 d -0.0125\r\n");
        return -1;
    }
    cnc_nonvolatile_config_t* cfg;
    cnc_nonvolatile_config_t initial = {0};
    cnc_system_state_config_load(&cfg);
    if (cfg == NULL) {
        printf("[WARN]: initial config. table param is not valid.\r\n");
        cfg = &initial;
    }
    for (int i = 0; i < (argc - 1) / 2; i++) {
        char param_name = argv[1 + 2 * i][0];
        float val = 0;
        if (my_atof(argv[1 + 2 * i + 1], &val) < 0) {
            printf("invalid paramter.\r\n");
            return -1;
        }
        switch (param_name) {
            case 'p':
                cfg->spindle_cfg.kp = val;
                break;
            case 'i':
                cfg->spindle_cfg.ki = val;
                break;
            case 'd':
                cfg->spindle_cfg.kd = val;
                break;
            default:
                printf("invalid param name.\r\n");
                break;
        }
    }
    int r = cnc_system_state_config_save(cfg);
    if (r == FSP_SUCCESS) {
        spindle_set_control_param(&g_spindle_motor, cfg->spindle_cfg.kp, cfg->spindle_cfg.ki, cfg->spindle_cfg.kd);
        printf("Spindle config was saved and applied.\r\n");
    }
    return r;
}

static command_entry_t subcmd[] = {
    {"table", table_cfg_cmd, "table axis conf."},
    {"spindle", spindle_cfg_cmd, "spindle conf."},
    {NULL, NULL, NULL}};

int config_cmd(int argc, char** argv) {
    return apply_sub_cmd(argc, argv, subcmd);
}