#include "shared.h"

static uint8 block[0x4000];
uint32 rom_size;

uint8 flag_japan;
uint8 flag_usa;
uint8 flag_europe;
uint8 hardware;
uint8 vdp_rate;
uint8 hw;
uint8 country = 0;

t_sram sram;

void deinterleave_block(uint8 *src)
{
    int i;
    memcpy(block, src, 0x4000);
    for(i = 0; i < 0x2000; i += 1)
    {
        src[i*2+0] = block[0x2000 + (i)];
        src[i*2+1] = block[0x0000 + (i)];
    }
}

int load_rom(char *filename)
{
    extern int lines_per_frame;
    int size, offset = 0;
    uint8 header[0x200];
    uint8 *ptr;

    ptr = load_archive(filename, &size);
    if(!ptr) return (0);

    if((size / 512) & 1)
    {
        int i;

        size -= 512;
        offset += 512;

        memcpy(header, ptr, 512);

        for(i = 0; i < (size / 0x4000); i += 1)
        {
            deinterleave_block(ptr + offset + (i * 0x4000));
        }
    }

    memset(cart_rom, 0, 0x400000);
    if(size > 0x400000) size = 0x400000;
    memcpy(cart_rom, ptr + offset, size);

    rom_size = size;

    /* Free allocated file data */
    free(ptr);

   hardware = 0;

   int i;
   for (i = 0x1f0; i < 0x1ff; i++)
     switch (cart_rom[i]) {
     case 'U':
       hardware |= 4;
       break;
     case 'J':
       hardware |= 1;
       break;
     case 'E':
       hardware |= 8;
       break;
     }

    if (cart_rom[0x1f0] >= '1' && cart_rom[0x1f0] <= '9') {
      hardware = cart_rom[0x1f0] - '0';
    } else if (cart_rom[0x1f0] >= 'A' && cart_rom[0x1f0] <= 'F') {
      hardware = cart_rom[0x1f0] - 'A' + 10;
    }

    if (country) hardware=country; //simple autodetect override

    //From PicoDrive
    if (hardware&8) 		{ hw=0xc0; vdp.pal=1; } // Europe
    else if (hardware&4) 	{ hw=0x80; vdp.pal=0; } // USA
    else if (hardware&2) 	{ hw=0x40; vdp.pal=1; } // Japan PAL
    else if (hardware&1)   	{ hw=0x00; vdp.pal=0; } // Japan NTSC
    else hw=0x80; // USA

     
    if (vdp.pal) {
	vdp_rate = 50; 
	lines_per_frame = 312;
	} else {
	vdp_rate = 60;
	lines_per_frame = 262;
	};

        /*SRAM*/    
if(cart_rom[0x1b1] == 'A' && cart_rom[0x1b0] == 'R')
    {
      sram.start = cart_rom[0x1b4] << 24 | cart_rom[0x1b5] << 16 | 
                   cart_rom[0x1b6] << 8  | cart_rom[0x1b7] << 0;
      sram.len = cart_rom[0x1b8] << 24 | cart_rom[0x1b9] << 16 |
  	         cart_rom[0x1ba] << 8  | cart_rom[0x1bb] << 0;

// Make sure start is even, end is odd, for alignment
// A ROM that I came across had the start and end bytes of
// the save ram the same and wouldn't work.  Fix this as seen
// fit, I know it could probably use some work. [PKH]

      sram.eeprom = 0;

      int mlen = sram.len-sram.start;
	
      	if (mlen<0) {
        	sram.start = 0x200000;
        	sram.len   = 0x004000;
		sram.end   = sram.start+sram.len;
	 	};
	if (mlen==0) {
          // EEPROM SRAM
          // what kind of EEPROMs are actually used? X24C02? X24C04? (X24C01 has only 128), but we will support up to 8K
          sram.start = sram.start & ~1; // zero address is used for clock by some games
          sram.len  = 0x2000;
	  sram.end = sram.start+2;
          sram.eeprom = 1;
		};	
	if (mlen>0) {
          	if(sram.start & 1) --sram.start;
          	if(!(sram.len & 1)) ++sram.len;
          	sram.len -= (sram.start - 1);
		sram.end = sram.start+sram.len;
		};

       	sram.mem = malloc(sram.len);

	sram.reg |= 4;
	sram.reg |= 0x10;

        // If save RAM does not overlap main ROM, set it active by default since
	// a few games can't manage to properly switch it on/off.

	if(sram.start >= rom_size)
	sram.reg = 1;

} else {
        sram.start = sram.len = 0;
        sram.mem = NULL;
        }

//    fprintf(stderr,"EEPROM: %x; Active: %x; %x-%x-%x; %x\n",sram.eeprom, sram.reg&1, sram.start, sram.len, sram.end, sram.mem);

    return (1);
}

