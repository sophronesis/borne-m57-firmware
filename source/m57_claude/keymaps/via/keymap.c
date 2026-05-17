#include QMK_KEYBOARD_H
#include "theme.h"

/* ============================================================
 * DEBUG LOGGING
 * Build with: make m57:via OPT_DEFS+="-DDEBUG_LEVEL=3"
 * ============================================================ */
#define DEBUG_NONE    0
#define DEBUG_ERROR   1
#define DEBUG_WARN    2
#define DEBUG_INFO    3
#define DEBUG_VERBOSE 4

#ifndef DEBUG_LEVEL
#  define DEBUG_LEVEL DEBUG_NONE
#endif

#define LOG(level, fmt, ...) do { \
    if (DEBUG_LEVEL >= (level)) uprintf(fmt "\n", ##__VA_ARGS__); \
} while (0)

/* ============================================================
 * ENUMS
 * ============================================================ */

/* Layer indices — used with MO(), TG(), layer_state, and get_highest_layer(). */
typedef enum {
    LAYER_BASE = 0,
    LAYER_FN   = 1,
    LAYER_NAV  = 2,
    LAYER_GAME = 3,
} layer_id_t;

/* RGB UI modes — each maps 1:1 to a custom RGB effect at offset RGB_MATRIX_CUSTOM+N.
 * Also indexes theme_table[] and user_config.theme. */
typedef enum {
    UI_MODE_DEFAULT = 0,
    UI_MODE_DEBUG   = 1,
    UI_MODE_GAMING  = 2,
    UI_MODE_COUNT   = 3,
} ui_mode_t;

/* Stored OS type — controls which unicode input method is active and
 * which color is shown on the OS indicator zone (LEDs 0-3). */
typedef enum {
    OS_LINUX   = 0,
    OS_WINDOWS = 1,
    OS_MAC     = 2,
    OS_ANDROID = 3,
    OS_COUNT   = 4,
} os_type_t;

/* ============================================================
 * EEPROM USER CONFIG  (QMK user_config_t pattern)
 * ============================================================ */

typedef union {
    uint32_t raw;
    struct {
        uint8_t os_type : 8;
        uint8_t theme   : 8;
    };
} user_config_t;

static user_config_t user_config;

void eeconfig_init_user(void) {
    user_config.raw     = 0;
    user_config.os_type = OS_WINDOWS;
    user_config.theme   = UI_MODE_DEFAULT;
    eeconfig_update_user(user_config.raw);
}

static void save_config(void) {
    eeconfig_update_user(user_config.raw);
    LOG(DEBUG_INFO, "config saved: os=%u theme=%u", user_config.os_type, user_config.theme);
}

static void load_config(void) {
    user_config.raw = eeconfig_read_user();
    if (user_config.os_type >= OS_COUNT)      user_config.os_type = OS_WINDOWS;
    if (user_config.theme   >= UI_MODE_COUNT) user_config.theme   = UI_MODE_DEFAULT;
    LOG(DEBUG_INFO, "config loaded: os=%u theme=%u", user_config.os_type, user_config.theme);
}

/* ============================================================
 * CUSTOM KEYCODES
 * ============================================================ */

enum custom_keycodes {
    OS_SET_LINUX   = SAFE_RANGE,
    OS_SET_WINDOWS,
    OS_SET_MAC,
    OS_SET_ANDROID,
    THEME_NEXT,
    THEME_PREV,
};

/* ============================================================
 * UNICODE SYNC
 * Must be called every time os_type changes — in three places:
 *   1. keyboard_post_init_user (boot from EEPROM)
 *   2. each OS_SET_* case in process_record_user (keypress)
 *   3. encoder 1 OS cycling in encoder_update_user
 * Missing any one means Swedish characters break after that path.
 * ============================================================ */
static void sync_unicode_mode(void) {
    switch (user_config.os_type) {
        case OS_MAC:                        unicode_input_mode_set(UNICODE_MODE_MACOS); break;
        case OS_LINUX: case OS_ANDROID:     unicode_input_mode_set(UNICODE_MODE_LINUX); break;
        default: /* OS_WINDOWS */           unicode_input_mode_set(UNICODE_MODE_WIN);   break;
    }
}

/* ============================================================
 * KEYMAPS
 * AltGr combinations (via key_override, defined below):
 *   AltGr + [  ->  aa (KC_LBRC)
 *   AltGr + ;  ->  oe (KC_SCLN)
 *   AltGr + '  ->  ae (KC_QUOT) -- KC_QUOT must remain on BASE R76
 * ============================================================ */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  [LAYER_BASE] = LAYOUT(
    //,----+------+------+------+------+------.                   ,------+------+------+------+------+-------.
       KC_ESC,  KC_1,  KC_2,  KC_3,  KC_4,  KC_5,                  KC_6,  KC_7,  KC_8,  KC_9,  KC_0, KC_BSPC,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
       KC_TAB,  KC_Q,  KC_W,  KC_E,  KC_R,  KC_T, KC_UP,  KC_LEFT, KC_Y,  KC_U,  KC_I,  KC_O,  KC_P, KC_LBRC,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      KC_CAPS,  KC_A,  KC_S,  KC_D,  KC_F,  KC_G, KC_DOWN, KC_RGHT, KC_H,  KC_J,  KC_K,  KC_L, KC_SCLN, KC_QUOT,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      KC_LSFT,  KC_Z,  KC_X,  KC_C,  KC_V,  KC_B, KC_MUTE, RGB_MOD, KC_N,  KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_DEL,
    //`----+------+------+------+------+------+------.  ,------+------+------+------+------+------+------'
                           KC_LALT, MO(LAYER_FN), KC_SPC,    KC_ENT, MO(LAYER_NAV), KC_RCTL
  ),

  [LAYER_FN] = LAYOUT(
    //,----+------+------+------+------+------.                   ,------+------+------+------+------+-------.
       KC_GRV, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5,                 KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_DEL,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      _______, _______, _______, _______, KC_PGUP, KC_HOME, KC_PGUP, KC_PGDN, KC_F11, KC_F12, _______, _______, QK_CAPS_WORD_TOGGLE, KC_RBRC,
    //                                                            ^--- 2 slots free: future KC_PSCR, QK_LOCK
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      _______, THEME_PREV, _______, THEME_NEXT, KC_PGDN, KC_END, KC_PGDN, KC_END, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_QUOT, _______,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      _______, _______, _______, _______, TG(LAYER_GAME), QK_BOOT, KC_VOLD, KC_VOLU, OS_SET_WINDOWS, OS_SET_MAC, OS_SET_ANDROID, OS_SET_LINUX, KC_BSLS, _______,
    //`----+------+------+------+------+------+------.  ,------+------+------+------+------+------+------'
                        _______, MO(LAYER_FN), _______,    _______, MO(LAYER_NAV), _______
  ),

  [LAYER_NAV] = LAYOUT(
    /* Right HJKL = mouse movement (VIM-style)
     * Left ASD   = speed: A=ACCEL2(fast), S=ACCEL1(medium), D=ACCEL0(sniper)
     * Inner cols  = scroll wheel; Right YUI = mouse buttons */
    //,----+------+------+------+------+------.                   ,------+------+------+------+------+-------.
      _______, _______, _______, _______, _______, _______,        _______, _______, _______, _______, _______, _______,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      _______, _______, _______, _______, KC_PGUP, KC_HOME, KC_MS_WH_UP, KC_MS_WH_DOWN, KC_MS_BTN1, KC_MS_BTN2, KC_MS_BTN3, _______, _______, _______,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      _______, KC_MS_ACCEL2, KC_MS_ACCEL1, KC_MS_ACCEL0, KC_PGDN, KC_END, KC_MS_WH_DOWN, KC_MS_WH_UP, KC_MS_LEFT, KC_MS_DOWN, KC_MS_UP, KC_MS_RGHT, _______, _______,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    //`----+------+------+------+------+------+------.  ,------+------+------+------+------+------+------'
                        _______, MO(LAYER_FN), _______,    _______, MO(LAYER_NAV), _______
  ),

  [LAYER_GAME] = LAYOUT(
    /* Standard gaming remap: Caps Lock position -> KC_LCTL.
     * All other keys transparent. RGB auto-switches to GAMING_MODE. */
    //,----+------+------+------+------+------.                   ,------+------+------+------+------+-------.
      _______, _______, _______, _______, _______, _______,        _______, _______, _______, _______, _______, _______,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
       KC_LCTL, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    //,----+------+------+------+------+------+------.  ,------+------+------+------+------+------+-------.
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    //`----+------+------+------+------+------+------.  ,------+------+------+------+------+------+------'
                        _______, MO(LAYER_FN), _______,    _______, MO(LAYER_NAV), _______
  ),
};

/* ============================================================
 * ENCODER MAP
 * Encoder 0 BASE: CCW=RIGHT, CW=LEFT intentionally reversed (vendor firmware).
 * OS/theme cycling overrides happen in encoder_update_user on FN layer.
 * ============================================================ */
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [LAYER_BASE] = { ENCODER_CCW_CW(KC_RIGHT, KC_LEFT),  ENCODER_CCW_CW(KC_VOLD, KC_VOLU)  },
    [LAYER_FN]   = { ENCODER_CCW_CW(RGB_HUD,  RGB_HUI),  ENCODER_CCW_CW(RGB_SAD,  RGB_SAI)  },
    [LAYER_NAV]  = { ENCODER_CCW_CW(RGB_VAD,  RGB_VAI),  ENCODER_CCW_CW(RGB_SPD,  RGB_SPI)  },
    [LAYER_GAME] = { ENCODER_CCW_CW(RGB_RMOD, RGB_MOD),  ENCODER_CCW_CW(KC_RIGHT, KC_LEFT)  },
};
#endif

/* ============================================================
 * COMBOS  (zero hardcoded; all managed via Vial runtime editor)
 * ============================================================ */
combo_t key_combos[COMBO_COUNT];

/* ============================================================
 * KEY OVERRIDES -- Swedish AltGr characters
 * KC_QUOT must remain on BASE R76 as the ae trigger key.
 * ============================================================ */
static const key_override_t ao_override = ko_make_basic(MOD_BIT(KC_RALT), KC_LBRC, UC(0x00E5)); /* a-ring */
static const key_override_t oo_override = ko_make_basic(MOD_BIT(KC_RALT), KC_SCLN, UC(0x00F6)); /* o-umlaut */
static const key_override_t ae_override = ko_make_basic(MOD_BIT(KC_RALT), KC_QUOT, UC(0x00E4)); /* a-umlaut */

const key_override_t *key_overrides[] = {
    &ao_override,
    &oo_override,
    &ae_override,
    NULL,
};

/* ============================================================
 * LED ZONES
 * ============================================================ */

static const uint8_t os_led_zone[]    = {0, 1, 2, 3};    /* left top row -- OS indicator */
static const uint8_t layer_led_zone[] = {29, 30, 31, 32}; /* right top row -- layer indicator */
#define OS_LED_ZONE_SIZE    (sizeof(os_led_zone)    / sizeof(os_led_zone[0]))
#define LAYER_LED_ZONE_SIZE (sizeof(layer_led_zone) / sizeof(layer_led_zone[0]))
#define LED_IDX_CAPS 13  /* matrix row 2, col 0 -- Caps Lock position */

static inline bool led_in_os_zone(uint8_t led) {
    for (uint8_t i = 0; i < OS_LED_ZONE_SIZE; i++) {
        if (os_led_zone[i] == led) return true;
    }
    return false;
}

static inline bool led_in_layer_zone(uint8_t led) {
    for (uint8_t i = 0; i < LAYER_LED_ZONE_SIZE; i++) {
        if (layer_led_zone[i] == led) return true;
    }
    return false;
}

/* ============================================================
 * HIGHLIGHT KEYS  (painted with theme->accent in render step 2)
 * ============================================================ */
static const uint16_t highlight_keys[] = { KC_ENT, KC_BSPC, KC_DEL, KC_SPC };
#define HIGHLIGHT_KEY_COUNT (sizeof(highlight_keys) / sizeof(highlight_keys[0]))

/* ============================================================
 * KEYCODE-TO-LED CACHE
 * Always use keymap_key_to_keycode() -- Vial remaps only appear there.
 * ============================================================ */
#ifdef RGB_MATRIX_LED_COUNT
#  define M57_LED_COUNT RGB_MATRIX_LED_COUNT
#else
#  define M57_LED_COUNT 58
#endif

static uint16_t  led_key_cache[M57_LED_COUNT];
static uint32_t  cache_refresh_timer = 0;
#define CACHE_REFRESH_MS 100

#define MOD_DIM_FACTOR 4
#define MOD_LED_MAX    16
static uint8_t mod_leds[MOD_LED_MAX];
static uint8_t mod_led_count = 0;

static inline bool is_mod_key(uint16_t kc) {
    /* KC_LCTL through KC_RGUI covers all 8 standard modifiers in QMK
     * keycode order: LCTL, LSFT, LALT, LGUI, RCTL, RSFT, RALT, RGUI. */
    return (kc >= KC_LCTL && kc <= KC_RGUI);
}

/* Rebuilds led_key_cache for the current highest layer.
 * Must use keymap_key_to_keycode() — Vial remaps are stored in EEPROM
 * and only returned by this function; keymaps[][] returns compile-time values. */
static void refresh_key_cache(void) {
    uint8_t layer = get_highest_layer(layer_state);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led != NO_LED) {
                led_key_cache[led] = keymap_key_to_keycode(layer, (keypos_t){.row = row, .col = col});
            }
        }
    }
}

/* Precomputes mod_leds[] and mod_led_count from the current key cache.
 * Iterating only mod_leds[] during render (step 4) avoids scanning all 58 LEDs. */
static void refresh_mod_cache(void) {
    mod_led_count = 0;
    for (uint8_t i = 0; i < M57_LED_COUNT && mod_led_count < MOD_LED_MAX; i++) {
        if (is_mod_key(led_key_cache[i])) {
            mod_leds[mod_led_count++] = i;
        }
    }
}

/* ============================================================
 * REACTIVE KEYPRESS SYSTEM  (GAMING_MODE only)
 * Ring buffer storing LED index + timestamp per keypress.
 * ============================================================ */
#define REACTIVE_BUF_SIZE  8
#define REACTIVE_FADE_MS   300
#define REACTIVE_SLOT_FREE 0xFF

typedef struct {
    uint8_t  led;       /* LED index, or REACTIVE_SLOT_FREE (0xFF) when slot is empty */
    uint32_t timestamp; /* timer_read32() value captured at keypress */
} reactive_hit_t;

static reactive_hit_t reactive_hits[REACTIVE_BUF_SIZE];
static uint8_t        reactive_write_pos = 0;

/* ============================================================
 * BOOT ANIMATION
 * Left-to-right sweep using g_led_config.point[i].x (range 0-210).
 * 4 phases x 250 ms = 1000 ms total.
 * ============================================================ */
#define BOOT_SWEEP_PHASE_MS   250
#define BOOT_LED_MAX_X        210
#define BOOT_ANIM_BRIGHTNESS  255

static bool     boot_anim_active = false;
static uint32_t boot_anim_timer  = 0;

/* ============================================================
 * THEME TABLE  (indexed by ui_mode_t == RGB_MATRIX_CUSTOM offset)
 * ui_mode_t value N also means the mode is at RGB_MATRIX_CUSTOM+N,
 * so this table serves double duty for color lookup and mode mapping.
 * ============================================================ */
static const rgb_theme_t* const theme_table[UI_MODE_COUNT] = {
    [UI_MODE_DEFAULT] = &theme_ui,
    [UI_MODE_DEBUG]   = &theme_debug,
    [UI_MODE_GAMING]  = &theme_gaming,
};

/* Returns the current theme; falls back to theme_ui if stored index is OOB. */
static const rgb_theme_t *get_active_theme(void) {
    if (user_config.theme < UI_MODE_COUNT) {
        return theme_table[user_config.theme];
    }
    LOG(DEBUG_WARN, "theme OOB: %u", user_config.theme);
    return &theme_ui;
}

/* ============================================================
 * RENDER PIPELINE
 * Steps 1-4 skip os_led_zone. Step 8 skips os+layer zones.
 * ============================================================ */

/* Step 1: background -- theme->base on all non-OS-zone LEDs */
static void render_background(uint8_t led_min, uint8_t led_max,
                               const rgb_theme_t *theme, uint8_t brightness) {
    for (uint8_t i = led_min; i < led_max; i++) {
        if (!led_in_os_zone(i)) {
            apply_color(i, theme->base, brightness);
        }
    }
}

/* Step 2: accent keys -- Enter/Bspc/Del/Space with theme->accent */
static void render_action_keys(uint8_t led_min, uint8_t led_max,
                                const rgb_theme_t *theme, uint8_t brightness) {
    for (uint8_t i = led_min; i < led_max; i++) {
        if (led_in_os_zone(i)) continue;
        uint16_t kc = led_key_cache[i];
        for (uint8_t j = 0; j < HIGHLIGHT_KEY_COUNT; j++) {
            if (kc == highlight_keys[j]) {
                apply_color(i, theme->accent, brightness);
                break;
            }
        }
    }
}

/* Step 3: body keys -- letters, digits, F-keys with theme->body */
static void render_key_classes(uint8_t led_min, uint8_t led_max,
                                const rgb_theme_t *theme, uint8_t brightness) {
    for (uint8_t i = led_min; i < led_max; i++) {
        if (led_in_os_zone(i)) continue;
        uint16_t kc = led_key_cache[i];
        /* Note: KC_0 (0x27) > KC_9 (0x26) in QMK keycode order, so the range
         * KC_1..KC_0 (0x1E-0x27) covers all ten digit keycodes contiguously. */
        bool is_body = (kc >= KC_A  && kc <= KC_Z)  ||
                       (kc >= KC_1  && kc <= KC_0)  ||
                       (kc >= KC_F1 && kc <= KC_F12);
        if (is_body) apply_color(i, theme->body, brightness);
    }
}

/* Step 4: modifier keys -- color by type, dim when idle */
static void render_mod_keys(uint8_t led_min, uint8_t led_max, uint8_t brightness) {
    uint8_t mods = get_mods() | get_oneshot_mods();
    for (uint8_t i = 0; i < mod_led_count; i++) {
        uint8_t  led = mod_leds[i];
        if (led < led_min || led >= led_max) continue;
        if (led_in_os_zone(led)) continue;
        uint16_t kc = led_key_cache[led];
        rgb_t color;
        bool  active;
        switch (kc) {
            case KC_LSFT: case KC_RSFT:
                color  = (mods & MOD_MASK_CTRL)  ? COLOR_MOD_SHIFT_CTRL : COLOR_MOD_SHIFT;
                active = (mods & MOD_MASK_SHIFT); break;
            case KC_LCTL: case KC_RCTL:
                color  = (mods & MOD_MASK_SHIFT) ? COLOR_MOD_SHIFT_CTRL : COLOR_MOD_CTRL;
                active = (mods & MOD_MASK_CTRL);  break;
            case KC_LALT: case KC_RALT:
                color  = (mods & MOD_MASK_GUI)   ? COLOR_MOD_ALT_GUI    : COLOR_MOD_ALT;
                active = (mods & MOD_MASK_ALT);   break;
            case KC_LGUI: case KC_RGUI:
                color  = (mods & MOD_MASK_ALT)   ? COLOR_MOD_ALT_GUI    : COLOR_MOD_GUI;
                active = (mods & MOD_MASK_GUI);   break;
            default: continue;
        }
        apply_color(led, color, active ? brightness : brightness / MOD_DIM_FACTOR);
    }
}

/* Step 5: OS indicator -- always overwrites os_led_zone */
static void render_os_indicator(uint8_t led_min, uint8_t led_max, uint8_t brightness) {
    rgb_t color;
    switch (user_config.os_type) {
        case OS_LINUX:   color = COLOR_OS_LINUX;   break;
        case OS_WINDOWS: color = COLOR_OS_WINDOWS; break;
        case OS_MAC:     color = COLOR_OS_MAC;     break;
        case OS_ANDROID: color = COLOR_OS_ANDROID; break;
        default:         color = COLOR_OFF;        break;
    }
    for (uint8_t i = 0; i < OS_LED_ZONE_SIZE; i++) {
        uint8_t led = os_led_zone[i];
        if (led >= led_min && led < led_max) {
            apply_color(led, color, brightness);
        }
    }
}

/* Step 6: layer indicator -- layer_led_zone; skip on LAYER_BASE */
static void render_layer_indicator(uint8_t led_min, uint8_t led_max, uint8_t brightness) {
    uint8_t layer = get_highest_layer(layer_state);
    if (layer == LAYER_BASE) return;
    rgb_t color;
    switch (layer) {
        case LAYER_FN:   color = COLOR_LAYER_FN;   break;
        case LAYER_NAV:  color = COLOR_LAYER_NAV;  break;
        case LAYER_GAME: color = COLOR_LAYER_GAME; break;
        default:         color = COLOR_OFF;        break;
    }
    for (uint8_t i = 0; i < LAYER_LED_ZONE_SIZE; i++) {
        uint8_t led = layer_led_zone[i];
        if (led >= led_min && led < led_max) {
            apply_color(led, color, brightness);
        }
    }
}

/* Step 7: lock indicators -- Caps/Num/Scroll/CapsWord */
static void render_lock_indicators(uint8_t led_min, uint8_t led_max, uint8_t brightness) {
    led_t locks = host_keyboard_led_state();
    /* Caps Word overrides Caps Lock on the same LED when active */
    if (is_caps_word_on() && LED_IDX_CAPS >= led_min && LED_IDX_CAPS < led_max) {
        apply_color(LED_IDX_CAPS, COLOR_CAPS_WORD, brightness);
    } else if (locks.caps_lock && LED_IDX_CAPS >= led_min && LED_IDX_CAPS < led_max) {
        apply_color(LED_IDX_CAPS, COLOR_LOCK_CAPS, brightness);
    }
    if (locks.num_lock || locks.scroll_lock) {
        for (uint8_t i = led_min; i < led_max; i++) {
            uint16_t kc = led_key_cache[i];
            if (locks.num_lock    && kc == KC_NUM)  apply_color(i, COLOR_LOCK_NUM,  brightness);
            if (locks.scroll_lock && kc == KC_SCRL) apply_color(i, COLOR_LOCK_SCRL, brightness);
        }
    }
}

/* Step 8: reactive (GAMING_MODE only) -- brightness-scaled fade from theme->reactive */
static void render_reactive(uint8_t led_min, uint8_t led_max,
                             const rgb_theme_t *theme, uint8_t brightness) {
    for (uint8_t i = 0; i < REACTIVE_BUF_SIZE; i++) {
        if (reactive_hits[i].led == REACTIVE_SLOT_FREE) continue;
        uint32_t elapsed = timer_elapsed32(reactive_hits[i].timestamp);
        if (elapsed >= REACTIVE_FADE_MS) {
            reactive_hits[i].led = REACTIVE_SLOT_FREE;
            continue;
        }
        uint8_t led = reactive_hits[i].led;
        if (led < led_min || led >= led_max) continue;
        if (led_in_os_zone(led))             continue;
        if (led_in_layer_zone(led))          continue;
        uint8_t intensity = (uint8_t)(((uint32_t)(REACTIVE_FADE_MS - elapsed) * brightness) / REACTIVE_FADE_MS);
        apply_color(led, theme->reactive, intensity);
    }
}

/* ============================================================
 * CUSTOM RGB EFFECT STUBS
 * Canvas is cleared here; actual rendering happens in
 * rgb_matrix_indicators_advanced_user.
 * ============================================================ */
#ifdef RGB_MATRIX_CUSTOM_USER
static bool FIRMWARE_UI(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) apply_color(i, COLOR_OFF, 255);
    return led_max < RGB_MATRIX_LED_COUNT;
}
static bool DEBUG_MODE(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) apply_color(i, COLOR_OFF, 255);
    return led_max < RGB_MATRIX_LED_COUNT;
}
static bool GAMING_MODE(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) apply_color(i, COLOR_OFF, 255);
    return led_max < RGB_MATRIX_LED_COUNT;
}
#endif

/* ============================================================
 * QMK HOOKS
 * ============================================================ */

/* QMK callback: called for every encoder tick.
 * On LAYER_FN (when not in GAMING_MODE), encoder 0 cycles theme and
 * encoder 1 cycles OS. Guard prevents accidental changes during gameplay. */
bool encoder_update_user(uint8_t index, bool clockwise) {
    uint8_t layer = get_highest_layer(layer_state);
    uint8_t mode  = rgb_matrix_get_mode();
    bool gaming_rgb_active = (mode >= RGB_MATRIX_CUSTOM &&
                              (mode - RGB_MATRIX_CUSTOM) == UI_MODE_GAMING);

    if (layer == LAYER_FN && !gaming_rgb_active) {
        if (index == 0) {
            /* Encoder 0 on FN: cycle theme */
            if (clockwise) {
                user_config.theme = (user_config.theme + 1) % UI_MODE_COUNT;
            } else {
                user_config.theme = (user_config.theme + UI_MODE_COUNT - 1) % UI_MODE_COUNT;
            }
            LOG(DEBUG_INFO, "theme: %u", user_config.theme);
            save_config();
            return false;
        }
        if (index == 1) {
            /* Encoder 1 on FN: cycle OS + sync unicode */
            if (clockwise) {
                user_config.os_type = (user_config.os_type + 1) % OS_COUNT;
            } else {
                user_config.os_type = (user_config.os_type + OS_COUNT - 1) % OS_COUNT;
            }
            sync_unicode_mode(); /* place 3 of 3 for unicode sync */
            LOG(DEBUG_INFO, "os: %u", user_config.os_type);
            save_config();
            return false;
        }
    }
    return true; /* pass through to encoder_map */
}

/* QMK callback: called on every key event. Returns false to consume the key.
 * Only acts on key-down events; key-up is passed through immediately. */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return true;

    switch (keycode) {
        /* All OS_SET_* cases call sync_unicode_mode() — unicode sync place 2 of 3 */
        case OS_SET_LINUX:
            user_config.os_type = OS_LINUX;
            save_config();
            sync_unicode_mode();
            return false;
        case OS_SET_WINDOWS:
            user_config.os_type = OS_WINDOWS;
            save_config();
            sync_unicode_mode();
            return false;
        case OS_SET_MAC:
            user_config.os_type = OS_MAC;
            save_config();
            sync_unicode_mode();
            return false;
        case OS_SET_ANDROID:
            user_config.os_type = OS_ANDROID;
            save_config();
            sync_unicode_mode();
            return false;
        case THEME_NEXT:
            user_config.theme = (user_config.theme + 1) % UI_MODE_COUNT;
            save_config();
            return false;
        case THEME_PREV:
            user_config.theme = (user_config.theme + UI_MODE_COUNT - 1) % UI_MODE_COUNT;
            save_config();
            return false;
        default: break;
    }

    /* Feed reactive buffer when GAMING_MODE RGB is active */
    uint8_t mode = rgb_matrix_get_mode();
    bool gaming_rgb_active = (mode >= RGB_MATRIX_CUSTOM &&
                              (mode - RGB_MATRIX_CUSTOM) == UI_MODE_GAMING);
    if (gaming_rgb_active) {
        uint8_t led = g_led_config.matrix_co[record->event.key.row][record->event.key.col];
        if (led != NO_LED) {
            reactive_hits[reactive_write_pos].led       = led;
            reactive_hits[reactive_write_pos].timestamp = timer_read32();
            reactive_write_pos = (reactive_write_pos + 1) % REACTIVE_BUF_SIZE;
        }
    }

    return true;
}

/* QMK callback: runs once after all hardware is ready, before the main loop.
 * Loads EEPROM config, syncs unicode mode, seeds reactive buffer, starts boot anim. */
void keyboard_post_init_user(void) {
    load_config();
    sync_unicode_mode(); /* place 1 of 3 */

    for (uint8_t i = 0; i < REACTIVE_BUF_SIZE; i++) {
        reactive_hits[i].led = REACTIVE_SLOT_FREE;
    }

    refresh_key_cache();
    refresh_mod_cache();

    boot_anim_active = true;
    boot_anim_timer  = timer_read32();

    LOG(DEBUG_INFO, "M57 init: leds=%u mods=%u os=%u theme=%u",
        (uint8_t)M57_LED_COUNT, mod_led_count,
        user_config.os_type, user_config.theme);
}

/* QMK callback: called on every layer state change.
 * Refreshes key/mod caches and handles GAME layer RGB auto-switch. */
layer_state_t layer_state_set_user(layer_state_t state) {
    refresh_key_cache();
    refresh_mod_cache();

    /* GAME layer -> auto-switch RGB mode */
    static bool    game_layer_was_on = false;
    static uint8_t saved_rgb_mode    = 0;
    bool is_gaming = (state & ((layer_state_t)1 << LAYER_GAME)) != 0;

    if (is_gaming && !game_layer_was_on) {
        saved_rgb_mode = rgb_matrix_get_mode();
        rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM + UI_MODE_GAMING);
        LOG(DEBUG_INFO, "game on, saved mode=%u", saved_rgb_mode);
    } else if (!is_gaming && game_layer_was_on) {
        rgb_matrix_mode_noeeprom(saved_rgb_mode);
        LOG(DEBUG_INFO, "game off, restored mode=%u", saved_rgb_mode);
    }
    game_layer_was_on = is_gaming;

    return state;
}

/* QMK callback: called every RGB frame to paint custom indicators over the
 * base RGB effect. Runs boot animation first; in steady state only active
 * when the current mode is one of the three custom RGB modes. */
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    /* Boot animation: overrides everything for ~1 second */
    if (boot_anim_active) {
        uint32_t elapsed = timer_elapsed32(boot_anim_timer);
        if (elapsed < (uint32_t)BOOT_SWEEP_PHASE_MS * 4) {
            uint8_t  phase   = (uint8_t)(elapsed / BOOT_SWEEP_PHASE_MS);
            uint32_t phase_t = elapsed - (uint32_t)phase * BOOT_SWEEP_PHASE_MS;
            uint8_t  sweep   = (uint8_t)((phase_t * BOOT_LED_MAX_X) / BOOT_SWEEP_PHASE_MS);
            for (uint8_t i = led_min; i < led_max; i++) {
                uint8_t lx  = g_led_config.point[i].x;
                bool    lit;
                switch (phase) {
                    case 0:  lit = (lx <= sweep);                  break;
                    case 1:  lit = (lx >  sweep);                  break;
                    case 2:  lit = (lx >= BOOT_LED_MAX_X - sweep); break;
                    default: lit = (lx <  BOOT_LED_MAX_X - sweep); break;
                }
                apply_color(i, lit ? COLOR_WHITE : COLOR_OFF, BOOT_ANIM_BRIGHTNESS);
            }
            return false;
        }
        boot_anim_active = false;
    }

    /* Only run custom pipeline in custom RGB modes */
    uint8_t mode = rgb_matrix_get_mode();
    if (mode < RGB_MATRIX_CUSTOM) return false;

    ui_mode_t active_ui_mode = (ui_mode_t)(mode - RGB_MATRIX_CUSTOM);

    /* Periodic keycode cache refresh (catches live Vial remaps) */
    if (timer_elapsed32(cache_refresh_timer) > CACHE_REFRESH_MS) {
        refresh_key_cache();
        refresh_mod_cache();
        cache_refresh_timer = timer_read32();
    }

    const rgb_theme_t *theme    = get_active_theme();
    uint8_t            brightness = rgb_matrix_get_val();

    render_background(led_min, led_max, theme, brightness);
    render_action_keys(led_min, led_max, theme, brightness);
    render_key_classes(led_min, led_max, theme, brightness);
    render_mod_keys(led_min, led_max, brightness);
    render_os_indicator(led_min, led_max, brightness);
    render_layer_indicator(led_min, led_max, brightness);
    render_lock_indicators(led_min, led_max, brightness);

    if (active_ui_mode == UI_MODE_GAMING) {
        render_reactive(led_min, led_max, theme, brightness);
    }

    return false;
}