#include "rom.h"

int count_bits_word(int word)
{
    int res = 0;
    for (int i = 0; i < 32; i++)
        res += (word >> i) & 1;
    return res % 2;
}

int prepare_word(char a, char b, char c)
{
    int res = 0;
    // first 3 bits are 000
    //r0 r1
    res <<= 1; // x0
    res += a & 0b10000000 >> 7;
    res <<= 1; // r2
    res <<= 3; // x1 x2 x3
    res += a & 0b01110000 >> 4;
    res <<= 1; //r2
    res <<= 4; // x4 x5 x6 x7
    res += a & 0b00001111; // a ready
    res <<= 3; // x8 x9 x10
    res += b & 0b11100000 >> 5;
    res <<= 1; //r3
    res <<= 5; // x11 - x15
    res += b & 0b00011111; //b ready
    res <<= 8; // x16 - x23
    res += c;

    int r0, r1, r2, r3, r4;

    r0 = res & R0_MASK;
    res |= count_bits_word(r0) << 28;

    r1 = res & R1_MASK;
    res |= count_bits_word(r1) << 27;

    r2 = res & R2_MASK;
    res |= count_bits_word(r2) << 25;

    r3 = res & R3_MASK;
    res |= count_bits_word(r3) << 21;

    r4 = res & R4_MASK;
    res |= count_bits_word(r4) << 13;


    return res;
}


int check_word(int word)
{
    int err = 0;
    err |= count_bits_word(word & R0_MASK);
    err |= count_bits_word(word & R1_MASK) << 1;
    err |= count_bits_word(word & R2_MASK) << 2;
    err |= count_bits_word(word & R3_MASK) << 3;
    err |= count_bits_word(word & R4_MASK) << 4;

    return err;
}

void fix_word(int *word, int err)
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

void rom_save_word(rom_word_t *data)
{

}

void rom_load_word(rom_word_t *data)
{
    data->mark_byte = 0;
    data->first_byte = 255;
    data->second_byte = 0;
    data->third_byte = 0;
}
