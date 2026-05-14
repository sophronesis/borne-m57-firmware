#include QMK_KEYBOARD_H
#include "theme.h"
#include "eeconfig.h"
#include <avr/eeprom.h>


// ============================================================
// CUSTOM MODE SYSTEM DESIGN
// ============================================================

/*
============================================================
CUSTOM RGB MODE SYSTEM
============================================================

Maps QMK RGB modes → firmware-defined behavior modes.

These modes determine WHICH rendering layers are active.

Modes:
- UI      → structured / informative display
- DEBUG   → diagnostic visualization (future use)
- GAMING  → enables reactive animation layer

IMPORTANT:
Mode selection does NOT change theme directly.
Theme and mode are intentionally decoupled systems.
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

/*
Logical mode identifiers.
Order MUST match rgb_matrix_name_for_cust[].
*/
typedef enum {
    CUSTOM_MODE_UI,
    CUSTOM_MODE_DEBUG,
    CUSTOM_MODE_GAMING,
    CUSTOM_MODE_COUNT
} custom_mode_t;

/*
Derived every frame from QMK RGB mode.
Represents current visualization strategy.
*/
static uint8_t current_custom_mode = CUSTOM_MODE_UI;


// ============================================================
// OS STATE SYSTEM (EEPROM)
// ============================================================

/*
============================================================
OS STATE SYSTEM
============================================================

Maintains current operating system selection.

Used for:
✔ visual indication (RGB)
✔ potential OS-specific behavior

Persistence:
Stored in EEPROM so it survives reboot.

Single source of truth:
    current_os
*/

typedef enum {
    OS_LINUX,
    OS_WINDOWS,
    OS_MAC,
    OS_ANDROID,
    OS_COUNT
} os_type_t;

static os_type_t current_os = OS_WINDOWS;

/*
EEPROM USER SPACE LAYOUT:

Offset +0 → OS
Offset +1 → Theme

Using EECONFIG_USER ensures no collision with QMK internals.
*/
#define EEPROM_OS_ADDR (uint8_t*)(EECONFIG_USER + 0)

/*
Loads OS from EEPROM.

Includes safety check:
If EEPROM contains invalid value → fallback to Windows.
*/
static void load_os_from_eeprom(void) {
    uint8_t stored = eeprom_read_byte(EEPROM_OS_ADDR);
    current_os = (stored < OS_COUNT) ? stored : OS_WINDOWS;
}

/*
Stores OS to EEPROM.

Uses update (not write):
→ avoids unnecessary EEPROM wear
*/
static void save_os_to_eeprom(void) {
    eeprom_update_byte(EEPROM_OS_ADDR, current_os);
}


// ============================================================
// THEME STATE SYSTEM (EEPROM)
// ============================================================

/*
============================================================
THEME SYSTEM (PERSISTENT)
============================================================

Defines which color palette is active.

Themes affect:
✔ base color
✔ reactive color
✔ overlay colors

IMPORTANT:
Theme is global and affects ALL render layers.

Decoupled from mode:
→ Mode = behavior
→ Theme = appearance
*/

static uint8_t current_theme_index = CUSTOM_MODE_UI;

#define EEPROM_THEME_ADDR (uint8_t*)(EECONFIG_USER + 1)

/*
Load theme safely from EEPROM.
*/
static void load_theme_from_eeprom(void) {
    uint8_t stored = eeprom_read_byte(EEPROM_THEME_ADDR);
    current_theme_index =
        (stored < CUSTOM_MODE_COUNT) ? stored : CUSTOM_MODE_UI;
}

/*
Persist theme change.
*/
static void save_theme_to_eeprom(void) {
    eeprom_update_byte(EEPROM_THEME_ADDR, current_theme_index);
}


// ============================================================
// THEME SELECTION
// ============================================================

/*
Central theme resolution.

ALL rendering goes through this function.

Changing this affects:
✔ base layer
✔ reactive effects
✔ any color-driven logic

Provides abstraction layer between logic and visuals.
*/
static const rgb_theme_t* get_active_theme(void) {

    switch (current_theme_index) {
        case CUSTOM_MODE_UI:     return &theme_ui;
        case CUSTOM_MODE_DEBUG:  return &theme_debug;
        case CUSTOM_MODE_GAMING: return &theme_gaming;
    }

    return &theme_ui; // Safety fallback
}


// ============================================================
// OS + THEME KEYCODES
// ============================================================

/*
============================================================
CUSTOM CONTROL KEYCODES
============================================================

Extends QMK input system with firmware-specific actions.

These keycodes:
✔ modify persistent state
✔ integrate with Vial
✔ complement encoder-based control

SAFE_RANGE ensures no overlap with standard QMK keycodes.
*/
enum custom_keycodes {
    OS_LINUX_SET = SAFE_RANGE,
    OS_WINDOWS_SET,
    OS_MAC_SET,
    OS_ANDROID_SET,

    THEME_NEXT,
    THEME_PREV
};


// ============================================================
// OS INDICATOR LED ZONE
// ============================================================

/*
Defines which LEDs represent OS state.

This is a "reserved zone":
→ ONLY render_os() is allowed to modify these LEDs.

All other layers must explicitly skip them.
*/
static const uint8_t os_indicator_zone[] = {0,1,2,3};

#define OS_INDICATOR_LED_COUNT (sizeof(os_indicator_zone))


// ============================================================
// LED OWNERSHIP HELPERS
// ============================================================

/*
============================================================
LED OWNERSHIP SYSTEM
============================================================

Ensures that certain LEDs are controlled ONLY by specific layers.

This prevents visual conflicts such as:
❌ reactive effects overwriting OS indicator
❌ modifiers hiding system state

Current ownership:
✔ OS LEDs → only render_os()

Future extension:
✔ modifier / encoder / debug zones
*/

/*
Returns true if LED belongs to OS indicator zone.
*/
static bool is_os_led(uint8_t led) {
    for (uint8_t i = 0; i < OS_INDICATOR_LED_COUNT; i++) {
        if (os_indicator_zone[i] == led)
            return true;
    }
    return false;
}


// ============================================================
// KEYCODE CACHE SYSTEM
// ============================================================

/*
============================================================
KEYCODE CACHE
============================================================

Maps:
    LED → keycode

Purpose:
✔ avoid expensive lookups per frame
✔ enable fast key-based rendering

Updated:
✔ periodically (timer)
✔ on layer changes

Tradeoff:
freshness vs CPU usage
*/

static uint16_t led_keycode_cache[DRIVER_LED_TOTAL];

/*
List of LEDs flagged as modifiers.

Precomputed once → improves performance.
*/
static uint8_t modifier_leds[DRIVER_LED_TOTAL];
static uint8_t modifier_count = 0;

/*
Timer controlling cache refresh.
*/
static uint32_t keycode_cache_timer = 0;

#define KEYCODE_CACHE_REFRESH_INTERVAL 100


/*
Rebuilds LED → keycode mapping.

Important:
Skips NO_LED entries to avoid invalid mappings.
*/
static void update_keycode_cache(void) {

    uint8_t layer = get_highest_layer(layer_state);

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {

        uint8_t row = g_led_config.matrix_co[i][0];
        if (row == NO_LED) continue;

        uint8_t col = g_led_config.matrix_co[i][1];

        led_keycode_cache[i] =
            keymap_key_to_keycode(layer, (keypos_t){row,col});
    }
}


/*
Builds modifier LED list once at init.

Avoids scanning flags every frame.
*/
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

/*
============================================================
REACTIVE EFFECT BUFFER
============================================================

Captures recent key presses and drives animations.

Design:
✔ circular buffer (fixed size)
✔ time-based decay
✔ per-frame rendering

Each entry:
- LED(s)
- timestamp
- count
*/

#define MAX_REACTIVE 8

typedef struct {
    uint8_t leds[4];   // LEDs affected
    uint8_t count;     // number of LEDs used
    uint16_t timer;    // timestamp of event
} reactive_cluster_t;

static reactive_cluster_t reactive_clusters[MAX_REACTIVE];
static uint8_t reactive_index = 0;


// ============================================================
// ENCODER OS SWITCHING
// ============================================================

/*
Handles encoder rotation.

Only active on FN layer (layer 1).

Clockwise / Counterclockwise:
→ cycles OS forward/backward

Important:
return false → suppress default encoder behavior
*/
bool encoder_update_user(uint8_t index, bool clockwise) {

    if (get_highest_layer(layer_state) == 1) {

        if (clockwise)
            current_os = (current_os + 1) % OS_COUNT;
        else
            current_os = (current_os == 0)
                ? OS_COUNT - 1
                : current_os - 1;

        save_os_to_eeprom();
        return false;
    }

    return true;
}


// ============================================================
// PROCESS RECORD USER
// ============================================================

/*
Central input handler.

Handles:
✔ OS switching via keycodes
✔ theme switching
✔ reactive event creation

This function bridges INPUT → STATE → VISUAL EFFECTS.
*/
bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!record->event.pressed)
        return true;

    // ---------- OS ----------
    switch (keycode) {
        case OS_WINDOWS_SET: current_os = OS_WINDOWS; break;
        case OS_MAC_SET:     current_os = OS_MAC; break;
        case OS_LINUX_SET:   current_os = OS_LINUX; break;
        case OS_ANDROID_SET: current_os = OS_ANDROID; break;
    }

    if (keycode >= OS_LINUX_SET && keycode <= OS_ANDROID_SET) {
        save_os_to_eeprom();
        return false;
    }

    // ---------- THEME ----------
    switch (keycode) {

        case THEME_NEXT:
            current_theme_index =
                (current_theme_index + 1) % CUSTOM_MODE_COUNT;
            save_theme_to_eeprom();
            return false;

        case THEME_PREV:
            current_theme_index =
                (current_theme_index == 0)
                ? CUSTOM_MODE_COUNT - 1
                : current_theme_index - 1;
            save_theme_to_eeprom();
            return false;
    }

    // ---------- REACTIVE ----------
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {

        if (g_led_config.matrix_co[i][0] == row &&
            g_led_config.matrix_co[i][1] == col) {

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
// RENDER LAYERS
// ============================================================

/*
Rendering is layered.

Order defines priority:
later layers override earlier ones.

Ownership rules apply:
OS LEDs are protected from all other layers.
*/

static void render_flag_base(uint8_t brightness) {

    const rgb_theme_t* theme = get_active_theme();

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {

        if (is_os_led(i)) continue;

        apply_color(i, theme->base, brightness);
    }
}


static void render_key_based(uint8_t brightness) {

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {

        if (is_os_led(i)) continue;

        uint16_t kc = led_keycode_cache[i];
        if (kc == KC_NO) continue;

        if (kc == KC_ENT)
            apply_color(i, COLOR_MOD_SHIFT, brightness);
    }
}


static void render_key_groups(uint8_t brightness) {

    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {

        if (is_os_led(i)) continue;

        uint16_t kc = led_keycode_cache[i];
        if (kc == KC_NO) continue;

        if (kc >= KC_A && kc <= KC_Z)
            apply_color(i, COLOR_MOD_ALT, brightness);

        else if ((kc >= KC_1 && kc <= KC_9) || kc == KC_0)
            apply_color(i, COLOR_MOD_CTRL, brightness);

        else if (kc >= KC_F1 && kc <= KC_F12)
            apply_color(i, COLOR_MOD_GUI, brightness);
    }
}


static void render_modifier(uint8_t brightness) {

    uint8_t mods = get_mods();

    for (uint8_t i = 0; i < modifier_count; i++) {

        uint8_t led = modifier_leds[i];

        if (is_os_led(led)) continue;

        if (mods & MOD_MASK_SHIFT) apply_color(led, COLOR_MOD_SHIFT, brightness);
        if (mods & MOD_MASK_CTRL)  apply_color(led, COLOR_MOD_CTRL, brightness);
        if (mods & MOD_MASK_ALT)   apply_color(led, COLOR_MOD_ALT, brightness);
        if (mods & MOD_MASK_GUI)   apply_color(led, COLOR_MOD_GUI, brightness);
    }
}


static void render_os(uint8_t brightness) {

    rgb_t color = COLOR_OS_WINDOWS;

    switch (current_os) {
        case OS_LINUX:   color = COLOR_OS_LINUX; break;
        case OS_MAC:     color = COLOR_OS_MAC; break;
        case OS_ANDROID: color = COLOR_OS_ANDROID; break;
    }

    for (uint8_t i = 0; i < OS_INDICATOR_LED_COUNT; i++) {
        apply_color(os_indicator_zone[i], color, brightness);
    }
}


static void render_event(uint8_t brightness) {

    const rgb_theme_t* theme = get_active_theme();

    for (uint8_t i = 0; i < MAX_REACTIVE; i++) {

        uint16_t t = timer_elapsed(reactive_clusters[i].timer);
        if (t > 300) continue;

        uint8_t intensity = 255 - (t * 255 / 300);

        for (uint8_t j = 0; j < reactive_clusters[i].count; j++) {

            uint8_t led = reactive_clusters[i].leds[j];

            if (is_os_led(led)) continue;

            apply_color(
                led,
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

/*
Executed every frame.

Flow:
BASE → KEY → GROUP → MOD → OS → EVENT

Key properties:
✔ deterministic layering
✔ protected zones
✔ efficient execution
*/
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

/*
Initialization lifecycle:
1. Build static lookup data
2. Build cache
3. Load persistent state
*/
void keyboard_post_init_user(void) {

    build_modifier_list();
    update_keycode_cache();

    load_os_from_eeprom();
    load_theme_from_eeprom();
}

layer_state_t layer_state_set_user(layer_state_t state) {

    update_keycode_cache();
    return state;
}