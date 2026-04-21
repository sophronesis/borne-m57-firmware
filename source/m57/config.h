

#pragma once

#undef STM32_HSECLK
#define STM32_HSECLK 16000000

#define TAPPING_TOGGLE 2
#define DYNAMIC_KEYMAP_LAYER_COUNT 10
#define DYNAMIC_KEYMAP_MACRO_COUNT 15



#define MATRIX_ROWS 10
#define MATRIX_COLS 7
/*
 * Keyboard Matrix Assignments
 *
 * Change this to how you wired your keyboard
 * COLS: AVR pins used for columns, left to right
 * ROWS: AVR pins used for rows, top to bottom
 * DIODE_DIRECTION: COL2ROW = COL = Anode (+), ROW = Cathode (-, marked on diode)
 *                  ROW2COL = ROW = Anode (+), COL = Cathode (-, marked on diode)
 *
 */

#define MATRIX_ROW_PINS { C8, C7, B2, A6, A5 }
#define MATRIX_COL_PINS { B13, B14, B15, C6, C9, A8, C12 }
#define MATRIX_ROW_PINS_RIGHT { A1, B7, C5, B0, B1 }
#define MATRIX_COL_PINS_RIGHT {  B8, C10, C8, C7, C6, B15, B14 }

//#define MATRIX_ROW_PINS { A1, B7, C5, B0, B1 }
//#define MATRIX_COL_PINS { B8, C9, C8, C7, C6, B15, B14 }
//#define MATRIX_ROW_PINS_RIGHT { C8, C7, B2, A6, A5 }
//#define MATRIX_COL_PINS_RIGHT {  B13, B14, B15, C6, C9, A8, C12 }

#define DIODE_DIRECTION COL2ROW
#define DEBOUNCE 5
//#define FORCE_NKRO


//#define LOCKING_SUPPORT_ENABLE
//#define LOCKING_RESYNC_ENABLE
//#define TAPPING_TERM 100
//#define TAP_CODE_DELAY 15

//#define USB_POLLING_INTERVAL_MS 1
//#define QMK_KEYS_PER_SCAN 4

//#define EE_HANDS

#define ENCODER_MAP_KEY_DELAY 10

#define SERIAL_USART_FULL_DUPLEX   // Enable full duplex operation mode.
#define SERIAL_USART_TX_PIN A9     // USART TX pin
#define SERIAL_USART_RX_PIN A10     // USART RX pin
//#define USART1_REMAP             // Remap USART TX and RX pins on STM32F103 MCUs, see table below.
//#define SERIAL_USART_PIN_SWAP    // Swap TX and RX pins if keyboard is master halve.
                                   // Check if this feature is necessary with your keyboard design and available on the mcu.
#define SELECT_SOFT_SERIAL_SPEED 5 // or 0, 2, 3, 4, 5
                                   //  0: 460800 baud
                                   //  1: 230400 baud (default)
                                   //  2: 115200 baud
                                   //  3: 57600 baud
                                   //  4: 38400 baud
                                   //  5: 19200 baud
#define SERIAL_USART_DRIVER SD1    // USART driver of TX and RX pin. default: SD1
#define SERIAL_USART_TX_PAL_MODE 7 // Pin "alternate function", see the respective datasheet for the appropriate values for your MCU. default: 7
#define SERIAL_USART_RX_PAL_MODE 7 // Pin "alternate function", see the respective datasheet for the appropriate values for your MCU. default: 7
#define SERIAL_USART_TIMEOUT 20    // USART driver timeout. default 20

#define MASTER_RIGHT

#define SPLIT_HAND_PIN C1
//#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_HAND_PIN_LOW_IS_LEFT


//#define SPLIT_LED_STATE_ENABLE
//#define VIA_HAS_BROKEN_KEYCODES




#    define WS2812_PWM_DRIVER PWMD3  // default: PWMD2
#    define WS2812_PWM_CHANNEL 2  // default: 2
#    define WS2812_PWM_PAL_MODE 2  // Pin "alternate function", see the respective datasheet for the appropriate values for your MCU. default: 2
#    define WS2812_DMA_STREAM STM32_DMA1_STREAM2  // DMA Stream for TIMx_UP, see the respective reference manual for the appropriate values for your MCU.
#    define WS2812_DMA_CHANNEL 5  // DMA Channel for TIMx_UP, see the respective reference manual for the appropriate values for your MCU.
//#    define WS2812_DMAMUX_ID STM32_DMAMUX1_TIM2_UP // DMAMUX configuration for TIMx_UP -- only required if your MCU has a DMAMUX peripheral, see the respective reference manual for the appropriate values for your MCU.
#define RGB_MATRIX_SLEEP

#define WEAR_LEVELING_LOGICAL_SIZE 4096
#define WEAR_LEVELING_BACKING_SIZE (WEAR_LEVELING_LOGICAL_SIZE * 2)
#define DYNAMIC_KEYMAP_EEPROM_MAX_ADDR 4095

/* disable these deprecated features by default */
//#define NO_ACTION_MACRO
//#define NO_ACTION_FUNCTION

#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT 2000
#define USB_POLLING_INTERVAL_MS 1
#define SPLIT_MODS_ENABLE
#define SPLIT_USB_TIMEOUT_POLL 10
#define SPLIT_WATCHDOG_TIMEOUT 3000
#define SPLIT_WATCHDOG_ENABLE
#define NEW_401_BL