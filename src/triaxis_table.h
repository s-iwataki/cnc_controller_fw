#pragma once
#include <stdint.h>

typedef enum {
    X_AXIS_MOTION_COMPLETE = 1<<7,
    Y_AXIS_MOTION_COMPLETE = 1<<8,
    Z_AXIS_MOTION_COMPLETE = 1<<9
} table_3d_event_type_t;

typedef struct {
    table_3d_event_type_t id;
    struct position {
        float x;
        float y;
        float z;
    } pos;
    struct inmotion {
        int x;
        int y;
        int z;
    } inmotion;
} table_3d_event_t;

typedef struct {
    float x_pos;
    float y_pos;
    float z_pos;
    float x_speed;
    float y_speed;
    float z_speed;
} table_state_t;

typedef struct {
    float x;
    float y;
    float z;
} table_mm_per_count_t;

typedef struct axis_sign {
    int x;
    int y;
    int z;
} table_axis_sign_t;

typedef struct table_3d_driver {
    void* hw_driver_instance;
    void (*moveto)(void*, float x, float y, float z, float vx, float vy, float vz, void (*callback)(void*, const table_3d_event_t*), void* ctx);
    void (*movedelta)(void*, float x, float y, float z, float vx, float vy, float vz, void (*callback)(void*, const table_3d_event_t*), void* ctx);
    void (*getpos)(void*, float* x, float* y, float* z);
    void (*cancel)(void*);
    void (*setzero)(void*);
    void (*enable)(void*, int);
} table_3d_driver_t;

/**
 * @brief テーブルを位置(x,y,z)へ動かす
 *
 * @param d ドライバデバイス
 * @param x x目標位置 [mm]
 * @param y y目標位置 [mm]
 * @param z z目標位置 [mm]
 * @param vx x軸移動スピード [mm/s]
 * @param vy y軸移動スピード [mm/s]
 * @param vz z軸移動スピード [mm/s]
 * @param callback 動作完了時に呼ばれるコールバック
 * @param ctx コールバックにわたす引数
 */
void table_moveto(table_3d_driver_t* d, float x, float y, float z, float vx, float vy, float vz, void (*callback)(void*, const table_3d_event_t*), void* ctx);
void table_movedelta(table_3d_driver_t* d, float dx, float dy, float dz, float vx, float vy, float vz, void (*callback)(void*, const table_3d_event_t*), void* ctx);
void table_getpos(table_3d_driver_t* d, float* x, float* y, float* z);
void table_move_cancel(table_3d_driver_t* d);
void table_setzero(table_3d_driver_t* d);
void table_enable(table_3d_driver_t* d, int enable);
void table_init(void (*table_hw_driver_provider)(table_3d_driver_t* d, const table_mm_per_count_t* mmpc, const table_axis_sign_t* sign), const table_mm_per_count_t* mmpc, const table_axis_sign_t* sign);
table_3d_driver_t* table_get_driver();
