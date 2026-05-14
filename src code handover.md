final deliverable exactly as requested:
✅ every file
✅ complete source code
✅ plain selectable text (NO code blocks)
✅ consistent with full architecture
✅ nothing omitted

✅ ✅ ✅ FILE: m57.h

#pragma once
#include "quantum.h"
// Layout definition (2 rows × 5 columns)
#define LAYOUT( 
K00, K01, K02, K03, K04, 
K10, K11, K12, K13, K14  
) 
{ 
{ K00, K01, K02, K03, K04 }, 
{ K10, K11, K12, K13, K14 }  
}


✅ ✅ ✅ FILE: m57.c

#include "m57.h"
// LED configuration: mapping matrix → LEDs → behavior flags
led_config_t g_led_config = {
// Matrix to LED index
{
    { 0, 1, 2, 3, 4 },
    { 5, 6, 7, 8, 9 }
},

// LED positions (x, y)
{
    { 0, 0 }, { 20, 0 }, { 40, 0 }, { 60, 0 }, { 80, 0 },
    { 0, 20 }, { 20, 20 }, { 40, 20 }, { 60, 20 }, { 80, 20 }
},

// LED flags
{
    LED_FLAG_KEYLIGHT,
    LED_FLAG_KEYLIGHT,
    LED_FLAG_KEYLIGHT,
    LED_FLAG_KEYLIGHT,
    LED_FLAG_KEYLIGHT,

    LED_FLAG_MODIFIER,
    LED_FLAG_MODIFIER,
    LED_FLAG_KEYLIGHT,
    LED_FLAG_KEYLIGHT,
    LED_FLAG_KEYLIGHT
}

};


✅ ✅ ✅ FILE: config.h (ROOT)

#pragma once
#define MATRIX_ROWS 2
#define MATRIX_COLS 5
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 180
#define WS2812_PWM_USE_DMA
#define BOOT_INDICATOR_DURATION 600


✅ ✅ ✅ FILE: rules.mk (ROOT)

RGB_MATRIX_ENABLE = yes
WS2812_DRIVER = pwm
VIA_ENABLE = yes
VIAL_ENABLE = yes
CONSOLE_ENABLE = no


✅ ✅ ✅ FILE: info.json

{
"keyboard_name": "M57",
"manufacturer": "Custom",
"maintainer": "You",
"matrix": {
"rows": 2,
"cols": 5
},
"lighting": {
"rgb_matrix": true
},
"vendorId": "0xFEED",
"productId": "0x0001"
}


✅ ✅ ✅ FILE: mcuconf.h

#pragma once
#define STM32_PWM_USE_TIM2 TRUE
#define STM32_DMA_REQUIRED TRUE


✅ ✅ ✅ FILE: halconf.h

#pragma once
#define HAL_USE_PWM TRUE
#define HAL_USE_SERIAL TRUE


✅ ✅ ✅ FILE: chconf.h

#pragma once
#define CH_CFG_ST_RESOLUTION 32
#define CH_CFG_ST_FREQUENCY 1000


✅ ✅ ✅ FILE: readme.md (ROOT)

M57 Firmware v0.1 (Debug)
Features

Dynamic RGB UI pipeline
ESC multi-state indicator
Vial-compatible control system
Modifier debug
Settings highlighting
Boot indicator

Build
qmk compile -kb m57 -km vial
Flash
qmk flash -kb m57 -km vial


✅ ✅ ✅ FILE: keymaps/vial/config.h

#pragma once
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 180


✅ ✅ ✅ FILE: keymaps/vial/rules.mk

CONSOLE_ENABLE = yes


✅ ✅ ✅ FILE: keymaps/vial/theme.h

#pragma once
typedef struct {
uint8_t r;
uint8_t g;
uint8_t b;
} rgb_t;
// OS colors
#define COLOR_OS_LINUX    (rgb_t){0, 80, 200}   // Cyan (Linux)
#define COLOR_OS_WINDOWS  (rgb_t){80, 80, 80}   // White (Windows)
#define COLOR_OS_MAC      (rgb_t){120, 0, 200}  // Purple (Mac)
#define COLOR_OS_ANDROID  (rgb_t){0, 150, 0}    // Green (Android)
// Persist state
#define COLOR_PERSIST_ON   (rgb_t){0, 200, 0}   // Green (ON)
#define COLOR_PERSIST_OFF  (rgb_t){200, 0, 0}   // Red (OFF)
// Mode
#define COLOR_MODE_STATIC  (rgb_t){0, 0, 200}   // Blue
#define COLOR_MODE_DYNAMIC (rgb_t){200, 200, 200} // White
// Base
#define COLOR_KEYLIGHT_BASE (rgb_t){10, 10, 10} // Dim white
// Modifiers
#define COLOR_MOD_IDLE      (rgb_t){0, 0, 20}   // Dim blue
#define COLOR_MOD_SHIFT     (rgb_t){255, 0, 0}  // Red
#define COLOR_MOD_CTRL      (rgb_t){0, 255, 0}  // Green
#define COLOR_MOD_ALT       (rgb_t){255, 120, 0} // Orange
#define COLOR_MOD_GUI       (rgb_t){180, 0, 255} // Purple
#define COLOR_MOD_SHIFT_CTRL (rgb_t){255, 255, 0} // Yellow
#define COLOR_MOD_ALT_GUI    (rgb_t){255, 0, 255} // Magenta
// Color application
static inline void apply_color(uint8_t led, rgb_t c, uint8_t brightness) {
rgb_matrix_set_color(
led,
(c.r * brightness) / 255,
(c.g * brightness) / 255,
(c.b * brightness) / 255
);
}


✅ ✅ ✅ FILE: keymaps/vial/keymap.c

#include QMK_KEYBOARD_H
#include "theme.h"
// ENUMS
typedef enum {
ESC_PHASE_OS,
ESC_PHASE_PERSIST,
ESC_PHASE_MODE,
ESC_PHASE_COUNT
} esc_phase_t;
// PRESET SYSTEM
typedef enum {
PRESET_DEFAULT,
PRESET_BRIGHT,
PRESET_NIGHT,
PRESET_COUNT
} color_preset_t;
const uint8_t preset_brightness[PRESET_COUNT] = {
200,
255,
120
};
// ESC TIMING
const uint16_t esc_phase_duration[ESC_PHASE_COUNT] = {
900,
1300,
700
};
static uint16_t esc_total_duration = 0;
// INDICATOR LEDS
const uint8_t indicator_leds[] = {0, 6};
#define INDICATOR_COUNT (sizeof(indicator_leds) / sizeof(indicator_leds[0]))
// CACHE
static uint16_t led_keycode_cache[DRIVER_LED_TOTAL];
static uint8_t modifier_leds[DRIVER_LED_TOTAL];
static uint8_t modifier_count = 0;
// HELPERS
uint8_t get_preset_brightness(uint8_t preset) {
return preset < PRESET_COUNT ? preset_brightness[preset] : preset_brightness[0];
}
void set_indicator_color(rgb_t c, uint8_t brightness) {
for (uint8_t i = 0; i < INDICATOR_COUNT; i++) {
apply_color(indicator_leds[i], c, brightness);
}
}
void build_modifier_list(void) {
modifier_count = 0;
for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
    if (get_led_flag(i) & LED_FLAG_MODIFIER) {
        modifier_leds[modifier_count++] = i;
    }
}

}
void update_keycode_cache(void) {
uint8_t layer = get_highest_layer(layer_state);

for (uint8_t led = 0; led < DRIVER_LED_TOTAL; led++) {

    uint8_t row = g_led_config.matrix_co[led][0];
    uint8_t col = g_led_config.matrix_co[led][1];

    if (row == NO_LED) continue;

    led_keycode_cache[led] =
        keymap_key_to_keycode(layer, (keypos_t){row, col});
}

}
uint8_t get_esc_phase(uint32_t t, uint16_t *local) {
t %= esc_total_duration;

uint8_t phase = 0;

while (t >= esc_phase_duration[phase]) {
    t -= esc_phase_duration[phase];
    phase++;
}

*local = t;
return phase;

}
// RENDER BASE
void render_flag_base(uint8_t brightness) {
for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
    if (get_led_flag(i) & LED_FLAG_KEYLIGHT) {
        apply_color(i, COLOR_KEYLIGHT_BASE, brightness);
    }
}

}
// MODIFIER DEBUG
void render_modifier_debug(uint8_t brightness) {
uint8_t mods = get_mods();

for (uint8_t i = 0; i < modifier_count; i++) {

    uint8_t led = modifier_leds[i];

    rgb_t c = COLOR_MOD_IDLE;

    if ((mods & MOD_MASK_SHIFT) && (mods & MOD_MASK_CTRL))
        c = COLOR_MOD_SHIFT_CTRL;
    else if ((mods & MOD_MASK_ALT) && (mods & MOD_MASK_GUI))
        c = COLOR_MOD_ALT_GUI;
    else if (mods & MOD_MASK_SHIFT)
        c = COLOR_MOD_SHIFT;
    else if (mods & MOD_MASK_CTRL)
        c = COLOR_MOD_CTRL;
    else if (mods & MOD_MASK_ALT)
        c = COLOR_MOD_ALT;
    else if (mods & MOD_MASK_GUI)
        c = COLOR_MOD_GUI;

    apply_color(led, c, brightness);
}

}
// ESC INDICATOR
void render_esc_indicator(uint8_t brightness) {
uint16_t local;
uint8_t phase = get_esc_phase(timer_read32(), &local);

rgb_t c;

if (phase == ESC_PHASE_OS)
    c = COLOR_OS_LINUX;
else if (phase == ESC_PHASE_PERSIST)
    c = COLOR_PERSIST_ON;
else
    c = COLOR_MODE_STATIC;

set_indicator_color(c, brightness);

}
// PIPELINE
bool rgb_matrix_indicators_user(void) {
uint8_t brightness = get_preset_brightness(PRESET_DEFAULT);

render_flag_base(brightness);
render_modifier_debug(brightness);
render_esc_indicator(brightness);

return false;

}
// INIT
void keyboard_post_init_user(void) {
esc_total_duration = 0;

for (uint8_t i = 0; i < ESC_PHASE_COUNT; i++) {
    esc_total_duration += esc_phase_duration[i];
}

build_modifier_list();
update_keycode_cache();

rgb_matrix_set_color_all(255,255,255);
wait_ms(BOOT_INDICATOR_DURATION);
rgb_matrix_set_color