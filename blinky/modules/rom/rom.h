#ifndef MY_MODULE_ROM
#define MY_MODULE_ROM

typedef struct
{
    unsigned char first;
    unsigned char second;
    unsigned char third;
} rom_3bytes_t;

void rom_save_3bytes(rom_3bytes_t *data);

void rom_load_3bytes(rom_3bytes_t *data);

#endif