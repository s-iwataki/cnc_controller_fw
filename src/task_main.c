#include "task_main.h"
#include "bsp_pin_cfg.h"
#include "common_data.h"
#include "hal_data.h"
#include "r_ioport.h"
StackType_t main_task_stack[MAIN_TASK_STACK_DEPTH];
StaticTask_t main_task_tcb;
void task_main(void*param){
    for(;;){
        R_IOPORT_PinWrite(&g_ioport_ctrl, LED_GREEN, BSP_IO_LEVEL_HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        R_IOPORT_PinWrite(&g_ioport_ctrl,LED_GREEN,BSP_IO_LEVEL_LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}