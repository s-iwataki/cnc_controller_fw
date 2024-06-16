#pragma once
#include <stddef.h>
#include <stdint.h>

#include "cnc_systemstate.h"
#include "triaxis_table.h"


typedef enum {
    XY_PLANE = G17_XY_PLANE,
    YZ_PLANE = G19_YZ_PLANE,
    ZX_PLANE = G18_ZX_PLANE
} CIRCLE_MOTION_PLANE_t;

typedef enum{
    CCW=G03_CIRCULAR_CCW,
    CW=G02_CIRCULAR_CW
} CIRCLE_MOTION_DIR_t;

/**
 * @brief 
 * 
 * @param table 3軸テーブルドライバ
 * @param plane 作業平面
 * @param dir 回転方向
 * @param p1 円弧終点の第一引数 (xy平面:x,yz平面:y, zx平面:z) mm 絶対値
 * @param p2 円弧終点の第二引数 (xy平面:y,yz平面:z, zx平面:x) mm 絶対値
 * @param c1 円弧中心の第一引数 (xy平面:x,yz平面:y, zx平面:z) mm 現在の位置からの相対値
 * @param c2 円弧中心の第二引数 (xy平面:y,yz平面:z, zx平面:x) mm 現在の位置からの相対値
 * @param speed 送り速度 mm/s
 * @return int 
 */
int move_circular(table_3d_driver_t*table,CIRCLE_MOTION_PLANE_t plane,CIRCLE_MOTION_DIR_t dir,float p1,float p2,float c1,float c2,float speed);