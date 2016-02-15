#include "shared.h"

//This code was fully taken from Notaz's Picodrive

// rarely used EEPROM SRAM code
// known games which use this:
// Wonder Boy in Monster World, Megaman - The Wily Wars (X24C01, 128 bytes)
// NFL Quarterback Club*, Frank Thomas Big Hurt Baseball (X24C04?)
// College Slam, Blockbuster World Video Game Championship II, NBA Jam (X24C04?)
// HardBall '95

// the above sports games use addr 0x200000 for SCL line (handled in Memory.c)

uint32 lastSSRamWrite = 0xffff0000;

// sram_reg: LAtd sela (L=pending SCL, A=pending SDA, t=type(1==uses 0x200000 for SCL and 2K bytes),
//                      d=SRAM was detected (header or by access), s=started, e=save is EEPROM, l=old SCL, a=old SDA)
void SRAMWriteEEPROM(uint32 d) // ???? ??la (l=SCL, a=SDA)
{
  unsigned int sreg = sram.reg, saddr = sram.addr, scyc = sram.cycle, ssa = sram.slave;

  //dprintf("[%02x]", d);
  sreg |= saddr&0xc000; // we store word count in add reg: dw?a aaaa ...  (d=word count detected, w=words(0==use 2 words, else 1))
  saddr&=0x1fff;

  if(sreg & d & 2) {
    // SCL was and is still high..
    if((sreg & 1) && !(d&1)) {
      // ..and SDA went low, means it's a start command, so clear internal addr reg and clock counter
      //dprintf("-start-");
	  if(!(sreg&0x8000) && scyc >= 9) {
	    if(scyc != 28) sreg |= 0x4000; // 1 word
        //dprintf("detected word count: %i", scyc==28 ? 2 : 1);
		sreg |= 0x8000;
	  }
      //saddr = 0;
      scyc = 0;
      sreg |= 8;
    } else if(!(sreg & 1) && (d&1)) {
      // SDA went high == stop command
      //dprintf("-stop-");
      sreg &= ~8;
    }
  }
  else if((sreg & 8) && !(sreg & 2) && (d&2)) {
    // we are started and SCL went high - next cycle
    scyc++; // pre-increment
	if(sreg & 0x20) {
      // X24C02+
	  if((ssa&1) && scyc == 18) {
	    scyc = 9;
		saddr++; // next address in read mode
		if(sreg&0x4000) saddr&=0xff; else saddr&=0x1fff; // mask
	  }
	  else if((sreg&0x4000) && scyc == 27) scyc = 18;
	  else if(scyc == 36) scyc = 27;
	} else {
	  // X24C01
      if(scyc == 18) {
        scyc = 9;  // wrap
        if(saddr&1) { saddr+=2; saddr&=0xff; } // next addr in read mode
	  }
	}
	//dprintf("scyc: %i", scyc);
  }
  else if((sreg & 8) && (sreg & 2) && !(d&2)) {
    // we are started and SCL went low (falling edge)
    if(sreg & 0x20) {
	  // X24C02+
	  if(scyc == 9 || scyc == 18 || scyc == 27); // ACK cycles
	  else if( (!(sreg&0x4000) && scyc > 27) || ((sreg&0x4000) && scyc > 18) ) {
        if(!(ssa&1)) {
          // data write
          unsigned char *pm=sram.mem+saddr;
          *pm <<= 1; *pm |= d&1;
          if(scyc == 26 || scyc == 35) {
            saddr=(saddr&~0xf)|((saddr+1)&0xf); // only 4 (?) lowest bits are incremented
            //dprintf("w done: %02x; addr inc: %x", *pm, saddr);
          }
          //SRam.changed = 1;
        }
      } else if(scyc > 9) {
        if(!(ssa&1)) {
          // we latch another addr bit
		  saddr<<=1;
		  if(sreg&0x4000) saddr&=0xff; else saddr&=0x1fff; // mask
		  saddr|=d&1;
          //if(scyc==17||scyc==26) dprintf("addr reg done: %x", saddr);
		}
      } else {
	    // slave address
		ssa<<=1; ssa|=d&1;
        //if(scyc==8) dprintf("slave done: %x", ssa);
      }
	} else {
	  // X24C01
      if(scyc == 9); // ACK cycle, do nothing
      else if(scyc > 9) {
        if(!(saddr&1)) {
          // data write
          unsigned char *pm=sram.mem+(saddr>>1);
          *pm <<= 1; *pm |= d&1;
          if(scyc == 17) {
            saddr=(saddr&0xf9)|((saddr+2)&6); // only 2 lowest bits are incremented
            //dprintf("addr inc: %x", saddr>>1);
          }
          //SRam.changed = 1;
        }
      } else {
        // we latch another addr bit
        saddr<<=1; saddr|=d&1; saddr&=0xff;
        //if(scyc==8) dprintf("addr done: %x", saddr>>1);
      }
	}
  }

  sreg &= ~3; sreg |= d&3; // remember SCL and SDA
  sram.reg  = (unsigned char)  sreg;
  sram.addr = (unsigned short)(saddr|(sreg&0xc000));
  sram.cycle= (unsigned char)  scyc;
  sram.slave= (unsigned char)  ssa;
}

uint32 SRAMReadEEPROM()
{
  unsigned int shift, d=0;
  unsigned int sreg, saddr, scyc, ssa;

  // flush last pending write
  SRAMWriteEEPROM(sram.reg>>6);

  sreg = sram.reg; saddr = sram.addr&0x1fff; scyc = sram.cycle; ssa = sram.slave;
//  if(!(sreg & 2) && (sreg&0x80)) scyc++; // take care of raising edge now to compensate lag

  if(m68k_total_cycles()-lastSSRamWrite < 46) {
    // data was just written, there was no time to respond (used by sports games)
    d = (sreg>>6)&1;
  } else if((sreg & 8) && scyc > 9 && scyc != 18 && scyc != 27) {
    // started and first command word received
    shift = 17-scyc;
    if(sreg & 0x20) {
	  // X24C02+
      if(ssa&1) {
        //dprintf("read: addr %02x, cycle %i, reg %02x", saddr, scyc, sreg);
	    d = (sram.mem[saddr]>>shift)&1;
	  }
	} else {
	  // X24C01
      if(saddr&1) {
		d = (sram.mem[saddr>>1]>>shift)&1;
	  }
	}
  }
  //else dprintf("r ack");

  return d;
}

void SRAMUpdPending(uint32 a, uint32 d)
{
  unsigned int sreg = sram.reg;

  if(!(a&1)) sreg|=0x20;

  if(sreg&0x20) { // address through 0x200000
    if(!(a&1)) {
      sreg&=~0x80;
      sreg|=d<<7;
    } else {
      sreg&=~0x40;
      sreg|=(d<<6)&0x40;
    }
  } else {
    sreg&=~0xc0;
    sreg|=d<<6;
  }

  sram.reg = (unsigned char) sreg;
}
