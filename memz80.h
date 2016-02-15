
#ifndef _MEMZ80_H_
#define _MEMZ80_H_

/* Function prototypes */
unsigned int cpu_readmem16(unsigned int address);
void cpu_writemem16(unsigned int address, unsigned int data);
unsigned int cpu_readport16(unsigned int port);
void cpu_writeport16(unsigned int port, unsigned int data);
int z80_vdp_r(int address);
void z80_vdp_w(int address, int data);

unsigned int drz80_rebasePC(unsigned short address);
unsigned int drz80_rebaseSP(unsigned short address);
unsigned char drz80_in(unsigned char p);
void drz80_out(unsigned char p, unsigned char d);
unsigned char drz80_read8(unsigned short address);
unsigned short drz80_read16(unsigned short address);
void drz80_write8(unsigned char data,unsigned short address);
void drz80_write16(unsigned short data,unsigned short address);


#endif /* _MEMZ80_H_ */
