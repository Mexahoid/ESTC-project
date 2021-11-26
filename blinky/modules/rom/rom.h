#ifndef MY_MODULE_ROM
#define MY_MODULE_ROM

// 32-bit words on a page
// Page size is 4kB - 4096 bytes - 32768 bits
// So 1 page is 1024 words
// Mark byte is first byte that marks data.

#define R0_MASK 0b00010101010101010101010101010101
#define R1_MASK 0b00001100110011001100110011001100
#define R2_MASK 0b00000011110000111100001111000011
#define R3_MASK 0b00000000001111111100000000111111
#define R4_MASK 0b00000000000000000011111111111111




typedef struct
{
    unsigned char mark_byte;
    unsigned char first_byte;
    unsigned char second_byte;
    unsigned char third_byte;
} rom_word_t;

void rom_save_word(rom_word_t *data);

void rom_load_word(rom_word_t *data);

#endif