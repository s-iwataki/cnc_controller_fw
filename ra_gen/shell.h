/* generated thread header file - do not edit */
#ifndef SHELL_H_
#define SHELL_H_
#include "bsp_api.h"
                #include "FreeRTOS.h"
                #include "task.h"
                #include "semphr.h"
                #include "hal_data.h"
                #ifdef __cplusplus
                extern "C" void shell_entry(void * pvParameters);
                #else
                extern void shell_entry(void * pvParameters);
                #endif
FSP_HEADER
FSP_FOOTER
#endif /* SHELL_H_ */
