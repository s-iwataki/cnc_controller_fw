/*
 * font.h
 *
 *  Created on: 2020/03/16
 *      Author: @ˆê˜Y
 */

#ifndef FONT_H_
#define FONT_H_
#ifdef __AVR__
#include <avr/pgmspace.h>
extern const unsigned char font[] PROGMEM;
#else
extern const unsigned char font[];
#endif


#endif /* FONT_H_ */
