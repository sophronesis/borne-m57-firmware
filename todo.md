# todo

## done

### led_config rewrite [shipped, commit `8c3e666`]

vendor mapping had matrix rows in L/R/L/R alternating order instead of 0-4=left, 5-9=right. rewrote to match actual hardware chain after per-key verification on both halves via `solid_reactive_simple`. final state captured in `source/m57/m57.c` and verified working.

### bootloader_jump for QK_BOOT [shipped, commit `6ab532f`]

vendor set `BOOTLOADER=custom` in info.json but never wrote the implementation - QK_BOOT and vial reset were both no-ops. found PlumBL upstream at https://github.com/HaiMianBBao/PlumBL: magic `0xc220b134` at `0x2000fc00`, then `NVIC_SystemReset()`. now QK_BOOT triggers DFU mode, no tweezers needed.

### build pipeline + flash automation [shipped]

- vial-qmk source tree at `~/projects/vial-qmk` with `source/m57` symlinked into `keyboards/`, ld scripts in `platforms/chibios/boards/common/ld/`
- `nix-shell -p qmk --run "make m57:via"` builds cleanly
- `scripts/flash.sh` watches for `Adafruit PlumBootloader`, mounts the disk, copies uf2, syncs, polls for firmware reboot. one command, ~5 seconds end-to-end

### docs + github [shipped]

- README with hardware spec, patches, build, flash procedure, repo layout
- photo of reset pad location (`docs/reset-pads.png`)
- public repo at https://github.com/sophronesis/borne-m57-firmware

## open / future

### feature: per-key static color override

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

alt path for reactive-tint only: change default hue via `rgb_matrix_sethsv_noeeprom(h,s,v)` in `keyboard_post_init_user` - cheaper than indicators but affects all effects.

### feature: modifier-held highlighting

same hook (`rgb_matrix_indicators_advanced_user`), use `get_mods() | get_oneshot_mods()` to read current modifier state. recommended variant: highlight the mod key itself - catches stuck mods and one-shot state with low visual noise.

```c
uint8_t mods = get_mods() | get_oneshot_mods();
if (mods & MOD_MASK_SHIFT) { rgb_matrix_set_color(led_for_shift, R, G, B); }
// ... CTRL / ALT / GUI similarly
```

### feature: layer indicator

with 10 dynamic layers configured, a single LED (or zone) showing active layer pays off quickly. same hook + `get_highest_layer(layer_state)`. consider mapping each layer to a distinct hue.

### nice-to-have: fix info.json syntax

`source/m57/info.json` has a missing comma + trailing commas around the `[9, 2]` / `[9, 3]` block near the end of layout. qmk parses it anyway but `qmk lint -kb m57` complains. low priority - only matters if someone runs the linter as part of CI.

### nice-to-have: right half asymmetry doc

right half led_config doesn't perfectly mirror left - PCB chain order differs slightly (e.g., LED 48 is at home row inner extra physically but in chain order it's between home and Z rows). worth a short comment in `m57.c` explaining the asymmetry for future readers.
