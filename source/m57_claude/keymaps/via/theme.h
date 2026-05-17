#pragma once
#include "quantum.h"

/* ============================================================
 * rgb_t — lightweight color struct used throughout this keymap.
 * All color values live here; keymap.c never contains raw {r,g,b}.
 * ============================================================ */

typedef struct { uint8_t r, g, b; } rgb_t;

/* Helper: set one LED with per-channel brightness scaling.
 * Uses >>8 (÷256) rather than ÷255 — 0.4% error at max, imperceptible,
 * but avoids a division instruction on ARM Cortex-M4. */
static inline void apply_color(uint8_t led, rgb_t c, uint8_t brightness) {
    rgb_matrix_set_color(led,
        (uint8_t)(((uint16_t)c.r * brightness) >> 8),
        (uint8_t)(((uint16_t)c.g * brightness) >> 8),
        (uint8_t)(((uint16_t)c.b * brightness) >> 8));
}

/* Convenience macro for color literals */
#define RGB_COLOR(r, g, b) ((rgb_t){(r), (g), (b)})

/* ============================================================
 * OS indicator colors  (painted on os_led_zone, LEDs 0–3)
 * ============================================================ */
#define COLOR_OS_LINUX   RGB_COLOR(  0,  80, 200)
#define COLOR_OS_WINDOWS RGB_COLOR(  0, 120, 255)
#define COLOR_OS_MAC     RGB_COLOR(180, 180, 180)
#define COLOR_OS_ANDROID RGB_COLOR(  0, 150,   0)

/* ============================================================
 * Layer indicator colors  (painted on layer_led_zone, LEDs 29–32)
 * ============================================================ */
#define COLOR_LAYER_FN   RGB_COLOR(255, 100,   0)
#define COLOR_LAYER_NAV  RGB_COLOR(150,   0, 255)
#define COLOR_LAYER_GAME RGB_COLOR(  0, 255,   0)

/* ============================================================
 * Modifier colors  (idle = color at brightness / MOD_DIM_FACTOR)
 * ============================================================ */
#define COLOR_MOD_SHIFT      RGB_COLOR(255,   0,   0)
#define COLOR_MOD_CTRL       RGB_COLOR(255, 120,   0)
#define COLOR_MOD_ALT        RGB_COLOR(255, 200,   0)
#define COLOR_MOD_GUI        RGB_COLOR(200,   0, 200)
#define COLOR_MOD_SHIFT_CTRL RGB_COLOR(255, 255,   0)  /* blend shown on both when Shift+Ctrl held */
#define COLOR_MOD_ALT_GUI    RGB_COLOR(255,   0, 255)  /* blend shown on both when Alt+GUI held */

/* ============================================================
 * Lock indicator colors  (Caps/CapsWord: LED_IDX_CAPS=13; Num/Scroll: host_keyboard_led_state)
 * ============================================================ */
#define COLOR_LOCK_CAPS  RGB_COLOR(255,   0,   0)
#define COLOR_CAPS_WORD  RGB_COLOR(255, 165,   0)  /* orange — distinct from solid Caps Lock red */
#define COLOR_LOCK_NUM   RGB_COLOR(  0, 200, 100)
#define COLOR_LOCK_SCRL  RGB_COLOR(200, 100,   0)

/* ============================================================
 * Utility colors
 * ============================================================ */
#define COLOR_WHITE      RGB_COLOR(255, 255, 255)  /* boot animation lit phase */
#define COLOR_OFF        RGB_COLOR(  0,   0,   0)  /* boot animation dark phase; canvas clear */
#define COLOR_PERSIST_ON RGB_COLOR(  0, 200,   0)  /* reserved — not currently used */
#define COLOR_PERSIST_OFF RGB_COLOR(200,  0,   0)  /* reserved — not currently used */

/* ============================================================
 * rgb_theme_t — per-UI-mode color profile.
 * Fields:
 *   base     — background for all non-indicator LEDs
 *   accent   — action keys: Enter, Backspace, Delete, Space
 *   body     — letters A-Z, digits 0-9, F1-F12
 *   reactive — keypress fade-out color (GAMING_MODE only)
 * ============================================================ */
typedef struct {
    rgb_t base;
    rgb_t accent;
    rgb_t body;
    rgb_t reactive;
} rgb_theme_t;

/* UI_MODE_DEFAULT — calm blue scheme */
static const rgb_theme_t theme_ui = {
    .base     = RGB_COLOR(  0,  40, 120),
    .accent   = RGB_COLOR(  0,   0, 255),
    .body     = RGB_COLOR(255,   0, 255),
    .reactive = RGB_COLOR(255, 200,   0),
};

/* UI_MODE_DEBUG — red diagnostic scheme */
static const rgb_theme_t theme_debug = {
    .base     = RGB_COLOR(120,   0,   0),
    .accent   = RGB_COLOR(255,   0,   0),
    .body     = RGB_COLOR(255, 100, 100),
    .reactive = RGB_COLOR(255, 255, 255),
};

/* UI_MODE_GAMING — green gaming scheme */
static const rgb_theme_t theme_gaming = {
    .base     = RGB_COLOR(  0, 120,   0),
    .accent   = RGB_COLOR(  0, 255,   0),
    .body     = RGB_COLOR(255, 255,   0),
    .reactive = RGB_COLOR(255, 100,   0),
};
