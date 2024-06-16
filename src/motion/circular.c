#include "circular.h"

#include <math.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "arm_math_types.h"
#include "dsp/fast_math_functions.h"
#include "fsp_common_api.h"
#include "motion/circular.h"
#include "projdefs.h"
#include "task.h"
#include "triaxis_table.h"

#define CIRCULAR_MOTION_COMPLETE (1 << 10)
// interrupt rate per sec
#define CIRCULAR_MOTION_INTERRUPT_RATE 100
#define CIRCULAR_MOTION_DL_MIN 0.0125f

typedef struct {
    table_3d_driver_t* table;
    CIRCLE_MOTION_PLANE_t plane;
    CIRCLE_MOTION_DIR_t dir;
    float p1;
    float p2;
    float c1;
    float c2;
    float speed;
    float current_p1;
    float current_p2;
    float r;
    TaskHandle_t caller_task;
    int x_complete;
    int y_complete;
    int z_complete;
    int last;
} circular_motion_ctx_t;

static int iterate_motion(circular_motion_ctx_t* ctx);
static void table_motion_isr(void* ctx, const table_3d_event_t* evt);

static void table_motion_isr(void* ctx, const table_3d_event_t* evt) {
    circular_motion_ctx_t* m = ctx;
    m->x_complete = ((evt->id == X_AXIS_MOTION_COMPLETE) || (evt->inmotion.x == 0)) || (m->x_complete);
    m->y_complete = ((evt->id == Y_AXIS_MOTION_COMPLETE) || (evt->inmotion.y == 0)) || (m->y_complete);
    m->z_complete = ((evt->id == Z_AXIS_MOTION_COMPLETE) || (evt->inmotion.z == 0)) || (m->z_complete);
    if (!(m->x_complete && m->y_complete && m->z_complete)) {  // 他の軸が稼働中
        return;
    }
    switch (m->plane) {
        case XY_PLANE:
            m->current_p1 = evt->pos.x;
            m->current_p2 = evt->pos.y;
            break;
        case YZ_PLANE:
            m->current_p1 = evt->pos.y;
            m->current_p2 = evt->pos.z;
            break;
        case ZX_PLANE:
            m->current_p1 = evt->pos.z;
            m->current_p2 = evt->pos.x;
            break;
    }
    if (m->last) {  // 動作完了
        BaseType_t other_task_woken = 0;
        xTaskNotifyFromISR(m->caller_task, CIRCULAR_MOTION_COMPLETE, eSetBits, &other_task_woken);
        portYIELD_FROM_ISR(other_task_woken);
        return;
    }
    float d1 = m->current_p1 - m->p1;
    float d2 = m->current_p2 - m->p2;
    if ((fabsf(d1) <=  CIRCULAR_MOTION_DL_MIN) && (fabsf(d2) <=  CIRCULAR_MOTION_DL_MIN)) {  // 終了判定
        m->last = 1;
        m->x_complete = 0;
        m->y_complete = 0;
        m->z_complete = 0;
        float v1 = 0, v2 = 0;
        if (d1 != 0.0f) {
            v1 = m->speed;
        }
        if (d2 != 0.0f) {
            v2 = m->speed;
        }
        switch (m->plane) {
            case XY_PLANE:
                table_moveto(m->table, m->p1, m->p2, 0, v1, v2, 0, table_motion_isr, ctx);
                break;
            case YZ_PLANE:
                table_moveto(m->table, 0, m->p1, m->p2, 0, v1, v2, table_motion_isr, ctx);
                break;
            case ZX_PLANE:
                table_moveto(m->table, m->p2, 0, m->p1, v2, 0, v1, table_motion_isr, ctx);
                break;
        }
        return;
    }
    iterate_motion(m);
    return;
}

static int compare_dist_vs_radius(float p1, float p2, float c1, float c2, float r) {
    float d = (p1 - c1) * (p1 - c1) + (p2 - c2) * (p2 - c2) - r * r;
    if (d > 0) {
        return 1;
    }
    if (d < 0) {
        return -1;
    }
    return 0;
}
static int find_quodrant(float p1, float p2, float c1, float c2) {
    int q = 0;
    if (p1 > c1) {
        if (p2 > c2) {
            q = 1;
        } else {
            q = 4;
        }
    } else {
        if (p2 > c2) {
            q = 2;
        } else {
            q = 3;
        }
    }
    return q;
}

static int iterate_motion(circular_motion_ctx_t* ctx) {
    ctx->x_complete = 0;
    ctx->y_complete = 0;
    ctx->z_complete = 0;
    float d1 = 0;
    float d2 = 0;
    float v1 = 0;
    float v2 = 0;
    int q = find_quodrant(ctx->current_p1, ctx->current_p2, ctx->c1, ctx->c2);
    if (compare_dist_vs_radius(ctx->current_p1, ctx->current_p2, ctx->c1, ctx->c2, ctx->r) > 0) {  // 円の外
        if (ctx->dir == CCW) {
            switch (q) {
                case 1:
                    d1 = -CIRCULAR_MOTION_DL_MIN;
                    v1 = ctx->speed;
                    break;
                case 2:
                    d2 = -CIRCULAR_MOTION_DL_MIN;
                    v2 = ctx->speed;
                    break;
                case 3:
                    d1 = CIRCULAR_MOTION_DL_MIN;
                    v1 = ctx->speed;
                    break;
                case 4:
                    d2 = CIRCULAR_MOTION_DL_MIN;
                    v2 = ctx->speed;
                    break;
                default:
                    break;
            }
        } else {
            switch (q) {
                case 1:
                    d2 = -CIRCULAR_MOTION_DL_MIN;
                    v2 = ctx->speed;
                    break;
                case 2:
                    d1 = CIRCULAR_MOTION_DL_MIN;
                    v1 = ctx->speed;
                    break;
                case 3:
                    d2 = CIRCULAR_MOTION_DL_MIN;
                    v2 = ctx->speed;
                    break;
                case 4:
                    d1 = -CIRCULAR_MOTION_DL_MIN;
                    v1 = ctx->speed;
                    break;
                default:
                    break;
            }
        }
    } else {  // 円の内側
        if (ctx->dir == CCW) {
            switch (q) {
                case 1:
                    d2 = CIRCULAR_MOTION_DL_MIN;
                    v2 = ctx->speed;
                    break;
                case 2:
                    d1 = -CIRCULAR_MOTION_DL_MIN;
                    v1 = ctx->speed;
                    break;
                case 3:
                    d2 = -CIRCULAR_MOTION_DL_MIN;
                    v2 = ctx->speed;
                    break;
                case 4:
                    d1 = CIRCULAR_MOTION_DL_MIN;
                    v1 = ctx->speed;
                    break;
                default:
                    break;
            }
        } else {
            switch (q) {
                case 1:
                    d1 = CIRCULAR_MOTION_DL_MIN;
                    v1 = ctx->speed;
                    break;
                case 2:
                    d2 = CIRCULAR_MOTION_DL_MIN;
                    v2 = ctx->speed;
                    break;
                case 3:
                    d1 = -CIRCULAR_MOTION_DL_MIN;
                    v1 = ctx->speed;
                    break;
                case 4:
                    d2 = -CIRCULAR_MOTION_DL_MIN;
                    v2 = ctx->speed;
                    break;
                default:
                    break;
            }
        }
    }
    switch (ctx->plane) {
        case XY_PLANE:
            table_movedelta(ctx->table, d1, d2, 0, v1, v2, 0, table_motion_isr, ctx);
            break;
        case YZ_PLANE:
            table_movedelta(ctx->table, 0, d1, d2, 0, v1, v2, table_motion_isr, ctx);
            break;
        case ZX_PLANE:
            table_movedelta(ctx->table, d2, 0, d1, v2, 0, v1, table_motion_isr, ctx);
            break;
    }
    return 0;
}

static float get_estimate_dulation(CIRCLE_MOTION_DIR_t dir, float p01, float p02, float p1, float p2, float c1, float c2, float speed) {
    float r;
    float sx = (p01 - c1), sy = (p02 - c2), ex = (p1 - c1), ey = (p2 - c2);
    arm_sqrt_f32(sx * sx + sy * sy, &r);
    float theta;
    arm_atan2_f32(sx * ey - sy * ex, sx * ex + sy * ey, &theta);  // atan(ベクトル外積のz成分/内積)
    if (dir == CW) {
        if (theta > 0) {
            theta -= 2 * PI;
        }
    } else {  // CCW
        if (theta < 0) {
            theta += 2 * PI;
        }
    }
    if (theta < 0) {
        theta *= -1;
    }
    float L = r * theta;
    return L / speed;
}

static int check_args(float p01, float p02, float p1, float p2, float c1, float c2, float* r) {
    float d_sc;
    arm_sqrt_f32((p01 - c1) * (p01 - c1) + (p02 - c2) * (p02 - c2), &d_sc);
    float d_se;
    arm_sqrt_f32((p1 - c1) * (p1 - c1) + (p2 - c2) * (p2 - c2), &d_se);
    if (fabsf(d_sc - d_se) > 2 * CIRCULAR_MOTION_DL_MIN) {  // 半径rが一致（誤差程度で一致)
        return -1;
    }
    *r = d_sc;
    return 0;
}

int move_circular(table_3d_driver_t* table, CIRCLE_MOTION_PLANE_t plane, CIRCLE_MOTION_DIR_t dir, float p1, float p2, float c1, float c2, float speed) {
    float x0, y0, z0;
    table_getpos(table, &x0, &y0, &z0);
    TaskHandle_t h = xTaskGetCurrentTaskHandle();

    circular_motion_ctx_t ctx = {.table = table, .c1 = c1, .c2 = c2, .caller_task = h, .dir = dir, .p1 = p1, .p2 = p2, .plane = plane, .speed = speed, .last = 0};
    switch (plane) {
        case XY_PLANE:
            ctx.current_p1 = x0;
            ctx.current_p2 = y0;
            break;
        case YZ_PLANE:
            ctx.current_p1 = y0;
            ctx.current_p2 = z0;
            break;
        case ZX_PLANE:
            ctx.current_p1 = z0;
            ctx.current_p2 = x0;
    }
    ctx.c1 += ctx.current_p1;
    ctx.c2 += ctx.current_p2;  // 絶対位置に変換
    // エラーチェック 現在位置と中心位置と終点位置
    if (check_args(ctx.current_p1, ctx.current_p2, p1, p2, ctx.c1, ctx.c2, &ctx.r) != 0) {
        return -FSP_ERR_INVALID_DATA;
    }
    float estimated_time = get_estimate_dulation(dir, ctx.current_p1, ctx.current_p2, p1, p2, ctx.c1, ctx.c2, speed);
    uint32_t notif_val = 0;
    int wait_ms = (int)(estimated_time * 2000.0f);
    iterate_motion(&ctx);
    xTaskNotifyWait(CIRCULAR_MOTION_COMPLETE, CIRCULAR_MOTION_COMPLETE, &notif_val, pdMS_TO_TICKS(wait_ms));
    if (notif_val & CIRCULAR_MOTION_COMPLETE) {
        return 0;
    }
    return -FSP_ERR_TIMEOUT;
}