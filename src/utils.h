#pragma once

int my_atof(const char* s, float* result);
/**
 * @brief 文字列を浮動小数点にする
 * 変換できない文字か，文字列終端に出会うまで変換を試み，変換できた文字数を返す
 *
 * @param s 文字列
 * @param result 変換された浮動少数点数
 * @return int 変換成功:変換した文字数，変換できなかったら0を返す
 */
int parse_num(const char* s, float* result) ;