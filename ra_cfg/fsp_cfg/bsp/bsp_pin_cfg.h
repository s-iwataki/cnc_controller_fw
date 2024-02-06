/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define UI_LCD_A0 (BSP_IO_PORT_01_PIN_00) /* LCD A0 pin */
#define UI_LCD_DATA (BSP_IO_PORT_01_PIN_01) /* lcd module data */
#define UI_LCD_CLK (BSP_IO_PORT_01_PIN_02) /* lcd module clock */
#define UI_LCD_CS (BSP_IO_PORT_01_PIN_03) /* lcd cs pin */
#define X_MOTOR_CLK (BSP_IO_PORT_01_PIN_04) /* X axis stepper clock */
#define X_MOTOR_DIR (BSP_IO_PORT_01_PIN_05) /* X axis stepper dir */
#define Y_MOTOR_CLK (BSP_IO_PORT_01_PIN_06) /* Y axis stepper clock */
#define Y_MOTOR_DIR (BSP_IO_PORT_01_PIN_07) /* y axis stepper direction */
#define USART_TXD (BSP_IO_PORT_01_PIN_09)
#define USART_RXD (BSP_IO_PORT_01_PIN_10)
#define Z_MOTOR_CLK (BSP_IO_PORT_01_PIN_11) /* z axis motor clock */
#define Z_MOTOR_DIR (BSP_IO_PORT_01_PIN_12) /* z axis direction */
#define Z_MOTOR_TQ (BSP_IO_PORT_01_PIN_13) /* z motor torque */
#define Y_MOTOR_TQ (BSP_IO_PORT_01_PIN_14) /* y motor torque set */
#define X_MOTOR_TQ (BSP_IO_PORT_01_PIN_15) /* X motor torque set */
#define LED_RED (BSP_IO_PORT_02_PIN_08) /* LED indicatior red */
#define SPINDLE_ALERT (BSP_IO_PORT_02_PIN_09) /* spindle alert in */
#define LED_GREEN (BSP_IO_PORT_02_PIN_10) /* led indicatior green */
#define SPINDLE_SPEED_PWM (BSP_IO_PORT_03_PIN_02) /* spindle speed control pwm out */
#define USART_CTS (BSP_IO_PORT_03_PIN_03)
#define SPINDLE_SPEED_FEEDBACK (BSP_IO_PORT_03_PIN_04) /* spindle speed pulse input */
#define SPINDLE_BREAK (BSP_IO_PORT_03_PIN_05) /* spindle break out */
#define SPINDLE_ON (BSP_IO_PORT_03_PIN_06) /* spindle on */
#define SPINDLE_DIR (BSP_IO_PORT_03_PIN_07) /* spindle rotation dir */
#define UI_ENC_A (BSP_IO_PORT_04_PIN_00) /* ui_endoder_a */
#define UI_ENC_B (BSP_IO_PORT_04_PIN_01) /* ui_encoder_b */
#define UI_ZSEL (BSP_IO_PORT_04_PIN_02) /* Z_AXIS_SELECT */
#define UI_YSEL (BSP_IO_PORT_04_PIN_03) /* ui y axis select */
#define UI_XSEL (BSP_IO_PORT_04_PIN_04) /* ui x axis select */
#define UI_AUTOFEED_DIR (BSP_IO_PORT_04_PIN_07) /* ui autofeed direction */
#define UI_SPINDLE_ON (BSP_IO_PORT_04_PIN_08) /* ui spindle switch */
#define UI_AUTOFEED_ON (BSP_IO_PORT_04_PIN_09) /* ui autofeed on sw */
#define UI_SET_ORIG (BSP_IO_PORT_04_PIN_10) /* ui fet origin btn */
#define ZM_LIM (BSP_IO_PORT_04_PIN_11) /* Z- axis limit sw */
#define ZP_LIM (BSP_IO_PORT_04_PIN_12) /* Z+ axis limit sw */
#define YM_LIM (BSP_IO_PORT_04_PIN_13) /* Y- axis limit sw */
#define YP_LIM (BSP_IO_PORT_04_PIN_14) /* Y+ axis limit sw */
#define XM_LIM (BSP_IO_PORT_04_PIN_15) /* X- axis limit sw */
#define UI_LCD_RESET (BSP_IO_PORT_05_PIN_00) /* lcd module reset */
#define UI_SPINDLE_SPEED (BSP_IO_PORT_05_PIN_02) /* ui spindle spped setting */
#define UI_AUTOFEED_SPEED (BSP_IO_PORT_05_PIN_03) /* ui autofeed speedseetting */
#define STEPPER_RESET (BSP_IO_PORT_06_PIN_08) /* stepping motor reset */
#define STEPPER_ENABLE (BSP_IO_PORT_06_PIN_09) /* setpping motor enable */
#define STEPPER_ALERT (BSP_IO_PORT_06_PIN_10) /* steppeing motor alert */
#define XP_LIM (BSP_IO_PORT_07_PIN_08) /* X+ axis limit sw */
extern const ioport_cfg_t g_bsp_pin_cfg; /* R7FA6T1AB3CFP.pincfg */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER

#endif /* BSP_PIN_CFG_H_ */
