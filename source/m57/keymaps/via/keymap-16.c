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
// OS STATE SYSTEM (PERSISTENT)
// ============================================================

typedef enum {
    OS_LINUX,
    OS_WINDOWS,
    OS_MAC,
    OS_ANDROID,
    OS_COUNT
} os_type_t;

static os_type_t current_os = OS_WINDOWS;

#define EEPROM_OS_ADDR (uint8_t*)(EECONFIG_USER)

static void load_os_from_eeprom(void) {
    uint8_t stored = eeprom_read_byte(EEPROM_OS_ADDR);
    current_os = (stored < OS_COUNT) ? stored : OS_WINDOWS;
}

static void save_os_to_eeprom(void) {
    eeprom_update_byte(EEPROM_OS_ADDR, current_os);
}


// ============================================================
// OS KEYCODES
// ============================================================

/*
Default suggested FN bindings:

FN + W → OS_WINDOWS_SET
FN + M → OS_MAC_SET
FN + L → OS_LINUX_SET
FN + A → OS_ANDROID_SET

Fully remappable in Vial.
*/

enum custom_keycodes {
    OS_LINUX_SET = SAFE_RANGE,
    OS_WINDOWS_SET,
    OS_MAC_SET,
    OS_ANDROID_SET
};


// ============================================================
// OS INDICATOR ZONE
// ============================================================

static const uint8_t os_indicator_zone[] = {0,1,2,3};
#define OS_INDICATOR_LED_COUNT (sizeof(os_indicator_zone))


// ============================================================
// KEYCODE CACHE SYSTEM
// ============================================================

/*
Cache refresh interval (ms):
50 → very responsive
100 → default
200 → low CPU usage
*/

static uint16_t led_keycode_cache[DRIVER_LED_TOTAL];
static uint8_t modifier_leds[DRIVER_LED_TOTAL];
static uint8_t modifier_count = 0;
static uint32_t keycode_cache_timer = 0;

#define KEYCODE_CACHE_REFRESH_INTERVAL 100


static void update_keycode_cache(void) {
    uint8_t layer = get_highest_layer(layer_state);

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        uint8_t row = g_led_config.matrix_co[i][0];
        if (row == NO_LED) continue;
        uint8_t col = g_led_config.matrix_co[i][1];
        led_keycode_cache[i] = keymap_key_to_keycode(layer, (keypos_t){row,col});
    }
}

static void build_modifier_list(void) {
    modifier_count = 0;

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (get_led_flag(i) & LED_FLAG_MODIFIER)
            modifier_leds[modifier_count++] = i;
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
// ENCODER OS SWITCH
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
// PROCESS RECORD USER
// ============================================================

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!record->event.pressed)
        return true;

    // === OS KEYCODES ===
    switch (keycode) {
        case OS_WINDOWS_SET: current_os = OS_WINDOWS; break;
        case OS_MAC_SET:     current_os = OS_MAC;     break;
        case OS_LINUX_SET:   current_os = OS_LINUX;   break;
        case OS_ANDROID_SET: current_os = OS_ANDROID; break;
        default: break;
    }

    if (keycode >= OS_LINUX_SET && keycode <= OS_ANDROID_SET) {
        save_os_to_eeprom();
        return false;
    }

    // === REACTIVE ===
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        if (g_led_config.matrix_co[i][0] == row &&
            g_led_config.matrix_co[i][1] == col) {

            reactive_cluster_t *c = &reactive_clusters[reactive_index];
            c->count = 1;
            c->timer = timer_read();
            c->leds[0] = i;

            reactive_index = (reactive_index + 1) % MAX_REACTIVE;
            break;
        }
    }

    return true;
}


// ============================================================
// RENDER LAYERS
// ============================================================

static void render_flag_base(uint8_t brightness) {
    const rgb_theme_t* theme = &theme_ui;

    switch (current_custom_mode) {
        case CUSTOM_MODE_UI: theme = &theme_ui; break;
        case CUSTOM_MODE_DEBUG: theme = &theme_debug; break;
        case CUSTOM_MODE_GAMING: theme = &theme_gaming; break;
    }

    for (uint8_t i=0;i<DRIVER_LED_TOTAL;i++)
        apply_color(i, theme->base, brightness);
}


static void render_key_based(uint8_t brightness) {
    for (uint8_t i=0;i<DRIVER_LED_TOTAL;i++)
        if (led_keycode_cache[i] == KC_ENT)
            apply_color(i, COLOR_MOD_SHIFT, brightness);
}


static void render_key_groups(uint8_t brightness) {
    for (uint8_t i=0;i<DRIVER_LED_TOTAL;i++) {
        uint16_t kc = led_keycode_cache[i];

        if (kc>=KC_A && kc<=KC_Z) apply_color(i,COLOR_MOD_ALT,brightness);
        else if (kc>=KC_1 && kc<=KC_0) apply_color(i,COLOR_MOD_CTRL,brightness);
        else if (kc>=KC_F1 && kc<=KC_F12) apply_color(i,COLOR_MOD_GUI,brightness);
    }
}


static void render_modifier(uint8_t brightness) {
    uint8_t mods = get_mods();

    for (uint8_t i=0;i<modifier_count;i++) {
        uint8_t led = modifier_leds[i];
        if (mods & MOD_MASK_SHIFT) apply_color(led,COLOR_MOD_SHIFT,brightness);
        if (mods & MOD_MASK_CTRL)  apply_color(led,COLOR_MOD_CTRL,brightness);
        if (mods & MOD_MASK_ALT)   apply_color(led,COLOR_MOD_ALT,brightness);
        if (mods & MOD_MASK_GUI)   apply_color(led,COLOR_MOD_GUI,brightness);
    }
}


static void render_os(uint8_t brightness) {
    rgb_t c = COLOR_OS_WINDOWS;

    switch (current_os) {
        case OS_LINUX: c=COLOR_OS_LINUX; break;
        case OS_MAC: c=COLOR_OS_MAC; break;
        case OS_ANDROID: c=COLOR_OS_ANDROID; break;
        default: break;
    }

    for (uint8_t i=0;i<OS_INDICATOR_LED_COUNT;i++)
        apply_color(os_indicator_zone[i], c, brightness);
}


static void render_event(uint8_t brightness) {
    const rgb_theme_t* theme = &theme_gaming;

    for (uint8_t i=0;i<MAX_REACTIVE;i++) {
        uint16_t t = timer_elapsed(reactive_clusters[i].timer);
        if (t>300) continue;

        uint8_t intensity = 255 - (t*255/300);

        for (uint8_t j=0;j<reactive_clusters[i].count;j++) {
            uint8_t led = reactive_clusters[i].leds[j];

            apply_color(led,(rgb_t){
                (theme->reactive.r*intensity)/255,
                (theme->reactive.g*intensity)/255,
                (theme->reactive.b*intensity)/255
            },brightness);
        }
    }
}


// ============================================================
// MAIN PIPELINE (FULLY RESTORED)
// ============================================================

bool rgb_matrix_indicators_user(void) {

    current_custom_mode = rgb_matrix_get_mode() - RGB_MATRIX_CUSTOM;

    if (timer_elapsed32(keycode_cache_timer) > KEYCODE_CACHE_REFRESH_INTERVAL) {
        update_keycode_cache();
        keycode_cache_timer = timer_read32();
    }

    uint8_t brightness = 200;

    render_flag_base(brightness);
    render_key_based(brightness);
    render_key_groups(brightness);
    render_modifier(brightness);
    render_os(brightness);

    if (current_custom_mode == CUSTOM_MODE_GAMING)
        render_event(brightness);

    return false;
}


// ============================================================
// INIT
// ============================================================

void keyboard_post_init_user(void) {
    build_modifier_list();
    update_keycode_cache();
    load_os_from_eeprom();
}

layer_state_t layer_state_set_user(layer_state_t state) {
    update_keycode_cache();
    return state;
}