#include "encoder.h"

ENCODER_t g_ui_encoder;

void set_encoder_val(ENCODER_t* enc, int val) {
    enc->set_encoder_val_raw(enc->device, val);
}
int get_encoder_val(ENCODER_t* enc) {
    return enc->get_encoder_val_raw(enc->device);
}
int get_encoder_diff(ENCODER_t* enc) {
    return enc->get_encoder_diff_raw(enc->device);
}
void update_encoder(ENCODER_t* enc) {
    enc->update_encoder_raw(enc->device);
}