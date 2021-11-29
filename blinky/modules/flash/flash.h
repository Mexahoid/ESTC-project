#ifndef MY_MODULE_FLASH
#define MY_MODULE_FLASH

#include "nrf_nvmc.h"

// Min addr of 1st page.
#define FLASH_PAGE1_MIN_ADDR 0x000DD000
// Max addr of 1st page.
#define FLASH_PAGE1_MAX_ADDR 0x000DE000
// Min addr of 2nd page (also is max for 1st).
#define FLASH_PAGE2_MIN_ADDR FLASH_PAGE1_MAX_ADDR
// Max addr of 2nd page.
#define FLASH_PAGE2_MAX_ADDR 0x000DF000
// Offset for word addresses.
#define FLASH_ADDR_STEP 0x4

// 32 bits, dirst 3 bits are mark bits, other 29 are Hamming code.
#define FLASH_R0_MASK 0b00010101010101010101010101010101
#define FLASH_R1_MASK 0b00001100110011001100110011001100
#define FLASH_R2_MASK 0b00000011110000111100001111000011
#define FLASH_R3_MASK 0b00000000001111111100000000111111
#define FLASH_R4_MASK 0b00000000000000000011111111111111

// ROM module data word.
typedef struct
{
    // First byte of word.
    unsigned char first_byte;
    // Second byte of word.
    unsigned char second_byte;
    // Third byte of word.
    unsigned char third_byte;
    // Fourth marking byte, not used.
    unsigned char mark_byte;
} flash_word_t;

// Inits module, return true if there any data records found.
bool flash_init();

// Saves word in a EEPROM.
void flash_save_word(flash_word_t* const data);

// Loads word from a EEPROM.
void flash_load_word(flash_word_t* const data);

#endif