#pragma once
// AUTO-GENERATED from borne_status.toml.
// Do not edit by hand - run scripts/borne_status_gen.py to regenerate.
//
// included once from rgb_matrix_user.inc inside the
// RGB_MATRIX_CUSTOM_EFFECT_IMPLS guard so symbols stay file-scoped.

#include "quantum.h"

static uint8_t borne_breath_v(void) {
    return scale8(
        abs8(sin8(scale16by8(g_rgb_timer, rgb_matrix_config.speed / 8)) - 128) * 2,
        rgb_matrix_config.hsv.v);
}

// chart-encoded modifier blend: shift->R, ctrl->G, alt->B,
// gui bumps base 0x00->0x55 and high 0xAA->0xFF.
// returns RGB(0,0,0) when no mods are held - this is how the spec's
// "home row black unless pressed" baseline is implemented.
static void borne_compute_mod_blend(uint8_t br, bool pulsate,
                                    uint8_t *out_r, uint8_t *out_g, uint8_t *out_b) {
    uint8_t mods = get_mods() | get_weak_mods() | get_oneshot_mods();
    bool sft = mods & MOD_MASK_SHIFT;
    bool alt = mods & MOD_MASK_ALT;
    bool ctl = mods & MOD_MASK_CTRL;
    bool gui = mods & MOD_MASK_GUI;
    uint8_t base = gui ? 0x55 : 0x00;
    uint8_t high = gui ? 0xFF : 0xAA;
    uint8_t r = sft ? high : base;
    uint8_t g = ctl ? high : base;
    uint8_t b = alt ? high : base;
    if (pulsate) { r = scale8(r, br); g = scale8(g, br); b = scale8(b, br); }
    *out_r = r; *out_g = g; *out_b = b;
}

static inline void borne_set(uint8_t led, uint8_t led_min, uint8_t led_max,
                              uint8_t r, uint8_t g, uint8_t b) {
    if (led >= led_min && led < led_max) rgb_matrix_set_color(led, r, g, b);
}

static bool borne_status(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t br = borne_breath_v();
    uint8_t layer = get_highest_layer(layer_state);
    led_t leds = host_keyboard_led_state();
    (void)leds;  // referenced only by gated rules; silence -Wunused if none

    // priority 0: background
    for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, br, br, br);

    switch (layer) {
        case 0:
            // priority 1: home_row_left
            {
                uint8_t mr, mg, mb;
                borne_compute_mod_blend(br, true, &mr, &mg, &mb);
                borne_set(14, led_min, led_max, mr, mg, mb);
                borne_set(15, led_min, led_max, mr, mg, mb);
                borne_set(16, led_min, led_max, mr, mg, mb);
                borne_set(17, led_min, led_max, mr, mg, mb);
                borne_set(18, led_min, led_max, mr, mg, mb);
            }
            // priority 1: home_row_right
            {
                uint8_t mr, mg, mb;
                borne_compute_mod_blend(br, true, &mr, &mg, &mb);
                borne_set(43, led_min, led_max, mr, mg, mb);
                borne_set(44, led_min, led_max, mr, mg, mb);
                borne_set(45, led_min, led_max, mr, mg, mb);
                borne_set(46, led_min, led_max, mr, mg, mb);
                borne_set(47, led_min, led_max, mr, mg, mb);
            }
            // priority 2: num_lock
            if (leds.num_lock) {
                borne_set(41, led_min, led_max, 0, br, 0);
            }
            // priority 2: caps_lock
            if (leds.caps_lock) {
                borne_set(48, led_min, led_max, 0, br, 0);
            }
            // priority 2: scroll_lock
            if (leds.scroll_lock) {
                borne_set(54, led_min, led_max, 0, br, 0);
            }
            // priority 2: to5_indicator
            borne_set(6, led_min, led_max, 0, br, 0);
            // priority 2: l0_yellow_specials
            borne_set(13, led_min, led_max, br, br, 0);
            borne_set(20, led_min, led_max, br, br, 0);
            borne_set(27, led_min, led_max, br, br, 0);
            borne_set(56, led_min, led_max, br, br, 0);
            break;
        case 1:
            // priority 1: home_row_left
            {
                uint8_t mr, mg, mb;
                borne_compute_mod_blend(br, true, &mr, &mg, &mb);
                borne_set(14, led_min, led_max, mr, mg, mb);
                borne_set(15, led_min, led_max, mr, mg, mb);
                borne_set(16, led_min, led_max, mr, mg, mb);
                borne_set(17, led_min, led_max, mr, mg, mb);
                borne_set(18, led_min, led_max, mr, mg, mb);
            }
            // priority 1: home_row_right
            {
                uint8_t mr, mg, mb;
                borne_compute_mod_blend(br, true, &mr, &mg, &mb);
                borne_set(43, led_min, led_max, mr, mg, mb);
                borne_set(44, led_min, led_max, mr, mg, mb);
                borne_set(45, led_min, led_max, mr, mg, mb);
                borne_set(46, led_min, led_max, mr, mg, mb);
                borne_set(47, led_min, led_max, mr, mg, mb);
            }
            // priority 2: num_lock
            if (leds.num_lock) {
                borne_set(41, led_min, led_max, 0, br, 0);
            }
            // priority 2: caps_lock
            if (leds.caps_lock) {
                borne_set(48, led_min, led_max, 0, br, 0);
            }
            // priority 2: scroll_lock
            if (leds.scroll_lock) {
                borne_set(54, led_min, led_max, 0, br, 0);
            }
            // priority 2: to5_indicator
            borne_set(6, led_min, led_max, 0, br, 0);
            // priority 2: l1_bootloader
            borne_set(0, led_min, led_max, 255, 0, 0);
            // priority 2: l1_fkeys
            borne_set(20, led_min, led_max, br, br, 0);
            borne_set(21, led_min, led_max, br, br, 0);
            borne_set(22, led_min, led_max, br, br, 0);
            borne_set(23, led_min, led_max, br, br, 0);
            borne_set(24, led_min, led_max, br, br, 0);
            borne_set(25, led_min, led_max, br, br, 0);
            borne_set(49, led_min, led_max, br, br, 0);
            borne_set(50, led_min, led_max, br, br, 0);
            borne_set(51, led_min, led_max, br, br, 0);
            borne_set(52, led_min, led_max, br, br, 0);
            borne_set(53, led_min, led_max, br, br, 0);
            borne_set(54, led_min, led_max, br, br, 0);
            // priority 2: l1_minus_equal
            borne_set(55, led_min, led_max, 0, 0, br);
            borne_set(56, led_min, led_max, 0, 0, br);
            break;
        case 2:
            // priority 1: home_row_left
            {
                uint8_t mr, mg, mb;
                borne_compute_mod_blend(br, true, &mr, &mg, &mb);
                borne_set(14, led_min, led_max, mr, mg, mb);
                borne_set(15, led_min, led_max, mr, mg, mb);
                borne_set(16, led_min, led_max, mr, mg, mb);
                borne_set(17, led_min, led_max, mr, mg, mb);
                borne_set(18, led_min, led_max, mr, mg, mb);
            }
            // priority 2: num_lock
            if (leds.num_lock) {
                borne_set(41, led_min, led_max, 0, br, 0);
            }
            // priority 2: caps_lock
            if (leds.caps_lock) {
                borne_set(48, led_min, led_max, 0, br, 0);
            }
            // priority 2: scroll_lock
            if (leds.scroll_lock) {
                borne_set(54, led_min, led_max, 0, br, 0);
            }
            // priority 2: arrows
            borne_set(38, led_min, led_max, br, br, 0);
            borne_set(44, led_min, led_max, br, br, 0);
            borne_set(45, led_min, led_max, br, br, 0);
            borne_set(46, led_min, led_max, br, br, 0);
            break;
        case 3:
            // priority 1: home_row_right
            {
                uint8_t mr, mg, mb;
                borne_compute_mod_blend(br, true, &mr, &mg, &mb);
                borne_set(43, led_min, led_max, mr, mg, mb);
                borne_set(44, led_min, led_max, mr, mg, mb);
                borne_set(45, led_min, led_max, mr, mg, mb);
                borne_set(46, led_min, led_max, mr, mg, mb);
                borne_set(47, led_min, led_max, mr, mg, mb);
            }
            // priority 1: l3_lock_positions
            borne_set(41, led_min, led_max, br, br, 0);
            borne_set(48, led_min, led_max, br, br, 0);
            borne_set(54, led_min, led_max, br, br, 0);
            // priority 2: num_lock
            if (leds.num_lock) {
                borne_set(41, led_min, led_max, 0, br, 0);
            }
            // priority 2: caps_lock
            if (leds.caps_lock) {
                borne_set(48, led_min, led_max, 0, br, 0);
            }
            // priority 2: scroll_lock
            if (leds.scroll_lock) {
                borne_set(54, led_min, led_max, 0, br, 0);
            }
            // priority 2: l3_bootloader
            borne_set(34, led_min, led_max, 255, 0, 0);
            // priority 2: l3_mouse_arrows
            borne_set(9, led_min, led_max, br, br, 0);
            borne_set(15, led_min, led_max, br, br, 0);
            borne_set(16, led_min, led_max, br, br, 0);
            borne_set(17, led_min, led_max, br, br, 0);
            // priority 2: l3_mouse_wheel
            borne_set(11, led_min, led_max, 0, br, 0);
            borne_set(18, led_min, led_max, 0, br, 0);
            borne_set(24, led_min, led_max, 0, br, 0);
            borne_set(25, led_min, led_max, 0, br, 0);
            // priority 2: l3_mouse_buttons
            borne_set(26, led_min, led_max, 0, 0, br);
            borne_set(27, led_min, led_max, 0, 0, br);
            borne_set(28, led_min, led_max, 0, 0, br);
            break;
        case 4:
            // priority 1: home_row_right
            {
                uint8_t mr, mg, mb;
                borne_compute_mod_blend(br, true, &mr, &mg, &mb);
                borne_set(43, led_min, led_max, mr, mg, mb);
                borne_set(44, led_min, led_max, mr, mg, mb);
                borne_set(45, led_min, led_max, mr, mg, mb);
                borne_set(46, led_min, led_max, mr, mg, mb);
                borne_set(47, led_min, led_max, mr, mg, mb);
            }
            // priority 2: num_lock
            if (leds.num_lock) {
                borne_set(41, led_min, led_max, 0, br, 0);
            }
            // priority 2: caps_lock
            if (leds.caps_lock) {
                borne_set(48, led_min, led_max, 0, br, 0);
            }
            // priority 2: scroll_lock
            if (leds.scroll_lock) {
                borne_set(54, led_min, led_max, 0, br, 0);
            }
            // priority 2: l4_numpad_digits
            borne_set(8, led_min, led_max, 0, br, 0);
            borne_set(9, led_min, led_max, 0, br, 0);
            borne_set(10, led_min, led_max, 0, br, 0);
            borne_set(15, led_min, led_max, 0, br, 0);
            borne_set(16, led_min, led_max, 0, br, 0);
            borne_set(17, led_min, led_max, 0, br, 0);
            borne_set(21, led_min, led_max, 0, br, 0);
            borne_set(22, led_min, led_max, 0, br, 0);
            borne_set(23, led_min, led_max, 0, br, 0);
            borne_set(24, led_min, led_max, 0, br, 0);
            // priority 2: l4_numpad_other
            borne_set(7, led_min, led_max, 0, 0, br);
            borne_set(11, led_min, led_max, 0, 0, br);
            borne_set(12, led_min, led_max, 0, 0, br);
            borne_set(14, led_min, led_max, 0, 0, br);
            borne_set(18, led_min, led_max, 0, 0, br);
            borne_set(19, led_min, led_max, 0, 0, br);
            borne_set(25, led_min, led_max, 0, 0, br);
            break;
        case 5:
            // priority 2: num_lock
            if (leds.num_lock) {
                borne_set(41, led_min, led_max, 0, br, 0);
            }
            // priority 2: caps_lock
            if (leds.caps_lock) {
                borne_set(48, led_min, led_max, 0, br, 0);
            }
            // priority 2: scroll_lock
            if (leds.scroll_lock) {
                borne_set(54, led_min, led_max, 0, br, 0);
            }
            // priority 2: to5_indicator
            borne_set(6, led_min, led_max, 0, br, 0);
            // priority 2: thumb_shift
            borne_set(27, led_min, led_max, 255, 0, 0);
            // priority 2: thumb_ctrl
            borne_set(26, led_min, led_max, 0, 255, 0);
            // priority 2: thumb_alt
            borne_set(56, led_min, led_max, 0, 0, 255);
            // priority 2: thumb_gui
            borne_set(57, led_min, led_max, 255, 255, 255);
            break;
        case 6:
            // priority 2: num_lock
            if (leds.num_lock) {
                borne_set(41, led_min, led_max, 0, br, 0);
            }
            // priority 2: caps_lock
            if (leds.caps_lock) {
                borne_set(48, led_min, led_max, 0, br, 0);
            }
            // priority 2: scroll_lock
            if (leds.scroll_lock) {
                borne_set(54, led_min, led_max, 0, br, 0);
            }
            // priority 2: to5_indicator
            borne_set(6, led_min, led_max, 0, br, 0);
            // priority 2: arrows
            borne_set(38, led_min, led_max, br, br, 0);
            borne_set(44, led_min, led_max, br, br, 0);
            borne_set(45, led_min, led_max, br, br, 0);
            borne_set(46, led_min, led_max, br, br, 0);
            // priority 2: thumb_shift
            borne_set(27, led_min, led_max, 255, 0, 0);
            // priority 2: thumb_ctrl
            borne_set(26, led_min, led_max, 0, 255, 0);
            // priority 2: thumb_alt
            borne_set(56, led_min, led_max, 0, 0, 255);
            // priority 2: thumb_gui
            borne_set(57, led_min, led_max, 255, 255, 255);
            break;
        default:
            // priority 2: num_lock
            if (leds.num_lock) {
                borne_set(41, led_min, led_max, 0, br, 0);
            }
            // priority 2: caps_lock
            if (leds.caps_lock) {
                borne_set(48, led_min, led_max, 0, br, 0);
            }
            // priority 2: scroll_lock
            if (leds.scroll_lock) {
                borne_set(54, led_min, led_max, 0, br, 0);
            }
            break;
    }

    return rgb_matrix_check_finished_leds(led_max);
}
