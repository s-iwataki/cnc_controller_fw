#pragma once

typedef struct {
    void* device;
    void (*set_encoder_val_raw)(void* device, int val);
    int (*get_encoder_val_raw)(void* device);
    int (*get_encoder_diff_raw)(void* device);
    void (*update_encoder_raw)(void* device);
} ENCODER_t;

extern ENCODER_t g_ui_encoder;

void set_encoder_val(ENCODER_t* enc, int val);
int get_encoder_val(ENCODER_t* enc);
int get_encoder_diff(ENCODER_t* enc);
void update_encoder(ENCODER_t* enc);