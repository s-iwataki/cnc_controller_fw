#pragma once
#include <stdint.h>

typedef struct graphic_apis_s{
    void (*set_rotation)(const struct graphic_apis_s*instance,uint8_t rotaion);
    void (*draw_hline)(const struct graphic_apis_s*instance,uint8_t x1,uint8_t y1,uint8_t x2,uint16_t color);
    void (*draw_vline)(const struct graphic_apis_s*instance,uint8_t x1,uint8_t y1,uint8_t y2,uint16_t color);
    void (*draw_line)(const struct graphic_apis_s*instance,uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t color);
    void (*draw_char)(const struct graphic_apis_s*instance,uint8_t x1,uint8_t y1,uint8_t size,uint8_t c,uint16_t text_color,uint16_t background_color);
    void (*fill_rect)(const struct graphic_apis_s*instance,uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint16_t color);
    void (*fill_screen)(const struct graphic_apis_s*instance,uint16_t color);
    void (*draw_bmp_mono)(const struct graphic_apis_s*instance,uint8_t x1,uint8_t y1,uint8_t w,uint8_t h,const uint8_t*data,uint16_t dot_color,uint16_t background_color);
    void (*draw_bmp)(const struct graphic_apis_s*instance,uint8_t x1,uint8_t y1,uint8_t w,uint8_t h,const uint16_t*data);//RGB565
    uint8_t (*get_width)(const struct graphic_apis_s*instace);
    uint8_t (*get_height)(const struct graphic_apis_s*instace);
}graphic_apis_t;