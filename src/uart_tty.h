#pragma once

#include "r_sci_uart.h"

//attach sci as tty
void uart_tty_attach(uart_ctrl_t*const uart,const uart_cfg_t*const cfg);