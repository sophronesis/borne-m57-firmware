# todo

## fix: reactive rgb highlights wrong keys [applied, awaiting flash+verify]

root cause: `source/m57/m57.c` `g_led_config` first block (matrix → led index) was in L/R/L/R interleaved order, but `m57.h` matrix has rows 0-4 = left, 5-9 = right. so pressing left-row-2 lit right-top-row leds.

applied fix: rows reshuffled to match matrix layout, formatted as paired L | R per physical row for readability. correct first block now:

```c
// matrix rows 0-4 = left half, rows 5-9 = right half (paired visually L | R)
{ 0,  1,  2,  3,  4,  5,  NO_LED },                 { NO_LED, 29, 30, 31, 32, 33, 34 },              // top
{ 6,  7,  8,  9,  10, 11, 12 },                     { 35, 36, 37, 38, 39, 40, 41 },                  // r2
{ 13, 14, 15, 16, 17, 18, 19 },                     { 42, 43, 44, 45, 46, 47, 48 },                  // r3
{ 20, 21, 22, 23, 24, 25, NO_LED },                 { NO_LED, 49, 50, 51, 52, 53, 54 },              // r4
{ NO_LED, NO_LED, NO_LED, 26, 27, 28, NO_LED },     { NO_LED, 55, 56, 57, NO_LED, NO_LED, NO_LED },  // thumb
```

note: row 8 (R r4) kept `NO_LED` at col 0 (not col 6) - mirrors rows 5 and 9 on the right side where col 0 is the phantom/inner.

verify after flash: press each key, watch only the led under it light with `RGB_MATRIX_SOLID_REACTIVE`. open edge case: rows 5 and 8 col 0 - if the physical key there has an led after all, move the `NO_LED` to col 6 on whichever row is wrong.

## build/flash setup (needed before verification)

no qmk toolchain installed, no qmk source tree. plan:

1. clone **vial-qmk** (`https://github.com/vial-kb/vial-qmk`) to `~/projects/vial-qmk` (~1-2 GB with submodules) - stock qmk doesn't support `VIAL_ENABLE`
2. `cd ~/projects/vial-qmk && make git-submodule` to pull chibios + deps
3. symlink (or copy) `source/m57` → `~/projects/vial-qmk/keyboards/m57`
4. drop linker scripts from `source/ld/` into `~/projects/vial-qmk/platforms/chibios/boards/GENERIC_STM32_F401XC/ld/` (or wherever the board config expects - check `rules.mk`: `BOARD = GENERIC_STM32_F401XC`, `MCU_LDSCRIPT=QF_STM32F401`)
5. possibly fix `info.json` syntax issues (missing comma around the `[9, 2]`/`[9, 3]` block, trailing commas) - qmk lint may reject it
6. build: `nix-shell -p qmk --run "cd ~/projects/vial-qmk && make m57:via"`, output in `vial-qmk/.build/m57_via.uf2`
7. copy resulting uf2 back to `firmware/` in this repo

pre-flash backup: save current vial layout to `vil_backups/current_preflash_YYYYMMDD.vil` in case eeprom gets reset by the firmware change.

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

same hook (`rgb_matrix_indicators_advanced_user`), use `get_mods()` to read current modifier state. recommended variant: **highlight the mod key itself** (catches stuck mods and one-shot state, low visual noise).

api shape:

```c
uint8_t mods = get_mods() | get_oneshot_mods();
if (mods & MOD_MASK_SHIFT) { rgb_matrix_set_color(led, R, G, B); }
if (mods & MOD_MASK_CTRL)  { ... }
if (mods & MOD_MASK_ALT)   { ... }
if (mods & MOD_MASK_GUI)   { ... }
```

decide: color per modifier, and whether to include one-shot mods (yes, recommended).

followup: layer indicator using same hook with `get_highest_layer(layer_state)` - with 10 layers configured, this earns its keep.

## blocked until flash+verify

don't tune per-key colors or modifier highlighting until the led config fix is flashed and confirmed - wrong buttons make color tuning untestable.
