/*
 * digitalout.h
 *
 *  Created on: 2020/03/18
 *      Author: è@àÍòY
 */

#ifndef DIGITALOUT_H_
#define DIGITALOUT_H_
#include <inttypes.h>
#include "FreeRTOS.h"
#include "bsp_pin_cfg.h"
#include "common_data.h"
#include "hal_data.h"
#include "portmacro.h"
#include "projdefs.h"
#include "r_ioport.h"

typedef struct {
	bsp_io_port_pin_t pin;
}DIGITALOUT_t;

void init_digitalout(DIGITALOUT_t*d);
void digitalout_set(DIGITALOUT_t*d);
void digitalout_reset(DIGITALOUT_t*d);

#endif /* DIGITALOUT_H_ */
