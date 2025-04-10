#pragma once

typedef enum{
    LED_INDICATE_STANDBY,
    LED_INDICATE_RUN,
    LED_INDICATE_ERROR
}led_indicator_pattern_t ;

void led_indicator_set(led_indicator_pattern_t pattern);