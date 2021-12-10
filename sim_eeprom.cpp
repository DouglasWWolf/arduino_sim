#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define FILENAME "eeprom.bin"

class CSimEEPROM
{
public:

    CSimEEPROM();

    void    save();

    unsigned char data[0x1000];
} SimEEPROM;


CSimEEPROM::CSimEEPROM()
{
    FILE* ifile;

    // Start our data block off blank
    memset(data, 0xFF, sizeof data);

    // Open the file, and if we can't, complain
    if (fopen_s(&ifile, FILENAME, "rb") != 0) return;

    // Read in as much of the data as exists
    fread(data, 1, sizeof data, ifile);

    // We now have all of our data from the EEPROM file
    fclose(ifile);
}


void CSimEEPROM::save()
{
    FILE* ofile;

    // Open the file, and if we can't, complain
    if (fopen_s(&ofile, FILENAME, "wb") != 0) return;

    // Write all of our data out
    fwrite(data, 1, sizeof data, ofile);

    // We now have all of our data from the EEPROM file
    fclose(ofile);
}



void eeprom_update_block(const void* src, void* dest, size_t count)
{
    unsigned int index = (unsigned int)dest;
        
    memcpy(SimEEPROM.data + index, src, count);

    SimEEPROM.save();
}


void eeprom_write_byte(uint8_t* addr, uint8_t value)
{
    unsigned int index = (unsigned int)addr;

    SimEEPROM.data[index] = value;

    SimEEPROM.save();
}


void eeprom_read_block(void* dest, const void* src, size_t count)
{
    unsigned int index = (unsigned int)src;

    memcpy(dest, SimEEPROM.data + index, count);
}