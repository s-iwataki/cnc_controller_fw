#include "gcode_cmd.h"

#include <stdio.h>

#include "arm_math_types.h"
#include "cnc_systemstate.h"
#include "commands/gcode/gcode_errortype.h"
#include "dsp/fast_math_functions.h"
#include "fsp_common_api.h"
#include "gcode_errortype.h"
#include "motion/circular.h"
#include "projdefs.h"
#include "spindle.h"
#include "triaxis_table.h"
#include "utils.h"


typedef struct {
    float val;
    int valid;
} g_code_block_args_t;
typedef struct {
    g_code_abs_inc_state_t abs_incr;
    g_code_interpolation_state_t interpolation_mode;
    g_code_working_plane_t working_plane;
    g_code_motor_cmd_t motor;
    int g92_set_coord;
    g_code_block_args_t x;
    g_code_block_args_t y;
    g_code_block_args_t z;
    g_code_block_args_t i;
    g_code_block_args_t j;
    g_code_block_args_t k;
    g_code_block_args_t s;
    g_code_block_args_t f;
    g_code_block_args_t r;
} g_code_block_t;

/**
 * @brief 文字列を浮動小数点にする
 * 変換できない文字か，文字列終端に出会うまで変換を試み，変換できた文字数を返す
 *
 * @param s 文字列
 * @param result 変換された浮動少数点数
 * @return int 変換成功:変換した文字数,変換失敗:-1
 */
static int parse_num(const char* s, float* result) {
    *result = 0;
    int consumed = 0;
    int sign = 1;
    int i = 0;
    int int_part = 0;
    for (i = 0; s[i] != 0; i++) {
        int_part = int_part * 10;
        if (s[i] == '-' && i == 0) {
            sign = -1;
            consumed++;
        } else if (s[i] == '+' && i == 0) {
            sign = 1;
            consumed++;
        } else if ((s[i] >= '0') && (s[i] <= '9')) {
            int_part += (s[i] - '0');
            consumed++;
        } else if (s[i] == '.') {
            consumed++;
            break;
        } else {
            *result = sign * int_part;
            return consumed;
        }
    }
    float float_part = 0;
    float digit = 1.0f;
    for (i++; s[i] != 0; i++) {
        digit = digit * 0.1f;
        if ((s[i] >= '0') && (s[i] <= '9')) {
            float_part += digit * (s[i] - '0');
            consumed++;
        } else {
            break;
        }
    }
    *result = (float)sign * ((float)int_part + float_part);
    return consumed;
}

static int parse_gcode_block(g_code_state_t* gcode_state, const char* line, g_code_block_t* block) {
    for (int pos = 0; line[pos] != 0;) {
        char ltr = line[pos++];
        if ((ltr < 'A') || (ltr > 'Z')) {
            return -GCODE_ERROR_BLOCK_START_INVALID_CHAR;
        }
        float val;
        int consumed = parse_num(&line[pos], &val);
        if (consumed == 0) {
            // Gコードでは文字のあとは必ず数字
            return -GCODE_ERROR_EXPECTED_NUMBER;
        }
        pos += consumed;
        int int_num = (int)val;
        // ltr Gxx Mxx それ以外判定
        switch (ltr) {
            case 'G': {
                if ((int_num >= 0) && (int_num < 4)) {
                    if (block->interpolation_mode != G01_03_UNSET) {
                        return -GCODE_ERROR_DUPLICATED_INTERPOLATION;
                    }
                    if (block->g92_set_coord != 0) {
                        return -GCODE_ERROR_G92_AND_TABLE_MOTION;
                    }
                    switch (int_num) {
                        case 0:
                            block->interpolation_mode = G00_FAST_FEED;
                            break;
                        case 1:
                            block->interpolation_mode = G01_LINEAR;
                            break;
                        case 2:
                            block->interpolation_mode = G02_CIRCULAR_CW;
                            break;
                        case 3:
                            block->interpolation_mode = G03_CIRCULAR_CCW;
                            break;
                    }
                } else if ((int_num >= 17) && (int_num < 20)) {
                    if (block->working_plane != G17_19_UNSET) {
                        return -GCODE_ERROR_DUPLICATED_WORKPLANE;
                    }
                    switch (int_num) {
                        case 17:
                            block->working_plane = G17_XY_PLANE;
                            break;
                        case 18:
                            block->working_plane = G18_ZX_PLANE;
                            break;
                        case 19:
                            block->working_plane = G19_YZ_PLANE;
                            break;
                    }
                } else if (int_num == 90) {
                    if (block->abs_incr != G90_91_UNSET) {
                        return -GCODE_ERROR_DUPLICATED_POSITIONING_MODE;
                    }
                    block->abs_incr = G90_ABS;
                } else if (int_num == 91) {
                    if (block->abs_incr != G90_91_UNSET) {
                        return -GCODE_ERROR_DUPLICATED_POSITIONING_MODE;
                    }
                    block->abs_incr = G91_INCR;
                } else if (int_num == 92) {  // G92 set work coord
                    if ((block->g92_set_coord != 0) || (block->interpolation_mode != G01_03_UNSET)) {
                        return -GCODE_ERROR_G92_AND_TABLE_MOTION;
                    }
                }else{
                    return -GCODE_ERROR_UNSUPPORTED_COMMAND;
                }
            } break;
            case 'M': {
                if (block->motor != M03_05_UNSET) {
                    return -GCODE_ERROR_DUPLICATED_MOTOR_COMMAND;
                }
                if (int_num == 3) {
                    block->motor = M03_MOTOR_ON;
                } else if (int_num == 5) {
                    block->motor = M05_MOTOR_OFF;
                } else {
                    return -GCODE_ERROR_UNSUPPORTED_COMMAND;
                }

            } break;
            case 'X': {
                if (block->x.valid != 0) {
                    return -GCODE_ERROR_DUPLICATED_ARGS;
                }
                block->x.val = val;
                block->x.valid = 1;
            } break;
            case 'Y': {
                if (block->y.valid != 0) {
                    return -GCODE_ERROR_DUPLICATED_ARGS;
                }
                block->y.val = val;
                block->y.valid = 1;
            } break;
            case 'Z': {
                if (block->z.valid != 0) {
                    return -GCODE_ERROR_DUPLICATED_ARGS;
                }
                block->z.val = val;
                block->z.valid = 1;
            } break;
            case 'I': {
                if (block->i.valid != 0) {
                    return -GCODE_ERROR_DUPLICATED_ARGS;
                }
                block->i.val = val;
                block->i.valid = 1;
            } break;
            case 'J': {
                if (block->j.valid != 0) {
                    return -GCODE_ERROR_DUPLICATED_ARGS;
                }
                block->j.val = val;
                block->j.valid = 1;
            } break;
            case 'K': {
                if (block->k.valid != 0) {
                    return -GCODE_ERROR_DUPLICATED_ARGS;
                }
                block->k.val = val;
                block->k.valid = 1;
            } break;
            case 'R': {
                if (block->r.valid != 0) {
                    return -GCODE_ERROR_DUPLICATED_ARGS;
                }
                block->r.val = val;
                block->r.valid = 1;
            } break;
            case 'S': {
                if (block->s.valid != 0) {
                    return -GCODE_ERROR_DUPLICATED_ARGS;
                }
                block->s.val = val;
                block->s.valid = 1;
            } break;
            case 'F': {
                if (block->f.valid != 0) {
                    return -GCODE_ERROR_DUPLICATED_ARGS;
                }
                block->f.val = val;
                block->f.valid = 1;
            } break;
        }
    }
    // filled parameter
    if (block->abs_incr == G90_91_UNSET) {
        block->abs_incr = gcode_state->abs_incr;
    }
    if ((block->interpolation_mode == G01_03_UNSET) && (block->g92_set_coord == 0)) {
        block->interpolation_mode = gcode_state->interpolation_mode;
    }

    if (block->working_plane == G17_19_UNSET) {
        block->working_plane = gcode_state->working_plane;
    }
    return 0;
}

static int execute_g01(g_code_state_t* gcode_state, g_code_block_t* block) {
    table_3d_driver_t* tbl = table_get_driver();
    float x = 0, y = 0, z = 0;
    if (block->i.valid || block->j.valid || block->k.valid || block->r.valid) {
        return -GCODE_ERROR_UNEXPECTED_ARGS;
    }
    if (block->f.valid) {
        gcode_state->feed = block->f.val;
    }
    if (block->abs_incr == G91_INCR) {
        if (block->x.valid) {
            x = block->x.val;
        }
        if (block->y.valid) {
            y = block->y.val;
        }
        if (block->z.valid) {
            z = block->z.val;
        }

        float d = 0;
        float f = gcode_state->feed;
        arm_sqrt_f32(x * x + y * y + z * z, &d);
        if (d == 0.0f) {
            return 0;
        };

        float32_t vx = x / d * f;
        float32_t vy = y / d * f;
        float32_t vz = z / d * f;
        table_movedelta(tbl, x, y, z, vx, vy, vz, NULL, NULL);
    } else {
        table_getpos(tbl, &x, &y, &z);
        float dx = 0, dy = 0, dz = 0;
        if (block->x.valid) {
            dx = block->x.val - x;
            x = block->x.val;
        }
        if (block->y.valid) {
            dy = block->y.val - y;
            y = block->y.val;
        }
        if (block->z.valid) {
            dz = block->z.val - z;
            z = block->z.val;
        }

        float d = 0;
        float f = gcode_state->feed;
        arm_sqrt_f32(dx * dx + dy * dy + dz * dz, &d);
        if (d == 0.0f) {
            return 0;
        };

        float32_t vx = dx / d * f;
        float32_t vy = dy / d * f;
        float32_t vz = dz / d * f;
        table_moveto(tbl, x, y, z, vx, vy, vz, NULL, NULL);
    }
}
static int g0203_chk_param(g_code_block_t* block, CIRCLE_MOTION_DIR_t ccwcw, float x, float y, float z, float* p1, float* p2, float* c1, float* c2) {
    float p1_work = 0, p2_work = 0;
    float c1_work = 0, c2_work = 0;
    float b1 = 0, b2 = 0;
    switch (block->working_plane) {
        case G17_XY_PLANE:
            if (((block->i.valid != 0) || (block->j.valid != 0)) && (block->r.valid != 0)) {  // i,jとrは同時指定不可
                return -GCODE_ERROR_UNEXPECTED_ARGS;
            }
            if (block->k.valid) {  // xy平面時は中心座標k設定不可
                return -GCODE_ERROR_UNEXPECTED_ARGS;
            }
            if (block->x.valid) {
                p1_work = block->x.val;
            }
            if (block->y.valid) {
                p2_work = block->y.val;
            }
            if (block->abs_incr == G91_INCR) {
                p1_work += x;
                p2_work += y;
            }
            b1 = x;
            b2 = y;
            if (block->i.valid) {
                c1_work = block->i.val;
            }
            if (block->j.valid) {
                c2_work = block->j.val;
            }
            break;
        case G18_ZX_PLANE:
            if (((block->i.valid != 0) || (block->k.valid != 0)) && (block->r.valid != 0)) {  // k,iとrは同時指定不可
                return -GCODE_ERROR_UNEXPECTED_ARGS;
            }
            if (block->j.valid) {  // zx平面時は中心座標j設定不可
                return -GCODE_ERROR_UNEXPECTED_ARGS;
            }
            if (block->z.valid) {
                p1_work = block->z.val;
            }
            if (block->x.valid) {
                p2_work = block->x.val;
            }
            if (block->abs_incr == G91_INCR) {
                p1_work += z;
                p2_work += x;
            }
            b1 = z;
            b2 = x;
            if (block->k.valid) {
                c1_work = block->k.val;
            }
            if (block->i.valid) {
                c2_work = block->i.val;
            }
            break;
        case G19_YZ_PLANE:
            if (((block->j.valid != 0) || (block->k.valid != 0)) && (block->r.valid != 0)) {  // j,kとrは同時指定不可
                return -GCODE_ERROR_UNEXPECTED_ARGS;
            }
            if (block->i.valid) {  // yz平面時は中心座標i設定不可
                return -GCODE_ERROR_UNEXPECTED_ARGS;
            }
            if (block->y.valid) {
                p1_work = block->y.val;
            }
            if (block->z.valid) {
                p2_work = block->z.val;
            }
            if (block->abs_incr == G91_INCR) {
                p1_work += y;
                p2_work += z;
            }
            b1 = y;
            b2 = z;
            if (block->j.valid) {
                c1_work = block->j.val;
            }
            if (block->k.valid) {
                c2_work = block->k.val;
            }
            break;
    }
    *p1 = p1_work;
    *p2 = p2_work;
    if (block->r.valid == 0) {
        *c1 = c1_work;
        *c2 = c2_work;
        return 0;
    }
    float d_2 = (p1_work - b1) * (p1_work - b1) + (p2_work - b2) * (p2_work - b2);
    float r = block->r.val;
    if (d_2 >= 4.0f * r * r) {
        return -GCODE_ERROR_INVALID_CIRCLE_PARAMS;
    }
    float k;
    arm_sqrt_f32(r * r / d_2 - 0.25f, &k);
    if (((ccwcw == CCW) && (r < 0)) || ((ccwcw == CW) && (r > 0))) {
        k = -k;
    }
    *c1 = (-b1 + p1_work) / 2.0f - k * (p2_work - b2);
    *c2 = (-b2 + p2_work) / 2.0f + k * (p1_work - b1);
    return 0;
}
static int execute_g0203(g_code_state_t* gcode_state, g_code_block_t* block, CIRCLE_MOTION_DIR_t ccwcw) {
    float x = 0, y = 0, z = 0;
    table_3d_driver_t* tbl = table_get_driver();
    table_getpos(tbl, &x, &y, &z);

    if (block->f.valid) {
        gcode_state->feed = block->f.val;
    }
    if (block->abs_incr == G91_INCR) {
        if (block->x.valid) {
            x += block->x.val;
        }
        if (block->y.valid) {
            y += block->y.val;
        }
        if (block->z.valid) {
            z += block->z.val;
        }
    }
    float p1, p2, c1, c2;
    int ret = g0203_chk_param(block, ccwcw, x, y, z, &p1, &p2, &c1, &c2);
    if (ret < 0) {
        return ret;
    }
    return move_circular(tbl, (CIRCLE_MOTION_PLANE_t)block->working_plane, ccwcw, p1, p2, c1, c2, gcode_state->feed);
}
// CW
static int execute_g02(g_code_state_t* gcode_state, g_code_block_t* block) {
    return execute_g0203(gcode_state, block, CW);
}
// CCW
static int execute_g03(g_code_state_t* gcode_state, g_code_block_t* block) {
    return execute_g0203(gcode_state, block, CCW);
}
static int execute_g92(g_code_state_t* gcode_state, g_code_block_t* block) {
    table_3d_driver_t* tbl = table_get_driver();
    float x, y, z;
    table_getpos(tbl, &x, &y, &z);
    if (block->f.valid || block->i.valid || block->j.valid || block->k.valid || block->r.valid || block->s.valid) {
        return -1;
    }
    if (block->x.valid) {
        x = block->x.val;
    }
    if (block->y.valid) {
        y = block->y.val;
    }
    if (block->z.valid) {
        z = block->z.val;
    }
    table_setpos(tbl, x, y, z);
    return 0;
}
typedef struct {
    g_code_interpolation_state_t gcode;
    int (*func)(g_code_state_t*, g_code_block_t*);
} gcode_table_operation_t;

static gcode_table_operation_t gcode_table_ops[] = {
    {.gcode = G00_FAST_FEED, .func = execute_g01},
    {.gcode = G01_LINEAR, .func = execute_g01},
    {.gcode = G02_CIRCULAR_CW, .func = execute_g02},
    {.gcode = G03_CIRCULAR_CCW, .func = execute_g03},
};

static int execute_block(g_code_state_t* s, g_code_block_t* block) {
    if (block->s.valid) {
        s->spindle_rpm_ref = block->s.val;
        spindle_set_speed(&g_spindle_motor, (int)s->spindle_rpm_ref);
    }
    if (block->motor == M03_MOTOR_ON) {
        spindle_enable(&g_spindle_motor, pdTRUE);
        spindle_control_mode_set(&g_spindle_motor, SPINDLE_SPEED_CONTROL);
        spindle_set_speed(&g_spindle_motor, (int)s->spindle_rpm_ref);
    } else if (block->motor == M05_MOTOR_OFF) {
        spindle_enable(&g_spindle_motor, pdFALSE);
    }
    if (block->interpolation_mode != G01_03_UNSET) {
        for (int i = 0; i < sizeof(gcode_table_ops) / sizeof(gcode_table_operation_t); i++) {
            if (gcode_table_ops[i].gcode == block->interpolation_mode) {
                return gcode_table_ops[i].func(s, block);
            }
        }
    }
    if (block->g92_set_coord) {
        execute_g92(s, block);
    }
}

int gcode_cmd(int argc, char** argv) {
    char buf[256];
    cnc_system_state_set_gcode_exec();
    while (1) {
        printf("OK\r\n");
        g_code_block_t block = {0};
        block.abs_incr = G90_91_UNSET;
        block.interpolation_mode = G01_03_UNSET;
        block.motor = M03_05_UNSET;
        block.working_plane = G17_19_UNSET;
        block.g92_set_coord = 0;

        fgets(buf, sizeof(buf), stdin);
        int retval = parse_gcode_block(buf);
        if (retval < 0) {
            printf("ERR:%d\r\n", retval);
        } else if (retval == 0) {
            printf("END\r\n");
            break;
        }
    }
    cnc_system_state_unset_gcode_exec();
}