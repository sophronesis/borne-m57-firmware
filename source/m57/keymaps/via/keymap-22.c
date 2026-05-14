#include QMK_KEYBOARD_H#include "theme.h"
#include "eeconfig.h"
#include <avr/eeprom.h>
#include "print.h"   // ✅ debug logging


// ============================================================
// DEBUG LOG SYSTEM (NEW)
// ============================================================

/*
Debug levels:
0 = OFF
1 = ERRORS
2 = WARN
3 = INFO (state changes)
4 = VERBOSE (events)
*/

#define DEBUG_NONE    0
#define DEBUG_ERROR   1
#define DEBUG_WARN    2
#define DEBUG_INFO    3
#define DEBUG_VERBOSE 4

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_INFO
#endif


#define LOG(level, fmt, ...) \
    do { if (DEBUG_LEVEL >= level) uprintf(fmt "\n", ##__VA_ARGS__); } while (0)


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
// OS STATE SYSTEM (EEPROM)
// ============================================================

typedef enum {
    OS_LINUX,
    OS_WINDOWS,
    OS_MAC,
    OS_ANDROID,
    OS_COUNT
} os_type_t;

static os_type_t current_os = OS_WINDOWS;

#define EEPROM_OS_ADDR (uint8_t*)(EECONFIG_USER + 0)

static void load_os_from_eeprom(void) {
    uint8_t stored = eeprom_read_byte(EEPROM_OS_ADDR);
    current_os = (stored < OS_COUNT) ? stored : OS_WINDOWS;

    LOG(DEBUG_INFO, "[OS] Loaded OS: %d", current_os);
}

static void save_os_to_eeprom(void) {
    eeprom_update_byte(EEPROM_OS_ADDR, current_os);
    LOG(DEBUG_INFO, "[OS] Saved OS: %d", current_os);
}


// ============================================================
// THEME STATE SYSTEM (EEPROM)
// ============================================================

static uint8_t current_theme_index = CUSTOM_MODE_UI;

#define EEPROM_THEME_ADDR (uint8_t*)(EECONFIG_USER + 1)

static void load_theme_from_eeprom(void) {
    uint8_t stored = eeprom_read_byte(EEPROM_THEME_ADDR);
    current_theme_index =
        (stored < CUSTOM_MODE_COUNT) ? stored : CUSTOM_MODE_UI;

    LOG(DEBUG_INFO, "[THEME] Loaded theme: %d", current_theme_index);
}

static void save_theme_to_eeprom(void) {
    eeprom_update_byte(EEPROM_THEME_ADDR, current_theme_index);
    LOG(DEBUG_INFO, "[THEME] Saved theme: %d", current_theme_index);
}


// ============================================================
// THEME SELECTION
// ============================================================

static const rgb_theme_t* get_active_theme(void) {

    switch (current_theme_index) {
        case CUSTOM_MODE_UI:     return &theme_ui;
        case CUSTOM_MODE_DEBUG:  return &theme_debug;
        case CUSTOM_MODE_GAMING: return &theme_gaming;
    }

    LOG(DEBUG_WARN, "[THEME] Invalid theme, fallback to UI");
    return &theme_ui;
}


// ============================================================
// OS + THEME KEYCODES
// ============================================================

enum custom_keycodes {
    OS_LINUX_SET = SAFE_RANGE,
    OS_WINDOWS_SET,
    OS_MAC_SET,
    OS_ANDROID_SET,
    THEME_NEXT,
    THEME_PREV
};


// ============================================================
// OS INDICATOR LED ZONE + OWNERSHIP
// ============================================================

static const uint8_t os_indicator_zone[] = {0,1,2,3};
#define OS_INDICATOR_LED_COUNT (sizeof(os_indicator_zone))

static bool is_os_led(uint8_t led) {
    for (uint8_t i = 0; i < OS_INDICATOR_LED_COUNT; i++) {
        if (os_indicator_zone[i] == led)
            return true;
    }
    return false;
}


// ============================================================
// CACHE SYSTEM
// ============================================================

static uint16_t led_keycode_cache[DRIVER_LED_TOTAL];
static uint32_t keycode_cache_timer = 0;
#define KEYCODE_CACHE_REFRESH_INTERVAL 100

static void update_keycode_cache(void) {

    uint8_t layer = get_highest_layer(layer_state);

    LOG(DEBUG_VERBOSE, "[CACHE] Updating cache (layer=%d)", layer);

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {

        uint8_t row = g_led_config.matrix_co[i][0];

        if (row == NO_LED) {
            led_keycode_cache[i] = KC_NO;
            continue;
        }

        uint8_t col = g_led_config.matrix_co[i][1];

        led_keycode_cache[i] =
            keymap_key_to_keycode(layer, (keypos_t){row,col});
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
// ENCODER OS SWITCHING
// ============================================================

bool encoder_update_user(uint8_t index, bool clockwise) {

    if (get_highest_layer(layer_state) == 1) {

        current_os = clockwise
            ? (current_os + 1) % OS_COUNT
            : (current_os == 0 ? OS_COUNT - 1 : current_os - 1);

        LOG(DEBUG_INFO, "[ENC] OS changed via encoder → %d", current_os);

        save_os_to_eeprom();
        return false;
    }

    return true;
}


// ============================================================
// PROCESS RECORD USER
// ============================================================

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!record->event.pressed)
        return true;

    switch (keycode) {
        case OS_WINDOWS_SET: current_os = OS_WINDOWS; break;
        case OS_MAC_SET:     current_os = OS_MAC; break;
        case OS_LINUX_SET:   current_os = OS_LINUX; break;
        case OS_ANDROID_SET: current_os = OS_ANDROID; break;
    }

    if (keycode >= OS_LINUX_SET && keycode <= OS_ANDROID_SET) {
        LOG(DEBUG_INFO, "[KEY] OS switched via key → %d", current_os);
        save_os_to_eeprom();
        return false;
    }

    switch (keycode) {

        case THEME_NEXT:
            current_theme_index =
                (current_theme_index + 1) % CUSTOM_MODE_COUNT;
            LOG(DEBUG_INFO, "[KEY] Theme NEXT → %d", current_theme_index);
            save_theme_to_eeprom();
            return false;

        case THEME_PREV:
            current_theme_index =
                (current_theme_index == 0)
                ? CUSTOM_MODE_COUNT - 1
                : current_theme_index - 1;
            LOG(DEBUG_INFO, "[KEY] Theme PREV → %d", current_theme_index);
            save_theme_to_eeprom();
            return false;
    }

    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {

        if (g_led_config.matrix_co[i][0] == row &&
            g_led_config.matrix_co[i][1] == col) {

            LOG(DEBUG_VERBOSE, "[REACT] LED triggered → %d", i);

            reactive_cluster_t *c =
                &reactive_clusters[reactive_index];

            c->count = 1;
            c->timer = timer_read();
            c->leds[0] = i;

            reactive_index =
                (reactive_index + 1) % MAX_REACTIVE;
            break;
        }
    }

    return true;
}


// ============================================================
// MAIN PIPELINE
// ============================================================

bool rgb_matrix_indicators_user(void) {

    uint8_t mode = rgb_matrix_get_mode();

    if (mode < RGB_MATRIX_CUSTOM) {
        LOG(DEBUG_WARN, "[RGB] Invalid mode: %d", mode);
        return false;
    }

    current_custom_mode = mode - RGB_MATRIX_CUSTOM;

    if (timer_elapsed32(keycode_cache_timer) > KEYCODE_CACHE_REFRESH_INTERVAL) {
        update_keycode_cache();
        keycode_cache_timer = timer_read32();
    }

    uint8_t brightness = rgb_matrix_get_val();

    render_flag_base(brightness);
    render_key_based(brightness);
    render_key_groups(brightness);
    render_modifier(brightness);
    render_os(brightness);

    if (current_custom_mode == CUSTOM_MODE_GAMING)
        render_event(brightness);

    return false;
}
