/*
 * font.h
 *
 *  Created on: 2020/03/16
 *      Author: è@àÍòY
 */

#ifndef FONT_H_
#define FONT_H_
#ifdef __AVR__
#include <avr/pgmspace.h>
extern const unsigned char font[] PROGMEM;
#else
extern const unsigned char font[];
#endif
#define ASCII_FONT_WIDTH 5
#define ASCII_FONT_HEIGHT 7

#endif /* FONT_H_ */
