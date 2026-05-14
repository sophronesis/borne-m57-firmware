#include QMK_KEYBOARD_H

// =====================
// LAYERS
// =====================

enum layers {
    _BASE = 0,
    _FN1,
    _FN2,
    _FN3
};

// =====================
// EEPROM SYSTEM
// =====================

#define CONFIG_VERSION 2
#define IDLE_SAVE_DELAY_MS 1500
#define FORCE_SAVE_MS 5000

bool persist_enabled = true;

typedef struct {
    bool invalid_config;
} debug_flags_t;

static debug_flags_t debug_flags = {0};

typedef struct {
    uint8_t version;
    uint8_t os_mode;
    uint8_t anim_mode;
    uint8_t anim_speed;
    uint8_t checksum;
} user_config_t;

static user_config_t user_config;
static user_config_t last_saved_config;

// =====================
// CHECKSUM
// =====================

uint8_t calc_checksum(user_config_t *cfg) {
    return cfg->version ^
           cfg->os_mode ^
           cfg->anim_mode ^
           cfg->anim_speed;
}

// =====================
// LOAD
// =====================

void load_user_config(void) {
    eeprom_read_block(&user_config, (void*)EECONFIG_USER, sizeof(user_config));

    if (user_config.checksum != calc_checksum(&user_config)) {
        debug_flags.invalid_config = true;
        user_config.os_mode = 0;
        user_config.anim_mode = 0;
        user_config.anim_speed = 3;
    }

    user_config.version = CONFIG_VERSION;
    last_saved_config = user_config;
}

// =====================
// SAVE LOGIC
// =====================

bool config_changed(void) {
    return memcmp(&user_config, &last_saved_config, sizeof(user_config) - 1) != 0;
}

void save_user_config(void) {
    if (!persist_enabled) return;
    if (!config_changed()) return;

    user_config.version = CONFIG_VERSION;
    user_config.checksum = calc_checksum(&user_config);

    eeprom_update_block(&user_config, (void*)EECONFIG_USER, sizeof(user_config));
    last_saved_config = user_config;

    debug_flags.invalid_config = false;
}

// =====================
// IDLE SAVE
// =====================

static uint32_t last_change_time = 0;

void mark_config_changed(void) {
    last_change_time = timer_read32();
}

void eeprom_task(void) {
    if (!config_changed()) return;

    uint32_t elapsed = timer_elapsed32(last_change_time);

    if (elapsed > IDLE_SAVE_DELAY_MS || elapsed > FORCE_SAVE_MS) {
        save_user_config();
    }
}

// =====================
// RGB INDICATORS
// =====================

#define LED_ESC 0
#define LED_TAB 6

void render_persist_indicator(void) {
    uint8_t r = 0, g = 100, b = 255; // default OS color

    if (persist_enabled) {
        rgb_matrix_set_color(LED_ESC, r, g, b);
        return;
    }

    uint16_t t = timer_read32() % 2000;
    uint8_t brightness = (t < 500) ? t/2 : (1000-t)/2;

    rgb_matrix_set_color(LED_ESC, brightness, 0, 0);
}

void render_debug_warnings(void) {
    if (debug_flags.invalid_config) {
        uint16_t t = timer_read32() % 2000;
        uint8_t b = (t < 1000) ? t/4 : (2000-t)/4;
        rgb_matrix_set_color(LED_TAB, b, b, 0);
    }
}

bool rgb_matrix_indicators_user(void) {
    render_persist_indicator();
    render_debug_warnings();
    return false;
}

// =====================
// HOOKS
// =====================

void matrix_scan_user(void) {
    eeprom_task();
}

void keyboard_post_init_user(void) {
    load_user_config();
}

// =====================
// KEYMAP
// =====================

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

[_BASE] = LAYOUT(
 KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5,           KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,
 KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_UP,    KC_LEFT, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC,
 KC_CAPS, KC_A, KC_S, KC_D, KC_F, KC_G, KC_DOWN, KC_RIGHT, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,
 KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_MUTE, RGB_MOD,  KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_DEL,
              KC_LALT, MO(_FN1), KC_SPC,          KC_ENT, MO(_FN2), KC_RCTL
)

};