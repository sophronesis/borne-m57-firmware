# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## what this is

qmk/vial firmware drop-in for a split keyboard called **borne** (variant `m57`, based on foostan/crkbd/corne). not a standalone buildable project - it's a keyboard directory that gets copied into a qmk tree and built from there.

## layout

- `source/m57/` - the keyboard definition. drop into `qmk_firmware/keyboards/m57/` (or wherever your tree expects) to build
- `source/ld/` - linker scripts for stm32f401 (`QF_STM32F401.ld`, `PlumBL_STM32F401.ld`) and stm32l433 (`STM32L433xC.ld`). these go into `platforms/chibios/boards/.../ld/` in the qmk tree
- `source/borne_source_vendor.zip` + `source/m36.rar` - vendor-supplied source snapshots, reference only
- `firmware/borne_m57_via_*.uf2` - pre-built uf2 firmware images (vendor + a 10-layer variant)
- `vil_backups/*.vil` - user's saved vial layouts (json). not code, just keymap dumps

## build

assuming the qmk tree has this keyboard merged in as `m57`:

```
make m57:via
```

the vendor readme (`source/m57/readme.md`) references `make sh01/m36:via` which points at the older m36 variant under a `sh01/` subtree - use whatever path matches how you've placed it.

output format is `uf2` (see `rules.mk`: `FIRMWARE_FORMAT = uf2`, `UF2_FAMILY = 0xabcdf401`). flash by dropping the uf2 onto the mass-storage bootloader that appears when the board enters dfu.

there's no lint/test - qmk's own `qmk lint -kb m57` is the only check that applies.

## architecture notes

**split keyboard, master-right.** `config.h` sets `MASTER_RIGHT` and `SPLIT_HAND_PIN C1` with `SPLIT_HAND_PIN_LOW_IS_LEFT`. transport is full-duplex usart1 on A9/A10 at 19200 baud (`SELECT_SOFT_SERIAL_SPEED 5`). both halves run the same binary - hand detection decides role at boot.

**matrix is 10x7, not 5x7.** despite being a split, rows 0-4 are the left half and rows 5-9 are the right half in a single unified matrix (see `m57.h` `LAYOUT` macro). keymaps in `keymaps/via/keymap.c` mirror this - one `LAYOUT(...)` per layer covers both halves.

**leftkb/ and rightkb/ subdirectories** each define `I_AM_LEFT` / `I_AM_RIGHT` via their `config.h` and tweak `mcuconf.h` (left disables `STM32_PWM_USE_TIM3` because no rgb driver on that side). these look like per-half build variants but the empty `rules.mk` files mean they aren't wired up as separate targets in the current tree - treat them as reference/vendor artifacts unless extending the build system.

**rgb matrix: 58 leds, 29 per side**, ws2812 on pin A7 driven by pwm (`PWMD3` ch2). `m57.c` has two `g_led_config` blocks - the first (commented out) is for a smaller matrix; the live one maps the 10x7 matrix to led indices.

**storage: wear leveling over flash.** `EEPROM_DRIVER = wear_leveling` / `legacy`, 4kb logical / 8kb backing, so don't raise `DYNAMIC_KEYMAP_EEPROM_MAX_ADDR` past 4095 without growing the backing too. 10 dynamic layers, 15 macros.

**vial + via both enabled.** `keymaps/via/rules.mk` turns on `VIAL_ENABLE`, `VIALRGB_ENABLE`, `VIA_ENABLE`, and `VIAL_INSECURE` (lets vial unlock without the usual key combo). `vial.json` is the kle-style layout vial reads; `VIAL_KEYBOARD_UID` in `keymaps/via/config.h` identifies the board to vial.

## known rough edges

- `info.json` has a missing comma and stray trailing commas (around the `{"matrix": [9, 2], ...}` / `{"matrix": [9, 3], ...}` block near the end of the layout). qmk may warn or fail the lint step - fix only when it actually blocks a build, since qmk still parses it in many tool paths
- the `LAYOUT` macro in `m57.h` and the layout list in `info.json` have overlapping x/y coordinates on several keys (multiple keys at `x:4, y:0.2` etc.) - that's the vendor state, not necessarily wrong

## editing keymaps

two routes:
1. **source edit**: modify `source/m57/keymaps/via/keymap.c`, rebuild, reflash
2. **vial gui**: flash once, then use vial to edit layers/macros live - dumps back to `.vil` files like the ones in `vil_backups/`. these live in eeprom and survive reflash as long as the eeprom layout doesn't change

encoder behavior is in the `encoder_map` at the bottom of `keymap.c` (two encoders: left = pins B8/B9, right = pins B4/B6, both resolution 2).
