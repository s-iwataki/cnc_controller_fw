#include "circular.h"

#include <math.h>
#include <stdint.h>

#include "FreeRTOS.h"
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
    float dl;
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
    m->x_complete = ((evt->id == X_AXIS_MOTION_COMPLETE) || (evt->inmotion.x == 0));
    m->y_complete = ((evt->id == Y_AXIS_MOTION_COMPLETE) || (evt->inmotion.y == 0));
    m->z_complete = ((evt->id == Z_AXIS_MOTION_COMPLETE) || (evt->inmotion.z == 0));
    if (!(m->x_complete && m->y_complete && m->z_complete)) {  // 他の軸が稼働中
        return;
    }
    switch (m->plane) {
        case XY_PLANE:
            m->current_p1=evt->pos.x;
            m->current_p2=evt->pos.y;
            break;
        case YZ_PLANE:
            m->current_p1=evt->pos.y;
            m->current_p2=evt->pos.z;
            break;
        case ZX_PLANE:
            m->current_p1=evt->pos.z;
            m->current_p2=evt->pos.x;
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
    if ((d1 * d1 + d2 * d2) < 4 * CIRCULAR_MOTION_DL_MIN * CIRCULAR_MOTION_DL_MIN) {  // 終了判定
        m->last = 1;
        switch (m->plane) {
            case XY_PLANE:
                table_moveto(m->table, m->p1, m->p2, 0, m->speed, m->speed, 0, table_motion_isr, ctx);
                break;
            case YZ_PLANE:
                table_moveto(m->table, 0, m->p1, m->p2, 0, m->speed, m->speed, table_motion_isr, ctx);
                break;
            case ZX_PLANE:
                table_moveto(m->table, m->p2, 0, m->p1, m->speed, 0, m->speed, table_motion_isr, ctx);
                break;
        }
        return;
    }
    iterate_motion(m);
    return;
}

static int iterate_motion(circular_motion_ctx_t* ctx) {
    ctx->x_complete = 0;
    ctx->y_complete = 0;
    ctx->z_complete = 0;
    float d = ctx->dl;
    float d1 = d * (ctx->current_p2 - ctx->c2) / ctx->r;
    float d2 = d * (ctx->current_p1 - ctx->c1) / ctx->r;
    float v1 = ctx->speed * (ctx->current_p2 - ctx->c2) / ctx->r;
    float v2 = ctx->speed * (ctx->current_p1 - ctx->c1) / ctx->r;
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
    float d_sc = (p01 - c1) * (p01 - c1) + (p02 - c2) * (p02 - c2);
    float d_se = (p1 - c1) * (p1 - c1) + (p2 - c2) * (p2 - c2);
    if (fabsf(d_sc - d_se) > CIRCULAR_MOTION_DL_MIN) {  // 半径rが一致（誤差程度で一致)
        return -1;
    }
    arm_sqrt_f32(d_sc, r);
    return 0;
}

int move_circular(table_3d_driver_t* table, CIRCLE_MOTION_PLANE_t plane, CIRCLE_MOTION_DIR_t dir, float p1, float p2, float c1, float c2, float speed) {
    float x0, y0, z0;
    table_getpos(table, &x0, &y0, &z0);
    TaskHandle_t h = xTaskGetCurrentTaskHandle();
    float dl = speed / (1.0f * CIRCULAR_MOTION_INTERRUPT_RATE);
    if (dl < CIRCULAR_MOTION_DL_MIN) {
        dl = CIRCULAR_MOTION_DL_MIN;
    }

    circular_motion_ctx_t ctx = {.table = table, .c1 = c1, .c2 = c2, .caller_task = h, .dir = dir, .p1 = p1, .p2 = p2, .plane = plane, .speed = speed, .dl = dl, .last = 0};
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
    int wait_ms = (int)(estimated_time * 1000.0f);
    printf("start circle motion. time %d\r\n", wait_ms);
    iterate_motion(&ctx);
    printf("wait circle motion for %d ms\r\n", wait_ms);
    xTaskNotifyWait(CIRCULAR_MOTION_COMPLETE, CIRCULAR_MOTION_COMPLETE, &notif_val, pdMS_TO_TICKS(wait_ms));
    if (notif_val & CIRCULAR_MOTION_COMPLETE) {
        return 0;
    }
    return -FSP_ERR_TIMEOUT;
}