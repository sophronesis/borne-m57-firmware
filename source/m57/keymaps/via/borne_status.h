#pragma once
// borne_status: per-layer key-aware rgb matrix effect.
// included once from rgb_matrix_user.inc inside the
// RGB_MATRIX_CUSTOM_EFFECT_IMPLS guard so symbols stay file-scoped.
//
// rgb_matrix.c (the host TU for the inc) pulls in rgb_matrix.h + lib8tion
// only, so quantum.h is required here for KC_*, MO(), keymap_key_to_keycode,
// get_mods, host_keyboard_led_state, led_t, and MOD_MASK_*.

#include "quantum.h"
//
// per-layer behavior:
//   layer 0  - breathing white background
//              + home row letters take chart-encoded mod color when any of
//                shift/alt/ctrl/gui is held (shift=R, alt=G, ctrl=B,
//                gui = shift base 0x00 -> 0x55 and high 0xAA -> 0xFF)
//              + MO(1) and MO(3) keys yellow-breathe
//              + caps/num/scroll lock keys yellow-breathe when their host
//                lock state is on
//   layer 1  - F1-F12 yellow-breathe
//   layer 2  - arrow keys yellow-breathe
//   layer 3  - mouse buttons blue-breathe, mouse movement keys yellow-breathe
//   layer 4  - top-row digits and KP_0..KP_9 yellow-breathe
//   layer 5  - LCTL / LSFT / RALT / RGUI keys yellow-breathe
//   layer 6+ - lights off

static const uint8_t BORNE_HOMEROW_LEDS[8] = { 14, 15, 16, 17, 44, 45, 46, 47 };

typedef bool (*borne_kc_pred_t)(uint16_t);

static uint8_t borne_breath_v(void) {
    return scale8(
        abs8(sin8(scale16by8(g_rgb_timer, rgb_matrix_config.speed / 8)) - 128) * 2,
        rgb_matrix_config.hsv.v);
}

static bool borne_kc_fkey(uint16_t c)       { return c >= KC_F1     && c <= KC_F12;     }
static bool borne_kc_arrow(uint16_t c)      { return c == KC_LEFT || c == KC_RIGHT || c == KC_UP || c == KC_DOWN; }
static bool borne_kc_ms_btn(uint16_t c)     { return c >= QK_MOUSE_BUTTON_1 && c <= QK_MOUSE_BUTTON_8; }
static bool borne_kc_ms_mov(uint16_t c)     { return c >= QK_MOUSE_CURSOR_UP && c <= QK_MOUSE_CURSOR_RIGHT; }
static bool borne_kc_digit(uint16_t c)      { return (c >= KC_1 && c <= KC_0) || (c >= KC_KP_1 && c <= KC_KP_0); }
static bool borne_kc_target_mod(uint16_t c) { return c == KC_LCTL || c == KC_LSFT || c == KC_RALT || c == KC_RGUI; }
static bool borne_kc_mo_1_3(uint16_t c)     { return c == MO(1) || c == MO(3); }

static void borne_paint_match(uint8_t layer, uint8_t led_min, uint8_t led_max,
                              borne_kc_pred_t pred,
                              uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led == NO_LED || led < led_min || led >= led_max) continue;
            uint16_t code = keymap_key_to_keycode(layer, (keypos_t){.col = col, .row = row});
            if (pred(code)) rgb_matrix_set_color(led, r, g, b);
        }
    }
}

static void borne_fill_off(uint8_t led_min, uint8_t led_max) {
    for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, 0, 0, 0);
}

static void borne_layer_0(uint8_t led_min, uint8_t led_max, uint8_t br) {
    for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, br, br, br);

    borne_paint_match(0, led_min, led_max, borne_kc_mo_1_3, br, br, 0);

    uint8_t mods = get_mods() | get_weak_mods() | get_oneshot_mods();
    if (mods) {
        bool sft = mods & MOD_MASK_SHIFT;
        bool alt = mods & MOD_MASK_ALT;
        bool ctl = mods & MOD_MASK_CTRL;
        bool gui = mods & MOD_MASK_GUI;
        uint8_t base = gui ? 0x55 : 0x00;
        uint8_t high = gui ? 0xFF : 0xAA;
        uint8_t r = sft ? high : base;
        uint8_t g = alt ? high : base;
        uint8_t b = ctl ? high : base;
        for (uint8_t k = 0; k < 8; k++) {
            uint8_t led = BORNE_HOMEROW_LEDS[k];
            if (led >= led_min && led < led_max) rgb_matrix_set_color(led, r, g, b);
        }
    }

    led_t leds = host_keyboard_led_state();
    if (!(leds.caps_lock || leds.num_lock || leds.scroll_lock)) return;
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led == NO_LED || led < led_min || led >= led_max) continue;
            uint16_t c = keymap_key_to_keycode(0, (keypos_t){.col = col, .row = row});
            if ((c == KC_CAPS        && leds.caps_lock)   ||
                (c == KC_NUM_LOCK    && leds.num_lock)    ||
                (c == KC_SCROLL_LOCK && leds.scroll_lock)) {
                rgb_matrix_set_color(led, br, br, 0);
            }
        }
    }
}

static bool borne_status(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t br    = borne_breath_v();
    uint8_t layer = get_highest_layer(layer_state);

    switch (layer) {
        case 0:
            borne_layer_0(led_min, led_max, br);
            break;
        case 1:
            borne_fill_off(led_min, led_max);
            borne_paint_match(1, led_min, led_max, borne_kc_fkey, br, br, 0);
            break;
        case 2:
            borne_fill_off(led_min, led_max);
            borne_paint_match(2, led_min, led_max, borne_kc_arrow, br, br, 0);
            break;
        case 3:
            borne_fill_off(led_min, led_max);
            borne_paint_match(3, led_min, led_max, borne_kc_ms_btn, 0, 0, br);
            borne_paint_match(3, led_min, led_max, borne_kc_ms_mov, br, br, 0);
            break;
        case 4:
            borne_fill_off(led_min, led_max);
            borne_paint_match(4, led_min, led_max, borne_kc_digit, br, br, 0);
            break;
        case 5:
            borne_fill_off(led_min, led_max);
            borne_paint_match(5, led_min, led_max, borne_kc_target_mod, br, br, 0);
            break;
        default:
            borne_fill_off(led_min, led_max);
            break;
    }

    return rgb_matrix_check_finished_leds(led_max);
}
