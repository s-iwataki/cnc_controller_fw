/*
 * textbox.c
 *
 *  Created on: 2020/03/18
 *      Author: �@��Y
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "textbox.h"
#include "gui.h"

static void print_text(uint8_t x, uint8_t y, uint8_t size, const int8_t *text, uint16_t text_color, uint16_t background_color)
{
	const graphic_apis_t *apis = gui_get_graphic_driver();
	uint8_t xstep = 6 * size;
	for (size_t i = 0; text[i] != 0; i++)
	{
		apis->draw_char(apis, x + xstep * i, y, size, text[i], text_color, background_color);
	}
}

void textbox_create(TEXTBOX_t *textbox, uint8_t x, uint8_t y, uint8_t width, uint8_t size)
{
	uint8_t chars = width / (6 * size) + 1; // width based text count + \0
	textbox->text = (int8_t *)malloc(chars);
	textbox->size = size;
	textbox->text_capacity = chars;
	textbox->x = x;
	textbox->y = y;
	textbox->background_color = GUI_DEFAULT_BACKGROUND_COLOR;
	textbox->text_color = GUI_DEFAULT_TEXT_COLOR;
	memset(textbox->text, 0, chars);
}
void textbox_printf(TEXTBOX_t *textbox, int8_t *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	vsnprintf(textbox->text, textbox->text_capacity, fmt, arg);
	va_end(arg);

	print_text(textbox->x, textbox->y, textbox->size, textbox->text, textbox->text_color, textbox->background_color);
}
void textbox_setcolor(TEXTBOX_t *textbox, uint16_t string_color, uint16_t background_color, uint8_t reflect_immidiate)
{
	textbox->background_color = background_color;
	textbox->text_color = string_color;
	if (reflect_immidiate)
	{
		print_text(textbox->x, textbox->y, textbox->size, textbox->text, textbox->text_color, textbox->background_color);
	}
}
