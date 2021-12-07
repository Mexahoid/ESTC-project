#include "flash.h"

// Special record addr.
#define FLASH_START_ADDR 0x000DD000
// Special marking word.
#define FLASH_MARK_WORD 0b00001010101010101010101010101010
// Min addr of 1st page.
#define FLASH_PAGE1_MIN_ADDR 0x000DD004
// Max addr of 1st page.
#define FLASH_PAGE1_MAX_ADDR 0x000DE000
// Min addr of 2nd page (also is max for 1st).
#define FLASH_PAGE2_MIN_ADDR FLASH_PAGE1_MAX_ADDR
// Max addr of 2nd page.
#define FLASH_PAGE2_MAX_ADDR 0x000DF000
// Offset for word addresses.
#define FLASH_ADDR_STEP 0x4

// Defines Hamming code.
#define HAMMING

// 32 bits, dirst 3 bits are mark bits, other 29 are Hamming code.
#define FLASH_R0_MASK 0b00010101010101010101010101010101
#define FLASH_R1_MASK 0b00001100110011001100110011001100
#define FLASH_R2_MASK 0b00000011110000111100001111000011
#define FLASH_R3_MASK 0b00000000001111111100000000111111
#define FLASH_R4_MASK 0b00000000000000000011111111111111

// Current address in a memory.
static uint32_t curr_addr;

#ifdef HAMMING
// Returns parity bit of a 32bit word.
static int count_bits_word(uint32_t word)
{
    int res = 0;
    for (int i = 0; i < 32; i++)
        res += (word >> i) & 1;
    return res % 2;
}
#endif

// Makes Hamming code for a word.
static uint32_t prepare_word(uint8_t a, uint8_t b, uint8_t c)
{
    uint32_t res = 0;
#ifdef HAMMING
    // first 3 bits are 000
    //r0 r1
    res <<= 1; // x0
    res += (a & 0b10000000) >> 7;
    res <<= 1; // r2
    res <<= 3; // x1 x2 x3
    res += (a & 0b01110000) >> 4;
    res <<= 1;             //r2
    res <<= 4;             // x4 x5 x6 x7
    res += a & 0b00001111; // a ready
    res <<= 3;             // x8 x9 x10
    res += (b & 0b11100000) >> 5;
    res <<= 1;             //r3
    res <<= 5;             // x11 - x15
    res += b & 0b00011111; //b ready
    res <<= 8;             // x16 - x23
    res += c;

    res |= count_bits_word(res & FLASH_R0_MASK) << 28;
    res |= count_bits_word(res & FLASH_R1_MASK) << 27;
    res |= count_bits_word(res & FLASH_R2_MASK) << 25;
    res |= count_bits_word(res & FLASH_R3_MASK) << 21;
    res |= count_bits_word(res & FLASH_R4_MASK) << 13;
#endif

#ifndef HAMMING
    res += a;
    res <<= 8;
    res += b;
    res <<= 8;
    res += c;
#endif
    return res;
}

#ifdef HAMMING
// Checks if there any bit errors in a word.
static int check_word(uint32_t word)
{
    int err = 0;
    err |= count_bits_word(word & FLASH_R0_MASK);
    err |= count_bits_word(word & FLASH_R1_MASK) << 1;
    err |= count_bits_word(word & FLASH_R2_MASK) << 2;
    err |= count_bits_word(word & FLASH_R3_MASK) << 3;
    err |= count_bits_word(word & FLASH_R4_MASK) << 4;

    return err;
}

// Fixes 1 error in a word.
static void fix_word(uint32_t *word, int err)
{
    if (err == 0)
        return;
    int pos = 29 - err;
    int mask = 1 << pos;
    int bit = *word & mask;
    if (bit == 0)
        *word |= mask;
    else
    {
        uint32_t x = 0b11011111111111111111111111111111;
        mask = x >> err;
        *word &= mask;
    }
}
#endif


// Reads word from ROM.
static uint32_t get_word(uint32_t addr)
{
    uint32_t *ptr = (uint32_t *)addr;
    uint32_t value = *ptr;

    return value;
}

// Writes word to ROM.
static void set_word(uint32_t word)
{
    nrf_nvmc_write_word((uint32_t)curr_addr, word);
}

// Parses word from Hamming code.
static void parse_word(uint32_t word, flash_word_t *data)
{
    uint32_t x = word;

#ifdef HAMMING
    int errors = check_word(word);
    fix_word(&word, errors);

    uint8_t b = x & 0b11111111;
    x = x >> 8;

    uint8_t g = x & 0b00011111;
    x >>= 5;
    x >>= 1;
    g += (x & 0b111) << 5;
    x >>= 3;

    uint8_t r = x & 0b1111;
    x >>= 4;
    x >>= 1;
    r += (x & 0b111) << 4;
    x >>= 1;
    r += (x & 0b1) << 7;
#endif
#ifndef HAMMING
    uint8_t b = x & 0b11111111;
    uint8_t g = (x >> 8) & 0b11111111;
    uint8_t r = (x >> 16) & 0b11111111;

#endif
    data->first_byte = r;
    data->second_byte = g;
    data->third_byte = b;
}

// Checks if it is data record or a 111..... record.
static bool is_word_null(uint32_t word)
{
    return (word & 0b11100000000000000000000000000000) != 0;
}

// Clears page.
static void erase_page(uint32_t addr)
{
    nrf_nvmc_page_erase(addr);
}

// Finds 111... record on a page.
static uint32_t find_on_page(uint32_t start_addr, uint32_t stop_addr)
{
    uint32_t word;
    int counter = 0;

    for (uint32_t paddr = start_addr; paddr < stop_addr; paddr += FLASH_ADDR_STEP)
    {
        word = get_word(paddr);
        if (is_word_null(word))
            return paddr;
        counter++; // It doesn't work without it.
    }
    return stop_addr;
}

// Clears 1st page and adds marking word.
void reinit_first_page()
{
    uint32_t word = FLASH_MARK_WORD;
    erase_page(FLASH_START_ADDR);
    curr_addr = FLASH_START_ADDR;
    set_word(word);
    curr_addr = FLASH_PAGE1_MIN_ADDR;
}

// Pre-inits memory.
void memory_init()
{
    uint32_t start_addr = FLASH_START_ADDR;
    uint32_t word = get_word(start_addr);
    // If 111.......
    if(is_word_null(word) || ((word & FLASH_MARK_WORD) != FLASH_MARK_WORD))
    {
        reinit_first_page();
        erase_page(FLASH_PAGE2_MIN_ADDR);
        return;
    }

    if(!is_word_null(word))
        return;

}

bool flash_init()
{
    /*erase_page(FLASH_PAGE1_MIN_ADDR);
    erase_page(FLASH_PAGE2_MIN_ADDR);
    return false;*/

    memory_init();
    // Start from first page
    curr_addr = FLASH_PAGE1_MIN_ADDR;
    uint32_t a1 = find_on_page(FLASH_PAGE1_MIN_ADDR, FLASH_PAGE1_MAX_ADDR);
    uint32_t a2 = find_on_page(FLASH_PAGE2_MIN_ADDR, FLASH_PAGE2_MAX_ADDR);

    // When both pages are clean
    if (a1 == FLASH_PAGE1_MIN_ADDR && a2 == FLASH_PAGE2_MIN_ADDR)
    {
        curr_addr = FLASH_PAGE1_MIN_ADDR;
        return 0;
    }

    // When both pages are full, for now no way of identifying what record is last
    if (a1 == FLASH_PAGE1_MAX_ADDR && a2 == FLASH_PAGE2_MAX_ADDR)
    {
        curr_addr = FLASH_PAGE1_MIN_ADDR;
        return false;
    }

    // When only one page is full and next one is partially
    if (a1 == FLASH_PAGE1_MAX_ADDR && a2 > FLASH_PAGE2_MIN_ADDR && a2 < FLASH_PAGE2_MAX_ADDR)
    {
        curr_addr = a2;
        return true;
    }
    if (a2 == FLASH_PAGE2_MAX_ADDR && a1 > FLASH_PAGE1_MIN_ADDR && a1 < FLASH_PAGE1_MAX_ADDR)
    {
        curr_addr = a1;
        return true;
    }

    // When only one page is full
    if (a1 == FLASH_PAGE1_MIN_ADDR && a2 == FLASH_PAGE2_MAX_ADDR)
    {
        curr_addr = FLASH_PAGE1_MIN_ADDR;
        return true;

    }
    if (a2 == FLASH_PAGE2_MIN_ADDR && a1 == FLASH_PAGE1_MAX_ADDR)
    {
        curr_addr = FLASH_PAGE2_MIN_ADDR;
        return true;
    }

    // When there are some records on a page
    if (a1 != FLASH_PAGE1_MIN_ADDR && a2 == FLASH_PAGE2_MIN_ADDR)
    {
        curr_addr = a1;
        return true;
    }

    if (a2 != FLASH_PAGE2_MIN_ADDR && a1 == FLASH_PAGE1_MIN_ADDR)
    {
        curr_addr = a2;
        return true;
    }
    return false;
}

void flash_save_word(flash_word_t* const data)
{
    uint32_t word = prepare_word(data->first_byte, data->second_byte, data->third_byte);
    if (curr_addr == FLASH_PAGE1_MAX_ADDR)
    {
        erase_page(FLASH_PAGE2_MIN_ADDR);
        curr_addr = FLASH_PAGE2_MIN_ADDR; //irrelevant but still
    }

    if (curr_addr == FLASH_PAGE2_MAX_ADDR)
    {
        reinit_first_page();
        curr_addr = FLASH_PAGE1_MIN_ADDR;
    }

    set_word(word);
    curr_addr += FLASH_ADDR_STEP;
}

void flash_load_word(flash_word_t* const data)
{
    uint32_t addr = curr_addr == FLASH_PAGE1_MIN_ADDR ? FLASH_PAGE2_MAX_ADDR : curr_addr;
    addr -= FLASH_ADDR_STEP;
    uint32_t word = get_word(addr);
    parse_word(word, data);
}
