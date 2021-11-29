#ifndef MY_MODULE_FLASH
#define MY_MODULE_FLASH

#include "nrf_nvmc.h"

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