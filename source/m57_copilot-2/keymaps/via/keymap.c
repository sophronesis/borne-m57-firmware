#include QMK_KEYBOARD_H
#include "theme.h"
#include "eeconfig.h"
#include "print.h"
#include "host.h"


// ============================================================
// DEBUG LOG SYSTEM
// ============================================================

#define DEBUG_NONE    0
#define DEBUG_ERROR   1
#define DEBUG_WARN    2
#define DEBUG_INFO    3
#define DEBUG_VERBOSE 4

#ifndef DEBUG_LEVEL
#  define DEBUG_LEVEL DEBUG_NONE  // silent by default; set to DEBUG_INFO or higher
#endif                            // and enable CONSOLE_ENABLE = yes in rules.mk

#define LOG(level, fmt, ...) \
    do { if (DEBUG_LEVEL >= level) uprintf(fmt "\n", ##__VA_ARGS__); } while (0)


// ============================================================
// LAYER NAMES
// ============================================================

typedef enum {
    LAYER_BASE = 0,
    LAYER_FN,
    LAYER_NAV,
    LAYER_GAME
} layer_id_t;


// ============================================================
// CUSTOM MODE IDENTIFIERS
// ============================================================

/*
 * Mode = behavior (what the render pipeline does)
 * Theme = colors (which rgb_theme_t profile is active)
 *
 * Decoupled intentionally: same mode can use different themes
 * and vice versa.
 */

typedef enum {
    CUSTOM_MODE_UI     = 0,
    CUSTOM_MODE_DEBUG,
    CUSTOM_MODE_GAMING,
    CUSTOM_MODE_COUNT
} custom_mode_t;

static custom_mode_t current_custom_mode = CUSTOM_MODE_UI;


// ============================================================
// OS STATE
// ============================================================

typedef enum {
    OS_LINUX   = 0,
    OS_WINDOWS,
    OS_MAC,
    OS_ANDROID,
    OS_COUNT
} os_type_t;


// ============================================================
// EEPROM USER CONFIG  (packed into one uint32_t for eeconfig API)
// ============================================================

typedef union {
    uint32_t raw;
    struct {
        uint8_t os_type : 8;
        uint8_t theme   : 8;
    };
} user_config_t;

static user_config_t user_config;

void eeconfig_init_user(void) {
    user_config.os_type = OS_WINDOWS;
    user_config.theme   = CUSTOM_MODE_UI;
    eeconfig_update_user(user_config.raw);
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
// KEYMAPS  (Vial can remap layers 4–9 at runtime)
// ============================================================

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  // ── LAYER 0: BASE ──────────────────────────────────────────────────────────
  [LAYER_BASE] = LAYOUT(
   //,-------------------------------------------------------------.     ,-------------------------------------------------------------.
     KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,   KC_8,    KC_9,   KC_0,    KC_BSPC,
   //,-------------------------------------------------------------.     ,-------------------------------------------------------------.
     KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,  KC_UP,     KC_LEFT, KC_Y,    KC_U,   KC_I,    KC_O,   KC_P,    KC_LBRC,
   //|--------+--------+--------+--------+--------+-------+------| |---------+--------+-------+--------+-------+---------+---------|
     KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,  KC_DOWN,   KC_RGHT, KC_H,    KC_J,   KC_K,    KC_L,   KC_SCLN, KC_NO,
   //|--------+--------+--------+--------+--------+-------+------| |---------+--------+-------+--------+-------+---------+---------|
     KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,  KC_MUTE,   RGB_MOD, KC_N,    KC_M,   KC_COMM, KC_DOT, KC_SLSH, KC_DEL,
   //|--------+--------+--------+--------+--------+-------+---.   .---+-------+--------+-------+--------+-------+---------+---------|
                                KC_LALT, MO(LAYER_FN),  KC_SPC,    KC_ENT,  MO(LAYER_NAV), KC_RCTL
                         //`-------------------------------------------'  `-----------------------------------'
  ),

  // ── LAYER 1: FN ────────────────────────────────────────────────────────────
  // Left:  F-keys, PgUp/Dn, Home/End on row extras, Vol, Boot, theme cycle
  // Right: F11/F12, PrintScrn, Scroll/NumLk, HJKL arrows (vim), OS SET keys
  // TG(GAME) = FN + V
  [LAYER_FN] = LAYOUT(
   //,-------------------------------------------------------------.     ,-------------------------------------------------------------.
     KC_GRV,  KC_F1,      KC_F2,      KC_F3,         KC_F4,   KC_F5,               KC_F6,   KC_F7,         KC_F8,         KC_F9,         KC_F10,        KC_DEL,
   //,-------------------------------------------------------------.     ,-------------------------------------------------------------.
     _______, _______,    _______,    _______,       KC_PGUP, KC_HOME, KC_PGUP, KC_PGDN, KC_F11,        KC_F12,        _______,       _______,       _______, KC_RBRC,
   // Freed slots (row1 right: O, P, [):  KC_PSCR (screenshot), KC_PSCR+mod (region snip),
   //   KC_SCRL (ScrollLk — rarely needed; good for app-specific binds),
   //   KC_NUM (NumLk — useful if numpad layer active), QK_LOCK (caps-word),
   //   LSG(KC_S) (Win snip), LGUI(KC_PSCR) (Win fullscreen snip)
   //|--------+-----------+-----------+---------------+---------+--------+-----| |--------+--------------+--------------+--------------+--------------+--------+---------|
     _______, THEME_PREV, _______,    THEME_NEXT,    KC_PGDN, KC_END,  KC_PGDN, KC_END,  KC_LEFT,       KC_DOWN,       KC_UP,         KC_RGHT,       KC_QUOT, _______,
   //|--------+-----------+-----------+---------------+---------+--------+-----| |--------+--------------+--------------+--------------+--------------+--------+---------|
     _______, _______,    _______,    _______,       TG(LAYER_GAME), QK_BOOT, KC_VOLD, KC_VOLU, OS_WINDOWS_SET, OS_MAC_SET, OS_ANDROID_SET, OS_LINUX_SET, KC_BSLS, _______,
   //|--------+-----------+-----------+---------------+---------+--------+---.   .---+------+--------------+--------------+--------------+--------------+--------+---------|
                                       _______, MO(LAYER_FN),  _______,         _______,  MO(LAYER_NAV),  _______
  ),

  // ── LAYER 2: NAV / MOUSE ───────────────────────────────────────────────────
  // Right HJKL:  H=←  J=↓  K=↑  L=→  (VIM-style mouse movement)
  // Right Y/U/I: left / right / middle click
  // Left A/S/D:  speed — A=fast (ACCEL2)  S=medium (ACCEL1)  D=sniper (ACCEL0)
  //              Hold left-hand speed key + right-hand HJKL = two-hand precision control
  // Middle col:  WH_UP (row1) / WH_DOWN (row2) — scroll wheel, reachable from either hand
  // Left R/T F/G: PgUp/Home  PgDn/End — text navigation unchanged
  // Right thumb Enter → pass-through (base Enter); no numpad Enter needed
  [LAYER_NAV] = LAYOUT(
   //,-------------------------------------------------------------.     ,-------------------------------------------------------------.
     _______, _______, _______, _______, _______, _______,               _______, _______, _______, _______, _______, _______,
   //,-------------------------------------------------------------.     ,-------------------------------------------------------------.
     _______, _______, _______, _______, KC_PGUP, KC_HOME, KC_MS_WH_UP,  KC_MS_WH_DOWN, KC_MS_BTN1, KC_MS_BTN2, KC_MS_BTN3, _______, _______, _______,
   //|--------+--------+--------+--------+--------+--------+------| |--------+----------+----------+----------+--------+--------+---------|
     _______, KC_MS_ACCEL2, KC_MS_ACCEL1, KC_MS_ACCEL0, KC_PGDN, KC_END, KC_MS_WH_DOWN, KC_MS_WH_UP, KC_MS_LEFT, KC_MS_DOWN, KC_MS_UP, KC_MS_RGHT, _______, _______,
   //|--------+--------+--------+--------+--------+--------+------| |--------+----------+----------+----------+--------+--------+---------|
     _______, _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______, _______, _______,
   //|--------+--------+--------+--------+--------+--------+---.   .---+------+----------+----------+----------+--------+--------+---------|
                                _______, MO(LAYER_FN), _______,         _______, MO(LAYER_NAV), _______
  ),

  // ── LAYER 3: GAME ──────────────────────────────────────────────────────────
  // Activated via TG(LAYER_GAME) on FN+V.  RGB auto-switches to GAMING_MODE.
  // Only change from BASE: CAPS → LCTL (standard gaming remapping).
  [LAYER_GAME] = LAYOUT(
   //,-------------------------------------------------------------.     ,-------------------------------------------------------------.
     _______, _______, _______, _______, _______, _______,               _______, _______, _______, _______, _______, _______,
   //,-------------------------------------------------------------.     ,-------------------------------------------------------------.
     _______, _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______, _______, _______,
   //|--------+--------+--------+--------+--------+--------+------| |--------+--------+--------+--------+--------+--------+---------|
     KC_LCTL, _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______, _______, _______,
   //|--------+--------+--------+--------+--------+--------+------| |--------+--------+--------+--------+--------+--------+---------|
     _______, _______, _______, _______, _______, _______, _______,  _______, _______, _______, _______, _______, _______, _______,
   //|--------+--------+--------+--------+--------+--------+---.   .---+------+--------+--------+--------+--------+--------+---------|
                                _______, MO(LAYER_FN), _______,         _______, MO(LAYER_NAV), _______
  )
};


// ============================================================
// ENCODER MAP
// ============================================================

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [0] = { ENCODER_CCW_CW(KC_RIGHT, KC_LEFT),  ENCODER_CCW_CW(KC_VOLD,  KC_VOLU) },
    [1] = { ENCODER_CCW_CW(RGB_HUD,  RGB_HUI),  ENCODER_CCW_CW(RGB_SAD,  RGB_SAI) },
    [2] = { ENCODER_CCW_CW(RGB_VAD,  RGB_VAI),  ENCODER_CCW_CW(RGB_SPD,  RGB_SPI) },
    [3] = { ENCODER_CCW_CW(RGB_RMOD, RGB_MOD),  ENCODER_CCW_CW(KC_RIGHT, KC_LEFT) },
};
#endif


// ============================================================
// COMBOS
// ============================================================

// J + K simultaneously → Esc  (vim-style insert-mode escape)
const uint16_t PROGMEM combo_jk[] = {KC_J, KC_K, COMBO_END};

combo_t key_combos[COMBO_COUNT] = {
    COMBO(combo_jk, KC_ESC),
};


// ============================================================



static const uint8_t os_indicator_zone[] = {0, 1, 2, 3};
#define OS_INDICATOR_LED_COUNT (sizeof(os_indicator_zone) / sizeof(os_indicator_zone[0]))

// Layer indicator zone: first 4 LEDs of right top row (keys 6,7,8,9).
// Mirrors the OS zone on the left — left corner = OS, right corner = layer.
static const uint8_t layer_indicator_zone[] = {29, 30, 31, 32};
#define LAYER_INDICATOR_LED_COUNT (sizeof(layer_indicator_zone) / sizeof(layer_indicator_zone[0]))

// CAPS key is always at LED 13 (matrix row 2, col 0) regardless of active layer.
// Hardcoded so the Caps Lock indicator works on every layer without cache lookups.
#define CAPS_KEY_LED 13

// Action keys rendered with theme->layer1 (edit this list to change which keys are highlighted)
static const uint16_t action_keys[] = {KC_ENT, KC_BSPC, KC_DEL, KC_SPC};
#define ACTION_KEY_COUNT (sizeof(action_keys) / sizeof(action_keys[0]))

static bool is_os_led(uint8_t led) {
    for (uint8_t i = 0; i < OS_INDICATOR_LED_COUNT; i++) {
        if (os_indicator_zone[i] == led) return true;
    }
    return false;
}

static bool is_layer_led(uint8_t led) {
    for (uint8_t i = 0; i < LAYER_INDICATOR_LED_COUNT; i++) {
        if (layer_indicator_zone[i] == led) return true;
    }
    return false;
}


// ============================================================
// KEYCODE CACHE  (rebuilt on layer change; refreshed on timer)
// ============================================================

#ifdef RGB_MATRIX_LED_COUNT
#  define M57_LED_COUNT RGB_MATRIX_LED_COUNT
#else
#  define M57_LED_COUNT 58
#endif

static uint16_t  led_keycode_cache[M57_LED_COUNT];
static uint32_t  keycode_cache_timer = 0;
#define KEYCODE_CACHE_REFRESH_INTERVAL 100
#define MODIFIER_DIM_DIVISOR           4

#define MAX_MODIFIER_LEDS 16
static uint8_t modifier_leds[MAX_MODIFIER_LEDS];
static uint8_t modifier_led_count = 0;


// ============================================================
// REACTIVE SYSTEM  (keypress fade-out, used in GAMING MODE)
// ============================================================

#define MAX_REACTIVE         8
#define REACTIVE_DECAY_MS    300
#define REACTIVE_SLOT_UNUSED 0xFF

typedef struct {
    uint8_t  led;       // REACTIVE_SLOT_UNUSED (0xFF) = slot inactive
    uint32_t timestamp;
} reactive_event_t;

static reactive_event_t reactive_buffer[MAX_REACTIVE];
static uint8_t          reactive_index = 0;


// ============================================================
// BOOT ANIMATION  (curtain sweep — 4 phases, ~1 second total)
// ============================================================
//
//  Phase 0:  columns light up left → right   (curtain closes from left)
//  Phase 1:  columns go dark  left → right   (curtain opens  to  right)
//  Phase 2:  columns light up right → left   (curtain closes from right)
//  Phase 3:  columns go dark  right → left   (curtain opens  to  left)
//
//  LED x-coordinates in g_led_config.point[] span 0–BOOT_LED_MAX_X.
//  Each phase advances a sweep edge across that range in BOOT_PHASE_DURATION ms.
//  Per-LED cost: one x-lookup + one compare = negligible.

#define BOOT_PHASE_DURATION   250   // ms per phase  (4 × 250 = 1 000 ms total)
#define BOOT_LED_MAX_X        210   // highest x value in g_led_config.point[]
#define BOOT_INDICATOR_BRIGHTNESS 255

static bool     boot_indicator_active = false;
static uint32_t boot_timer            = 0;


// ============================================================
// EEPROM HELPERS
// ============================================================

static void load_config_from_eeprom(void) {
    user_config.raw = eeconfig_read_user();

    if (user_config.os_type >= OS_COUNT) {
        user_config.os_type = OS_WINDOWS;
    }
    if (user_config.theme >= CUSTOM_MODE_COUNT) {
        user_config.theme = CUSTOM_MODE_UI;
    }

    LOG(DEBUG_INFO, "[EEPROM] os=%d theme=%d", user_config.os_type, user_config.theme);
}

static void save_config(void) {
    eeconfig_update_user(user_config.raw);
    LOG(DEBUG_INFO, "[CFG] saved os=%d theme=%d", user_config.os_type, user_config.theme);
}


// ============================================================
// THEME SELECTION
// ============================================================

static const rgb_theme_t* const themes[CUSTOM_MODE_COUNT] = {
    [CUSTOM_MODE_UI]     = &theme_ui,
    [CUSTOM_MODE_DEBUG]  = &theme_debug,
    [CUSTOM_MODE_GAMING] = &theme_gaming,
};

static const rgb_theme_t* get_active_theme(void) {
    if (user_config.theme < CUSTOM_MODE_COUNT) return themes[user_config.theme];
    LOG(DEBUG_WARN, "[THEME] invalid index=%d, fallback UI", user_config.theme);
    return &theme_ui;
}


// ============================================================
// CACHE HELPERS
// ============================================================

/*
 * All key lookups use keymap_key_to_keycode() so Vial remapping
 * is respected — never read keymaps[][] directly.
 */

static inline bool is_modifier_keycode(uint16_t kc) {
    return (kc >= KC_LCTL && kc <= KC_RGUI);
}

static void update_keycode_cache(void) {
    uint8_t layer = get_highest_layer(layer_state);
    LOG(DEBUG_VERBOSE, "[CACHE] update layer=%d", layer);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led != NO_LED) {
                led_keycode_cache[led] =
                    keymap_key_to_keycode(layer, (keypos_t){.row = row, .col = col});
            }
        }
    }
}

static void build_modifier_list(void) {
    modifier_led_count = 0;
    for (uint8_t i = 0; i < M57_LED_COUNT && modifier_led_count < MAX_MODIFIER_LEDS; i++) {
        if (is_modifier_keycode(led_keycode_cache[i])) {
            modifier_leds[modifier_led_count++] = i;
        }
    }
    LOG(DEBUG_INFO, "[MOD] modifier_count=%d", modifier_led_count);
}


// ============================================================
// RENDER PIPELINE
// ============================================================

// Step 1: base color from active theme — OS zone excluded.
static void render_flag_base(uint8_t led_min, uint8_t led_max, const rgb_theme_t* theme, uint8_t brightness) {
    for (uint8_t i = led_min; i < led_max; i++) {
        if (is_os_led(i)) continue;
        apply_color(i, theme->base, brightness);
    }
}

// Step 2: action keys highlighted with theme->layer1 (list defined in action_keys[]).
static void render_key_based(uint8_t led_min, uint8_t led_max, const rgb_theme_t* theme, uint8_t brightness) {
    for (uint8_t i = led_min; i < led_max; i++) {
        if (is_os_led(i)) continue;
        uint16_t kc = led_keycode_cache[i];
        for (uint8_t j = 0; j < ACTION_KEY_COUNT; j++) {
            if (kc == action_keys[j]) {
                apply_color(i, theme->layer1, brightness);
                break;
            }
        }
    }
}

// Step 3: key groups (letters, digits, function keys) with theme->layer2.
static void render_key_groups(uint8_t led_min, uint8_t led_max, const rgb_theme_t* theme, uint8_t brightness) {
    for (uint8_t i = led_min; i < led_max; i++) {
        if (is_os_led(i)) continue;
        uint16_t kc = led_keycode_cache[i];
        if ((kc >= KC_A  && kc <= KC_Z)   ||
            (kc >= KC_1  && kc <= KC_9)   ||
             kc == KC_0                   ||
            (kc >= KC_F1 && kc <= KC_F12)) {
            apply_color(i, theme->layer2, brightness);
        }
    }
}

// Step 4: modifier LEDs — colored by type, dim when idle, bright when held.
static void render_modifier(uint8_t led_min, uint8_t led_max, uint8_t brightness) {
    uint8_t mods = get_mods() | get_oneshot_mods();

    for (uint8_t i = 0; i < modifier_led_count; i++) {
        uint8_t  led = modifier_leds[i];
        if (led < led_min || led >= led_max) continue;
        if (is_os_led(led)) continue;

        uint16_t kc = led_keycode_cache[led];
        rgb_t    color;
        bool     active;

        switch (kc) {
            case KC_LSFT: case KC_RSFT:
                color  = (mods & MOD_MASK_CTRL)  ? COLOR_MOD_SHIFT_CTRL : COLOR_MOD_SHIFT;
                active = (mods & MOD_MASK_SHIFT) != 0; break;
            case KC_LCTL: case KC_RCTL:
                color  = (mods & MOD_MASK_SHIFT) ? COLOR_MOD_SHIFT_CTRL : COLOR_MOD_CTRL;
                active = (mods & MOD_MASK_CTRL)  != 0; break;
            case KC_LALT: case KC_RALT:
                color  = (mods & MOD_MASK_GUI)   ? COLOR_MOD_ALT_GUI : COLOR_MOD_ALT;
                active = (mods & MOD_MASK_ALT)   != 0; break;
            case KC_LGUI: case KC_RGUI:
                color  = (mods & MOD_MASK_ALT)   ? COLOR_MOD_ALT_GUI : COLOR_MOD_GUI;
                active = (mods & MOD_MASK_GUI)   != 0; break;
            default: continue;
        }
        apply_color(led, color, active ? brightness : brightness / MODIFIER_DIM_DIVISOR);
    }
}

// Step 5: OS indicator zone — always painted last in base pipeline.
static void render_os(uint8_t led_min, uint8_t led_max, uint8_t brightness) {
    rgb_t color;
    switch ((os_type_t)user_config.os_type) {
        case OS_LINUX:   color = COLOR_OS_LINUX;   break;
        case OS_WINDOWS: color = COLOR_OS_WINDOWS; break;
        case OS_MAC:     color = COLOR_OS_MAC;     break;
        case OS_ANDROID: color = COLOR_OS_ANDROID; break;
        default:         color = COLOR_OFF;        break;
    }
    for (uint8_t i = 0; i < OS_INDICATOR_LED_COUNT; i++) {
        uint8_t led = os_indicator_zone[i];
        if (led >= led_min && led < led_max) {
            apply_color(led, color, brightness);
        }
    }
}

// Step 6: Layer indicator zone (right top row, LEDs 29-32).
// Base layer: zone shows theme base color (render_flag_base already painted it — no override needed).
// Any other layer: overrides with a unique layer color.
static void render_layer(uint8_t led_min, uint8_t led_max, uint8_t brightness) {
    uint8_t layer = get_highest_layer(layer_state);
    rgb_t color;
    switch (layer) {
        case LAYER_FN:   color = COLOR_LAYER_FN;   break;
        case LAYER_NAV:  color = COLOR_LAYER_NAV;  break;
        case LAYER_GAME: color = COLOR_LAYER_GAME; break;
        default: return; // LAYER_BASE: leave as theme base (no override)
    }
    for (uint8_t i = 0; i < LAYER_INDICATOR_LED_COUNT; i++) {
        uint8_t led = layer_indicator_zone[i];
        if (led >= led_min && led < led_max) {
            apply_color(led, color, brightness);
        }
    }
}

// Step 7: Lock state indicators — painted last so they are always visible.
// Caps Lock: LED 13 (hardcoded, always present on every layer).
// Num / Scroll Lock: searched in the current keycode cache (only visible on layers
// where the key is explicitly assigned, e.g. FN layer).
static void render_lock_states(uint8_t led_min, uint8_t led_max, uint8_t brightness) {
    led_t lock = host_keyboard_led_state();

    if (lock.caps_lock && CAPS_KEY_LED >= led_min && CAPS_KEY_LED < led_max) {
        apply_color(CAPS_KEY_LED, COLOR_LOCK_CAPS, brightness);
    }

    if (lock.num_lock || lock.scroll_lock) {
        for (uint8_t i = led_min; i < led_max; i++) {
            uint16_t kc = led_keycode_cache[i];
            if (lock.num_lock    && kc == KC_NUM)  apply_color(i, COLOR_LOCK_NUM,  brightness);
            if (lock.scroll_lock && kc == KC_SCRL) apply_color(i, COLOR_LOCK_SCRL, brightness);
        }
    }
}

// Step 8 (GAMING MODE only): reactive fade-out on keypress.
static void render_event(uint8_t led_min, uint8_t led_max, const rgb_theme_t* theme, uint8_t brightness) {
    for (uint8_t i = 0; i < MAX_REACTIVE; i++) {
        if (reactive_buffer[i].led == REACTIVE_SLOT_UNUSED) continue;

        uint32_t elapsed = timer_elapsed32(reactive_buffer[i].timestamp);
        if (elapsed >= REACTIVE_DECAY_MS) {
            reactive_buffer[i].led = REACTIVE_SLOT_UNUSED;
            continue;
        }

        uint8_t led = reactive_buffer[i].led;
        if (led < led_min || led >= led_max) continue;
        if (is_os_led(led) || is_layer_led(led)) continue;

        uint8_t intensity = (uint8_t)(
            ((uint32_t)(REACTIVE_DECAY_MS - elapsed) * brightness) / REACTIVE_DECAY_MS
        );
        apply_color(led, theme->reactive, intensity);
    }
}


// ============================================================
// CUSTOM RGB EFFECT STUBS  (canvas clear — indicators hook renders)
// ============================================================

#ifdef RGB_MATRIX_CUSTOM_USER

bool FIRMWARE_UI(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) apply_color(i, COLOR_OFF, 255);
    return led_max < RGB_MATRIX_LED_COUNT;
}

bool DEBUG_MODE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) apply_color(i, COLOR_OFF, 255);
    return led_max < RGB_MATRIX_LED_COUNT;
}

bool GAMING_MODE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    for (uint8_t i = led_min; i < led_max; i++) apply_color(i, COLOR_OFF, 255);
    return led_max < RGB_MATRIX_LED_COUNT;
}

#endif


// ============================================================
// INPUT HOOKS
// ============================================================

bool encoder_update_user(uint8_t index, bool clockwise) {
    uint8_t layer = get_highest_layer(layer_state);

    // In GAMING_MODE, do not intercept FN+encoder — let encoder_map handle
    // RGB controls (HUD/HUI etc.) so gaming is not accidentally disrupted.
    uint8_t rgb_mode = rgb_matrix_get_mode();
    bool in_gaming_mode = (rgb_mode >= RGB_MATRIX_CUSTOM) &&
                          ((custom_mode_t)(rgb_mode - RGB_MATRIX_CUSTOM) == CUSTOM_MODE_GAMING);

    if (layer == LAYER_FN && !in_gaming_mode) {
        if (index == 0) {
            // Left encoder on FN: cycle theme
            if (clockwise) {
                user_config.theme = (user_config.theme + 1) % CUSTOM_MODE_COUNT;
            } else {
                user_config.theme = (user_config.theme == 0)
                    ? CUSTOM_MODE_COUNT - 1
                    : user_config.theme - 1;
            }
            LOG(DEBUG_INFO, "[ENC] THEME=%d", user_config.theme);
        } else {
            // Right encoder on FN: cycle OS
            // (right hand operates right encoder; left hand holds FN — ergonomic two-hand operation)
            if (clockwise) {
                user_config.os_type = (user_config.os_type + 1) % OS_COUNT;
            } else {
                user_config.os_type = (user_config.os_type == 0)
                    ? OS_COUNT - 1
                    : user_config.os_type - 1;
            }
            LOG(DEBUG_INFO, "[ENC] OS=%d", user_config.os_type);
        }
        save_config();
        return false;
    }
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return true;

    switch (keycode) {
        case OS_LINUX_SET:
            user_config.os_type = OS_LINUX;
            save_config();
            LOG(DEBUG_INFO, "[KEY] OS=LINUX");
            return false;
        case OS_WINDOWS_SET:
            user_config.os_type = OS_WINDOWS;
            save_config();
            LOG(DEBUG_INFO, "[KEY] OS=WINDOWS");
            return false;
        case OS_MAC_SET:
            user_config.os_type = OS_MAC;
            save_config();
            LOG(DEBUG_INFO, "[KEY] OS=MAC");
            return false;
        case OS_ANDROID_SET:
            user_config.os_type = OS_ANDROID;
            save_config();
            LOG(DEBUG_INFO, "[KEY] OS=ANDROID");
            return false;

        case THEME_NEXT:
            user_config.theme = (user_config.theme + 1) % CUSTOM_MODE_COUNT;
            save_config();
            LOG(DEBUG_INFO, "[KEY] THEME_NEXT=%d", user_config.theme);
            return false;
        case THEME_PREV:
            user_config.theme = (user_config.theme == 0)
                ? CUSTOM_MODE_COUNT - 1
                : user_config.theme - 1;
            save_config();
            LOG(DEBUG_INFO, "[KEY] THEME_PREV=%d", user_config.theme);
            return false;
    }

    // Record keypress LED for reactive system — GAMING MODE only.
    // Avoids filling the reactive buffer with events that are never rendered.
    uint8_t cur_mode = rgb_matrix_get_mode();
    bool is_gaming_active = (cur_mode >= RGB_MATRIX_CUSTOM) &&
                            ((custom_mode_t)(cur_mode - RGB_MATRIX_CUSTOM) == CUSTOM_MODE_GAMING);
    if (is_gaming_active) {
        uint8_t row = record->event.key.row;
        uint8_t col = record->event.key.col;
        uint8_t led = g_led_config.matrix_co[row][col];
        if (led != NO_LED) {
            LOG(DEBUG_VERBOSE, "[REACT] LED=%d", led);
            reactive_buffer[reactive_index].led       = led;
            reactive_buffer[reactive_index].timestamp = timer_read32();
            reactive_index = (reactive_index + 1) % MAX_REACTIVE;
        }
    }

    return true;
}


// ============================================================
// QMK HOOKS
// ============================================================

void keyboard_post_init_user(void) {
    load_config_from_eeprom();

    for (uint8_t i = 0; i < MAX_REACTIVE; i++) {
        reactive_buffer[i].led = REACTIVE_SLOT_UNUSED;
    }

    update_keycode_cache();
    build_modifier_list();

    boot_indicator_active = true;
    boot_timer            = timer_read32();

    LOG(DEBUG_INFO, "[INIT] leds=%u mods=%u os=%d theme=%d",
        (uint8_t)M57_LED_COUNT, modifier_led_count,
        user_config.os_type, user_config.theme);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    update_keycode_cache();
    build_modifier_list();

    // Auto-switch RGB to GAMING_MODE when GAME layer is toggled on, restore on toggle off.
    static bool    was_gaming       = false;
    static uint8_t pre_game_rgb_mode = 0;

    bool is_gaming = (state & ((layer_state_t)1 << LAYER_GAME)) != 0;
    if (is_gaming && !was_gaming) {
        pre_game_rgb_mode = rgb_matrix_get_mode();
        rgb_matrix_mode(RGB_MATRIX_CUSTOM + CUSTOM_MODE_GAMING);
        LOG(DEBUG_INFO, "[LAYER] GAME on, rgb saved=%d", pre_game_rgb_mode);
    } else if (!is_gaming && was_gaming) {
        rgb_matrix_mode(pre_game_rgb_mode);
        LOG(DEBUG_INFO, "[LAYER] GAME off, rgb restored=%d", pre_game_rgb_mode);
    }
    was_gaming = is_gaming;

    LOG(DEBUG_VERBOSE, "[LAYER] active=%u", get_highest_layer(state));
    return state;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    // Boot curtain animation: 4-phase column sweep (see BOOT ANIMATION section above)
    if (boot_indicator_active) {
        uint32_t elapsed = timer_elapsed32(boot_timer);

        if (elapsed >= (uint32_t)BOOT_PHASE_DURATION * 4) {
            boot_indicator_active = false;
        } else {
            uint8_t  phase   = (uint8_t)(elapsed / BOOT_PHASE_DURATION);
            uint32_t phase_t = elapsed - (uint32_t)phase * BOOT_PHASE_DURATION;

            // sweep: 0 → BOOT_LED_MAX_X over BOOT_PHASE_DURATION ms
            uint16_t sweep = (uint16_t)(((uint32_t)phase_t * BOOT_LED_MAX_X) / BOOT_PHASE_DURATION);

            for (uint8_t i = led_min; i < led_max; i++) {
                uint8_t lx = g_led_config.point[i].x;
                bool lit;
                switch (phase) {
                    case 0: lit = (lx <= sweep);                    break; // L→R on
                    case 1: lit = (lx >  sweep);                    break; // L→R off
                    case 2: lit = (lx >= (BOOT_LED_MAX_X - sweep)); break; // R→L on
                    case 3: lit = (lx <  (BOOT_LED_MAX_X - sweep)); break; // R→L off
                    default: lit = false;                           break;
                }
                apply_color(i, lit ? COLOR_WHITE : COLOR_OFF, BOOT_INDICATOR_BRIGHTNESS);
            }
            return false;
        }
    }

    // Only run custom pipeline in custom RGB modes; let QMK handle the rest.
    uint8_t mode = rgb_matrix_get_mode();
    if (mode < RGB_MATRIX_CUSTOM) return false;

    current_custom_mode = (custom_mode_t)(mode - RGB_MATRIX_CUSTOM);

    // Periodic cache refresh (catches Vial remaps applied at runtime)
    if (timer_elapsed32(keycode_cache_timer) > KEYCODE_CACHE_REFRESH_INTERVAL) {
        update_keycode_cache();
        build_modifier_list();
        keycode_cache_timer = timer_read32();
    }

    const rgb_theme_t* theme     = get_active_theme();
    uint8_t            brightness = rgb_matrix_get_val();

    render_flag_base  (led_min, led_max, theme, brightness);
    render_key_based  (led_min, led_max, theme, brightness);
    render_key_groups (led_min, led_max, theme, brightness);
    render_modifier   (led_min, led_max,        brightness);
    render_os         (led_min, led_max,        brightness);
    render_layer      (led_min, led_max,        brightness);  // layer indicator zone (right top row)
    render_lock_states(led_min, led_max,        brightness);  // Caps/Num/Scroll Lock LEDs

    if (current_custom_mode == CUSTOM_MODE_GAMING) {
        render_event(led_min, led_max, theme, brightness);
    }

    return false;
}