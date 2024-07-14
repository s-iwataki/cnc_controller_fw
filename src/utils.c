#include "utils.h"
#include <string.h>

/**
 * @brief 文字列を浮動小数点にする
 *
 * @param s 文字列
 * @param result 変換された浮動少数点数
 * @return int 変換成功:0,変換失敗:-1
 */
int my_atof(const char* s, float* result) {
    *result = 0;
    int len = strlen(s);
    int sign = 1;
    int i = 0;
    int int_part = 0;
    for (i = 0; i < len; i++) {
        int_part = int_part * 10;
        if (s[i] == '-' && i == 0) {
            sign = -1;
        } else if (s[i] == '+' && i == 0) {
            sign = 1;
        } else if ((s[i] >= '0') && (s[i] <= '9')) {
            int_part += (s[i] - '0');
        } else if (s[i] == '.') {
            break;
        } else {
            return -1;
        }
    }
    float float_part = 0;
    float digit = 1.0f;
    for (i++; i < len; i++) {
        digit = digit * 0.1f;
        if ((s[i] >= '0') && (s[i] <= '9')) {
            float_part += digit * (s[i] - '0');
        } else {
            return -1;
        }
    }
    *result = (float)sign * ((float)int_part + float_part);
    return 0;
}

/**
 * @brief 文字列を浮動小数点にする
 * 変換できない文字か，文字列終端に出会うまで変換を試み，変換できた文字数を返す
 *
 * @param s 文字列
 * @param result 変換された浮動少数点数
 * @return int 変換成功:変換した文字数,変換失敗:-1
 */
int parse_num(const char* s, float* result) {
    *result = 0;
    int consumed = 0;
    int sign = 1;
    int i = 0;
    int int_part = 0;
    for (i = 0; s[i] != 0; i++) {
        if (s[i] == '-' && i == 0) {
            sign = -1;
            consumed++;
        } else if (s[i] == '+' && i == 0) {
            sign = 1;
            consumed++;
        } else if ((s[i] >= '0') && (s[i] <= '9')) {
            int_part = int_part * 10;
            int_part += (s[i] - '0');
            consumed++;
        } else if (s[i] == '.') {
            consumed++;
            break;
        } else {
            *result = (float)(sign * int_part);
            return consumed;
        }
    }
    float float_part = 0;
    float digit = 1.0f;
    for (i++; s[i] != 0; i++) {
        if ((s[i] >= '0') && (s[i] <= '9')) {
            digit = digit * 0.1f;
            float_part += digit * (float)(s[i] - '0');
            consumed++;
        } else {
            break;
        }
    }
    *result = (float)sign * ((float)int_part + float_part);
    return consumed;
}