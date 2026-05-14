#pragma once

#undef STM32_HSECLK
#define STM32_HSECLK 16000000

#define TAPPING_TOGGLE 2
#define DYNAMIC_KEYMAP_LAYER_COUNT 10
#define DYNAMIC_KEYMAP_MACRO_COUNT 15 

#define MATRIX_ROWS 10
#define MATRIX_COLS 7

/*
- Keyboard Matrix Assignments
- Change this to how you wired your keyboard
- COLS: AVR pins used for columns, left to right
- ROWS: AVR pins used for rows, top to bottom
- DIODE_DIRECTION: COL2ROW = COL = Anode (+), ROW = Cathode (-)
*/

#define MATRIX_ROW_PINS { C8, C7, B2, A6, A5 }
#define MATRIX_COL_PINS { B13, B14, B15, C6, C9, A8, C12 }

#define MATRIX_ROW_PINS_RIGHT { A1, B7, C5, B0, B1 }
#define MATRIX_COL_PINS_RIGHT { B8, C10, C8, C7, C6, B15, B14 }

#define DIODE_DIRECTION COL2ROW
#define DEBOUNCE 5

#define ENCODER_MAP_KEY_DELAY 10 

#define SERIAL_USART_FULL_DUPLEX
#define SERIAL_USART_TX_PIN A9
#define SERIAL_USART_RX_PIN A10

#define SELECT_SOFT_SERIAL_SPEED 5

#define SERIAL_USART_DRIVER SD1
#define SERIAL_USART_TX_PAL_MODE 7
#define SERIAL_USART_RX_PAL_MODE 7
#define SERIAL_USART_TIMEOUT 20


// ============================================================
// SPLIT MASTER CONFIGURATION (DUAL-USB SUPPORT)
// ============================================================

//#define MASTER_RIGHT   // DISABLED: do NOT force right half as master

/*
Reason:

We intentionally disable fixed master configuration to allow
USB connection on either half of the keyboard.

Instead, master selection is handled dynamically via:
    - SPLIT_HAND_PIN
    - USB detection (SPLIT_USB_DETECT)

This enables:
✔ Plug USB into LEFT half  → LEFT becomes master
✔ Plug USB into RIGHT half → RIGHT becomes master

IMPORTANT:
Only ONE half should be connected to USB at a time.
Connecting both halves simultaneously can cause conflicts.
*/


// ============================================================
// SPLIT HAND DETECTION
// ============================================================

/*
Each half determines its role (LEFT / RIGHT) using a dedicated pin.

SPLIT_HAND_PIN must be wired so that:
    - One half reads HIGH
    - The other half reads LOW

SPLIT_HAND_PIN_LOW_IS_LEFT means:
    LOW  → LEFT half
    HIGH → RIGHT half
*/

#define SPLIT_HAND_PIN C1
#define SPLIT_HAND_PIN_LOW_IS_LEFT


// ============================================================
// BOOT SEQUENCE (RUNTIME ROLE SELECTION)
// ============================================================

/*
Boot sequence overview:

1. Both halves power on
2. Each half reads SPLIT_HAND_PIN
   → determines LEFT or RIGHT identity

3. Each half checks for USB connection
   → via SPLIT_USB_DETECT

4. Master selection:
   - The half connected to USB becomes MASTER
   - The other becomes SLAVE

5. Communication is established over USART (serial split)

6. MASTER half:
   - scans matrix
   - processes key events
   - controls RGB rendering
   - communicates with host

7. SLAVE half:
   - sends matrix state to master
   - mirrors LED updates

This system enables plugging USB into either half without firmware changes.
*/


#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT 2000
#define SPLIT_USB_TIMEOUT_POLL 10

#define SPLIT_MODS_ENABLE
#define SPLIT_WATCHDOG_TIMEOUT 3000
#define SPLIT_WATCHDOG_ENABLE

#define SPLIT_USB_POLLING_INTERVAL_MS 1


#define WS2812_PWM_DRIVER PWMD3
#define WS2812_PWM_CHANNEL 2
#define WS2812_PWM_PAL_MODE 2
#define WS2812_DMA_STREAM STM32_DMA1_STREAM2
#define WS2812_DMA_CHANNEL 5


#define RGB_MATRIX_SLEEP

#define WEAR_LEVELING_LOGICAL_SIZE 4096
#define WEAR_LEVELING_BACKING_SIZE (WEAR_LEVELING_LOGICAL_SIZE * 2)
#define DYNAMIC_KEYMAP_EEPROM_MAX_ADDR 4095

#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT 2000

#define USB_POLLING_INTERVAL_MS 1

#define NEW_401_BL