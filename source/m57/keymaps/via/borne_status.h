#pragma once
// borne_status: per-layer key-aware rgb matrix effect.
// included once from rgb_matrix_user.inc inside the
// RGB_MATRIX_CUSTOM_EFFECT_IMPLS guard so symbols stay file-scoped.
//
// rgb_matrix.c (the host TU for the inc) pulls in rgb_matrix.h + lib8tion
// only, so quantum.h is required here for KC_*, MO(), keymap_key_to_keycode,
// get_mods, host_keyboard_led_state, led_t, and MOD_MASK_*.
//
// shared baseline (every layer): breathing white background.
//
// per-layer overlays:
//   layer 0  - tab (LT2(TAB)) + grave (LT4(GRAVE)) yellow
//              TO(5) green
//              MO(1)/MO(3) yellow
//              home row (A S D F G + H J K L ;) gets chart-encoded mod color
//                blend when shift/alt/ctrl/gui held.
//                channel mapping: shift=R, ctrl=G, alt=B, gui=intensity
//                                 (0x00->0x55 base, 0xAA->0xFF high).
//              caps/num/scroll keys (right edge of right half, layer-3
//                positions) light green when their host lock state is on.
//   layer 1  - F1-F12 yellow
//              QK_BOOT (top-left) static red, no breathing
//              KC_MINUS / KC_EQUAL blue
//   layer 2  - arrow keys yellow
//   layer 3  - mouse buttons blue
//              mouse cursor movement yellow
//              mouse wheel green
//              QK_BOOT (top-row, right half col 6) static red
//   layer 4  - keypad digits 0-9 green; other keypad keys yellow
//   layer 5  - whole bottom (thumb) row yellow
//   layer 6+ - breathing white only.

#include "quantum.h"

// home-row LEDs that take the modifier color blend on layer 0.
// matches A S D F G on the left and H J K L ; on the right.
static const uint8_t BORNE_HOMEROW_LEDS[10] = { 14, 15, 16, 17, 18, 43, 44, 45, 46, 47 };

// thumb-row LEDs (left thumbs 26 27 28, right thumbs 55 56 57).
static const uint8_t BORNE_THUMB_LEDS[6] = { 26, 27, 28, 55, 56, 57 };

// fixed LED positions referenced by name.
#define BORNE_LED_TO5         6   // (1,0): TO(5)         on layer 0
#define BORNE_LED_TAB        13   // (2,0): LT2(TAB)      on layer 0
#define BORNE_LED_PGUP       12   // (1,6): KC_PGUP       on layer 0
#define BORNE_LED_PGDN       19   // (2,6): KC_PGDOWN     on layer 0
#define BORNE_LED_GRAVE      20   // (3,0): LT4(GRAVE)    on layer 0
#define BORNE_LED_BOOT_L1     0   // (0,0): QK_BOOT       on layer 1
#define BORNE_LED_MINUS      55   // (9,1): KC_MINUS      on layer 1
#define BORNE_LED_EQUAL      56   // (9,2): KC_EQUAL      on layer 1
#define BORNE_LED_BOOT_L3    34   // (5,6): QK_BOOT       on layer 3
#define BORNE_LED_NUM_LOCK   41   // (6,6): KC_NUMLOCK    on layer 3
#define BORNE_LED_CAPS_LOCK  48   // (7,6): KC_CAPSLOCK   on layer 3
#define BORNE_LED_SCRL_LOCK  54   // (8,6): KC_SCROLLLOCK on layer 3

typedef bool (*borne_kc_pred_t)(uint16_t);

static uint8_t borne_breath_v(void) {
    return scale8(
        abs8(sin8(scale16by8(g_rgb_timer, rgb_matrix_config.speed / 8)) - 128) * 2,
        rgb_matrix_config.hsv.v);
}

static bool borne_kc_fkey(uint16_t c)       { return c >= KC_F1 && c <= KC_F12; }
static bool borne_kc_arrow(uint16_t c)      { return c == KC_LEFT || c == KC_RIGHT || c == KC_UP || c == KC_DOWN; }
static bool borne_kc_ms_btn(uint16_t c)     { return c >= QK_MOUSE_BUTTON_1 && c <= QK_MOUSE_BUTTON_8; }
static bool borne_kc_ms_mov(uint16_t c)     { return c >= QK_MOUSE_CURSOR_UP && c <= QK_MOUSE_CURSOR_RIGHT; }
static bool borne_kc_ms_wheel(uint16_t c)   { return c >= QK_MOUSE_WHEEL_UP && c <= QK_MOUSE_WHEEL_RIGHT; }
static bool borne_kc_kp_digit(uint16_t c)   { return c >= KC_KP_1 && c <= KC_KP_0; }
static bool borne_kc_kp_other(uint16_t c)   {
    return c == KC_KP_SLASH || c == KC_KP_ASTERISK || c == KC_KP_MINUS ||
           c == KC_KP_PLUS  || c == KC_KP_ENTER    || c == KC_KP_DOT   ||
           c == KC_KP_EQUAL || c == KC_KP_COMMA;
}
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

static void borne_set_if_in_range(uint8_t led, uint8_t led_min, uint8_t led_max,
                                  uint8_t r, uint8_t g, uint8_t b) {
    if (led >= led_min && led < led_max) rgb_matrix_set_color(led, r, g, b);
}

static void borne_layer_0(uint8_t led_min, uint8_t led_max, uint8_t br) {
    borne_set_if_in_range(BORNE_LED_TAB,   led_min, led_max, br, br, 0);
    borne_set_if_in_range(BORNE_LED_GRAVE, led_min, led_max, br, br, 0);
    borne_set_if_in_range(BORNE_LED_TO5,   led_min, led_max, 0,  br, 0);

    borne_paint_match(0, led_min, led_max, borne_kc_mo_1_3, br, br, 0);

    uint8_t mods = get_mods() | get_weak_mods() | get_oneshot_mods();
    if (mods) {
        bool sft = mods & MOD_MASK_SHIFT;
        bool alt = mods & MOD_MASK_ALT;
        bool ctl = mods & MOD_MASK_CTRL;
        bool gui = mods & MOD_MASK_GUI;
        uint8_t base = gui ? 0x55 : 0x00;
        uint8_t high = gui ? 0xFF : 0xAA;
        // shift -> R, ctrl -> G, alt -> B
        uint8_t r = sft ? high : base;
        uint8_t g = ctl ? high : base;
        uint8_t b = alt ? high : base;
        for (uint8_t k = 0; k < 10; k++) {
            uint8_t led = BORNE_HOMEROW_LEDS[k];
            if (led >= led_min && led < led_max) rgb_matrix_set_color(led, r, g, b);
        }
    }

    led_t leds = host_keyboard_led_state();
    if (leds.num_lock)    borne_set_if_in_range(BORNE_LED_NUM_LOCK,  led_min, led_max, 0, br, 0);
    if (leds.caps_lock)   borne_set_if_in_range(BORNE_LED_CAPS_LOCK, led_min, led_max, 0, br, 0);
    if (leds.scroll_lock) borne_set_if_in_range(BORNE_LED_SCRL_LOCK, led_min, led_max, 0, br, 0);
}

static void borne_layer_1(uint8_t led_min, uint8_t led_max, uint8_t br) {
    borne_paint_match(1, led_min, led_max, borne_kc_fkey, br, br, 0);
    borne_set_if_in_range(BORNE_LED_BOOT_L1, led_min, led_max, 255, 0, 0);
    borne_set_if_in_range(BORNE_LED_MINUS,   led_min, led_max, 0,   0, br);
    borne_set_if_in_range(BORNE_LED_EQUAL,   led_min, led_max, 0,   0, br);
}

static void borne_layer_3(uint8_t led_min, uint8_t led_max, uint8_t br) {
    borne_paint_match(3, led_min, led_max, borne_kc_ms_btn,   0,  0,  br);
    borne_paint_match(3, led_min, led_max, borne_kc_ms_mov,   br, br, 0);
    borne_paint_match(3, led_min, led_max, borne_kc_ms_wheel, 0,  br, 0);
    borne_set_if_in_range(BORNE_LED_BOOT_L3, led_min, led_max, 255, 0, 0);
}

static void borne_layer_4(uint8_t led_min, uint8_t led_max, uint8_t br) {
    borne_paint_match(4, led_min, led_max, borne_kc_kp_other, br, br, 0);
    borne_paint_match(4, led_min, led_max, borne_kc_kp_digit, 0,  br, 0);
}

static void borne_layer_5(uint8_t led_min, uint8_t led_max, uint8_t br) {
    for (uint8_t k = 0; k < 6; k++) {
        uint8_t led = BORNE_THUMB_LEDS[k];
        if (led >= led_min && led < led_max) rgb_matrix_set_color(led, br, br, 0);
    }
}

static bool borne_status(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t br    = borne_breath_v();
    uint8_t layer = get_highest_layer(layer_state);

    // breathing white background for every layer.
    for (uint8_t i = led_min; i < led_max; i++) rgb_matrix_set_color(i, br, br, br);

    switch (layer) {
        case 0: borne_layer_0(led_min, led_max, br); break;
        case 1: borne_layer_1(led_min, led_max, br); break;
        case 2:
            borne_paint_match(2, led_min, led_max, borne_kc_arrow, br, br, 0);
            // pgup/pgdn positions stay dark; their layer-0 keys would otherwise
            // breathe white through the background.
            borne_set_if_in_range(BORNE_LED_PGUP, led_min, led_max, 0, 0, 0);
            borne_set_if_in_range(BORNE_LED_PGDN, led_min, led_max, 0, 0, 0);
            break;
        case 3: borne_layer_3(led_min, led_max, br); break;
        case 4: borne_layer_4(led_min, led_max, br); break;
        case 5: borne_layer_5(led_min, led_max, br); break;
        default: break;
    }

    return rgb_matrix_check_finished_leds(led_max);
}
