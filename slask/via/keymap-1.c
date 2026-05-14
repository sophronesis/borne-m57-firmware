#include QMK_KEYBOARD_H
#include "quantum.h"
#include "hal_pal.h"

// =====================
// LAYERS
// =====================

enum layers {
    _BASE = 0,
    _FN1,
    _FN2,
    _FN3
};

// =====================
// KEYMAPS
// =====================

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {


// ==================================================
// BASE LAYER (0)
// ==================================================

[_BASE] = LAYOUT(

 // Row0
 KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5,           KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,

 // Row1 (INNER: KC_UP, KC_LEFT)
 KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_UP,    KC_LEFT, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC,

 // Row2 (INNER: KC_DOWN, KC_RIGHT)
 KC_CAPS, KC_A, KC_S, KC_D, KC_F, KC_G, KC_DOWN, KC_RIGHT, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,

 // Row3 (INNER: KC_MUTE, RGB_MOD)
 KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_MUTE, RGB_MOD,  KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_DEL,

 // Thumb row
              KC_LALT, MO(_FN1), KC_SPC,          KC_ENT, MO(_FN2), KC_RCTL
),


// ==================================================
// FN1 LAYER (1)
// ==================================================

[_FN1] = LAYOUT(

 KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5,       KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_BSPC,

 KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_UP,     KC_LEFT, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC,

 KC_CAPS, KC_A, KC_S, KC_D, KC_F, KC_G, KC_DOWN,  KC_RIGHT, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,

 KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_MUTE,  RGB_MOD, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_DEL,

              KC_LCTL, MO(_FN1), KC_SPC,           KC_ENT, MO(_FN2), KC_RCTL
),


// ==================================================
// FN2 LAYER (2)
// ==================================================

[_FN2] = LAYOUT(

 KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5,           KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,

 KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_UP,    KC_LEFT, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC,

 KC_CAPS, KC_A, KC_S, KC_D, KC_F, KC_G, KC_DOWN, KC_RIGHT, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,

 KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_MUTE, RGB_MOD,  KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_DEL,

              KC_LCTL, MO(_FN1), KC_SPC,          KC_ENT, MO(_FN2), KC_RCTL
),


// ==================================================
// FN3 LAYER (3)
// ==================================================

[_FN3] = LAYOUT(

 KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5,           KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,

 KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_UP,    KC_LEFT, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC,

 KC_CAPS, KC_A, KC_S, KC_D, KC_F, KC_G, KC_DOWN, KC_RIGHT, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,

 KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_MUTE, RGB_MOD,  KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_DEL,

              KC_LCTL, MO(_FN1), KC_SPC,          KC_ENT, MO(_FN2), KC_RCTL
)

};


// ==================================================
// ENCODER MAP
// ==================================================

#if defined(ENCODER_MAP_ENABLE)

const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {

    [_BASE] = {
        ENCODER_CCW_CW(KC_RIGHT, KC_LEFT),
        ENCODER_CCW_CW(KC_VOLD, KC_VOLU)
    },

    [_FN1] = {
        ENCODER_CCW_CW(RGB_HUD, RGB_HUI),
        ENCODER_CCW_CW(RGB_SAD, RGB_SAI)
    },

    [_FN2] = {
        ENCODER_CCW_CW(RGB_VAD, RGB_VAI),
        ENCODER_CCW_CW(RGB_SPD, RGB_SPI)
    },

    [_FN3] = {
        ENCODER_CCW_CW(RGB_RMOD, RGB_MOD),
        ENCODER_CCW_CW(KC_RIGHT, KC_LEFT)
    }
};

#endif