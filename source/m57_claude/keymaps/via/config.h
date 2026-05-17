#pragma once

/* Vial keyboard identifier */
#define VIAL_KEYBOARD_UID {0x45, 0x89, 0xD8, 0xFA, 0xC7, 0x2A, 0x36, 0x89}

/* Combo — zero hardcoded; all combos managed at runtime via Vial */
#define COMBO_COUNT 0

/* Vial editor slot counts */
#define VIAL_TAP_DANCE_ENTRIES    8
#define VIAL_COMBO_ENTRIES        8
#define VIAL_KEY_OVERRIDE_ENTRIES 8

/* Unicode — populates the UC cycle list; active mode is set explicitly from stored OS */
#define UNICODE_SELECTED_MODES UNICODE_MODE_WIN, UNICODE_MODE_MACOS, UNICODE_MODE_LINUX

/* Tap/hold timing */
#define TAPPING_TERM    200
#define ONE_SHOT_TIMEOUT 1000
#define DOUBLE_TAP_SHIFT_TURNS_ON_CAPS_WORD  /* double-tap KC_LSFT activates Caps Word */

/* Mouse key speed (ACCEL0=sniper, ACCEL1=medium, ACCEL2=fast) */
#define MOUSEKEY_MOVE_DELTA  8    /* pixels per step at minimum speed */
#define MOUSEKEY_MAX_SPEED   10   /* multiplier ceiling → 80 px/step at max */
#define MOUSEKEY_TIME_TO_MAX 30   /* steps to reach max speed */
#define MOUSEKEY_INTERVAL    16   /* ms (~60 fps) */