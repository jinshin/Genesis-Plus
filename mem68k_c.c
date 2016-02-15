#include "shared.h"

uint16 fake_bus;

unsigned int m68k_read_bus_16_c(unsigned int address)
{
    uint16 temp = 0x4e71;

    if(address >= 0xC00000)
    {
        return (temp);
    }
    else
    {
        return (temp & 0xFF00);
    }
}

unsigned int m68k_read_bus_8_c(unsigned int address)
{
    uint16 temp = m68k_read_bus_16_c(address);
    return ((address & 1) ? (temp & 0xFF) : (temp >> 8));
}

/*--------------------------------------------------------------------------*/
/* 68000 memory handlers                                                    */
/*--------------------------------------------------------------------------*/

unsigned int m68k_read_memory_8_c(unsigned int address)
{

 address&=0xffffff;

//n0p
 //if ((address>>21)==7) { return READ_BYTE(work_ram + (address & 0xFFFF)); }
 if ((address&0xe00000)==0xe00000) { uint8 d = *(uint8 *)(work_ram+((address^1)&0xffff)); return d; } //Ram

    // Saveram
    if ((sram.mem) && (address >= sram.start) && (address < sram.end)) {
         if (!sram.eeprom) {
               if (sram.reg&1) return sram.mem[(address^1) - sram.start]; 
	       } else { 
               return SRAMReadEEPROM(); }
    }

    switch((address >> 21) & 7)
    {
        case 0: /* ROM */
        case 1:
            return READ_BYTE(cart_rom + address);
        case 5: /* Z80 & I/O */
            if(address <= 0xA0FFFF)
            {
                if(zbusack == 1)
                {
                    /* Z80 controls Z bus */
                    return (m68k_read_bus_8_c(address));
                }
                else
                {
                    /* Read data from Z bus */
                    switch(address & 0x6000)
                    {
                        case 0x0000: /* RAM */
                        case 0x2000:
                            return (zram[(address & 0x1FFF)]);
    
                        case 0x4000: /* YM2612 */
                            return (fm_read(address & 3));

                        case 0x6000: /* Unused */
                            return (0xFF);
                    }
                }
            }
            else
            {
            
                switch((address >> 8) & 0xFF)
                {
                    case 0x00: /* I/O CHIP */
                        if(address <= 0xA1001F)
                        {
                            return (gen_io_r((address >> 1) & 0x0F));
                        }
                        else
                        {
                            return (m68k_read_bus_8_c(address));
                        }
                        break;

                    case 0x10: /* MEMORY MODE */
                        return (m68k_read_bus_8_c(address));

                    case 0x11: /* BUSACK */
                        if((address & 1) == 0)
                        {
                            return (gen_busack_r() | (m68k_read_bus_8_c(address) & 0xFE));
                        }
                        else
                        return (m68k_read_bus_8_c(address));

                    case 0x12: /* RESET */
                    case 0x13: /* TIME */
                    case 0x20: /* UNKNOWN */
                    case 0x30: /* UNKNOWN */
                        return (m68k_read_bus_8_c(address));

                    default: /* Unused */
                        return (-1);
                }
            }
            break;

        case 6: /* VDP */
            if((address & 0xE700E0) == 0xC00000)
            {
                switch(address & 0x1F)
                {
                    case 0x00: /* DATA */
                    case 0x02:
                        return (vdp_data_r() >> 8);

                    case 0x01: /* DATA */
                    case 0x03:
                        return (vdp_data_r() & 0xFF);

                    case 0x04: /* CTRL */
                    case 0x06:
                        return ((m68k_read_bus_8_c(address) & 0xFC) | (vdp_ctrl_r() >> 8));

                    case 0x05: /* CTRL */
                    case 0x07:
                        return (vdp_ctrl_r() & 0xFF);

                    case 0x08: /* HVC */
                    case 0x0A:
                    case 0x0C:
                    case 0x0E:
                        return (vdp_hvc_r() >> 8);

                    case 0x09: /* HVC */
                    case 0x0B:
                    case 0x0D:
                    case 0x0F:
                        return (vdp_hvc_r() & 0xFF);

                    case 0x10: /* PSG */
                    case 0x11:
                    case 0x12:
                    case 0x13:
                    case 0x14:
                    case 0x15:
                    case 0x16:
                    case 0x17:
                        return (-1);

                    case 0x18: /* Unused */
                    case 0x19:
                    case 0x1A:
                    case 0x1B:
                    case 0x1C:
                    case 0x1D:
                    case 0x1E:
                    case 0x1F:
                        return (m68k_read_bus_8_c(address));
                }
            }
            else
            {
                /* Unused */
                return (-1);
            }
            break;

        case 2: /* Unused */
        case 3:
            return (m68k_read_bus_8_c(address));

        case 4: /* Unused */
            return (-1);
    }

    return -1;
}


unsigned int m68k_read_memory_16_c(unsigned int address)
{

  address&=0xfffffe;

  //n0p
  //if ((address>>21)==7) { return READ_WORD(work_ram + (address & 0xFFFF)); }
  if ((address&0xe00000)==0xe00000) { uint16 d=*(uint16 *)(work_ram+(address&0xffff)); return d; } // Ram

    switch((address >> 21) & 7)
    {

        case 0: /* ROM */
        case 1:
            return READ_WORD(cart_rom + (address) );

        case 5: /* Z80 & I/O */
            if(address <= 0xA0FFFF)
            {
                if(zbusack == 1)
                {
                    return (m68k_read_bus_16_c(address));
                }
                else
                {
                    uint8 temp;

                    switch(address & 0x6000)
                    {
                        case 0x0000: /* RAM */
                        case 0x2000:
                            temp = zram[address & 0x1FFF];
                            return (temp << 8 | temp);
    
                        case 0x4000: /* YM2612 */
                            temp = fm_read(address & 3);
                            return (temp << 8 | temp);

                        case 0x6000:
                                    return (0xFFFF);
                            break;
                    }
                }
            }
            else
            {
                if(address <= 0xA1001F)
                {
                    uint8 temp = gen_io_r((address >> 1) & 0x0F);
                    return (temp << 8 | temp);
                }
                else
                {
                    switch((address >> 8) & 0xFF)
                    {
                        case 0x10: /* MEMORY MODE */
                            return (m68k_read_bus_16_c(address));

                        case 0x11: /* BUSACK */
                            return ((fake_bus++)&0xFEFF | (gen_busack_r() << 8));  //Hack from Notaz
                            //return ((m68k_read_bus_16_c(address) & 0xFEFF) | (gen_busack_r() << 8));

                        case 0x12: /* RESET */
                        case 0x13: /* TIME */
                        case 0x20: /* UNKNOWN */
                        case 0x30: /* UNKNOWN */
                            return (m68k_read_bus_16_c(address));

                        default: /* Unused */
                            return (-1);
                    }
                }
            }
            break;

        case 6:
            if((address & 0xE700E0) == 0xC00000)
            {
                switch(address & 0x1F)
                {
                    case 0x00: /* DATA */
                    case 0x02:
                        return (vdp_data_r());

                    case 0x04: /* CTRL */                  
                    case 0x06:                             
                        return (vdp_ctrl_r() | (m68k_read_bus_16_c(address) & 0xFC00));

                    case 0x08: /* HVC */
                    case 0x0A:
                    case 0x0C:
                    case 0x0E:
                        return (vdp_hvc_r());

                    case 0x10: /* PSG */
                    case 0x12:
                    case 0x14:
                    case 0x16:
                        return (-1);

                    case 0x18: /* Unused */
                    case 0x1A:
                    case 0x1C:
                    case 0x1E:
                        return (m68k_read_bus_16_c(address));
                }
            }
            else
            {
                return (-1);
            }
            break;

        case 2:
        case 3:
            return (m68k_read_bus_16_c(address));

        case 4:
            return (-1);
    }

    return (0xA5A5);
}


unsigned int m68k_read_memory_32_c(unsigned int address)
{
    address&=0xfffffe;
    if ((address&0xe00000)==0xe00000) { uint16 *pm=(uint16 *)(work_ram+(address&0xffff)); uint32 d = (pm[0]<<16)|pm[1]; return d; } // Ram
    /* Split into 2 reads */
    return (m68k_read_memory_16_c(address + 0) << 16 | m68k_read_memory_16_c(address + 2));
}


void m68k_write_memory_8_c(unsigned int address, unsigned char value)
{

  address&=0xffffff;

  //n0p
  if ((address&0xe00000)==0xe00000) { uint8 *pm=(uint8 *)(work_ram+((address^1)&0xffff)); pm[0]=value; return; } // Ram
  //if ((address>>21)==7) { WRITE_BYTE(work_ram + (address & 0xFFFF), value); return; }


 // Saveram write
 if  (sram.mem && (address >= sram.start) && (address<sram.end)) {
        if (!sram.eeprom) {
	     if (!(sram.reg&2)) { sram.mem[(address^1) - sram.start] = value; return; };
	   } else {
	    if(m68k_total_cycles()-lastSSRamWrite < 46) {
	 	SRAMUpdPending(address, value);
	    } else {
	        SRAMWriteEEPROM(sram.reg>>6); // execute pending
		SRAMUpdPending(address, value);
        	lastSSRamWrite = m68k_total_cycles();
	    }
        }	
  }

    switch((address >> 21) & 7)
    {
        case 6:
            if((address & 0xE700E0) == 0xC00000)
            {
                switch(address & 0x1F)
                {
                    case 0x00: /* DATA */
                    case 0x01:
                    case 0x02:
                    case 0x03:
                        vdp_data_w((unsigned short)value << 8 | value);
                        return;

                    case 0x04: /* CTRL */
                    case 0x05:
                    case 0x06:
                    case 0x07:
                        vdp_ctrl_w((unsigned short)value << 8 | value);
                        return;

                    case 0x08: /* HVC */
                    case 0x09:
                    case 0x0A:
                    case 0x0B:
                    case 0x0C:
                    case 0x0D:
                    case 0x0E:
                    case 0x0F:
                        return;

                    case 0x10: /* PSG */
                    case 0x12:
                    case 0x14:
                    case 0x16:
                        return;

                    case 0x11: /* PSG */
                    case 0x13:
                    case 0x15:
                    case 0x17:
                        psg_write(value);
                        return;

                    case 0x18: /* Unused */
                    case 0x19:
                    case 0x1A:
                    case 0x1B:
                    case 0x1C:
                    case 0x1D:
                    case 0x1E:
                    case 0x1F:
                        return;
                }
            }
            else
            {
                return;
            }

        case 5:
            if(address <= 0xA0FFFF)
            {
                if(zbusack == 1)
                {
                    return;
                }
                else
                {
                    switch(address & 0x6000)
                    {
                        case 0x0000:
                        case 0x2000:
                            zram[(address & 0x1FFF)] = value;
                            return;
    
                        case 0x4000:
                            fm_write(address, value);
                            return;

                        case 0x6000:
                            switch(address & 0xFF00)
                            {
                                case 0x6000: /* BANK */
                                    gen_bank_w(value & 1);
                                    return;

                                case 0x7F00: /* VDP */
                                    return;

                                default: /* Unused */
                                    return;
                            }
                            break;
                    }
                }
            }
            else
            {
                if(address <= 0xA1001F)
                {
                    /* I/O chip only gets /LWR */
                    if(address & 1)
                        gen_io_w((address >> 1) & 0x0F, value);
                    return;
                }
                else
                {
                   	// Saveram status (thanks Steve :)
					  if(address==0xA130f1)
					  {
					    // Bit 0: 0=rom active, 1=sram active
					    // Bit 1: 0=writeable,  1=write protect
					    sram.reg    = value & 3;
					  }

                
                    /* Bus control chip registers */
                    switch((address >> 8) & 0xFF)
                    {
                        case 0x10: /* MEMORY MODE */
                            return;

                        case 0x11: /* BUSREQ */
                            if((address & 1) == 0)
                            {
                                gen_busreq_w(value & 1);
                            }
                            return;

                        case 0x12: /* RESET */
                            gen_reset_w(value & 1);
                            return;
                    }
                }
            }
            break;
    }

}




void m68k_write_memory_16_c(unsigned int address, unsigned short value)
{

  address&=0xfffffe;

  //n0p
  //if ((address>>21)==7) { WRITE_WORD(work_ram + (address & 0xFFFF), value); return; }
  if ((address&0xe00000)==0xe00000) { *(uint16 *)(work_ram+(address&0xfffe))=value; return; } // Ram

    switch((address >> 21) & 7)
    {
        case 5: /* Z80 area, I/O chip, miscellaneous. */
            if(address <= 0xA0FFFF)
            {
                /* Writes are ignored when the Z80 hogs the Z-bus */
                if(zbusack == 1) {
                    return;
                }

                /* Write into Z80 address space */
                switch(address & 0x6000)
                {
                    case 0x0000: /* Work RAM */
                    case 0x2000: /* Work RAM */
                        zram[(address & 0x1FFF)] = (value >> 8) & 0xFF;
                        return;
    
                    case 0x4000: /* YM2612 */
                        fm_write(address, (value >> 8) & 0xFF);
                        return;

                    case 0x6000: /* Bank register and VDP */
                        if ((address & 0x7F00)==0x6000)
                        {
                                gen_bank_w((value >> 8) & 1);
                                return;
                        }
                        break;
                }
            }
            else
            {
                /* I/O chip */
                if(address <= 0xA1001F)
                {
                    gen_io_w((address >> 1) & 0x0F, value & 0x00FF);
                    return;
                }
                else
                {
                    /* Bus control chip registers */
                    switch((address >> 8) & 0xFF)
                    {
                        case 0x10: /* MEMORY MODE */
                            return;

                        case 0x11: /* BUSREQ */
                            gen_busreq_w((value >> 8) & 1);
                            return;

                        case 0x12: /* RESET */
                            gen_reset_w((value >> 8) & 1);
                            return;
                    }
                }
            }
            break;

        case 6: /* VDP */
            if((address & 0xE700E0) == 0xC00000)
            {
                switch(address & 0x1C)
                {
                    case 0x00: /* DATA */
                        vdp_data_w(value);
                        return;

                    case 0x04: /* CTRL */
                        vdp_ctrl_w(value);
                        return;

                    case 0x10: /* PSG */
                    case 0x14: /* PSG */
                        psg_write(value & 0xFF);
                        return;
                }
            }

    }
}


void m68k_write_memory_32_c(unsigned int address, unsigned int value)
{
  address&=0xfffffe;

  if ((address&0xe00000)==0xe00000)
  {
    // Ram:
    uint16 *pm=(uint16 *)(work_ram+(address&0xfffe));
    pm[0]=(uint16)(value>>16); pm[1]=(uint16)value;
    return;
  }

    /* Split into 2 writes */
    m68k_write_memory_16_c(address, (value >> 16) & 0xFFFF);
    m68k_write_memory_16_c(address + 2, value & 0xFFFF);
}
