#pragma once
#include "graphic.h"

#define GUI_DEFAULT_TEXT_COLOR 0x0000
#define GUI_DEFAULT_BACKGROUND_COLOR 0xbdf7
#define GUI_DEFAULT_FOCUSED_BG_COLOR 0x07e0

void gui_register_graphic_driver(const graphic_apis_t*apis);
const graphic_apis_t* gui_get_graphic_driver(void);

void clear_screen(void);