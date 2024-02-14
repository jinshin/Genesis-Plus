
#ifndef _LOADROM_H_
#define _LOADROM_H_

/* Function prototypes */
void deinterleave_block(uint8 *src);
int load_rom(char *filename);

typedef struct {
int start;
int len;
int end;
int reg;
int eeprom;
uint8 *mem;
uint16 addr;    // EEPROM address register
uint8  cycle;  // EEPROM SRAM cycle number
uint8  slave;  // EEPROM slave word for X24C02 and better SRAMs
} t_sram;

extern t_sram sram;

extern uint8 vdp_rate;

#endif /* _LOADROM_H_ */

