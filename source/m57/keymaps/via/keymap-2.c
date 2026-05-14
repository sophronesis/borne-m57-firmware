#include QMK_KEYBOARD_H
#include "theme.h"

/*
============================================================
RGB SYSTEM DESIGN OVERVIEW
============================================================

Modes:

- Standard QMK/Vial modes:
    → RGB handled by QMK

- CUSTOM mode ("FIRMWARE UI"):
    → Firmware controls all LEDs

User selects mode in Vial:
    RGB Mode → "FIRMWARE UI"
*/


/*
============================================================
CUSTOM RGB MODE NAME
============================================================
*/

#define RGB_CUSTOM_MODE_NAME "FIRMWARE UI"

#ifdef RGB_MATRIX_CUSTOM_USER
const char* rgb_matrix_name_for_cust[] = {
    RGB_CUSTOM_MODE_NAME
};
#endif


/*
============================================================
VIAL-SAFE KEYMAP ACCESS
============================================================
*/

static inline uint16_t get_keycode_at(uint8_t layer, keypos_t pos) {
    return keymap_key_to_keycode(layer, pos);
}


/*
============================================================
CACHE SYSTEM (VIAL SAFE)
============================================================
*/

static uint16_t led_keycode_cache[DRIVER_LED_TOTAL];

static uint8_t modifier_leds[DRIVER_LED_TOTAL];
static uint8_t modifier_count = 0;

static uint32_t keycode_cache_timer = 0;
#define KEYCODE_CACHE_REFRESH_INTERVAL 100


/*
============================================================
ESC PHASE SYSTEM
============================================================
*/

typedef enum {
    ESC_PHASE_OS,
    ESC_PHASE_PERSIST,
    ESC_PHASE_MODE,
    ESC_PHASE_COUNT
} esc_phase_t;

static const uint16_t esc_phase_duration[ESC_PHASE_COUNT] = {
    900,
    1300,
    700
};

static uint16_t esc_total_duration = 0;


/*
============================================================
PRESET SYSTEM
============================================================
*/

typedef enum {
    PRESET_DEFAULT,
    PRESET_BRIGHT,
    PRESET_NIGHT,
    PRESET_COUNT
} color_preset_t;

static const uint8_t preset_brightness[PRESET_COUNT] = {
    200,
    255,
    120
};

static inline uint8_t get_preset_brightness(uint8_t p) {
    return (p < PRESET_COUNT) ? preset_brightness[p] : preset_brightness[0];
}


/*
============================================================
INDICATOR LEDS (ESC)
============================================================
*/

static const uint8_t indicator_leds[] = {0, 6};
#define INDICATOR_COUNT (sizeof(indicator_leds) / sizeof(indicator_leds[0]))

static void set_indicator_color(rgb_t c, uint8_t brightness) {
    for (uint8_t i = 0; i < INDICATOR_COUNT; i++) {
        apply_color(indicator_leds[i], c, brightness);
    }
}


/*
============================================================
CACHE BUILDERS
============================================================
*/

void build_modifier_list(void) {
    modifier_count = 0;
    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (get_led_flag(i) & LED_FLAG_MODIFIER) {
            modifier_leds[modifier_count++] = i;
        }
    }
}

void update_keycode_cache(void) {

    uint8_t layer = get_highest_layer(layer_state);

    for (uint8_t led = 0; led < DRIVER_LED_TOTAL; led++) {

        uint8_t row = g_led_config.matrix_co[led][0];
        if (row == NO_LED) continue;

        uint8_t col = g_led_config.matrix_co[led][1];

        led_keycode_cache[led] =
            get_keycode_at(layer, (keypos_t){row, col});
    }
}


/*
============================================================
ESC PHASE CALCULATION
============================================================
*/

uint8_t get_esc_phase(uint32_t t, uint16_t *local) {

    t %= esc_total_duration;

    uint8_t phase = 0;

    while (t >= esc_phase_duration[phase]) {
        t -= esc_phase_duration[phase];
        phase++;
    }

    *local = t;
    return phase;
}


/*
============================================================
RENDER PIPELINE (FULL)
============================================================
*/

void render_flag_base(uint8_t brightness) {

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (get_led_flag(i) & LED_FLAG_KEYLIGHT) {
            apply_color(i, COLOR_BASE, brightness);
        }
    }
}

void render_modifier_debug(uint8_t brightness) {

    uint8_t mods = get_mods();

    for (uint8_t i = 0; i < modifier_count; i++) {

        uint8_t led = modifier_leds[i];
        rgb_t c = COLOR_BASE;

        if ((mods & MOD_MASK_SHIFT) && (mods & MOD_MASK_CTRL))
            c = COLOR_MOD_SHIFT_CTRL;
        else if ((mods & MOD_MASK_ALT) && (mods & MOD_MASK_GUI))
            c = COLOR_MOD_ALT_GUI;
        else if (mods & MOD_MASK_SHIFT)
            c = COLOR_MOD_SHIFT;
        else if (mods & MOD_MASK_CTRL)
            c = COLOR_MOD_CTRL;
        else if (mods & MOD_MASK_ALT)
            c = COLOR_MOD_ALT;
        else if (mods & MOD_MASK_GUI)
            c = COLOR_MOD_GUI;

        apply_color(led, c, brightness);
    }
}

void render_esc_indicator(uint8_t brightness) {

    uint16_t local;
    uint8_t phase = get_esc_phase(timer_read32(), &local);

    rgb_t c;

    if (phase == ESC_PHASE_OS)
        c = COLOR_OS_LINUX;
    else if (phase == ESC_PHASE_PERSIST)
        c = COLOR_PERSIST_ON;
    else
        c = COLOR_MODE_STATIC;

    set_indicator_color(c, brightness);
}


/*
============================================================
MAIN RGB PIPELINE
============================================================
*/

bool rgb_matrix_indicators_user(void) {

    // ----------------------------------------------------------
    // MODE GUARD (VIAL SAFE)
    // ----------------------------------------------------------

    if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM) {
        return false;
    }

    // ----------------------------------------------------------
    // CACHE REFRESH
    // ----------------------------------------------------------

    if (timer_elapsed32(keycode_cache_timer) > KEYCODE_CACHE_REFRESH_INTERVAL) {
        update_keycode_cache();
        keycode_cache_timer = timer_read32();
    }

    uint8_t brightness = get_preset_brightness(PRESET_DEFAULT);

    render_flag_base(brightness);
    render_modifier_debug(brightness);
    render_esc_indicator(brightness);

    return false;
}


/*
============================================================
INIT + HOOKS
============================================================
*/

void keyboard_post_init_user(void) {

    esc_total_duration = 0;

    for (uint8_t i = 0; i < ESC_PHASE_COUNT; i++) {
        esc_total_duration += esc_phase_duration[i];
    }

    build_modifier_list();
    update_keycode_cache();
}

layer_state_t layer_state_set_user(layer_state_t state) {
    update_keycode_cache();
    return state;
}