#include <stdio.h>
#include <string.h>

#include "cnc_systemstate.h"
#include "command_list.h"
#include "resetcause.h"
#include "shell.h"
#include "spindle.h"
#include "syscalls.h"
#include "triaxis_table.h"

/* shell task entry function */
/* pvParameters contains TaskHandle_t */
char cmd_buff[256];
char* argv[16];

static void print_available_cmd(void) {
    printf("available commands:\r\n");
    for (int i = 0; command_list[i].cmd_name != NULL; i++) {
        printf("%s: %s\r\n", command_list[i].cmd_name, command_list[i].help_text);
    }
}
void shell_entry(void* pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);
    printf("cnc controller\r\n");
    printf("reset: %s\r\n", reset_cause_get_str());
    reset_cause_t reset_cause = reset_cause_get();
    if (!(reset_cause == RESET_EXTERNAL) || (reset_cause == RESET_POWERON)) {
        syscall_print_debug_dump();
    }
    reset_cause_checked();

    cnc_nonvolatile_config_t* cfg = 0;
    if (cnc_system_state_config_load(&cfg) != FSP_SUCCESS) {
        printf("[WARN]:Config is not set.\r\n");
    }
    if (cfg) {
        table_mm_per_count_t mm_per_count = cfg->table_cfg.table_mm_per_step;
        table_axis_sign_t sign = cfg->table_cfg.table_axis_direction;
        // table parameter set
        spindle_set_control_param(&g_spindle_motor, cfg->spindle_cfg.kp, cfg->spindle_cfg.ki, cfg->spindle_cfg.kd);
        table_3d_driver_t* table = table_get_driver();
        table_set_parameter(table, &mm_per_count, &sign);
    }
    cnc_system_state_init(cfg);

    /* TODO: add your own code here */
    while (1) {
        printf(">");
        fgets(cmd_buff, sizeof(cmd_buff), stdin);
        char* token = strtok(cmd_buff, " \n\t");
        if (token == NULL) {
            continue;
        }
        int argc = 0;
        for (; (argc < sizeof(argv)) && (token != NULL); token = strtok(NULL, " \n\t"), argc++) {
            argv[argc] = token;
        }
        int cmd_found = 0;
        for (int i = 0; command_list[i].cmd_name != NULL; i++) {
            if (strcmp(command_list[i].cmd_name, argv[0]) == 0) {
                int retval = command_list[i].cmd_func(argc, argv);
                if (retval != 0) {
                    printf("command_fail:%d\r\n", retval);
                }
                cmd_found = 1;
                break;
            }
        }
        if (cmd_found) {
            continue;
        }
        print_available_cmd();
    }
}
