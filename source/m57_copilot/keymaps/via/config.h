#pragma once

// ==================================================
// VIAL IDENTIFICATION
// ==================================================

// Unique identifier for this keyboard in Vial
// IMPORTANT: Do NOT change once firmware is distributed
#define VIAL_KEYBOARD_UID {0x89, 0x36, 0x2A, 0xC7, 0xFA, 0xD8, 0x89, 0x45}


// ==================================================
// NOTES
// ==================================================
//
// - STM32F401 uses FLASH-based EEPROM emulation (no physical EEPROM)
// - EEPROM size/layout is managed by QMK (wear_leveling driver)
// - Do NOT define EECONFIG_USER_DATA_SIZE unless absolutely necessary
//   (can conflict with Vial storage if misused)
//