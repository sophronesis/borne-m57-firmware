# todo

## fix: reactive rgb highlights wrong keys

root cause: `source/m57/m57.c` `g_led_config` first block (matrix → led index) is in L/R/L/R interleaved order, but `m57.h` matrix has rows 0-4 = left, 5-9 = right. so pressing left-row-2 lights right-top-row leds.

rearrange first block to:

```c
{ 0,  1,  2,  3,  4,  5,  NO_LED },        // row 0: L top, leds 0-5
{ 6,  7,  8,  9,  10, 11, 12 },            // row 1: L r2
{ 13, 14, 15, 16, 17, 18, 19 },            // row 2: L r3
{ 20, 21, 22, 23, 24, 25, NO_LED },        // row 3: L r4
{ NO_LED, NO_LED, NO_LED, 26, 27, 28, NO_LED }, // row 4: L thumb
{ NO_LED, 29, 30, 31, 32, 33, 34 },        // row 5: R top
{ 35, 36, 37, 38, 39, 40, 41 },            // row 6: R r2
{ 42, 43, 44, 45, 46, 47, 48 },            // row 7: R r3
{ 49, 50, 51, 52, 53, 54, NO_LED },        // row 8: R r4
{ NO_LED, 55, 56, 57, NO_LED, NO_LED, NO_LED }, // row 9: R thumb
```

open question before flashing: rows 5 and 8 col 0 - verify whether those physical keys have leds behind them. if R row 4 col 0 has no led, swap to `{ NO_LED, 49, 50, 51, 52, 53, 54 }`.

verify after flash: press each key, watch only the led under it light with `RGB_MATRIX_SOLID_REACTIVE`.

## feature: per-key color override

use `rgb_matrix_indicators_advanced_user` in `source/m57/keymaps/via/keymap.c` - paints on top of whatever animation is running, every frame.

decide first:
- which keys / which colors
- always on, or only on specific layer, or only when pressed

example shape (fill in matrix positions + rgb):

```c
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t led = g_led_config.matrix_co[row][col];
    if (led != NO_LED && led >= led_min && led < led_max) {
        rgb_matrix_set_color(led, R, G, B);
    }
    return false;
}
```

alt path if you want reactive-tint only: change default hue via `rgb_matrix_sethsv_noeeprom(h,s,v)` in `keyboard_post_init_user` - cheaper than indicators, but affects all effects not just reactive.

## feature: backlight change on modifiers held

same hook (`rgb_matrix_indicators_advanced_user`), use `get_mods()` to read current modifier state. three variants, pick one:

- **highlight the mod key itself**: when shift is held, paint both shift keys red. minimal and obvious
- **highlight a zone**: when ctrl is held, color the shortcut-relevant keys (c, v, x, z, a, s) a different hue
- **tint the whole board**: when any mod is held, override the base animation with a flat color - loud but unmissable

api shape:

```c
uint8_t mods = get_mods() | get_oneshot_mods();
if (mods & MOD_MASK_SHIFT) { rgb_matrix_set_color(led, R, G, B); }
if (mods & MOD_MASK_CTRL)  { ... }
if (mods & MOD_MASK_ALT)   { ... }
if (mods & MOD_MASK_GUI)   { ... }
```

decide: which of the three variants, color per modifier, and whether to include one-shot mods.

## blocked until led config fix

don't bother tuning per-key colors or modifier highlighting until matrix → led mapping is correct, or the "wrong buttons" will make it impossible to tell if the color code is right.
