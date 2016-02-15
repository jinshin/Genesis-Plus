#include "shared.h"

unsigned int m68k_read_memory_8(unsigned int address)
{
return m68k_read_memory_8_c(address);
}

unsigned int m68k_read_memory_16(unsigned int address)
{
return m68k_read_memory_16_c(address);
}

unsigned int m68k_read_memory_32(unsigned int address)
{
return m68k_read_memory_32_c(address);
}

void m68k_write_memory_8(unsigned int address, unsigned int value)
{
m68k_write_memory_8_c(address, value);
}

void m68k_write_memory_16(unsigned int address, unsigned int value)
{
m68k_write_memory_16_c(address, value);
}

void m68k_write_memory_32(unsigned int address, unsigned int value)
{
m68k_write_memory_32_c(address, value);
}
