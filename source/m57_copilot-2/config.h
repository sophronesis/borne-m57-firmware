#pragma once

// ============================================================
// SYSTEM / CLOCK
// ============================================================

#undef STM32_HSECLK
#define STM32_HSECLK 16000000


// ============================================================
// QMK CORE SETTINGS
// ============================================================

#define TAPPING_TOGGLE 2

// Vial dynamic keymaps
#define DYNAMIC_KEYMAP_LAYER_COUNT 10
#define DYNAMIC_KEYMAP_MACRO_COUNT 15


// ============================================================
// MATRIX CONFIGURATION
// ============================================================

#define MATRIX_ROWS 10
#define MATRIX_COLS 7

/*
Matrix wiring:
- Rows: top → bottom
- Cols: left → right
*/

#define MATRIX_ROW_PINS        { C8, C7, B2, A6, A5 }
#define MATRIX_COL_PINS        { B13, B14, B15, C6, C9, A8, C12 }

#define MATRIX_ROW_PINS_RIGHT  { A1, B7, C5, B0, B1 }
#define MATRIX_COL_PINS_RIGHT  { B8, C10, C8, C7, C6, B15, B14 }

#define DIODE_DIRECTION COL2ROW
#define DEBOUNCE 5


// ============================================================
// ENCODER
// ============================================================

#define ENCODER_MAP_KEY_DELAY 10


// ============================================================
// SPLIT COMMUNICATION (USART)
// ============================================================

#define SERIAL_USART_FULL_DUPLEX
#define SERIAL_USART_TX_PIN A9
#define SERIAL_USART_RX_PIN A10

#define SELECT_SOFT_SERIAL_SPEED 5

#define SERIAL_USART_DRIVER SD1
#define SERIAL_USART_TX_PAL_MODE 7
#define SERIAL_USART_RX_PAL_MODE 7
#define SERIAL_USART_TIMEOUT 20


// ============================================================
// SPLIT MASTER CONFIGURATION (DUAL USB SUPPORT)
// ============================================================

//#define MASTER_RIGHT   // DISABLED → dynamic master selection

/*
Dynamic split behavior:

- USB plugged LEFT  → LEFT becomes master
- USB plugged RIGHT → RIGHT becomes master

IMPORTANT:
Do NOT plug USB into both halves simultaneously.
*/


// ============================================================
// SPLIT HAND DETECTION
// ============================================================

#define SPLIT_HAND_PIN C1
#define SPLIT_HAND_PIN_LOW_IS_LEFT

/*
LOW  → LEFT half
HIGH → RIGHT half
*/


// ============================================================
// SPLIT RUNTIME FEATURES
// ============================================================

#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT 2000
#define SPLIT_USB_TIMEOUT_POLL 10

#define SPLIT_MODS_ENABLE
#define SPLIT_WATCHDOG_TIMEOUT 3000
#define SPLIT_WATCHDOG_ENABLE

#define USB_POLLING_INTERVAL_MS 1


// ============================================================
// RGB MATRIX (GROUPED SECTION)
// ============================================================

/*
IMPORTANT:

RGB matrix is enabled via rules.mk:
    RGB_MATRIX_ENABLE = yes

info.json alone is NOT sufficient.
*/

#define RGB_MATRIX_SLEEP


// ============================================================
// WS2812 (RGB DRIVER CONFIG)
// ============================================================

#define WS2812_PWM_DRIVER   PWMD3
#define WS2812_PWM_CHANNEL  2
#define WS2812_PWM_PAL_MODE 2

#define WS2812_DMA_STREAM   STM32_DMA1_STREAM2
#define WS2812_DMA_CHANNEL  5

// Optional (only needed on specific MCUs):
//#define WS2812_DMAMUX_ID STM32_DMAMUX1_TIM2_UP


// ============================================================
// EEPROM / WEAR LEVELING
// ============================================================

#define WEAR_LEVELING_LOGICAL_SIZE 4096
#define WEAR_LEVELING_BACKING_SIZE (WEAR_LEVELING_LOGICAL_SIZE * 2)

#define DYNAMIC_KEYMAP_EEPROM_MAX_ADDR 4095


// ============================================================
// FINAL FLAGS
// ============================================================

#define NEW_401_BL


// ============================================================
// LEGACY / UNUSED (COMMENTED OUT FOR REFERENCE)
// ============================================================

/*
//#define FORCE_NKRO
//#define LOCKING_SUPPORT_ENABLE
//#define LOCKING_RESYNC_ENABLE
//#define TAPPING_TERM 100
//#define TAP_CODE_DELAY 15
//#define QMK_KEYS_PER_SCAN 4
//#define EE_HANDS
*/