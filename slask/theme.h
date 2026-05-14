#pragma once

#include <stdint.h>

// =======================
// TYPE
// =======================

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

// =======================
// THEMES (compile-time)
// =======================

#define RGB_THEME_DARK 0
#define RGB_THEME_BRIGHT 1
#define RGB_THEME_GAMER 2

#ifndef RGB_THEME
#define RGB_THEME RGB_THEME_DARK
#endif

// =======================
// BRIGHTNESS SCALE PER PRESET
// =======================

static inline uint8_t apply_brightness(uint8_t v, uint8_t scale)
{
    return (v * scale) / 255;
}

// =======================
// USER PRESET SCALE
// =======================

static inline uint8_t get_preset_brightness(uint8_t preset)
{
    switch (preset)
    {
    case 0:
        return 200; // Default → normal brightness
    case 1:
        return 255; // Bright → maximum brightness
    case 2:
        return 120; // Night → dim
    }
    return 200;
}

// =======================
// THEME COLORS
// =======================

#if RGB_THEME == RGB_THEME_DARK

#define COLOR_OS_LINUX (rgb_t){0, 80, 200}   // Cyan (dim Linux)
#define COLOR_OS_WINDOWS (rgb_t){80, 80, 80} // Soft white (Windows)
#define COLOR_OS_MAC (rgb_t){120, 0, 200}    // Purple (Mac)
#define COLOR_OS_ANDROID (rgb_t){0, 150, 0}  // Green (Android)

#define COLOR_PERSIST_ON (rgb_t){0, 200, 0}  // Green (persist ON)
#define COLOR_PERSIST_OFF (rgb_t){200, 0, 0} // Red (persist OFF)

#define COLOR_MODE_STATIC (rgb_t){0, 0, 200}      // Blue (static)
#define COLOR_MODE_DYNAMIC (rgb_t){200, 200, 200} // White (dynamic)

#define COLOR_KEYLIGHT_BASE (rgb_t){10, 10, 10} // Dim white

#endif

#if RGB_THEME == RGB_THEME_BRIGHT

#define COLOR_OS_LINUX (rgb_t){0, 120, 255}     // Bright cyan (Linux)
#define COLOR_OS_WINDOWS (rgb_t){200, 200, 200} // Bright white (Windows)
#define COLOR_OS_MAC (rgb_t){180, 0, 255}       // Bright purple (Mac)
#define COLOR_OS_ANDROID (rgb_t){0, 255, 0}     // Bright green (Android)

#define COLOR_PERSIST_ON (rgb_t){0, 255, 0}  // Green
#define COLOR_PERSIST_OFF (rgb_t){255, 0, 0} // Red

#define COLOR_MODE_STATIC (rgb_t){0, 0, 255}      // Blue
#define COLOR_MODE_DYNAMIC (rgb_t){255, 255, 255} // White

#define COLOR_KEYLIGHT_BASE (rgb_t){25, 25, 25} // Soft white

#endif

#if RGB_THEME == RGB_THEME_GAMER

#define COLOR_OS_LINUX (rgb_t){0, 255, 255}     // Neon cyan (Linux)
#define COLOR_OS_WINDOWS (rgb_t){255, 255, 255} // Pure white
#define COLOR_OS_MAC (rgb_t){255, 0, 255}       // Neon magenta
#define COLOR_OS_ANDROID (rgb_t){0, 255, 0}     // Neon green

#define COLOR_PERSIST_ON (rgb_t){0, 255, 0}  // Bright green
#define COLOR_PERSIST_OFF (rgb_t){255, 0, 0} // Bright red

#define COLOR_MODE_STATIC (rgb_t){0, 0, 255}      // Blue
#define COLOR_MODE_DYNAMIC (rgb_t){255, 255, 255} // White

#define COLOR_KEYLIGHT_BASE (rgb_t){40, 40, 40} // Bright white base

#endif

// =======================
// MODIFIER COLORS
// =======================

#define COLOR_MOD_IDLE (rgb_t){0, 0, 20}   // Dim blue (idle)
#define COLOR_MOD_SHIFT (rgb_t){255, 0, 0} // Red (SHIFT)
#define COLOR_MOD_CTRL (rgb_t){0, 255, 0}  // Green (CTRL)
#define COLOR_MOD_ALT (rgb_t){255, 120, 0} // Orange (ALT)
#define COLOR_MOD_GUI (rgb_t){180, 0, 255} // Purple (GUI)

#define COLOR_MOD_SHIFT_CTRL (rgb_t){255, 255, 0} // Yellow (SHIFT+CTRL)
#define COLOR_MOD_ALT_GUI (rgb_t){255, 0, 255}    // Magenta (ALT+GUI)

// =======================
// COLOR APPLY HELPER
// =======================

static inline void apply_color(uint8_t led, rgb_t c, uint8_t brightness)
{

    uint8_t r = apply_brightness(c.r, brightness);
    uint8_t g = apply_brightness(c.g, brightness);
    uint8_t b = apply_brightness(c.b, brightness);

    rgb_matrix_set_color(led, r, g, b);
}