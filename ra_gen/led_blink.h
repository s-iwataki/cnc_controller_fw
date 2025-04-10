/* generated thread header file - do not edit */
#ifndef LED_BLINK_H_
#define LED_BLINK_H_
#include "FreeRTOS.h"
#include "bsp_api.h"
#include "hal_data.h"
#include "semphr.h"
#include "task.h"
#ifdef __cplusplus
extern "C" void led_blink_entry(void* pvParameters);
#else
extern void led_blink_entry(void* pvParameters);
#endif
FSP_HEADER
FSP_FOOTER
#endif /* LED_BLINK_H_ */
