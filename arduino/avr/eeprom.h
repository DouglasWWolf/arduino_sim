#pragma once
#include <stdint.h>

void eeprom_update_block(const void* src, void* dest, size_t count);
void eeprom_read_block( void* dest, const void* src, size_t count);
void eeprom_write_byte(uint8_t* addr, uint8_t value);
