# M57 Firmware – Debug Build (v0.1)

## Overview

This firmware implements a fully dynamic RGB-driven UI system on QMK with Vial support.
The design is modular, data-driven, and optimized for performance while remaining fully configurable via keycodes and Vial.

## Key Features

- Dynamic RGB rendering pipeline
- ESC multi-state system indicator
- Modifier debug visualization
- Vial-safe keycode-based control
- Data-driven configuration (no hardcoding)
- Boot indicator
- Performance optimizations
- Compile-time debug system

## System Architecture

Rendering pipeline order:

```
Base → Modifier Debug → Settings Highlight → ESC Indicator
```

- **Base** — default lighting for all keys
- **Modifier Debug** — overlays active modifier keys
- **Settings Highlight** — indicates active layer-switch controls
- **ESC Indicator** — highest-priority UI override

## ESC Indicator

The ESC LED cycles through multiple phases:

| Phase | Duration | Description |
|---|---|---|
| `ESC_PHASE_OS` | 900 ms | Current OS color |
| `ESC_PHASE_PERSIST` | 1300 ms | Persist state |
| `ESC_PHASE_MODE` | 700 ms | RGB mode |

**OS colors:** Linux → Cyan · Windows → Blue · Mac → Silver · Android → Green

**Persist:** ON → Green · OFF → Red

**Mode:** Static → Blue · Dynamic → White

## Modifier Debug

Modifier keys are color-coded when held:

| Key | Color |
|---|---|
| Shift | Red |
| Ctrl | Orange |
| Alt | Yellow |
| GUI | Magenta |
| Shift + Ctrl | Yellow (combo) |
| Alt + GUI | Magenta (combo) |

Uses a precomputed modifier LED list — only those LEDs are scanned per frame.

## Indicator LEDs

```c
indicator_leds[] = {0, 6};  // ESC, TAB
```

Modify only this array to change indicator keys. `INDICATOR_COUNT` adapts automatically.

## Preset System

| Preset | Brightness |
|---|---|
| `PRESET_DEFAULT` | 200 |
| `PRESET_BRIGHT` | 255 |
| `PRESET_NIGHT` | 120 |

Cycle: `(preset + 1) % PRESET_COUNT` / `(preset + PRESET_COUNT - 1) % PRESET_COUNT`

## Feature Toggles

Features are controlled via bitmask flags — disable any subsystem without removing code:

```c
FEATURE_MOD_DEBUG     // 1 << 0
FEATURE_SETTINGS_UI   // 1 << 1
FEATURE_ESC_INDICATOR // 1 << 2
```

## Vial Integration

Logic is keycode-based, not position-based. Keys remapped in Vial automatically carry their LED behavior with them.

## Boot Indicator

All LEDs flash white for 600 ms at startup to confirm firmware is running.

## Performance Optimizations

- Keycode cache rebuilt on layer change only — never per frame
- Modifier LED subset — only modifier LEDs scanned in debug render
- ESC total duration precomputed at init
- Feature flags skip unused render stages entirely

## File Structure

```
m57/
├── m57.c / m57.h          # hardware + LED config
├── config.h / rules.mk    # platform settings
├── info.json              # QMK/Vial keyboard definition
├── mcuconf.h / halconf.h / chconf.h  # ChibiOS/STM32 config
└── keymaps/via/
    ├── keymap.c           # full RGB UI system + keymaps
    ├── theme.h            # all color constants + apply_color()
    ├── config.h           # keymap-level overrides
    ├── rules.mk           # build flags
    └── vial.json          # Vial layout definition
```

## Build

```
qmk compile -kb m57 -km via
```

## Flash

```
qmk flash -kb m57 -km via
```

Output format: `.uf2` — drop onto the mass-storage bootloader that appears when the board enters DFU.

## Debug Mode

Enable in `keymaps/via/rules.mk`:

```makefile
CONSOLE_ENABLE = yes
OPT_DEFS += -DDEBUG_LEVEL=2
```

Debug levels: `DEBUG_OFF` (0) · `DEBUG_LIGHT` (1) · `DEBUG_VERBOSE` (2)

Use `hid_listen` or QMK Toolbox console to view output.

## Design Rules

**Always:**
- Use `ENUM + COUNT + ARRAY` pattern for every configurable system
- Use keycodes instead of LED indices for logic
- Keep hardware config in root files, logic in keymap
- Define all colors in `theme.h`

**Never:**
- Hardcode counts — use `sizeof(array) / sizeof(array[0])`
- Bind logic to physical LED positions
- Duplicate color values outside `theme.h`

## Version

`v0.1-debug`

## Future Improvements

- Additional ESC phases
- Runtime feature toggle via key combo
- Advanced animation profiles
- Vial-based settings UI
- Profile switching system
