#pragma once

#define VIAL_KEYBOARD_UID {0x45, 0x89, 0xD8, 0xFA, 0xC7, 0x2A, 0x36, 0x89}

// ============================================================
// VIAL RUNTIME EDITOR SLOT COUNTS
// ============================================================
// These control how many entries each Vial live-editor can hold.
// Increase if you need more slots; each slot costs a small amount of EEPROM.

// Number of hardcoded combos in keymap.c (compile-time, not Vial-editable)
#define COMBO_COUNT 1

// Tap-Dance slots available in Vial's Tap-Dance editor.
// Each slot: tap action / hold action / double-tap action / tap-hold action.
#define VIAL_TAP_DANCE_ENTRIES 8

// Combo slots available in Vial's Combo editor (runtime, stored in EEPROM).
// These coexist with the hardcoded COMBO_COUNT combos above.
#define VIAL_COMBO_ENTRIES 8

// Key Override slots in Vial's Key Override editor.
// Example use: Shift+Backspace → Delete; Shift+Esc → ~
#define VIAL_KEY_OVERRIDE_ENTRIES 8


// ============================================================
// TAP / HOLD TIMING
// ============================================================

// Time (ms) to distinguish a tap from a hold.  Used by Mod-Tap (MT), Layer-Tap (LT),
// and Tap-Dance.  Tune live at runtime with the DT_UP / DT_DOWN keycodes assigned
// in Vial — no reflash needed.  Typical range: 150–250 ms.
#define TAPPING_TERM 200

// One-Shot mods/layers (OSM / OSL): how long (ms) a one-shot mod waits for the next
// keypress before it self-cancels.  No Vial panel — change here and reflash if needed.
#define ONE_SHOT_TIMEOUT 1000


// ============================================================
// MOUSE KEY SPEED
// ============================================================
// No Vial panel for these — change here and reflash to tune.
//
// Without any KC_MS_ACCEL* held, movement starts at MOVE_DELTA px/step and
// accelerates up to MAX_SPEED × MOVE_DELTA over TIME_TO_MAX steps.
// KC_MS_ACCEL0 = slowest (sniper), ACCEL1 = medium, ACCEL2 = fastest.
//
//  Setting           Default   Notes
//  MOVE_DELTA            8     pixels per step at minimum speed
//  MAX_SPEED            10     multiplier ceiling  (→ 10×8 = 80 px/step at max)
//  TIME_TO_MAX          30     steps to reach max speed (~30 × 16 ms = 480 ms)
//  INTERVAL             16     ms between movement steps (~60 fps)

#define MOUSEKEY_MOVE_DELTA   8
#define MOUSEKEY_MAX_SPEED   10
#define MOUSEKEY_TIME_TO_MAX 30
#define MOUSEKEY_INTERVAL    16