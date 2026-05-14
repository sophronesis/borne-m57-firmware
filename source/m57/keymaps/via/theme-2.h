#pragma once

#include "quantum.h"

/*
============================================================
CENTRAL RGB COLOR SYSTEM (M57 FIRMWARE)
============================================================

Single source of truth for ALL color definitions.

Provides:

✔ reusable color constants (COLOR_*)
✔ theme profiles (rgb_theme_t)
✔ utility function (apply_color)

Design goals:

- no duplicated RGB literals
- semantic color naming
- full backward compatibility
*/


typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;


// ============================================================
// --- OS INDICATOR COLORS ---
// ============================================================

#define COLOR_OS_LINUX    ((rgb_t){  0,  80, 200})
#define COLOR_OS_WINDOWS  ((rgb_t){  0, 120, 255})
#define COLOR_OS_MAC      ((rgb_t){180, 180, 180})
#define COLOR_OS_ANDROID  ((rgb_t){  0, 150,   0})


// ============================================================
// --- MODIFIER COLORS ---
// ============================================================

#define COLOR_MOD_SHIFT       ((rgb_t){255,   0,   0})
#define COLOR_MOD_CTRL        ((rgb_t){255, 120,   0})
#define COLOR_MOD_ALT         ((rgb_t){255, 200,   0})
#define COLOR_MOD_GUI         ((rgb_t){200,   0, 200})

#define COLOR_MOD_SHIFT_CTRL  ((rgb_t){255, 255,   0})
#define COLOR_MOD_ALT_GUI     ((rgb_t){255,   0, 255})


// ============================================================
// --- PERSIST STATE COLORS ---
// ============================================================

#define COLOR_PERSIST_ON   ((rgb_t){  0, 200,   0})
#define COLOR_PERSIST_OFF  ((rgb_t){200,   0,   0})


// ============================================================
// --- BASE / UTILITY COLORS ---
// ============================================================

#define COLOR_BASE         ((rgb_t){  0,   0,  60})
#define COLOR_WHITE        ((rgb_t){255, 255, 255})
#define COLOR_OFF          ((rgb_t){  0,   0,   0})


// ============================================================
// --- THEME COLOR DEFINITIONS (NEW)
// ============================================================

/*
============================================================
THEME COLOR DEFINITIONS
============================================================

These define colors used in theme profiles.

All themes MUST reference these constants.
No raw RGB values allowed inside themes.
*/

// --- UI THEME COLORS ---
#define COLOR_BASE_UI        ((rgb_t){  0,  40, 120})
#define COLOR_LAYER1_UI      ((rgb_t){  0,   0, 255})
#define COLOR_LAYER2_UI      ((rgb_t){255,   0, 255})
#define COLOR_REACTIVE_UI    ((rgb_t){255, 200,   0})

// --- DEBUG THEME COLORS ---
#define COLOR_BASE_DEBUG     ((rgb_t){120,   0,   0})
#define COLOR_LAYER1_DEBUG   ((rgb_t){255,   0,   0})
#define COLOR_LAYER2_DEBUG   ((rgb_t){255, 100, 100})
#define COLOR_REACTIVE_DEBUG ((rgb_t){255, 255, 255})

// --- GAMING THEME COLORS ---
#define COLOR_BASE_GAMING     ((rgb_t){  0, 120,   0})
#define COLOR_LAYER1_GAMING   ((rgb_t){  0, 255,   0})
#define COLOR_LAYER2_GAMING   ((rgb_t){255, 255,   0})
#define COLOR_REACTIVE_GAMING ((rgb_t){255, 100,   0})


// ============================================================
// THEME SYSTEM (PROFILE-BASED RGB)
// ============================================================

typedef struct {
    rgb_t base;
    rgb_t layer1;
    rgb_t layer2;
    rgb_t reactive;
} rgb_theme_t;


// ============================================================
// --- THEME DEFINITIONS (PROFILES)
// ============================================================

/*
------------------------------------------------------------
UI THEME
------------------------------------------------------------
*/

static const rgb_theme_t theme_ui = {
    .base     = COLOR_BASE_UI,
    .layer1   = COLOR_LAYER1_UI,
    .layer2   = COLOR_LAYER2_UI,
    .reactive = COLOR_REACTIVE_UI,
};


/*
------------------------------------------------------------
DEBUG THEME
------------------------------------------------------------
*/

static const rgb_theme_t theme_debug = {
    .base     = COLOR_BASE_DEBUG,
    .layer1   = COLOR_LAYER1_DEBUG,
    .layer2   = COLOR_LAYER2_DEBUG,
    .reactive = COLOR_REACTIVE_DEBUG,
};


/*
------------------------------------------------------------
GAMING THEME
------------------------------------------------------------
*/

static const rgb_theme_t theme_gaming = {
    .base     = COLOR_BASE_GAMING,
    .layer1   = COLOR_LAYER1_GAMING,
    .layer2   = COLOR_LAYER2_GAMING,
    .reactive = COLOR_REACTIVE_GAMING,
};


// ============================================================
// APPLY COLOR UTILITY
// ============================================================

static inline void apply_color(uint8_t led_index, rgb_t color, uint8_t brightness) {

    rgb_matrix_set_color(
        led_index,
        (uint8_t)(((uint16_t)color.r * brightness) >> 8),
        (uint8_t)(((uint16_t)color.g * brightness) >> 8),
        (uint8_t)(((uint16_t)color.b * brightness) >> 8)
    );
}