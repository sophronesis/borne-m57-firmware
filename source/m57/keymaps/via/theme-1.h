#pragma once
#include QMK_KEYBOARD_H


/*
============================================================
THEME SYSTEM (PROFILE-BASED RGB)
============================================================

Defines centralized color themes used across:

✔ base layer
✔ context layer
✔ reactive layer

Design:
- themes define FULL color palette
- rendering pipeline consumes theme
*/


typedef struct {
    rgb_t base;
    rgb_t layer1;
    rgb_t layer2;
    rgb_t reactive;
} rgb_theme_t;


// ============================================================
// THEME DEFINITIONS (PROFILES)
// ============================================================

static const rgb_theme_t theme_ui = {
    .base     = {0, 40, 120},
    .layer1   = {0, 0, 255},
    .layer2   = {255, 0, 255},
    .reactive = {255, 200, 0},
};

static const rgb_theme_t theme_debug = {
    .base     = {120, 0, 0},
    .layer1   = {255, 0, 0},
    .layer2   = {255, 100, 100},
    .reactive = {255, 255, 255},
};

static const rgb_theme_t theme_gaming = {
    .base     = {0, 120, 0},
    .layer1   = {0, 255, 0},
    .layer2   = {255, 255, 0},
    .reactive = {255, 100, 0},
};