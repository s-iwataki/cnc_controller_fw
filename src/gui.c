#include "gui.h"

static const graphic_apis_t*api;

void gui_register_graphic_driver(const graphic_apis_t*apis){
    api=apis;
}
const graphic_apis_t* gui_get_graphic_driver(void){
    return api;
}

void clear_screen(void){
    api->fill_screen(api,GUI_DEFAULT_BACKGROUND_COLOR);
}
