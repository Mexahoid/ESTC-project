#include "flash.h"

// Current address in a memory.
static int curr_addr;

// Returns parity bit of a 32bit word.
static int count_bits_word(int word)
{
    int res = 0;
    for (int i = 0; i < 32; i++)
        res += (word >> i) & 1;
    return res % 2;
}

// Makes Hamming code for a word.
static int prepare_word(int a, int b, int c)
{
    int res = 0;
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

    return res;
}

// Checks if there any bit errors in a word.
static int check_word(int word)
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
static void fix_word(int *word, int err)
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
        int x = 0b11011111111111111111111111111111;
        mask = x >> err;
        *word &= mask;
    }
}

// Reads word from ROM.
static int get_word(int addr)
{
    int *ptr = (int *)addr;
    int value = *ptr;

    return value;
}

// Writes word to ROM.
static void set_word(int word)
{
    nrf_nvmc_write_word((uint32_t)curr_addr, word);
}

// Parses word from Hamming code.
static void parse_word(int word, flash_word_t *data)
{
    int x = word;

    int errors = check_word(word);
    fix_word(&word, errors);

    int b = x & 0b11111111;
    x = x >> 8;

    int g = x & 0b00011111;
    x >>= 5;
    x >>= 1;
    g += (x & 0b111) << 5;
    x >>= 3;

    int r = x & 0b1111;
    x >>= 4;
    x >>= 1;
    r += (x & 0b111) << 4;
    x >>= 1;
    r += (x & 0b1) << 7;

    data->first_byte = r;
    data->second_byte = g;
    data->third_byte = b;
}

// Checks if it is data record or a 111..... record.
static bool is_word_null(int word)
{
    return (word & 0b11100000000000000000000000000000) != 0;
}

// Clears page.
static void erase_page(int addr)
{
    nrf_nvmc_page_erase(addr);
}

// Finds 111... record on a page.
static int find_on_page(int start_addr, int stop_addr)
{
    int word;
    int counter = 0;

    for (int paddr = start_addr; paddr < stop_addr; paddr += FLASH_ADDR_STEP)
    {
        word = get_word(paddr);
        if (is_word_null(word))
            return paddr;
        counter++; // It doesn't work without it.
    }
    return stop_addr;
}

bool flash_init()
{

    /*erase_page(FLASH_PAGE1_MIN_ADDR);
    erase_page(FLASH_PAGE2_MIN_ADDR);
    return false;*/


    // Start from first page
    curr_addr = FLASH_PAGE1_MIN_ADDR;
    int a1 = find_on_page(FLASH_PAGE1_MIN_ADDR, FLASH_PAGE1_MAX_ADDR);
    int a2 = find_on_page(FLASH_PAGE2_MIN_ADDR, FLASH_PAGE2_MAX_ADDR);

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
    int word = prepare_word(data->first_byte, data->second_byte, data->third_byte);
    if (curr_addr == FLASH_PAGE1_MAX_ADDR)
    {
        erase_page(FLASH_PAGE2_MIN_ADDR);
        curr_addr = FLASH_PAGE2_MIN_ADDR; //irrelevant but still
    }

    if (curr_addr == FLASH_PAGE2_MAX_ADDR)
    {
        erase_page(FLASH_PAGE1_MIN_ADDR);
        curr_addr = FLASH_PAGE1_MIN_ADDR;
    }

    set_word(word);
    curr_addr += FLASH_ADDR_STEP;
}

void flash_load_word(flash_word_t* const data)
{
    int addr = curr_addr == FLASH_PAGE1_MIN_ADDR ? FLASH_PAGE2_MAX_ADDR : curr_addr;
    addr -= FLASH_ADDR_STEP;
    int word = get_word(addr);
    parse_word(word, data);
}
