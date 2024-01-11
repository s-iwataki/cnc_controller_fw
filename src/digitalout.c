/*
 * digitalout.c
 *
 *  Created on: 2020/03/18
 *      Author: @ˆê˜Y
 */

#include "digitalout.h"

#include "common_data.h"
#include "r_ioport.h"


void init_digitalout(DIGITALOUT_t* d) {
}
void digitalout_set(DIGITALOUT_t* d) {
    R_IOPORT_PinWrite(&g_ioport_ctrl, d->pin, BSP_IO_LEVEL_HIGH);
}
void digitalout_reset(DIGITALOUT_t* d) {
    R_IOPORT_PinWrite(&g_ioport_ctrl, d->pin, BSP_IO_LEVEL_LOW);
}
