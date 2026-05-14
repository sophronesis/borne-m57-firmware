#pragma once

#include "quantum.h"

// Centralized RGB color system for M57 firmware.
// All colors defined here; keymap.c and future modules use these constants only.

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

// --- OS indicator colors ---
#define COLOR_OS_LINUX    ((rgb_t){  0,  80, 200}) // cyan-blue: Linux
#define COLOR_OS_WINDOWS  ((rgb_t){  0, 120, 255}) // bright blue: Windows
#define COLOR_OS_MAC      ((rgb_t){180, 180, 180}) // silver: macOS
#define COLOR_OS_ANDROID  ((rgb_t){  0, 150,   0}) // green: Android

// --- Modifier key colors (shown when modifier is active) ---
#define COLOR_MOD_SHIFT       ((rgb_t){255,   0,   0}) // red: Shift
#define COLOR_MOD_CTRL        ((rgb_t){255, 120,   0}) // orange: Ctrl
#define COLOR_MOD_ALT         ((rgb_t){255, 200,   0}) // yellow: Alt
#define COLOR_MOD_GUI         ((rgb_t){200,   0, 200}) // magenta: GUI
#define COLOR_MOD_SHIFT_CTRL  ((rgb_t){255, 255,   0}) // yellow: Shift+Ctrl combo
#define COLOR_MOD_ALT_GUI     ((rgb_t){255,   0, 255}) // magenta: Alt+GUI combo

// --- Persist state colors ---
#define COLOR_PERSIST_ON   ((rgb_t){  0, 200,   0}) // green: persistent layer ON
#define COLOR_PERSIST_OFF  ((rgb_t){200,   0,   0}) // red: persistent layer OFF

// --- Layer / mode colors ---
#define COLOR_MODE_STATIC ((rgb_t){  0,   0, 200}) // blue: static RGB mode
#define COLOR_MODE_ANIM   ((rgb_t){  0, 200, 100}) // green: animation mode
#define COLOR_SETTINGS    ((rgb_t){ 80, 200,  80}) // green: settings layer active

// --- Base and utility colors ---
#define COLOR_BASE        ((rgb_t){  0,   0,  60}) // dim blue: default keylight
#define COLOR_WHITE       ((rgb_t){255, 255, 255}) // white: boot indicator
#define COLOR_OFF         ((rgb_t){  0,   0,   0}) // off / unlit

// Apply color to a single LED, scaled by brightness (0–255).
static inline void apply_color(uint8_t led_index, rgb_t color, uint8_t brightness) {
    rgb_matrix_set_color(
        led_index,
        (uint8_t)(((uint16_t)color.r * brightness) >> 8),
        (uint8_t)(((uint16_t)color.g * brightness) >> 8),
        (uint8_t)(((uint16_t)color.b * brightness) >> 8)
    );
}
