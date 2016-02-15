/*
    memz80.c --
    Memory handlers for Z80 memory and port access, and the Z80 to
    VDP interface.
*/

#define LOG_PORT 0      /* 1= Log Z80 I/O port accesses */

#include "shared.h"
#include "windows.h"

//DrZ80 
unsigned int drz80_rebasePC(unsigned short address)
{
        DrZ80CPU.Z80PC_BASE = (unsigned int)zram;
        return DrZ80CPU.Z80PC_BASE + address;
}

unsigned int drz80_rebaseSP(unsigned short address)
{
        DrZ80CPU.Z80SP_BASE = (unsigned int)zram;
        return DrZ80CPU.Z80SP_BASE + address;
}

unsigned char drz80_in(unsigned char p)
{
	return 0xFF;
}

void drz80_out(unsigned char p, unsigned char d)
{
	//Do nothing
}

uint32 cpu_readmem8(unsigned int address);
void cpu_writemem8(unsigned int address, uint8 data);

unsigned char drz80_read8(unsigned short a)
{      
  return cpu_readmem8(a);
}

unsigned short drz80_read16(unsigned short a)
{
  return (drz80_read8(a))|((uint32)drz80_read8(a+1)<<8);
}

void drz80_write8(unsigned char data,unsigned short a) {

  if ((a>>13)==2) // 0x4000-0x5fff (Charles MacDonald)
  {
    fm_write(a, data);
    return;
  }

  if ((a&0xfff8)==0x7f10&&(a&1)) // 7f11 7f13 7f15 7f17
  {
    psg_write(data);
    return;
  }

  cpu_writemem8(a, data);

}

void drz80_write16(unsigned short data, unsigned short a)
{
  drz80_write8((unsigned char)(data&0xff),a);
  drz80_write8((unsigned char)(data>>8),a+1);
}

//MAME z80

uint32 cpu_readmem8(unsigned int address)
{
    switch((address >> 13) & 7)
    {
        case 0: /* Work RAM */
        case 1:
            return zram[address & 0x1FFF];

        case 2: /* YM2612 */
            return fm_read(address & 3);

        case 3: /* VDP */
            if((address & 0xFF00) == 0x7F00)
                return z80_vdp_r(address);
            return 0xFF;

        default: /* V-bus bank */
            return z80_read_banked_memory(zbank | (address & 0x7FFF));
    }

    return 0xFF;
}


unsigned int cpu_readmem16(unsigned int address)
{
    switch((address >> 13) & 7)
    {
        case 0: /* Work RAM */
        case 1:
            return zram[address & 0x1FFF];

        case 2: /* YM2612 */
            return fm_read(address & 3);

        case 3: /* VDP */
            if((address & 0xFF00) == 0x7F00)
                return z80_vdp_r(address);
            return 0xFF;

        default: /* V-bus bank */
            return z80_read_banked_memory(zbank | (address & 0x7FFF));
    }

    return 0xFF;
}


void cpu_writemem8(unsigned int address, uint8 data)
{
    switch((address >> 13) & 7)
    {
        case 0: /* Work RAM */
        case 1: 
            zram[address & 0x1FFF] = data;
            return;

        case 2: /* YM2612 */
            fm_write(address, data);
            return;

        case 3: /* Bank register and VDP */
            switch(address & 0xFF00)
            {
                case 0x6000:
                    gen_bank_w(data & 1);
                    return;

                case 0x7F00:
                    z80_vdp_w(address, data);
                    return;

                default:
                    //z80_unused_w(address, data);
                    return;
            }
            return;

        default: /* V-bus bank */
            z80_write_banked_memory(zbank | (address & 0x7FFF), data);
            return;
    }
}


void cpu_writemem16(unsigned int address, unsigned int data)
{
    switch((address >> 13) & 7)
    {
        case 0: /* Work RAM */
        case 1: 
            zram[address & 0x1FFF] = data;
            return;

        case 2: /* YM2612 */
            fm_write(address, data);
            return;

        case 3: /* Bank register and VDP */
            switch(address & 0xFF00)
            {
                case 0x6000:
                    gen_bank_w(data & 1);
                    return;

                case 0x7F00:
                    z80_vdp_w(address, data);
                    return;

                default:
                    //z80_unused_w(address, data);
                    return;
            }
            return;

        default: /* V-bus bank */
            z80_write_banked_memory(zbank | (address & 0x7FFF), data);
            return;
    }
}


int z80_vdp_r(int address)
{
    switch(address & 0xFF)
    {
        case 0x00: /* VDP data port */
        case 0x02:
            return (vdp_data_r() >> 8) & 0xFF;
                        
        case 0x01: /* VDP data port */ 
        case 0x03:
            return (vdp_data_r() & 0xFF);

        case 0x04: /* VDP control port */
        case 0x06:
            return (0xFF | ((vdp_ctrl_r() >> 8) & 3));
                        
        case 0x05: /* VDP control port */
        case 0x07:
            return (vdp_ctrl_r() & 0xFF);

        case 0x08: /* HV counter */
        case 0x0A:
        case 0x0C:
        case 0x0E:
            return (vdp_hvc_r() >> 8) & 0xFF;

        case 0x09: /* HV counter */
        case 0x0B:
        case 0x0D:
        case 0x0F:
            return (vdp_hvc_r() & 0xFF);

        case 0x10: /* Unused (PSG) */
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
            return -1; //z80_lockup_r(address);

        case 0x18: /* Unused */
        case 0x19:
        case 0x1A:
        case 0x1B:
            return -1; //z80_unused_r(address);

        case 0x1C: /* Unused (test register) */
        case 0x1D:
        case 0x1E:
        case 0x1F:
            return -1; //z80_unused_r(address);

        default: /* Invalid VDP addresses */
            return -1; //z80_lockup_r(address);
    }

    return 0xFF;
}


void z80_vdp_w(int address, int data)
{
    switch(address & 0xFF)
    {
        case 0x00: /* VDP data port */
        case 0x01: 
        case 0x02:
        case 0x03:
            vdp_data_w(data << 8 | data);
            return;

        case 0x04: /* VDP control port */
        case 0x05:
        case 0x06:
        case 0x07:
            vdp_ctrl_w(data << 8 | data);
            return;

        case 0x08: /* Unused (HV counter) */
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            //z80_lockup_w(address, data);
            return;

        case 0x11: /* PSG */
        case 0x13:
        case 0x15:
        case 0x17:
            psg_write(data);
            return;

        case 0x10: /* Unused */
        case 0x12:
        case 0x14:
        case 0x16:             
            //z80_unused_w(address, data);
	    return;	

        case 0x18: /* Unused */
        case 0x19:
        case 0x1A:
        case 0x1B:
            //z80_unused_w(address, data);
            return;

        case 0x1C: /* Test register */
        case 0x1D: 
        case 0x1E:
        case 0x1F:
            vdp_test_w(data << 8 | data);
            return;

        default: /* Invalid VDP addresses */
            //z80_lockup_w(address, data);
            return;
    }
}


/*
    Port handlers. Ports are unused when not in Mark III compatability mode.

    Games that access ports anyway:
    - Thunder Force IV reads port $BF in it's interrupt handler.
*/

unsigned int cpu_readport16(unsigned int port)
{
#if LOG_PORT
    error("Z80 read port %04X (%04X)\n", port, z80_get_reg(Z80_PC));
#endif    
    return 0xFF;
}

void cpu_writeport16(unsigned int port, unsigned int data)
{
#if LOG_PORT
    error("Z80 write %02X to port %04X (%04X)\n", data, port, z80_get_reg(Z80_PC));
#endif
}
