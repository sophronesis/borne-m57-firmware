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

typedef enum {
    OS_LINUX,
    OS_WINDOWS,
    OS_MAC,
    OS_ANDROID,
    OS_COUNT
} os_type_t;

static os_type_t current_os = OS_WINDOWS;


// ============================================================
// OS KEYCODES (NEW)
// ============================================================

/*
============================================================
OS KEYCODE CONTROL
============================================================

Provides explicit OS switching via keymap.

Works together with:
✔ encoder-based switching
✔ EEPROM persistence
✔ Vial key remapping

Default intended bindings (example):

Layer 1 (FN):
    W → Windows
    M → macOS
    L → Linux
    A → Android

These are NOT enforced here (layout-dependent),
but are recommended defaults for keymap.

Example keymap usage:

[1] = LAYOUT(
    ..., 
    OS_WINDOWS_SET,  // on 'W' position
    OS_MAC_SET,      // on 'M'
    OS_LINUX_SET,    // on 'L'
    OS_ANDROID_SET   // on 'A'
)
*/

enum custom_keycodes {
    OS_LINUX_SET = SAFE_RANGE,
    OS_WINDOWS_SET,
    OS_MAC_SET,
    OS_ANDROID_SET
};


// ============================================================
// EEPROM STORAGE
// ============================================================

#define EEPROM_OS_ADDR (uint8_t*)(EECONFIG_USER)

static void load_os_from_eeprom(void) {
    uint8_t stored = eeprom_read_byte(EEPROM_OS_ADDR);
    if (stored < OS_COUNT) current_os = stored;
    else current_os = OS_WINDOWS;
}

static void save_os_to_eeprom(void) {
    eeprom_update_byte(EEPROM_OS_ADDR, current_os);
}


// ============================================================
// ENCODER OS SWITCHING
// ============================================================

bool encoder_update_user(uint8_t index, bool clockwise) {

    if (get_highest_layer(layer_state) == 1) {

        if (clockwise)
            current_os = (current_os + 1) % OS_COUNT;
        else
            current_os = (current_os == 0) ? OS_COUNT - 1 : current_os - 1;

        save_os_to_eeprom();

        return false;
    }

    return true;
}


// ============================================================
// PROCESS RECORD USER (EXTENDED)
// ============================================================

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!record->event.pressed)
        return true;

    // ========================================================
    // OS KEYCODE HANDLING
    // ========================================================

    switch (keycode) {

        case OS_LINUX_SET:
            current_os = OS_LINUX;
            save_os_to_eeprom();
            return false;

        case OS_WINDOWS_SET:
            current_os = OS_WINDOWS;
            save_os_to_eeprom();
            return false;

        case OS_MAC_SET:
            current_os = OS_MAC;
            save_os_to_eeprom();
            return false;

        case OS_ANDROID_SET:
            current_os = OS_ANDROID;
            save_os_to_eeprom();
            return false;
    }

    // ========================================================
    // EXISTING REACTIVE LOGIC (UNCHANGED)
    // ========================================================

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
// CACHE + INIT
// ============================================================

void keyboard_post_init_user(void) {

    build_modifier_list();
    update_keycode_cache();

    load_os_from_eeprom();
}