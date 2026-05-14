# ============================================================
# MCU / BOARD
# ============================================================

MCU = STM32F401
BOARD = GENERIC_STM32_F401XC
KEYBOARD_SHARED_EP = yes

FIRMWARE_FORMAT = uf2
UF2_FAMILY = 0xabcdf401
MCU_LDSCRIPT = QF_STM32F401


# ============================================================
# SERIAL / SPLIT
# ============================================================

SERIAL_DRIVER = usart
UARTENABLE = yes


# ============================================================
# OPTIMIZATION FLAGS
# ============================================================

OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE
OPT_DEFS += -DQMK_MCU_STM32F401
OPT_DEFS += -OPT_DEFS += -DBOOTLOADER_TINYUF2




# ============================================================
# RGB MATRIX ENABLE (EXPLICIT)
# ============================================================

RGB_MATRIX_ENABLE = yes

# REQUIRED:
# Enables per-key RGB matrix support in QMK.
#
# DO NOT rely on implicit enabling via info.json.
# Some QMK versions/build environments do NOT auto-enable it.
#
# Without this:
# - rgb_matrix_indicators_user() will not run
# - per-key control may silently fail
# - features may break between QMK updates


# ============================================================
# ENABLE CUSTOM RGB MODE
# ============================================================

RGB_MATRIX_CUSTOM_USER = yes

# Enables a dedicated "CUSTOM" RGB mode.
# This mode runs rgb_matrix_indicators_user().
#
# User can select this mode from Vial/QMK RGB mode list.
#
# Behavior:
# - Standard modes → QMK / Vial control LEDs
# - CUSTOM mode   → firmware controls LEDs


# ============================================================
# EEPROM
# ============================================================

EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = legacy