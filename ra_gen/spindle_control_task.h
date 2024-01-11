/* generated thread header file - do not edit */
#ifndef SPINDLE_CONTROL_TASK_H_
#define SPINDLE_CONTROL_TASK_H_
#include "bsp_api.h"
                #include "FreeRTOS.h"
                #include "task.h"
                #include "semphr.h"
                #include "hal_data.h"
                #ifdef __cplusplus
                extern "C" void spindle_control_task_entry(void * pvParameters);
                #else
                extern void spindle_control_task_entry(void * pvParameters);
                #endif
FSP_HEADER
FSP_FOOTER
#endif /* SPINDLE_CONTROL_TASK_H_ */
