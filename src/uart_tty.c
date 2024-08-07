#include "uart_tty.h"

#include <stdint.h>
#include <stdio.h>
#include <termios.h>

#include "FreeRTOS.h"
#include "fsp_common_api.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "r_sci_uart.h"
#include "r_uart_api.h"
#include "semphr.h"
#include "sys/termios.h"
#include "task.h"
#include "tty.h"

typedef struct {
    uart_ctrl_t* p_api_ctrl;
    TaskHandle_t rx_task_handle;
    SemaphoreHandle_t rx_task_mutex;
    StaticSemaphore_t rx_task_mutex_mem;
    TaskHandle_t tx_task_handle;
    SemaphoreHandle_t tx_task_mutex;
    StaticSemaphore_t tx_task_mutex_mem;
    struct termios tc_config;
} uart_tty_device_t;

void uart_tty_callback(uart_callback_args_t* p_arg) {
    uart_tty_device_t* d = (uart_tty_device_t*)p_arg->p_context;
    BaseType_t higher_priority_task_woken_on_rx = pdFALSE;
    BaseType_t higher_priority_task_woken_on_tx = pdFALSE;
    if ((p_arg->event & (UART_EVENT_ERR_FRAMING | UART_EVENT_BREAK_DETECT |
                         UART_EVENT_RX_COMPLETE | UART_EVENT_ERR_OVERFLOW |
                         UART_EVENT_ERR_PARITY)) != 0) {
        if (d->rx_task_handle != NULL) {
            xTaskNotifyFromISR(d->rx_task_handle,
                               (uint32_t)p_arg->event &
                                   (UART_EVENT_ERR_FRAMING | UART_EVENT_BREAK_DETECT |
                                    UART_EVENT_RX_COMPLETE | UART_EVENT_ERR_OVERFLOW |
                                    UART_EVENT_ERR_PARITY),
                               eSetBits, &higher_priority_task_woken_on_rx);
        }
    }
    if (p_arg->event & UART_EVENT_TX_COMPLETE) {
        if (d->tx_task_handle != NULL) {
            xTaskNotifyFromISR(d->tx_task_handle,
                               (uint32_t)p_arg->event & UART_EVENT_TX_COMPLETE,
                               eSetBits, &higher_priority_task_woken_on_tx);
        }
    }
    portYIELD_FROM_ISR(
        (higher_priority_task_woken_on_rx | higher_priority_task_woken_on_tx));
}

int uart_tty_read_bin(void* device_instance, char* ptr, int len) {
    uart_tty_device_t* d = (uart_tty_device_t*)device_instance;
    d->rx_task_handle = xTaskGetCurrentTaskHandle();
    R_SCI_UART_Read(d->p_api_ctrl, ptr, len);
    uint32_t uart_evt=0;
    while (1) {//uart以外からtask notificationを受け取るケースを考慮
        xTaskNotifyWait(UART_EVENT_ERR_FRAMING | UART_EVENT_BREAK_DETECT |
                            UART_EVENT_RX_COMPLETE | UART_EVENT_ERR_OVERFLOW |
                            UART_EVENT_ERR_PARITY,
                        UART_EVENT_ERR_FRAMING | UART_EVENT_BREAK_DETECT |
                            UART_EVENT_RX_COMPLETE | UART_EVENT_ERR_OVERFLOW |
                            UART_EVENT_ERR_PARITY,
                        &uart_evt, portMAX_DELAY);
        if (uart_evt & (UART_EVENT_ERR_FRAMING | UART_EVENT_BREAK_DETECT |
                        UART_EVENT_RX_COMPLETE | UART_EVENT_ERR_OVERFLOW |
                        UART_EVENT_ERR_PARITY)) {
            break;
        }
    }
    int read_len;
    if (uart_evt & (UART_EVENT_ERR_FRAMING | UART_EVENT_ERR_OVERFLOW |
                    UART_EVENT_ERR_PARITY | UART_EVENT_BREAK_DETECT)) {
        uint32_t remain;
        R_SCI_UART_ReadStop(d->p_api_ctrl, &remain);
        read_len = len - remain;
    } else {  // read complete
        read_len = len;
    }
    d->rx_task_handle = NULL;
    return read_len;
}
int uart_tty_write(void* device_instance, char* ptr, int len);
int uart_tty_read(void* device_instance, char* ptr, int len) {
    uart_tty_device_t* d = (uart_tty_device_t*)device_instance;
    xSemaphoreTake(d->rx_task_mutex, portMAX_DELAY);
    int read_len = 0;
    if (d->tc_config.c_lflag & ICANON) {
        // canonical mode
        // 現在の実装は，改行コードまで読む
        // 他のフラグの処理は未実装
        for (read_len = 0; read_len < len;) {
            char c;
            int r = uart_tty_read_bin(device_instance, &c, 1);
            if (r > 0) {
                if (d->tc_config.c_lflag & ECHO) {
                    uart_tty_write(device_instance, &c, 1);
                }
                *(ptr++) = c;
                read_len += r;
                if (c == '\n') {
                    break;
                }
            }
        }
    } else {
        // raw mode
        read_len = uart_tty_read_bin(device_instance, ptr, len);
    }

    xSemaphoreGive(d->rx_task_mutex);
    return read_len;
}
int uart_tty_write(void* device_instance, char* ptr, int len) {
    uart_tty_device_t* d = (uart_tty_device_t*)device_instance;
    xSemaphoreTake(d->tx_task_mutex, portMAX_DELAY);
    d->tx_task_handle = xTaskGetCurrentTaskHandle();
    if (R_SCI_UART_Write(d->p_api_ctrl, ptr, len) != FSP_SUCCESS) {
        d->tx_task_handle = NULL;
        xSemaphoreGive(d->tx_task_mutex);
        return -1;
    }
    uint32_t uart_evt=0;
    while (1) {//uart以外からtask notificationを受け取るケースを考慮
        xTaskNotifyWait(UART_EVENT_TX_COMPLETE, UART_EVENT_TX_COMPLETE, &uart_evt,
                        portMAX_DELAY);
        if (uart_evt & UART_EVENT_TX_COMPLETE) {
            break;
        }
    }
    d->tx_task_handle = NULL;
    xSemaphoreGive(d->tx_task_mutex);
    return len;
}

uart_tty_device_t default_tty_uart_device;
tty_device_t default_tty_uart = {.device_instance = &default_tty_uart_device,
                                 .tty_d_read = uart_tty_read,
                                 .tty_d_write = uart_tty_write};

void uart_tty_attach(uart_ctrl_t* const uart, const uart_cfg_t* const cfg) {
    default_tty_uart_device.p_api_ctrl = uart;
    default_tty_uart_device.rx_task_handle = NULL;
    default_tty_uart_device.tx_task_handle = NULL;
    default_tty_uart_device.rx_task_mutex =
        xSemaphoreCreateMutexStatic(&default_tty_uart_device.rx_task_mutex_mem);
    default_tty_uart_device.tx_task_mutex =
        xSemaphoreCreateMutexStatic(&default_tty_uart_device.tx_task_mutex_mem);
    default_tty_uart_device.tc_config.c_iflag = IGNBRK | IGNCR;
    default_tty_uart_device.tc_config.c_oflag = 0;
    default_tty_uart_device.tc_config.c_cflag = CS8 | CREAD;
    default_tty_uart_device.tc_config.c_lflag = ICANON;
    R_SCI_UART_Open(uart, cfg);
    R_SCI_UART_CallbackSet(uart, uart_tty_callback, &default_tty_uart_device,
                           NULL);
    tty_register_stdio_device(&default_tty_uart);
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
}

int tcgetattr(int fd, struct termios* termios_p) {
    if ((0 <= fd) && (fd <= 2)) {
        *termios_p = default_tty_uart_device.tc_config;
    }
    return 0;
}

int tcsetattr(int fd, int optional_actions,
              const struct termios* termios_p) {
    if ((0 <= fd) && (fd <= 2)) {
        default_tty_uart_device.tc_config = *termios_p;
    }
    return 0;
}
