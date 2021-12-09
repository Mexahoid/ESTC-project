#ifndef MY_MODULE_FLASH
#define MY_MODULE_FLASH

#include "nrf_nvmc.h"

// ROM module data word.
typedef struct
{
    // First byte of word.
    uint8_t first_byte;
    // Second byte of word.
    uint8_t second_byte;
    // Third byte of word.
    uint8_t third_byte;
    // Fourth marking byte, not used.
    uint8_t mark_byte;
} flash_word_t;

/**
 * @brief Inits module, return true if there any data records found.
 *
 * @return true
 * @return false
 */
bool flash_init();

/**
 * @brief Saves word in a EEPROM.
 *
 * @param data                  Flash word (4 bytes)
 */
void flash_save_word(flash_word_t* const data);

/**
 * @brief Loads word from a EEPROM.
 *
 * @param data                  Flash word (4 bytes)
 */
void flash_load_word(flash_word_t* const data);

#endif