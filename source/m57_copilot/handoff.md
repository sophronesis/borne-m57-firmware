📦 handoff.md — Firmware Architecture & Implementation Handoff

✅ 1. PROJECT OVERVIEW
This firmware implements a custom RGB matrix system on QMK (STM32F401) with:

Persistent system state (OS + theme)
Structured rendering pipeline
LED ownership enforcement
Performance cache layer
Reactive lighting system
Vial integration (UI + runtime control)
Debug logging system


✅ 2. SYSTEM ARCHITECTURE

🧠 High-level data flow
INPUT
 (keys / encoder / Vial)
        ↓
STATE
 (OS / theme / mode / layer)
        ↓
CACHE
 (LED → keycode mapping)
        ↓
RENDER PIPELINE
 (layered rendering system)
        ↓
OUTPUT
 (RGB matrix LEDs)


🧩 System boundaries

System					Responsibility
Vial					UI & mode/brightness control
QMK RGB Matrix			base engine + mode dispatch
Firmware (keymap.c)		custom rendering / logic
Theme system			color palette
EEPROM					persistent state

✅ 3. FEATURE INVENTORY (VERIFIED)

🎛️ RGB SYSTEM
✅ Custom RGB Modes
Defined in firmware:
FIRMWARE UI
DEBUG MODE
GAMING MODE


Selected via Vial
Only active when:

mode >= RGB_MATRIX_CUSTOM


✅ Render Pipeline
Order (strict):
1. render_flag_base()
2. render_key_based()
3. render_key_groups()
4. render_modifier()
5. render_os()
6. render_event() (conditional)


✅ Layer Behavior

Layer		Purpose
Base		full background (theme)
Key-based	specific keys (e.g. Enter)
Groups		A–Z / numbers / F keys
Modifier	dynamic input feedback
OS			system status
Event		reactive animation


💾 PERSISTENT STATE SYSTEM

✅ OS State

Values:

Linux / Windows / Mac / Android


Stored in EEPROM:

EECONFIG_USER + 0


Controlled via:

encoder
keycodes




✅ Theme State

Values:

UI / DEBUG / GAMING


Stored in EEPROM:

EECONFIG_USER + 1


Controlled via keycodes



🎯 INPUT SYSTEM

✅ Encoder system
FN layer (layer 1) + encoder
→ cycles OS


✅ Keycode system
Custom keycodes:
OS_*_SET
THEME_NEXT / PREV


✅ Vial integration
From keymaps/via:

remappable keys ✅
encoder map ✅
RGB control ✅



⚡ CACHE SYSTEM

✅ Purpose
Avoid repeated expensive calls:
keymap_key_to_keycode()


✅ Stored data
LED index → keycode


✅ Update triggers

1. Timer (~100 ms)
2. Layer change (layer_state_set_user)


✅ Critical fix
NO_LED entries explicitly initialized to KC_NO



🔥 REACTIVE SYSTEM

✅ Design
circular buffer (MAX_REACTIVE = 8)

Each event:

LED index
timestamp
decay duration (300 ms)


✅ Behavior
fade-out animation based on time



🎨 THEME SYSTEM

✅ Defined in:
theme.h


✅ Used via:
get_active_theme()


✅ Effects:

base color
reactive color
overlays



🔐 LED OWNERSHIP SYSTEM

✅ Implemented rule
OS LEDs are RESERVED

Defined via:
Cos_indicator_zone[]Show more lines

✅ Enforced in:

base ✅
key-based ✅
groups ✅
modifier ✅
reactive ✅


✅ Result
OS indicator NEVER overwritten



🧠 DEBUG SYSTEM

✅ Log system
Defined levels:
NONE / ERROR / WARN / INFO / VERBOSE


✅ Logging macro
LOG(level, ...)


✅ Coverage

EEPROM load/save ✅
encoder ✅
key events ✅
theme changes ✅
cache updates ✅
reactive triggers ✅
RGB mode validation ✅


✅ Control
rules.mk:
-DDEBUG_LEVEL=...



🎚️ VIAL + RGB INTEGRATION
From rules.mk:

✅ Enabled features
VIAL_ENABLE
VIALRGB_ENABLE
RGB_MATRIX_CUSTOM_USER
ENCODER_MAP_ENABLE


✅ Responsibility split
Component	Responsibility
Vial		select mode / brightness
QMK			run selected mode
Firmware	control LEDs in custom mode


✅ 4. FIX LOG (IMPORTANT)

🔴 Fix 1 — Cache corruption
Problem
NO_LED entries left uninitialized

Fix
set to KC_NO explicitly

Impact
✅ prevents undefined behavior


🔴 Fix 2 — Numeric key range
Problem
KC_1 → KC_0 used incorrectly as range

Fix
(KC_1–KC_9) || KC_0



🔴 Fix 3 — RGB mode underflow
Problem
mode < RGB_MATRIX_CUSTOM → invalid indexing

Fix
validate before subtraction



🔴 Fix 4 — LED ownership violation
Problem
layers overwriting OS LEDs

Fix
is_os_led() + skip logic



🔴 Fix 5 — Stale cache risk
Fix
timer refresh + layer hook



🔴 Fix 6 — Hardcoded brightness
Fix
rgb_matrix_get_val()



🔴 Fix 7 — Missing debug visibility
Fix
log system added



✅ 5. FILE RELATIONSHIPS

keymap.c
 ├── uses theme.h
 ├── uses QMK RGB engine
 ├── uses EEPROM
 ├── interacts with Vial via custom modes

m57.c / m57.h
 ├── board-specific logic

info.json
 ├── LED matrix definition

rules.mk
 ├── feature flags

keymaps/via/*
 ├── user-facing keymap + Vial interface

vial.json
 ├── UI schema + layout



✅ 6. DESIGN PRINCIPLES

✅ Separation of concerns
UI (Vial) != Logic (firmware)


✅ Deterministic rendering
fixed pipeline order


✅ Ownership enforcement
reserved zones protected


✅ Stateless rendering
state is external, rendering is pure


✅ Performance-first
cache system avoids runtime overhead



✅ 7. KNOWN LIMITATIONS


No debug overlay (only logs)
Ownership limited to OS zone only
No animation system beyond reactive
No per-zone brightness
No per-OS theme mapping



✅ 8. EXTENSION GUIDE

✅ Adding new render layer

Create render_*
Insert into pipeline
respect ownership rules



✅ Adding new persistent state

allocate EEPROM slot
add load/save
integrate with input



✅ Extending ownership
Add:
modifier zone
encoder zone
underglow zone



✅ Debug overlay (recommended next)
render_debug_overlay()

Use to visualize:

LED index
matrix mapping
cache validity



✅ ✅ ✅ FINAL SUMMARY

This firmware is now:
✔ fully stateful
✔ performance optimized
✔ visually deterministic
✔ debug-capable
✔ Vial-integrated
✔ production-ready