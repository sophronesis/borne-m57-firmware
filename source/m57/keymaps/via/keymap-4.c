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
- DEBUG: simplified diagnostic view
- GAMING: reactive, event-driven effects

Design rules:
- Modes must remain independent
- Expensive features belong in GAMING
- Avoid cross-mode interference
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
// LED MAPPING MODEL (MATRIX ↔ LED SPACE)
// ============================================================

/*
============================================================
LED MAPPING MODEL (MATRIX ↔ LED SPACE)
============================================================

Matrix space and LED space are NOT identical.

- Some matrix keys have NO_LED
- Example:
    L36 (row 3, col 6) → encoder press
    R80 (row 8, col 0) → encoder press

These keys require fallback mapping to nearby LEDs.

Rule:
- NEVER assume all keys have LEDs
- ALWAYS resolve mapping or fallback
*/


// ============================================================
// KEYCODE CACHE SYSTEM
// ============================================================

/*
============================================================
KEYCODE CACHE SYSTEM
============================================================

Caches LED → keycode mapping.

Purpose:
- avoid repeated expensive lookups
- maintain Vial compatibility

Updated:
- on layer change
- periodically via timer

Improves:
✔ performance
✔ consistency
*/

static uint16_t led_keycode_cache[DRIVER_LED_TOTAL];

static uint8_t modifier_leds[DRIVER_LED_TOTAL];
static uint8_t modifier_count = 0;

static uint32_t keycode_cache_timer = 0;

#define KEYCODE_CACHE_REFRESH_INTERVAL 100


// ============================================================
// VIAL SAFE KEYMAP ACCESS
// ============================================================

static inline uint16_t get_keycode_at(uint8_t layer, keypos_t pos) {
    return keymap_key_to_keycode(layer, pos);
}


// ============================================================
// REACTIVE SYSTEM DESIGN NOTES
// ============================================================

/*
============================================================
REACTIVE SYSTEM DESIGN NOTES (PERFORMANCE-OPTIMIZED)
============================================================

- Event buffer → fixed size (MAX_REACTIVE)
- No full-matrix scans
- Encoder fallback mapping used
- Runs only in GAMING mode
- Integer math only
- Short-lived (~300ms effects)
*/


#define MAX_REACTIVE 8

typedef struct {
    uint8_t leds[4];
    uint8_t count;
    uint16_t timer;
} reactive_cluster_t;

static reactive_cluster_t reactive_clusters[MAX_REACTIVE];
static uint8_t reactive_index = 0;


// ============================================================
// ENCODER RING MAPPING
// ============================================================

static const uint8_t left_encoder_ring[]  = {23, 24, 25};
static const uint8_t right_encoder_ring[] = {49, 50};


// ============================================================
// REACTIVE EVENT ENTRY POINT
// ============================================================

/*
Captures keypress events and generates reactive clusters.

- Maps matrix positions → LED clusters
- Handles NO_LED keys via fallback
- Inserts event into circular buffer
*/

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!record->event.pressed)
        return true;

    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;

    if (row == 3 && col == 6) {
        reactive_cluster_t *c = &reactive_clusters[reactive_index];
        c->count = 3; c->timer = timer_read();
        c->leds[0] = 23; c->leds[1] = 24; c->leds[2] = 25;
        reactive_index = (reactive_index + 1) % MAX_REACTIVE;
        return true;
    }

    if (row == 8 && col == 0) {
        reactive_cluster_t *c = &reactive_clusters[reactive_index];
        c->count = 2; c->timer = timer_read();
        c->leds[0] = 49; c->leds[1] = 50;
        reactive_index = (reactive_index + 1) % MAX_REACTIVE;
        return true;
    }

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
// RGB PIPELINE ARCHITECTURE
// ============================================================

/*
============================================================
RGB PIPELINE ARCHITECTURE (LAYERED RENDER MODEL)
============================================================

Pipeline:

1. BASE       → render_flag_base
2. CONTEXT    → layer feedback / indicators
3. EVENT      → reactive effects

Order:
    BASE → CONTEXT → EVENT

Ensures:
✔ predictability
✔ no conflicts
✔ minimal CPU work
*/


// ============================================================
// BASIC RENDERING
// ============================================================

void render_flag_base(uint8_t brightness) {

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (get_led_flag(i) & LED_FLAG_KEYLIGHT) {
            apply_color(i, COLOR_BASE, brightness);
        }
    }
}


void render_layer_feedback(uint8_t brightness) {

    uint8_t layer = get_highest_layer(layer_state);

    rgb_t c;

    switch (layer) {
        case 1: c = (rgb_t){0,0,255}; break;
        case 2: c = (rgb_t){255,0,255}; break;
        default: return;
    }

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (get_led_flag(i) & LED_FLAG_KEYLIGHT) {
            apply_color(i, c, brightness);
        }
    }
}


// ============================================================
// REACTIVE RENDER (OPTIMIZED)
// ============================================================

void render_reactive(uint8_t brightness) {

    for (uint8_t i = 0; i < MAX_REACTIVE; i++) {

        uint16_t t = timer_elapsed(reactive_clusters[i].timer);

        if (t > 300) continue;

        uint8_t intensity = 255 - (t * 255 / 300);

        for (uint8_t j = 0; j < reactive_clusters[i].count; j++) {

            apply_color(
                reactive_clusters[i].leds[j],
                (rgb_t){intensity, intensity/2, 0},
                brightness
            );
        }
    }
}


// ============================================================
// MAIN RGB PIPELINE
// ============================================================

bool rgb_matrix_indicators_user(void) {

    uint8_t mode = rgb_matrix_get_mode();

    if (mode < RGB_MATRIX_CUSTOM)
        return false;

    current_custom_mode = mode - RGB_MATRIX_CUSTOM;

    if (timer_elapsed32(keycode_cache_timer) > KEYCODE_CACHE_REFRESH_INTERVAL) {
        update_keycode_cache();
        keycode_cache_timer = timer_read32();
    }

    uint8_t brightness = get_preset_brightness(0);

    switch (current_custom_mode) {

        case CUSTOM_MODE_UI:
            render_flag_base(brightness);
            render_layer_feedback(brightness);
            break;

        case CUSTOM_MODE_DEBUG:
            render_flag_base(brightness);
            break;

        case CUSTOM_MODE_GAMING:
            render_flag_base(brightness);
            render_reactive(brightness);
            break;
    }

    return false;
}


// ============================================================
// CACHE + INIT
// ============================================================

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


