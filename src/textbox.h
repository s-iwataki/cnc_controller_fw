/*
 * textbox.h
 *
 *  Created on: 2020/03/18
 *      Author: �@��Y
 */

#ifndef TEXTBOX_H_
#define TEXTBOX_H_
#include <inttypes.h>

typedef struct{
	uint8_t x;
	uint8_t y;
	uint8_t size;
	uint8_t text_capacity;
	int8_t*text;
	uint16_t text_color;
	uint16_t background_color;
}TEXTBOX_t;

void textbox_create(TEXTBOX_t*textbox,uint8_t x,uint8_t y,uint8_t width,uint8_t size);
void textbox_printf(TEXTBOX_t*textbox,int8_t*fmt,...);
void textbox_setcolor(TEXTBOX_t *textbox,uint16_t string_color, uint16_t background_color,uint8_t reflect_immidiate);

#endif /* TEXTBOX_H_ */
