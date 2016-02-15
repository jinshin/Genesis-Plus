/*
    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "shared.h"
#include "cyclone.h"
#include "mem68k_c.h"
#include "windows.h"

uint8 cart_rom[0x400000];   /* Cartridge ROM */
uint8 work_ram[0x10000];    /* 68K work RAM */
uint8 zram[0x2000];         /* Z80 work RAM */
uint8 zbusreq;              /* /BUSREQ from Z80 */
uint8 zreset;               /* /RESET to Z80 */
uint8 zbusack;              /* /BUSACK to Z80 */
uint8 zirq;                 /* /IRQ to Z80 */
uint32 zbank;               /* Address of Z80 bank window */
uint8 gen_running;

struct Cyclone CycloneCPU;
struct DrZ80   DrZ80CPU;
int32 total_cycles;

#ifdef USE_DRZ80

#define drz80_execute(x) DrZ80Run(&DrZ80CPU, x)

//DrZ80

void drz80_irq_callback()
{
	zirq = 0;
	DrZ80CPU.Z80_IRQ = 0;
}

void drz80_init() {
  memset(&DrZ80CPU, 0, sizeof(struct DrZ80));
  DrZ80CPU.z80_rebasePC=drz80_rebasePC;
  DrZ80CPU.z80_rebaseSP=drz80_rebaseSP;
  DrZ80CPU.z80_read8   =drz80_read8;
  DrZ80CPU.z80_read16  =drz80_read16;
  DrZ80CPU.z80_write8  =drz80_write8;
  DrZ80CPU.z80_write16 =drz80_write16;
  DrZ80CPU.z80_in      =drz80_in;
  DrZ80CPU.z80_out     =drz80_out;
  DrZ80CPU.z80_irq_callback=drz80_irq_callback;
}

void drz80_reset()
{
  memset(&DrZ80CPU, 0, 0x4c);
  DrZ80CPU.Z80F  = (1<<2);  // set ZFlag
  DrZ80CPU.Z80F2 = (1<<2);  // set ZFlag
  DrZ80CPU.Z80IX = 0xFFFF << 16;
  DrZ80CPU.Z80IY = 0xFFFF << 16;
  DrZ80CPU.Z80IM = 0; // 1?
  DrZ80CPU.Z80PC = DrZ80CPU.z80_rebasePC(0);
  DrZ80CPU.Z80SP = DrZ80CPU.z80_rebaseSP(0x2000); // 0xf000 ?
  DrZ80CPU.z80_irq_callback=drz80_irq_callback;
}

#endif

#ifdef USE_CYCLONE
/**************************************************/
/********* CYCLONE STUFF **************************/
/**************************************************/

int CycloneDoRun(int cyc)
{
  if (cyc<=0) return cyc;
  CycloneCPU.cycles=cyc;
  CycloneRun(&CycloneCPU);
  return cyc-CycloneCPU.cycles;
}

int CycloneInterrupt(int irq)
{
  CycloneCPU.irq=(unsigned char)irq;
  return 0;
}

int CycloneMemBase(uint32 pc)
{
  int membase=0;

  if (pc<rom_size)
  {
    membase=(int)cart_rom; // Program Counter in Rom
  }
  else if ((pc&0xe00000)==0xe00000)
  {
    membase=(int)work_ram-(pc&0xff0000); // Program Counter in Ram
  }
  else
  {
    // Error - Program Counter is invalid
    membase=(int)cart_rom;
  }

  return membase;
}
static unsigned int CycloneCheckPc(unsigned int pc)
{
  pc-=CycloneCPU.membase; // Get real pc
  pc&=0xffffff;

  CycloneCPU.membase=CycloneMemBase(pc);

  return CycloneCPU.membase+pc;
}

int vdp_int_ack_callback(int int_level);

int C_vdp_int_ack_callback(int int_level);

/*--------------------------------------------------------------------------*/
/* Init, reset, shutdown functions                                          */
/*--------------------------------------------------------------------------*/

int  cyclone_init(void) {
	CycloneInit();
	memset(&CycloneCPU,0,sizeof(CycloneCPU));                   
	CycloneCPU.checkpc=CycloneCheckPc;
	CycloneCPU.fetch8 =CycloneCPU.read8 =m68k_read_memory_8_c;
	CycloneCPU.fetch16=CycloneCPU.read16=m68k_read_memory_16_c;
	CycloneCPU.fetch32=CycloneCPU.read32=m68k_read_memory_32_c;
	CycloneCPU.write8 =m68k_write_memory_8_c;
	CycloneCPU.write16=m68k_write_memory_16_c;
	CycloneCPU.write32=m68k_write_memory_32_c;
	CycloneCPU.IrqCallback=C_vdp_int_ack_callback;
return 0;
} 

int cyclone_reset() {
    CycloneCPU.srh =0x27; // Supervisor mode
    CycloneCPU.a[7]=CycloneCPU.read32(0); // Stack Pointer
    CycloneCPU.membase=0;
    CycloneCPU.pc=CycloneCPU.checkpc(CycloneCPU.read32(4)); // Program Counter
    CycloneCPU.state_flags=0;
    total_cycles=0;
return 0;
}

#endif

void gen_init(void)
{
    memset(&snd, 0, sizeof(snd));

    bswap(cart_rom, 0x400000);

#ifdef USE_CYCLONE
	cyclone_init();
#else
	m68k_set_cpu_type(M68K_CPU_TYPE_68000);
	m68k_pulse_reset(); 
#endif

#ifdef USE_DRZ80
	drz80_init();
#endif

    //error("PC:%08X\tSP:%08X\n", m68k_get_reg(NULL,M68K_REG_PC), m68k_get_reg(NULL,M68K_REG_SP));
    gen_running = 1;
}

void gen_reset(void)
{
    /* Clear RAM */
    memset(work_ram, 0, sizeof(work_ram));
    memset(zram, 0, sizeof(zram));

    gen_running = 1;
    zreset  = 0;    /* Z80 is reset */
    zbusreq = 0;    /* Z80 has control of the Z bus */
    zbusack = 1;    /* Z80 is busy using the Z bus */
    zbank   = 0;    /* Assume default bank is 000000-007FFF */
    zirq    = 0;    /* No interrupts occuring */

    io_reset();

    /* Reset the 68000 emulator */
#ifdef USE_CYCLONE
	cyclone_reset();
#else
	m68k_pulse_reset();
#endif
     //error("PC:%08X\tSP:%08X\n", m68k_get_reg(NULL,M68K_REG_PC), m68k_get_reg(NULL,M68K_REG_SP));
#ifdef USE_DRZ80
    drz80_reset();
#else
    z80_reset(0);
    z80_set_irq_callback(z80_irq_callback); 
#endif
}

void gen_shutdown(void)
{

}

/*--------------------------------------------------------------------------*/
/* Bus controller chip functions                                            */
/*--------------------------------------------------------------------------*/

int gen_busack_r(void)
{
    return (zbusack & 1);
}

void gen_busreq_w(int state)
{
    zbusreq = (state & 1);
    zbusack = 1 ^ (zbusreq & zreset);

    if(zbusreq == 0 && zreset == 1)
    {
      if (use_z80) {
#ifdef USE_DRZ80
	drz80_execute(32);
#else
        z80_execute(32);
#endif
        }
    }
}

void gen_reset_w(int state)
{
    zreset = (state & 1);
    zbusack = 1 ^ (zbusreq & zreset);

    if(zreset == 0)
    {
        if(snd.enabled)
        {
            YM2612ResetChip();
        }
#ifdef USE_DRZ80
	drz80_reset();
#else
        z80_reset(0);
        z80_set_irq_callback(z80_irq_callback);
#endif
    }
}


void gen_bank_w(int state)
{
    zbank = ((zbank >> 1) | ((state & 1) << 23)) & 0xFF8000;
}


#ifndef USE_DRZ80
int z80_irq_callback(int param)
{
    zirq = 0;
    z80_set_irq_line(0, CLEAR_LINE);
    return 0xFF;
}
#endif

int vdp_int_ack_callback(int int_level)
{
if (int_level==4)  { vdp.hint_pending = 0; vdp.vint_pending = 0; return M68K_INT_ACK_AUTOVECTOR;}
if (int_level==6)  {
        vdp.status &= ~0x0080;
        vdp.vint_pending = 0; 
	return M68K_INT_ACK_AUTOVECTOR;
}
return M68K_INT_ACK_AUTOVECTOR;
}

int C_vdp_int_ack_callback(int int_level)
{
if (int_level==4)  { vdp.hint_pending = 0; vdp.vint_pending = 0; CycloneCPU.irq=0; }
if (int_level==6)  {
        vdp.status &= ~0x0080;
        vdp.vint_pending = 0; 
	CycloneCPU.irq=0;
	}
return CYCLONE_INT_ACK_AUTOVECTOR;
}

void bswap(uint8 *mem, int length)
{
    int i;
    for(i = 0; i < length; i += 2)
    {
        uint8 temp = mem[i+0];
        mem[i+0] = mem[i+1];
        mem[i+1] = temp;
    }
}

