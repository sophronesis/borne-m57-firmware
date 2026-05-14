#include QMK_KEYBOARD_H
#include "theme.h"

// ============================================================
// ENUMS
// ============================================================

typedef enum { OS_LINUX = 0, OS_WINDOWS, OS_MAC, OS_COUNT }       os_mode_t;
typedef enum { PRESET_DEFAULT = 0, PRESET_BRIGHT, PRESET_NIGHT, PRESET_COUNT } preset_t;
typedef enum { ESC_PHASE_OS = 0, ESC_PHASE_PERSIST, ESC_PHASE_MODE, ESC_PHASE_COUNT } esc_phase_t;

// ============================================================
// FEATURE FLAGS  (bitmask — add new features as 1 << N)
// ============================================================

#define FEATURE_MOD_DEBUG     (1 << 0)  // color modifier keys by type/state
#define FEATURE_SETTINGS_UI   (1 << 1)  // highlight layer-switch keys when layer > 0
#define FEATURE_ESC_INDICATOR (1 << 2)  // cycle ESC LED through OS/layer/mode info

static uint8_t features = FEATURE_MOD_DEBUG | FEATURE_SETTINGS_UI | FEATURE_ESC_INDICATOR;

// ============================================================
// DEBUG LEVEL  (set via rules.mk: OPT_DEFS += -DDEBUG_LEVEL=2)
// ============================================================

#define DEBUG_OFF     0
#define DEBUG_LIGHT   1
#define DEBUG_VERBOSE 2

#ifndef DEBUG_LEVEL
#  define DEBUG_LEVEL DEBUG_OFF
#endif

// ============================================================
// PRESETS  (ENUM + COUNT + ARRAY pattern)
// ============================================================

static const uint8_t preset_brightness[PRESET_COUNT] = {
    [PRESET_DEFAULT] = 200,
    [PRESET_BRIGHT]  = 255,
    [PRESET_NIGHT]   = 120,
};

static preset_t current_preset = PRESET_DEFAULT;

// ============================================================
// OS MODE
// ============================================================

static os_mode_t current_os = OS_LINUX;

// ============================================================
// ESC INDICATOR PHASES  (ENUM + COUNT + ARRAY pattern)
// ============================================================

static const uint16_t esc_phase_duration[ESC_PHASE_COUNT] = {
    [ESC_PHASE_OS]      = 900,
    [ESC_PHASE_PERSIST] = 1300,
    [ESC_PHASE_MODE]    = 700,
};

static uint16_t esc_total_duration = 0;  // computed once at init

// ============================================================
// BOOT INDICATOR
// ============================================================

#define BOOT_INDICATOR_DURATION 600  // ms — all-white flash at startup

static bool     boot_indicator_active = false;
static uint32_t boot_timer            = 0;

// ============================================================
// INDICATOR LEDs  (ESC = 0, TAB = 6)
// ============================================================

static const uint8_t indicator_leds[] = {0, 6};
#define INDICATOR_COUNT (sizeof(indicator_leds) / sizeof(indicator_leds[0]))

// ============================================================
// KEYCODE CACHE  (rebuilt on layer change; never recomputed per frame)
// ============================================================

#ifdef RGB_MATRIX_LED_COUNT
#  define M57_LED_COUNT RGB_MATRIX_LED_COUNT
#else
#  define M57_LED_COUNT 58
#endif

static uint16_t led_keycode_cache[M57_LED_COUNT];

// ============================================================
// MODIFIER LED SUBSET  (precomputed — only loop this, not all LEDs)
// ============================================================

#define MAX_MODIFIER_LEDS 16
static uint8_t modifier_leds[MAX_MODIFIER_LEDS];
static uint8_t modifier_led_count = 0;

// ============================================================
// KEYMAPS  (unchanged — use Vial to customise layers 4-9)
// ============================================================

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT(
   //,-------------------------------------------------------.     ,-------------------------------------------------------------.
     KC_ESC,   KC_1,    KC_2,    KC_3,   KC_4,   KC_5,                        KC_6,  KC_7,   KC_8,    KC_9,   KC_0,    KC_BSPC,
  //,-------------------------------------------------------.      ,-------------------------------------------------------------.
     KC_TAB,   KC_Q,    KC_W,    KC_E,   KC_R,   KC_T,  KC_UP,      KC_LEFT,  KC_Y,  KC_U,   KC_I,    KC_O,   KC_P,    KC_LBRC,
  //|--------+--------+--------+--------+--------+--------+----|  |--------+--------+--------+--------+-+--------+-----+----------|
     KC_CAPS,  KC_A,    KC_S,    KC_D,    KC_F,  KC_G,  KC_DOWN,    KC_RIGHT, KC_H,  KC_J,   KC_K,    KC_L,   KC_SCLN, KC_ENTER, 
  //|--------+--------+--------+--------+--------+--------+----'  `--------+--------+--------+-------+-----------+--------+--------|
     KC_LSFT,  KC_Z,    KC_X,    KC_C,    KC_V,  KC_B,  KC_MUTE,       RGB_MOD,  KC_N,  KC_M,   KC_COMM, KC_DOT, KC_SLSH, KC_DEL,
  //|--------+--------+-------+-------+---------+--------+-----.  ,--------+--------+--------+--------+--------+--------+--------|
                       KC_LALT,MO(1),  KC_SPC,                        KC_ENT,  MO(2),  KC_RCTL
                 //`-----------------------------------------------'  `--------------------------'

  ),

  [1] = LAYOUT(
//--------------------------------Left Hand-----------------------------------| |--------------------------------Right Hand------------------------------------------------
   //,-------------------------------------------------------.     ,-------------------------------------------------------------.
     KC_ESC,   KC_F1,    KC_F2,    KC_F3,   KC_F4,   KC_F5,                        KC_F6,  KC_F7,   KC_F8,    KC_F9,   KC_F10,    KC_BSPC,
  //,-------------------------------------------------------.      ,-------------------------------------------------------------.
     KC_TAB,   KC_Q,    KC_W,    KC_E,   KC_R,   KC_T,  KC_UP,      KC_LEFT,  KC_Y,  KC_U,   KC_I,    KC_O,   KC_P,    KC_LBRC,
  //|--------+--------+--------+--------+--------+--------+----|  |--------+--------+--------+--------+-+--------+-----+----------|
     KC_CAPS,  KC_A,    KC_S,    KC_D,    KC_F,  KC_G,  KC_DOWN,    KC_RIGHT, KC_H,  KC_J,   KC_K,    KC_L,   KC_SCLN, KC_ENTER, 
  //|--------+--------+--------+--------+--------+--------+----'  `--------+--------+--------+-------+-----------+--------+--------|
     KC_LSFT,  KC_Z,    KC_X,    KC_C,    KC_V,  KC_B,  KC_MUTE,       RGB_MOD,  KC_N,  KC_M,   KC_COMM, KC_DOT, KC_SLSH, KC_DEL,
  //|--------+--------+-------+-------+---------+--------+-----.  ,--------+--------+--------+--------+--------+--------+--------|
                         KC_LCTL,MO(1),  KC_SPC,                     KC_ENT,  MO(2), KC_RCTL
  ),

  [2] = LAYOUT(
   //,-------------------------------------------------------.     ,-------------------------------------------------------------.
     KC_ESC,   KC_1,    KC_2,    KC_3,   KC_4,   KC_5,                        KC_6,  KC_7,   KC_8,    KC_9,   KC_0,    KC_BSPC,
  //,-------------------------------------------------------.      ,-------------------------------------------------------------.
     KC_TAB,   KC_Q,    KC_W,    KC_E,   KC_R,   KC_T,  KC_UP,      KC_LEFT,  KC_Y,  KC_U,   KC_I,    KC_O,   KC_P,    KC_LBRC,
  //|--------+--------+--------+--------+--------+--------+----|  |--------+--------+--------+--------+-+--------+-----+----------|
     KC_CAPS,  KC_A,    KC_S,    KC_D,    KC_F,  KC_G,  KC_DOWN,    KC_RIGHT, KC_H,  KC_J,   KC_K,    KC_L,   KC_SCLN, KC_ENTER, 
  //|--------+--------+--------+--------+--------+--------+----'  `--------+--------+--------+-------+-----------+--------+--------|
     KC_LSFT,  KC_Z,    KC_X,    KC_C,    KC_V,  KC_B,  KC_MUTE,       RGB_MOD,  KC_N,  KC_M,   KC_COMM, KC_DOT, KC_SLSH, KC_DEL,
  //|--------+--------+-------+-------+---------+--------+-----.  ,--------+--------+--------+--------+--------+--------+--------|
                          KC_LCTL,MO(1),  KC_SPC,                   KC_ENT,  MO(2), KC_RCTL
  ),

  [3] = LAYOUT(
    //,-------------------------------------------------------.     ,-------------------------------------------------------------.
     KC_ESC,   KC_1,    KC_2,    KC_3,   KC_4,   KC_5,                        KC_6,  KC_7,   KC_8,    KC_9,   KC_0,    KC_BSPC,
  //,-------------------------------------------------------.      ,-------------------------------------------------------------.
     KC_TAB,   KC_Q,    KC_W,    KC_E,   KC_R,   KC_T,  KC_UP,      KC_LEFT,  KC_Y,  KC_U,   KC_I,    KC_O,   KC_P,    KC_LBRC,
  //|--------+--------+--------+--------+--------+--------+----|  |--------+--------+--------+--------+-+--------+-----+----------|
     KC_CAPS,  KC_A,    KC_S,    KC_D,    KC_F,  KC_G,  KC_DOWN,    KC_RIGHT, KC_H,  KC_J,   KC_K,    KC_L,   KC_SCLN, KC_ENTER, 
  //|--------+--------+--------+--------+--------+--------+----'  `--------+--------+--------+-------+-----------+--------+--------|
     KC_LSFT,  KC_Z,    KC_X,    KC_C,    KC_V,  KC_B,  KC_MUTE,       RGB_MOD,  KC_N,  KC_M,   KC_COMM, KC_DOT, KC_SLSH, KC_DEL,
  //|--------+--------+-------+-------+---------+--------+-----.  ,--------+--------+--------+--------+--------+--------+--------|
                      KC_LCTL,MO(1),  KC_SPC,                        KC_ENT,  MO(2),  KC_RCTL
                    //`--------------------------'  `--------------------------'
  )
};
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [0] =   { ENCODER_CCW_CW(KC_RIGHT, KC_LEFT),          ENCODER_CCW_CW(KC_VOLD, KC_VOLU)  },
    [1] =   { ENCODER_CCW_CW(RGB_HUD, RGB_HUI),           ENCODER_CCW_CW(RGB_SAD, RGB_SAI)  },
    [2] =   { ENCODER_CCW_CW(RGB_VAD, RGB_VAI),           ENCODER_CCW_CW(RGB_SPD, RGB_SPI)  },
    [3] =   { ENCODER_CCW_CW(RGB_RMOD, RGB_MOD),          ENCODER_CCW_CW(KC_RIGHT, KC_LEFT) },
    //                  Encoder 1                                     Encoder 2
};
#endif

// ============================================================
// HELPERS
// ============================================================

/*
============================================================
VIAL KEYMAP ACCESS RULE
============================================================

All key lookups MUST use keymap_key_to_keycode().
Direct access to keymaps[][] will break Vial remapping.

This wrapper ensures consistency and provides a single
point of control if behavior must change later.
*/
static inline uint16_t get_keycode_at(uint8_t layer, keypos_t pos) {
    return keymap_key_to_keycode(layer, pos);
}

static inline bool is_modifier_keycode(uint16_t kc) {
    return (kc >= KC_LCTL && kc <= KC_RGUI);
}

// Rebuild led_keycode_cache for the current highest layer.
// Call on init and on every layer change.
static void update_keycode_cache(void) {
    uint8_t layer = get_highest_layer(layer_state);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led != NO_LED) {
                //led_keycode_cache[led] = keymap_key_to_keycode(layer, (keypos_t){.row = row, .col = col});
                led_keycode_cache[led] = get_keycode_at(layer, (keypos_t){.row = row, .col = col});
            }
        }
    }
}

// Precompute which LEDs correspond to modifier keycodes on the current layer.
// Only this subset is scanned in render_modifier_debug — not all LEDs.
static void build_modifier_list(void) {
    modifier_led_count = 0;
    for (uint8_t i = 0; i < M57_LED_COUNT && modifier_led_count < MAX_MODIFIER_LEDS; i++) {
        if (is_modifier_keycode(led_keycode_cache[i])) {
            modifier_leds[modifier_led_count++] = i;
        }
    }
}

// Returns the current ESC indicator phase based on a rolling timer.
static esc_phase_t get_esc_phase(void) {
    uint16_t t = (uint16_t)(timer_read32() % esc_total_duration);
    esc_phase_t phase = ESC_PHASE_OS;
    while (phase < ESC_PHASE_COUNT - 1 && t >= esc_phase_duration[phase]) {
        t -= esc_phase_duration[phase];
        phase++;
    }
    return phase;
}

// ============================================================
// RENDER PIPELINE  (each step overrides the previous)
// ============================================================

// Step 1: paint every LED with its base color from the LED flag.
static void render_flag_base(uint8_t led_min, uint8_t led_max) {
    uint8_t br = preset_brightness[current_preset];
    for (uint8_t i = led_min; i < led_max; i++) {
        if (g_led_config.flags[i] & LED_FLAG_MODIFIER) {
            apply_color(i, COLOR_BASE, br / 2);
        } else if (g_led_config.flags[i] & LED_FLAG_KEYLIGHT) {
            apply_color(i, COLOR_BASE, br);
        }
    }
}

// Step 2: override modifier key LEDs with their assigned color.
//         Dim when unpressed; full brightness when the modifier is held.
static void render_modifier_debug(uint8_t led_min, uint8_t led_max) {
    if (!(features & FEATURE_MOD_DEBUG)) return;
    uint8_t br   = preset_brightness[current_preset];
    uint8_t mods = get_mods() | get_oneshot_mods();

    for (uint8_t i = 0; i < modifier_led_count; i++) {
        uint8_t  led = modifier_leds[i];
        if (led < led_min || led >= led_max) continue;
        uint16_t kc  = led_keycode_cache[led];
        rgb_t    color;
        bool     active;

        switch (kc) {
            case KC_LSFT: case KC_RSFT:
                color  = (mods & MOD_MASK_CTRL) ? COLOR_MOD_SHIFT_CTRL : COLOR_MOD_SHIFT;
                active = (mods & MOD_MASK_SHIFT); break;
            case KC_LCTL: case KC_RCTL:
                color  = (mods & MOD_MASK_SHIFT) ? COLOR_MOD_SHIFT_CTRL : COLOR_MOD_CTRL;
                active = (mods & MOD_MASK_CTRL);  break;
            case KC_LALT: case KC_RALT:
                color  = (mods & MOD_MASK_GUI) ? COLOR_MOD_ALT_GUI : COLOR_MOD_ALT;
                active = (mods & MOD_MASK_ALT);   break;
            case KC_LGUI: case KC_RGUI:
                color  = (mods & MOD_MASK_ALT) ? COLOR_MOD_ALT_GUI : COLOR_MOD_GUI;
                active = (mods & MOD_MASK_GUI);   break;
            default: continue;
        }
        apply_color(led, color, active ? br : br / 4);
    }
}

// Step 3: when a non-base layer is active, highlight layer-switch keys.
static void render_settings_highlight(uint8_t led_min, uint8_t led_max) {
    if (!(features & FEATURE_SETTINGS_UI)) return;
    if (get_highest_layer(layer_state) == 0) return;
    uint8_t br = preset_brightness[current_preset];
    for (uint8_t i = led_min; i < led_max; i++) {
        uint16_t kc = led_keycode_cache[i];
        if (kc == MO(1) || kc == MO(2) || kc == MO(3)) {
            apply_color(i, COLOR_SETTINGS, br);
        }
    }
}

// Step 4: cycle ESC LED (LED 0) through OS → layer-persist → mode phases.
static void render_esc_indicator(uint8_t led_min, uint8_t led_max) {
    if (!(features & FEATURE_ESC_INDICATOR)) return;
    if (0 < led_min || 0 >= led_max) return;  // ESC is LED 0

    rgb_t color;
    switch (get_esc_phase()) {
        case ESC_PHASE_OS:
            switch (current_os) {
                case OS_LINUX:   color = COLOR_OS_LINUX;   break;
                case OS_WINDOWS: color = COLOR_OS_WINDOWS; break;
                case OS_MAC:     color = COLOR_OS_MAC;     break;
                default:         color = COLOR_WHITE;      break;
            }
            break;
        case ESC_PHASE_PERSIST: color = COLOR_PERSIST_ON;   break;
        case ESC_PHASE_MODE:    color = COLOR_MODE_STATIC;  break;
        default:                color = COLOR_OFF;          break;
    }
    apply_color(0, color, preset_brightness[current_preset]);
}

// ============================================================
// QMK HOOKS
// ============================================================

void keyboard_post_init_user(void) {
    esc_total_duration = 0;
    for (uint8_t i = 0; i < ESC_PHASE_COUNT; i++) {
        esc_total_duration += esc_phase_duration[i];
    }

    update_keycode_cache();
    build_modifier_list();

    boot_indicator_active = true;
    boot_timer            = timer_read32();

#if DEBUG_LEVEL >= DEBUG_LIGHT
    uprintf("M57 init: leds=%u mods=%u esc_total=%u\n",
            (uint8_t)M57_LED_COUNT, modifier_led_count, esc_total_duration);
#endif
}

layer_state_t layer_state_set_user(layer_state_t state) {
    update_keycode_cache();
    build_modifier_list();
#if DEBUG_LEVEL >= DEBUG_VERBOSE
    uprintf("Layer: %u\n", get_highest_layer(state));
#endif
    return state;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (boot_indicator_active) {
        if (timer_elapsed32(boot_timer) < BOOT_INDICATOR_DURATION) {
            for (uint8_t i = led_min; i < led_max; i++) {
                rgb_matrix_set_color(i, 255, 255, 255);
            }
            return false;
        }
        boot_indicator_active = false;
    }

    render_flag_base(led_min, led_max);
    render_modifier_debug(led_min, led_max);
    render_settings_highlight(led_min, led_max);
    render_esc_indicator(led_min, led_max);
    return false;
}

