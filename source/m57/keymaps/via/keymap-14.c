#include QMK_KEYBOARD_H
#include "theme.h"
#include "eeconfig.h"
#include <avr/eeprom.h>


// ============================================================
// CUSTOM MODE SYSTEM DESIGN
// ============================================================

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
// OS STATE SYSTEM
// ============================================================

/*
============================================================
OS STATE SYSTEM
============================================================

Adds persistent OS selection via EEPROM.

Behavior:
- OS is loaded at boot
- OS is updated whenever changed via encoder

Storage:
- Stored in EEPROM user space (EECONFIG_USER)

Safety:
- Invalid values fallback to default (WINDOWS)
*/

typedef enum {
    OS_LINUX,
    OS_WINDOWS,
    OS_MAC,
    OS_ANDROID,
    OS_COUNT
} os_type_t;

static os_type_t current_os = OS_WINDOWS;

// EEPROM storage location
#define EEPROM_OS_ADDR (uint8_t*)(EECONFIG_USER)

// Load OS from EEPROM
static void load_os_from_eeprom(void) {

    uint8_t stored = eeprom_read_byte(EEPROM_OS_ADDR);

    if (stored < OS_COUNT) {
        current_os = stored;
    } else {
        current_os = OS_WINDOWS;
    }
}

// Save OS to EEPROM
static void save_os_to_eeprom(void) {
    eeprom_update_byte(EEPROM_OS_ADDR, current_os);
}


// ============================================================
// OS INDICATOR LED ZONE
// ============================================================

static const uint8_t os_indicator_zone[] = { 0, 1, 2, 3 };

#define OS_INDICATOR_LED_COUNT (sizeof(os_indicator_zone) / sizeof(uint8_t))


// ============================================================
// KEYCODE CACHE SYSTEM
// ============================================================

static uint16_t led_keycode_cache[DRIVER_LED_TOTAL];

static uint8_t modifier_leds[DRIVER_LED_TOTAL];
static uint8_t modifier_count = 0;

static uint32_t keycode_cache_timer = 0;

/*
Cache refresh interval (ms):
50  → high responsiveness
100 → default
200 → low CPU usage
*/
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
// REACTIVE SYSTEM
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
// ENCODER OS SWITCHING (PERSISTENT)
// ============================================================

bool encoder_update_user(uint8_t index, bool clockwise) {

    if (get_highest_layer(layer_state) == 1) {

        if (clockwise) {
            current_os = (current_os + 1) % OS_COUNT;
        } else {
            current_os = (current_os == 0) ? OS_COUNT - 1 : current_os - 1;
        }

        save_os_to_eeprom(); // ✅ persist change

        return false;
    }

    return true;
}


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
// RENDER LAYERS
// ============================================================

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


static void render_modifier_highlighting(uint8_t brightness) {

    uint8_t mods = get_mods();

    for (uint8_t i = 0; i < modifier_count; i++) {

        uint8_t led = modifier_leds[i];

        if (mods & MOD_MASK_SHIFT) apply_color(led, COLOR_MOD_SHIFT, brightness);
        if (mods & MOD_MASK_CTRL)  apply_color(led, COLOR_MOD_CTRL, brightness);
        if (mods & MOD_MASK_ALT)   apply_color(led, COLOR_MOD_ALT, brightness);
        if (mods & MOD_MASK_GUI)   apply_color(led, COLOR_MOD_GUI, brightness);
    }
}


static void render_os_indicator(uint8_t brightness) {

    rgb_t color = COLOR_OS_WINDOWS;

    switch (current_os) {
        case OS_LINUX:   color = COLOR_OS_LINUX; break;
        case OS_WINDOWS: color = COLOR_OS_WINDOWS; break;
        case OS_MAC:     color = COLOR_OS_MAC; break;
        case OS_ANDROID: color = COLOR_OS_ANDROID; break;
    }

    for (uint8_t i = 0; i < OS_INDICATOR_LED_COUNT; i++) {
        apply_color(os_indicator_zone[i], color, brightness);
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

    render_key_groups(brightness);
    render_modifier_highlighting(brightness);
    render_os_indicator(brightness);

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

    load_os_from_eeprom(); // ✅ load OS at boot
}

layer_state_t layer_state_set_user(layer_state_t state) {

    update_keycode_cache();
    return state;
}
