#include QMK_KEYBOARD_H
#include "theme.h"


// ============================================================
// CUSTOM MODE SYSTEM DESIGN
// ============================================================

/*
============================================================
CUSTOM MODE SYSTEM DESIGN
============================================================

Defines independent RGB behavior modes selectable in Vial.

Modes:
- UI: state-based visualization (layers, indicators)
- DEBUG: diagnostic / visualization overlays
- GAMING: reactive, event-driven effects

Modes control which rendering layers are active.
*/

#define RGB_MODE_UI      "FIRMWARE UI"
#define RGB_MODE_DEBUG   "DEBUG MODE"
#define RGB_MODE_GAMING  "GAMING MODE"

#ifdef RGB_MATRIX_CUSTOM_USER
const char* rgb_matrix_name_for_cust[] = {
    RGB_MODE_UI,
    RGB_MODE_DEBUG,
    RGB_MODE_GAMING
};
#endif


typedef enum {
    CUSTOM_MODE_UI,
    CUSTOM_MODE_DEBUG,
    CUSTOM_MODE_GAMING,
    CUSTOM_MODE_COUNT
} custom_mode_t;

static uint8_t current_custom_mode = CUSTOM_MODE_UI;


// ============================================================
// OS STATE SYSTEM (NEW)
// ============================================================

/*
============================================================
OS STATE SYSTEM
============================================================

Defines active OS for indicator rendering.

Current implementation:
- Static variable (can be expanded)

Future extensions:
- Change via keybinding
- Detect via host
- Store in EEPROM

Supported OS:
- Linux
- Windows
- macOS
- Android
*/

typedef enum {
    OS_LINUX,
    OS_WINDOWS,
    OS_MAC,
    OS_ANDROID
} os_type_t;

static os_type_t current_os = OS_WINDOWS; // default


// ============================================================
// LED MAPPING MODEL (MATRIX ↔ LED SPACE)
// ============================================================

/*
============================================================
LED MAPPING MODEL (MATRIX ↔ LED SPACE)
============================================================

Two domains:

- Matrix (row, col)
- LED index

NOT 1:1 mapping.

Encoder keys have NO_LED mapping and require fallback.
*/


// ============================================================
// KEYCODE CACHE SYSTEM
// ============================================================

/*
============================================================
KEYCODE CACHE SYSTEM
============================================================

Caches LED → keycode mapping.

Refresh interval tuning:

- 50 ms → high responsiveness
- 100 ms → balanced (default)
- 150–200 ms → lower CPU usage

Ensures compatibility with:
- dynamic keymaps (Vial)
- layer changes
*/

static uint16_t led_keycode_cache[DRIVER_LED_TOTAL];

static uint8_t modifier_leds[DRIVER_LED_TOTAL];
static uint8_t modifier_count = 0;

static uint32_t keycode_cache_timer = 0;

#define KEYCODE_CACHE_REFRESH_INTERVAL 100


static inline uint8_t get_preset_brightness(uint8_t p) {
    const uint8_t presets[] = {200, 255, 120};
    return presets[p < 3 ? p : 0];
}

static void build_modifier_list(void) {
    modifier_count = 0;

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (get_led_flag(i) & LED_FLAG_MODIFIER) {
            modifier_leds[modifier_count++] = i;
        }
    }
}


// ============================================================
// REACTIVE SYSTEM DESIGN
// ============================================================

#define MAX_REACTIVE 8

typedef struct {
    uint8_t leds[4];
    uint8_t count;
    uint16_t timer;
} reactive_cluster_t;

static reactive_cluster_t reactive_clusters[MAX_REACTIVE];
static uint8_t reactive_index = 0;


// ============================================================
// REACTIVE EVENT ENTRY
// ============================================================

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!record->event.pressed)
        return true;

    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;

    for (uint8_t led = 0; led < DRIVER_LED_TOTAL; led++) {
        if (g_led_config.matrix_co[led][0] == row &&
            g_led_config.matrix_co[led][1] == col) {

            reactive_cluster_t *c = &reactive_clusters[reactive_index];
            c->count = 1;
            c->timer = timer_read();
            c->leds[0] = led;

            reactive_index = (reactive_index + 1) % MAX_REACTIVE;
            break;
        }
    }

    return true;
}


// ============================================================
// KEY-BASED RENDER LAYER
// ============================================================

/*
Uses cache to color keys based on keycode.
*/

static void render_key_based(uint8_t brightness) {
    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (led_keycode_cache[i] == KC_ENT) {
            apply_color(i, COLOR_MOD_SHIFT, brightness);
        }
    }
}


// ============================================================
// KEY GROUPS LAYER (REFINED)
// ============================================================

/*
Groups:
- letters
- numbers
- function keys
*/

static void render_key_groups(uint8_t brightness) {

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {

        uint16_t kc = led_keycode_cache[i];

        if (kc >= KC_A && kc <= KC_Z)
            apply_color(i, COLOR_MOD_ALT, brightness);

        else if (kc >= KC_1 && kc <= KC_0)
            apply_color(i, COLOR_MOD_CTRL, brightness);

        else if (kc >= KC_F1 && kc <= KC_F12)
            apply_color(i, COLOR_MOD_GUI, brightness);
    }
}


// ============================================================
// MODIFIER HIGHLIGHTING
// ============================================================

static void render_modifier_highlighting(uint8_t brightness) {

    uint8_t mods = get_mods();

    for (uint8_t i = 0; i < modifier_count; i++) {

        uint8_t led = modifier_leds[i];

        if (mods & MOD_MASK_SHIFT)
            apply_color(led, COLOR_MOD_SHIFT, brightness);

        if (mods & MOD_MASK_CTRL)
            apply_color(led, COLOR_MOD_CTRL, brightness);

        if (mods & MOD_MASK_ALT)
            apply_color(led, COLOR_MOD_ALT, brightness);

        if (mods & MOD_MASK_GUI)
            apply_color(led, COLOR_MOD_GUI, brightness);
    }
}


// ============================================================
// OS INDICATOR LAYER (DYNAMIC)
// ============================================================

/*
Displays active OS.

Current implementation:
- uses single LED (index 0)
- extendable to zones later
*/

static void render_os_indicator(uint8_t brightness) {

    rgb_t color = COLOR_OS_WINDOWS;

    switch (current_os) {
        case OS_LINUX:   color = COLOR_OS_LINUX; break;
        case OS_WINDOWS: color = COLOR_OS_WINDOWS; break;
        case OS_MAC:     color = COLOR_OS_MAC; break;
        case OS_ANDROID: color = COLOR_OS_ANDROID; break;
    }

    apply_color(0, color, brightness);
}


// ============================================================
// DEBUG VISUALIZATION LAYER
// ============================================================

static void render_debug_overlay(uint8_t brightness) {

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {

        if (led_keycode_cache[i] == KC_NO) {
            apply_color(i, COLOR_WHITE, brightness);
        }
    }
}


// ============================================================
// BASE LAYER
// ============================================================

static void render_flag_base_with_mode(uint8_t brightness) {

    const rgb_theme_t* theme;

    switch (current_custom_mode) {
        case CUSTOM_MODE_UI:     theme = &theme_ui; break;
        case CUSTOM_MODE_DEBUG:  theme = &theme_debug; break;
        case CUSTOM_MODE_GAMING: theme = &theme_gaming; break;
        default: theme = &theme_ui;
    }

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (get_led_flag(i) & LED_FLAG_KEYLIGHT) {
            apply_color(i, theme->base, brightness);
        }
    }
}


// ============================================================
// CONTEXT LAYER
// ============================================================

static void render_context(uint8_t brightness) {

    const rgb_theme_t* theme = &theme_ui;

    uint8_t layer = get_highest_layer(layer_state);

    if (layer == 1)
        theme = &theme_ui;
    else if (layer == 2)
        theme = &theme_debug;

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        apply_color(i, theme->layer1, brightness);
    }
}


// ============================================================
// EVENT LAYER
// ============================================================

static void render_event(uint8_t brightness) {

    const rgb_theme_t* theme = &theme_gaming;

    for (uint8_t i = 0; i < MAX_REACTIVE; i++) {

        uint16_t t = timer_elapsed(reactive_clusters[i].timer);
        if (t > 300) continue;

        uint8_t intensity = 255 - (t * 255 / 300);

        for (uint8_t j = 0; j < reactive_clusters[i].count; j++) {
            apply_color(
                reactive_clusters[i].leds[j],
                (rgb_t){
                    (theme->reactive.r * intensity) / 255,
                    (theme->reactive.g * intensity) / 255,
                    (theme->reactive.b * intensity) / 255
                },
                brightness
            );
        }
    }
}


// ============================================================
// MAIN RGB PIPELINE
// ============================================================

bool rgb_matrix_indicators_user(void) {

    if (timer_elapsed32(keycode_cache_timer) > KEYCODE_CACHE_REFRESH_INTERVAL) {
        update_keycode_cache();
        keycode_cache_timer = timer_read32();
    }

    uint8_t brightness = get_preset_brightness(0);

    render_flag_base_with_mode(brightness);
    render_key_based(brightness);
    render_key_groups(brightness);
    render_modifier_highlighting(brightness);
    render_os_indicator(brightness);

    if (current_custom_mode == CUSTOM_MODE_UI)
        render_context(brightness);

    if (current_custom_mode == CUSTOM_MODE_GAMING)
        render_event(brightness);

    if (current_custom_mode == CUSTOM_MODE_DEBUG)
        render_debug_overlay(brightness);

    return false;
}


// ============================================================
// CACHE + INIT
// ============================================================

static void update_keycode_cache(void) {

    uint8_t layer = get_highest_layer(layer_state);

    for (uint8_t led = 0; led < DRIVER_LED_TOTAL; led++) {

        uint8_t row = g_led_config.matrix_co[led][0];
        if (row == NO_LED) continue;

        uint8_t col = g_led_config.matrix_co[led][1];

        led_keycode_cache[led] =
            keymap_key_to_keycode(layer, (keypos_t){row, col});
    }
}


void keyboard_post_init_user(void) {
    build_modifier_list();
    update_keycode_cache();
}

layer_state_t layer_state_set_user(layer_state_t state) {
    update_keycode_cache();
    return state;
}