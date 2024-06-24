#include "triaxis_table.h"

void table_moveto(table_3d_driver_t* d, float x, float y, float z, float vx, float vy, float vz, void (*callback)(void*, const table_3d_event_t*), void* ctx) {
    d->moveto(d->hw_driver_instance, x, y, z, vx, vy, vz, callback, ctx);
}
void table_movedelta(table_3d_driver_t* d, float dx, float dy, float dz, float vx, float vy, float vz, void (*callback)(void*, const table_3d_event_t*), void* ctx) {
    d->movedelta(d->hw_driver_instance, dx, dy, dz, vx, vy, vz, callback, ctx);
}
void table_getpos(table_3d_driver_t* d, float* x, float* y, float* z) {
    d->getpos(d->hw_driver_instance, x, y, z);
}

void table_setpos(table_3d_driver_t* d, float x, float y, float z) {
    d->setpos(d->hw_driver_instance, x, y, z);
}
void table_move_cancel(table_3d_driver_t* d) {
    d->cancel(d->hw_driver_instance);
}
void table_setzero(table_3d_driver_t* d) {
    d->setzero(d->hw_driver_instance);
}

void table_enable(table_3d_driver_t* d, int enable) {
    d->enable(d->hw_driver_instance, enable);
}

static table_3d_driver_t driver;
void table_init(void (*table_hw_driver_provider)(table_3d_driver_t* d, const table_mm_per_count_t* mmpc, const table_axis_sign_t* sign), const table_mm_per_count_t* mmpc, const table_axis_sign_t* sign) {
    table_hw_driver_provider(&driver, mmpc, sign);
}
table_3d_driver_t* table_get_driver() {
    return &driver;
}