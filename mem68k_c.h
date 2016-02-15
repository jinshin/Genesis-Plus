#ifndef _MEM68K_C_H_
#define _MEM68K_C_H_

/* Function prototypes */
//static inline unsigned int m68k_read_bus_8_c(unsigned int address);
unsigned int m68k_read_bus_16_c(unsigned int address);

unsigned int   m68k_read_memory_8_c(unsigned int address);
unsigned int   m68k_read_memory_16_c(unsigned int address);
unsigned int   m68k_read_memory_32_c(unsigned int address);

void m68k_write_memory_8_c(unsigned int address, unsigned char value);
void m68k_write_memory_16_c(unsigned int address, unsigned short value);
void m68k_write_memory_32_c(unsigned int address, unsigned int value);

#endif /* _MEM68K_C_H_ */
