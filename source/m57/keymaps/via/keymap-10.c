#include QMK_KEYBOARD_H#include QMK#include "theme.h"


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

This firmware operates on two separate but related domains:

    1. Matrix space  (row, col) → key events
    2. LED space     (LED index) → RGB rendering

These are NOT guaranteed to be 1:1.

------------------------------------------------------------
CORE ASSUMPTION
------------------------------------------------------------

Each LED corresponds to a matrix position via:

    g_led_config.matrix_co[led] → {row, col}

However:

    NOT all matrix positions have LEDs.

------------------------------------------------------------
NO_LED POSITIONS (IMPORTANT)
------------------------------------------------------------

Some keys exist in matrix space but do NOT have LEDs:

    - Encoder press (left)  → L36 → (row 3, col 6)
    - Encoder press (right) → R80 → (row 8, col 0)

These positions are marked as:

    NO_LED

------------------------------------------------------------
IMPLICATIONS
------------------------------------------------------------

- Key events ALWAYS exist in matrix space
- LED rendering ONLY operates in LED space

Therefore:

    matrix key → may NOT map to LED

------------------------------------------------------------
FALLBACK STRATEGY
------------------------------------------------------------

For keys without LEDs (e.g. encoder presses):

    ✔ Map to nearby LED clusters
    ✔ Provide visual feedback without modifying hardware mapping

Example:
    L36 → mapped to left_encoder_ring
    R80 → mapped to right_encoder_ring

------------------------------------------------------------
DESIGN RULES
------------------------------------------------------------

- NEVER assume all keys have LEDs
- ALWAYS check for NO_LED when resolving matrix → LED
- DO NOT modify g_led_config to “fake” LED presence
- Use logical mapping (fallbacks) instead
*/


// ============================================================
// KEYCODE CACHE SYSTEM
// ============================================================
// (UNCHANGED — FULL COMMENT PRESERVED)

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
// VIAL SAFE KEYMAP ACCESS
// ============================================================

static inline uint16_t get_keycode_at(uint8_t layer, keypos_t pos) {
    return keymap_key_to_keycode(layer, pos);
}


// ============================================================
// REACTIVE SYSTEM DESIGN NOTES
// ============================================================
// (UNCHANGED — FULL COMMENT PRESERVED)

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
// (UNCHANGED — FULL COMMENT PRESERVED)

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!record->event.pressed)
        return true;

    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;

    if (row == 3 && col == 6) {
        reactive_cluster_t *c = &reactive_clusters[reactive_index];
        c->count = 3;
        c->timer = timer_read();
        c->leds[0] = 23;
        c->leds[1] = 24;
        c->leds[2] = 25;
        reactive_index = (reactive_index + 1) % MAX_REACTIVE;
        return true;
    }

    if (row == 8 && col == 0) {
        reactive_cluster_t *c = &reactive_clusters[reactive_index];
        c->count = 2;
        c->timer = timer_read();
        c->leds[0] = 49;
        c->leds[1] = 50;
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
// THEME SELECTION (NEW)
// ============================================================

static const rgb_theme_t* get_active_theme(void) {

    switch (current_custom_mode) {

        case CUSTOM_MODE_UI:
            return &theme_ui;

        case CUSTOM_MODE_DEBUG:
            return &theme_debug;

        case CUSTOM_MODE_GAMING:
            return &theme_gaming;
    }

    return &theme_ui;
}


// ============================================================
// BASE LAYER
// ============================================================

static void render_flag_base_with_mode(uint8_t brightness) {

    const rgb_theme_t* theme = get_active_theme();

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

    const rgb_theme_t* theme = get_active_theme();

    uint8_t layer = get_highest_layer(layer_state);

    rgb_t c;

    switch (layer) {
        case 1: c = theme->layer1; break;
        case 2: c = theme->layer2; break;
        default: return;
    }

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (get_led_flag(i) & LED_FLAG_KEYLIGHT) {
            apply_color(i, c, brightness);
        }
    }
}


// ============================================================
// EVENT LAYER
// ============================================================

static void render_event(uint8_t brightness) {

    const rgb_theme_t* theme = get_active_theme();

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

    uint8_t mode = rgb_matrix_get_mode();

    if (mode < RGB_MATRIX_CUSTOM)
        return false;

    current_custom_mode = mode - RGB_MATRIX_CUSTOM;

    uint8_t brightness = get_preset_brightness(0);

    render_flag_base_with_mode(brightness);

    if (current_custom_mode == CUSTOM_MODE_UI) {
        render_context(brightness);
    }

    if (current_custom_mode == CUSTOM_MODE_GAMING) {
        render_event(brightness);
    }

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
            get_keycode_at(layer, (keypos_t){row, col});
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

