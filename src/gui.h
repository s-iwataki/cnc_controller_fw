#pragma once
#include "graphic.h"

#define GUI_DEFAULT_TEXT_COLOR 0xffff
#define GUI_DEFAULT_BACKGROUND_COLOR 0x001f

void gui_register_graphic_driver(const graphic_apis_t*apis);
const graphic_apis_t* gui_get_graphic_driver(void);

void clear_screen(void);