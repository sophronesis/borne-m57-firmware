# ============================================================
# VIAL / VIA SUPPORT
# ============================================================

VIAL_ENABLE = yes
VIA_ENABLE  = yes

ENCODER_MAP_ENABLE = yes
COMBO_ENABLE       = yes

VIAL_INSECURE = yes

# ============================================================
# RUNTIME-CONFIGURABLE FEATURES
# ============================================================

# Tap-Dance: one key → different action on tap / hold / double-tap / tap-hold.
# Slot count and per-slot actions are configured live in Vial's Tap-Dance editor.
# VIAL_TAP_DANCE_ENTRIES in config.h controls how many slots are available.
TAP_DANCE_ENABLE = yes

# Key Overrides: remap any mod+key combination to a different keycode at runtime.
# Example: Shift+Backspace → Delete, or Shift+Esc → ~.
# Configured live in Vial's Key Override editor.
# VIAL_KEY_OVERRIDE_ENTRIES in config.h controls slot count.
KEY_OVERRIDE_ENABLE = yes

# Auto Shift: hold any key slightly longer than AUTO_SHIFT_TIMEOUT to get its
# shifted version — no Shift key needed.  OFF by default after flash; enable and
# tune the timeout live in Vial's Auto Shift panel (or assign AS_ON keycode).
AUTO_SHIFT_ENABLE = yes

# Mouse Keys: move the pointer, click, and scroll from the keyboard.
# Adds KC_MS_*, KC_MS_BTN*, KC_MS_WH_*, KC_MS_ACCEL* keycodes to the Vial picker.
# Speed tuning is done via #defines in config.h (no Vial panel for raw speed values).
MOUSEKEY_ENABLE = yes

# Repeat Key: re-sends the last pressed key.  Assign QK_REPEAT_KEY / QK_ALT_REPEAT_KEY
# anywhere in the keymap via Vial.  Useful for doubled letters (ll, tt) and arrow bursts.
REPEAT_KEY_ENABLE = yes

# Dynamic Tapping Term: adjust TAPPING_TERM live without reflashing.
# Assign DT_UP / DT_DOWN / DT_PRNT keycodes in Vial to tune tap-vs-hold threshold.
# Affects Tap-Dance, Mod-Tap (MT), and Layer-Tap (LT) simultaneously.
DYNAMIC_TAPPING_TERM_ENABLE = yes

# ============================================================
# CONSOLE / DEBUG OUTPUT
# ============================================================

# Uncomment to enable uprintf() output (reads via 'qmk console').
# Also set DEBUG_LEVEL in keymap.c to DEBUG_INFO or higher.
# Keep disabled for production — saves ~2 KB flash.
#CONSOLE_ENABLE = yes

# ============================================================
# VIAL RGB CONTROL (UI LAYER)
# ============================================================

VIALRGB_ENABLE = yes

# Vial RGB provides:
#   ✔ mode selection
#   ✔ brightness control
#   ✔ animation control
#
# It does NOT provide:
#   ✘ per-key logic
#   ✘ conditional behavior
#
# That logic is implemented in:
#   keymap.c (custom modes)


# ============================================================
# RGB RESPONSIBILITY MODEL
# ============================================================

# Mode selected in Vial determines behavior:
#
#   Standard mode:
#       → QMK handles LEDs
#
#   Custom mode:
#       → firmware pipeline runs
#
# User-visible result:
#
#   RGB Modes include:
#       - QMK animations
#       - FIRMWARE UI
#       - DEBUG MODE
#       - GAMING MODE