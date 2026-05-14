#pragma once

// =====================
// CLOCK
// =====================

#undef STM32_HSECLK
#define STM32_HSECLK 16000000

// =====================
// MATRIX
// =====================

#define MATRIX_ROWS 10
#define MATRIX_COLS 7

#define MATRIX_ROW_PINS {C8, C7, B2, A6, A5}
#define MATRIX_COL_PINS {B13, B14, B15, C6, C9, A8, C12}

#define MATRIX_ROW_PINS_RIGHT {A1, B7, C5, B0, B1}
#define MATRIX_COL_PINS_RIGHT {B8, C10, C8, C7, C6, B15, B14}

#define DIODE_DIRECTION COL2ROW
#define DEBOUNCE 5

// =====================
// SPLIT
// =====================

#define MASTER_RIGHT
#define SPLIT_HAND_PIN C1
#define SPLIT_HAND_PIN_LOW_IS_LEFT

#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT 2000
#define SPLIT_USB_TIMEOUT_POLL 10
#define SPLIT_WATCHDOG_ENABLE
#define SPLIT_WATCHDOG_TIMEOUT 3000

// =====================
// SERIAL (USART SPLIT)
// =====================

#define SERIAL_USART_FULL_DUPLEX
#define SERIAL_USART_TX_PIN A9
#define SERIAL_USART_RX_PIN A10
#define SERIAL_USART_DRIVER SD1

#define SERIAL_USART_TX_PAL_MODE 7
#define SERIAL_USART_RX_PAL_MODE 7

#define SERIAL_USART_TIMEOUT 20
#define SELECT_SOFT_SERIAL_SPEED 5

// =====================
// ENCODER
// =====================

#define ENCODER_MAP_KEY_DELAY 10

// =====================
// RGB
// =====================

#define RGB_MATRIX_SLEEP

// =====================
// EEPROM / WEAR LEVELING
// =====================

#define WEAR_LEVELING_LOGICAL_SIZE 4096
#define WEAR_LEVELING_BACKING_SIZE (WEAR_LEVELING_LOGICAL_SIZE * 2)

#define DYNAMIC_KEYMAP_LAYER_COUNT 10
#define DYNAMIC_KEYMAP_MACRO_COUNT 15
#define DYNAMIC_KEYMAP_EEPROM_MAX_ADDR 4095

// =====================
// USB
// =====================

#define USB_POLLING_INTERVAL_MS 1

// =====================
// BOOT
// =====================

#define NEW_401_BL

#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 180
#define WS2812_PWM_USE_DMA
#define BOOT_INDICATOR_DURATION 600