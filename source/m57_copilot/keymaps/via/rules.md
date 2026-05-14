# ==================================================
# BORNE KEYBOARD - VIAL BUILD CONFIGURATION
# ==================================================

# ---------------------------
# Vial / VIA Support
# ---------------------------

VIAL_ENABLE = yes          # Enable Vial runtime configuration support
VIA_ENABLE = yes           # VIA compatibility layer (required by Vial)

# Enable RGB control via Vial UI
VIALRGB_ENABLE = yes


# ---------------------------
# Development Mode (IMPORTANT)
# ---------------------------

# Allows Vial to write to EEPROM without restrictions
# Useful during development for rapid iteration and testing
#
# ⚠️ WARNING:
# This bypasses parts of our controlled EEPROM logic
# (idle-save, change detection, etc.)
#
# ✅ RECOMMENDED:
# Set to 'no' before final production firmware:
#   VIAL_INSECURE = no
#
VIAL_INSECURE = yes


# ---------------------------
# Input Devices
# ---------------------------

ENCODER_ENABLE = yes       # Enable physical encoders
ENCODER_MAP_ENABLE = yes   # Allow mapping encoders in Vial UI


# ---------------------------
# RGB Subsystem
# ---------------------------

RGB_MATRIX_ENABLE = yes    # Required for custom RGB indicators (ESC/TAB)

# (optional – depends on your hardware)
# RGB_MATRIX_DRIVER = ws2812


# ---------------------------
# EEPROM / Storage
# ---------------------------

# Use FLASH-based EEPROM emulation with wear leveling
# IMPORTANT: Required for STM32F401 (no real EEPROM)
EEPROM_DRIVER = wear_leveling


# ---------------------------
# Optional Optimizations
# ---------------------------

LTO_ENABLE = yes           # Reduce firmware size (important for STM32)
EXTRAKEY_ENABLE = yes      # Media/system key support


# ==================================================
# END OF FILE
# ==================================================