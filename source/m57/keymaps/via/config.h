#pragma once

#define VIAL_KEYBOARD_UID {0x89, 0x36, 0x2A, 0xC7, 0xFA, 0xD8, 0x89, 0x45}

// home row mods tuning (layer 0 only - see keymap.c).
// cross-hand combos resolve as hold immediately via CHORDAL_HOLD.
// same-hand combos: HOLD_ON_OTHER_KEY_PRESS makes any other keypress within
// the term confirm the hold; PERMISSIVE_HOLD lets a full press+release of
// another key while holding the mod-tap confirm it. together = urob-style.
#define TAPPING_TERM 200
#define PERMISSIVE_HOLD
#define HOLD_ON_OTHER_KEY_PRESS
// avoid a held mod-tap getting consumed as a repeated tap when you've just
// tapped the same key (e.g. double-tap 'a' won't accidentally hold gui).
#define QUICK_TAP_TERM 0

#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }
