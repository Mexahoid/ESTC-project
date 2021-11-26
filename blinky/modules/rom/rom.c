#include "rom.h"


void rom_save_3bytes(rom_3bytes_t *data)
{

}

void rom_load_3bytes(rom_3bytes_t *data)
{
    data->first = 255;
    data->second = 0;
    data->third = 0;
}
