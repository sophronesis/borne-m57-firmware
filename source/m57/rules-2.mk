# ============================================================# =========================================================
BOARD = GENERIC_STM32_F401XC

KEYBOARD_SHARED_EP = yes


# ============================================================
# FIRMWARE FORMAT / BOOTLOADER
# ============================================================

FIRMWARE_FORMAT = uf2
UF2_FAMILY = 0xabcdf401
MCU_LDSCRIPT = QF_STM32F401

# Bootloader type defined via OPT_DEFS (see below)


# ============================================================
# SERIAL / SPLIT COMMUNICATION
# ============================================================

SERIAL_DRIVER = usart
UARTENABLE   = yes


# ============================================================
# RGB MATRIX (PRIMARY SYSTEM)
# ============================================================

RGB_MATRIX_ENABLE = yes
RGB_MATRIX_CUSTOM_USER = yes

# RGB_MATRIX_ENABLE:
#   Enables per-key RGB matrix functionality.
#
# IMPORTANT:
#   Must be explicitly enabled here.
#   info.json alone is NOT sufficient.
#
# RGB_MATRIX_CUSTOM_USER:
#   Enables custom firmware RGB modes.
#
# Result:
#   - Standard modes → QMK / Vial animations
#   - Custom modes   → keymap.c controls LEDs


# ============================================================
# OPTIMIZATION FLAGS
# ============================================================

OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE
OPT_DEFS += -DQMK_MCU_STM32F401
OPT_DEFS += -DBOOTLOADER_TINYUF2

# CORTEX_ENABLE_WFI_IDLE:
#   Enables low-power idle (CPU sleeps between interrupts)
#
# QMK_MCU_STM32F401:
#   Explicit MCU define for QMK internals
#
# BOOTLOADER_TINYUF2:
#   Enables TinyUF2 bootloader compatibility


# ============================================================
# EEPROM / WEAR LEVELING
# ============================================================

EEPROM_DRIVER = wear_leveling
WEAR_LEVELING_DRIVER = legacy

# Ensures:
#   - safe EEPROM writes
#   - longer flash lifespan
#   - required for Vial dynamic keymaps


# ============================================================
# NOTES / DESIGN RULES
# ============================================================

# RGB SYSTEM ARCHITECTURE:
#
#   Vial UI:
#       - selects RGB mode
#       - adjusts brightness/effects
#
#   QMK RGB Matrix:
#       - runs selected mode
#
#   Firmware (keymap.c):
#       - runs ONLY in custom modes
#
# This separation ensures:
# ✔ no LED conflicts
# ✔ predictable behavior
# ✔ full runtime control
# MCU / BOARD
# ============================================================

