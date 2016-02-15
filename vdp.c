#include "shared.h"
#include "hcnt.h"
#include "vcnt.h"
#include "cyclone.h"

extern int frame_m68k;
extern uint32 total_m68k;

#define color_update color_update_32

/* Pack and unpack CRAM data */
#define PACK_CRAM(d)    ((((d)&0xE00)>>9)|(((d)&0x0E0)>>2)|(((d)&0x00E)<<5))
#define UNPACK_CRAM(d)  ((((d)&0x1C0)>>5)|((d)&0x038)<<2|(((d)&0x007)<<9))

/* Mark a pattern as dirty */
#define MARK_BG_DIRTY(addr)                                     \
{                                                               \
    int name = (addr >> 5) & 0x7FF;                             \
    if(bg.bg_name_dirty[name] == 0)                                \
    {                                                           \
        bg.bg_name_list[bg.bg_list_index++] = name;                   \
    }                                                           \
    bg.bg_name_dirty[name] |= (1 << ((addr >> 2) & 0x07));         \
}

/* Tables that define the playfield layout */
uint32 _x_;
uint8 shift_table[] = {6, 7, 0, 8};
uint8 col_mask_table[] = {0x1F, 0x3F, 0x1F, 0x7F};
uint16 row_mask_table[] = {0x0FF, 0x1FF, 0x2FF, 0x3FF};
uint32 y_mask_table[] = {0x1FC0, 0x1FC0, 0x1F80, 0x1F00};

t_bg  bg;

t_vdp vdp;

uint8 * vc_table;

//From PicoDrive (Notaz)
static int DmaSlowBurn(int len)
{
  // test: Legend of Galahad, Time Killers
  int burn,maxlen,line=vdp.v_counter;
  maxlen=(224-line)*18;
  if(len <= maxlen)
    burn = len*(((488<<8)/18))>>8;
  else {
    burn  = maxlen*(((488<<8)/18))>>8;
    burn += (len-maxlen)*(((488<<8)/180))>>8;
  }
  return burn;
}


static int DmaSlow(int len)
{
int burn;
  if((vdp.status&8)||!(vdp.reg[1]&0x40)) { // vblank?
      burn = (len*(((488<<8)/167))>>8); // very approximate
      if(!(vdp.status&8)) burn+=burn>>1; // a hack for Legend of Galahad
  } else 
  burn = DmaSlowBurn(len);
  frame_m68k+=burn;
  if(!(vdp.status&8))
  m68k_endrun(0);
}

/*--------------------------------------------------------------------------*/
/* Init, reset, shutdown functions                                          */
/*--------------------------------------------------------------------------*/

void vdp_init(void)
{
//build table
int i;
int v=0;
    for (i=0; i<313; i++) 
	{
	if (v>=0x103) v-=56;
	vc_pal[i]=v;
	v++;
	};

if (vdp.pal) vc_table=vc_pal; else vc_table=vc_ntsc;
	
}

void vdp_reset(void)
{
    memset(vdp.sat, 0, sizeof(vdp.sat));
    memset(vdp.vram, 0, sizeof(vdp.vram));
    memset(vdp.cram, 0, sizeof(vdp.cram));
    memset(vdp.vsram, 0, sizeof(vdp.vsram));
    memset(vdp.reg, 0, sizeof(vdp.reg));

    vdp.addr = vdp.addr_latch = vdp.code = vdp.pending = vdp.buffer = vdp.status = 0;
    vdp.ntab = vdp.ntbb = vdp.ntwb = vdp.satb = vdp.hscb = 0;
    vdp.sat_base_mask = 0xFE00;
    vdp.sat_addr_mask = 0x01FF;

    /* Mark all colors as dirty to force a palette update */
    vdp.border = 0x00;

    memset(bg.bg_name_dirty, 0, sizeof(bg.bg_name_dirty));
    memset(bg.bg_name_list, 0, sizeof(bg.bg_name_list));
    bg.bg_list_index = 0;
    memset(bg.bg_pattern_cache, 0, sizeof(bg.bg_pattern_cache));

    vdp.playfield_shift = 6;
    vdp.playfield_col_mask = 0x1F;
    vdp.playfield_row_mask = 0x0FF;
    vdp.y_mask = 0x1FC0;

    vdp.hint_pending = vdp.vint_pending = 0;
    vdp.frame_end = 0xE0;
    vdp.v_counter = vdp.v_update = 0;

    /* Initialize viewport */
    bitmap.viewport.x = 0x20;
    bitmap.viewport.y = 0x20;
    bitmap.viewport.w = 256;
    bitmap.viewport.h = 224;
    bitmap.viewport.oh = 256;
    bitmap.viewport.ow = 224;
    bitmap.viewport.changed = 1;
}

void vdp_shutdown(void)
{
}


/*--------------------------------------------------------------------------*/
/* Memory access functions                                                  */
/*--------------------------------------------------------------------------*/

void vdp_ctrl_w(uint16 data)
{
    if(vdp.pending == 0)
    {
        if((data & 0xC000) == 0x8000)
        {
            uint8 r = (data >> 8) & 0x1F;
            uint8 d = (data >> 0) & 0xFF;
            vdp_reg_w(r, d);
        }
        else
        {
            vdp.pending = 1;
        }

        vdp.addr = ((vdp.addr_latch & 0xC000) | (data & 0x3FFF)) & 0xFFFF;
        vdp.code = ((vdp.code & 0x3C) | ((data >> 14) & 0x03)) & 0x3F;
    }
    else
    {
        /* Clear vdp.pending flag */
        vdp.pending = 0;

        /* Update address and vdp.code registers */
        vdp.addr = ((vdp.addr & 0x3FFF) | ((data & 3) << 14)) & 0xFFFF;
        vdp.code = ((vdp.code & 0x03) | ((data >> 2) & 0x3C)) & 0x3F;

        /* Save address bits A15 and A14 */
        vdp.addr_latch = (vdp.addr & 0xC000);

        if((vdp.code & 0x20) && (vdp.reg[1] & 0x10))
        {
            switch(vdp.reg[23] & 0xC0)
            {
                case 0x00: /* V bus to VDP DMA */
                case 0x40: /* V bus to VDP DMA */		
                    dma_vbus();
                    break;

                case 0x80: /* vdp.vram fill */
                    vdp.dma_fill = 1;
                    break;

                case 0xC0: /* vdp.vram copy */
                    dma_copy();
                    break;
            }
        }
    }
}

uint16 vdp_ctrl_r(void)
{
    uint16 temp = (0x4e71 & 0xFC00)|vdp.pal;
    vdp.pending = 0;
    vdp.status |= vdp.collision<<5;
    vdp.status |= 0x200; //vdp.fifo;
    temp |= (vdp.status & 0x03BF); // clear spr over
    vdp.status &= ~0x0020; // clear sprite hit flag on reads
    vdp.collision = 0;
    return (temp);
}

void vdp_data_w(uint16 data)
{
    /* Clear vdp.pending flag */
    vdp.pending = 0;

    switch(vdp.code & 0x0F)
    {
        case 0x01: /* vdp.vram */
            /* Byte-swap data if A0 is set */
            if(vdp.addr & 1) data = (data >> 8) | (data << 8);

            /* Copy vdp.sat data to the internal vdp.sat */
            if((vdp.addr & vdp.sat_base_mask) == vdp.satb)
            {
                *(uint16 *)&vdp.sat[vdp.addr & vdp.sat_addr_mask] = data;
            }

            /* Only write unique data to vdp.vram */
            if(data != *(uint16 *)&vdp.vram[vdp.addr & 0xFFFE])
            {
                /* Write data to vdp.vram */
                *(uint16 *)&vdp.vram[vdp.addr & 0xFFFE] = data;

                /* Update the pattern cache */
                MARK_BG_DIRTY(vdp.addr);

            }

         if (!vdp.dma)
           if ( !vdp.im2_flag && !(vdp.status&8) && (vdp.reg[1]&40) )  
	   	frame_m68k+=50;

            break;

        case 0x03: /* vdp.cram */
            {
                uint16 *p = (uint16 *)&vdp.cram[(vdp.addr & 0x7E)];
                data = PACK_CRAM(data);
                if (data != *p)
                {
                    int index = (vdp.addr >> 1) & 0x3F;
                    *p = data;
                    if(index == vdp.border)
                    {
                            color_update(0x00, *p);
                            color_update(0x40, *p);
                            color_update(0x80, *p);                            
                    }  
                    color_update(index, *p);
                }
            }
            break;
          
        case 0x05: /* vdp.vsram */
            *(uint16 *)&vdp.vsram[(vdp.addr & 0x7E)] = data;
            break;
    }

    /* Bump address register */
    vdp.addr += vdp.reg[15];


    if(vdp.dma_fill)
    {
        int length = (vdp.reg[20] << 8 | vdp.reg[19]) & 0xFFFF;
        if(!length) length = 0x10000;
        do {
            vdp.vram[(vdp.addr & 0xFFFF)] = (data >> 8) & 0xFF;
            MARK_BG_DIRTY(vdp.addr);
            vdp.addr += vdp.reg[15];
        } while(--length);
        vdp.dma_fill = 0;
    }
}


uint16 vdp_data_r(void)
{
    uint16 temp = 0;

    /* Clear vdp.pending flag */
    vdp.pending = 0;

    switch(vdp.code & 0x0F)
    {
        case 0x00: /* vdp.vram */
            temp = *(uint16 *)&vdp.vram[(vdp.addr & 0xFFFE)];
            break;

        case 0x08: /* vdp.cram */
            temp = *(uint16 *)&vdp.cram[(vdp.addr & 0x7E)];
            temp = UNPACK_CRAM(temp);
            break;

        case 0x04: /* vdp.vsram */
            temp = *(uint16 *)&vdp.vsram[(vdp.addr & 0x7E)];
            break;
    }

    /* Bump address register */
    vdp.addr += vdp.reg[15];

    /* return data */
    return (temp);
}


/*
    The vdp.reg[] array is updated at the *end* of this function, so the new
    register data can be compared with the previous data.
*/
void update_irq_line()
{
     if(vdp.reg[1]&0x20) //V-Int Enabled
       if(vdp.vint_pending) {
	 #ifdef USE_CYCLONE
                CycloneInterrupt(6);
	 #else
		m68k_set_irq(6);
	 #endif
	 return; }
     if(vdp.reg[0]&0x10)
       if(vdp.hint_pending) {
	 #ifdef USE_CYCLONE
                CycloneInterrupt(4);
	 #else
		m68k_set_irq(4);
	 #endif
	 return; }
	 #ifdef USE_CYCLONE
                CycloneInterrupt(0);
	 #else
		m68k_set_irq(0);
	 #endif
}

void vdp_reg_w(uint8 r, uint8 d)
{
    switch(r)
    {
        case 0x00: /* CTRL #1 */
            vdp.reg[r]=d;
	    update_irq_line();	
            break;

        case 0x01: /* CTRL #2 */
            vdp.reg[r]=d;
	    update_irq_line();	

            /* Change the frame timing */
            vdp.frame_end = (d & 8) ? 0xF0 : 0xE0;

            /* Check if the viewport height has actually been changed */
            if((vdp.reg[1] & 8) != (d & 8))
            {                
                /* Update the height of the viewport */
                bitmap.viewport.oh = bitmap.viewport.h;
                bitmap.viewport.h = (d & 8) ? 240 : 224;
                bitmap.viewport.changed = 1;
            }
            break;

        case 0x02: /* vdp.ntab */
            vdp.ntab = (d << 10) & 0xE000;
            break;

        case 0x03: /* vdp.ntwb */
            vdp.ntwb = (d << 10) & 0xF800;
            if(vdp.reg[12] & 1) vdp.ntwb &= 0xF000;
            break;

        case 0x04: /* vdp.ntbb */
            vdp.ntbb = (d << 13) & 0xE000;
            break;

        case 0x05: /* vdp.satb */
            vdp.sat_base_mask = (vdp.reg[12] & 1) ? 0xFC00 : 0xFE00;
            vdp.sat_addr_mask = (vdp.reg[12] & 1) ? 0x03FF : 0x01FF;
            vdp.satb = (d << 9) & vdp.sat_base_mask;
            break;

        case 0x07:
            d &= 0x3F;

            /* Check if the vdp.border color has actually changed */
            if(vdp.border != d)
            {
                /* Mark the vdp.border color as modified */
                vdp.border = d;
                color_update(0x00, *(uint16 *)&vdp.cram[(vdp.border << 1)]);
                color_update(0x40, *(uint16 *)&vdp.cram[(vdp.border << 1)]);
                color_update(0x80, *(uint16 *)&vdp.cram[(vdp.border << 1)]);                
            }
            break;

        case 0x0C:

            /* Check if the viewport width has actually been changed */
            if((vdp.reg[0x0C] & 1) != (d & 1))
            {
                /* Update the width of the viewport */
                bitmap.viewport.ow = bitmap.viewport.w;
                bitmap.viewport.w = (d & 1) ? 320 : 256;
                bitmap.viewport.changed = 1;
            }

            /* See if the S/TE mode bit has changed */
            if((vdp.reg[0x0C] & 8) != (d & 8))
            {
                int i;
                vdp.reg[0x0C] = d;

                /* Update colors */
                {
                    for(i = 0; i < 0x40; i += 1)
                    {
                        color_update(i, *(uint16 *)&vdp.cram[i << 1]);                    
                    }
                    color_update(0x00, *(uint16 *)&vdp.cram[vdp.border << 1]);
                    color_update(0x40, *(uint16 *)&vdp.cram[vdp.border << 1]);
                    color_update(0x80, *(uint16 *)&vdp.cram[vdp.border << 1]);
                }
                /* Flush palette */
            }

            /* Check interlace mode 2 setting */
            vdp.im2_flag = ((d & 0x06) == 0x06) ? 1 : 0;

            /* The following register updates check this value */
            vdp.reg[0x0C] = d;

            /* Update display-dependant registers */
            vdp_reg_w(0x03, vdp.reg[0x03]);
            vdp_reg_w(0x05, vdp.reg[0x05]);

            break;

        case 0x0D: /* HSCB */
            vdp.hscb = (d << 10) & 0xFC00;
            break;

        case 0x10: /* Playfield size */
            vdp.playfield_shift = shift_table[(d & 3)];
            vdp.playfield_col_mask = col_mask_table[(d & 3)];
            vdp.playfield_row_mask = row_mask_table[(d >> 4) & 3];
            vdp.y_mask = y_mask_table[(d & 3)];
            break;
    }

    /* Write new register value */
    vdp.reg[r] = d;
}

extern int m68ki_remaining_cycles;
extern int line_cycles;
uint16 vdp_hvc_r(void)
{
	uint8 *hctab;
	int vc, hc;

    hctab = (vdp.reg[12] & 1) ? cycle2hc40 : cycle2hc32;

    vc = vc_table[vdp.v_counter];

//From Generator
    if((vdp.reg[12]>>1)&3) {
      vc <<= 1;
      if (vc&0xf00) vc|= 1;
    }

#ifdef USE_CYCLONE
       hc = hctab[line_cycles-CycloneCPU.cycles];
#else
       hc = hctab[line_cycles-m68ki_remaining_cycles];
#endif

    return (vc << 8 | hc);
}

/*
    vdp.vram to vdp.vram copy
    Read byte from vdp.vram (source), write to vdp.vram (vdp.addr),
    bump source and add r15 to vdp.addr.

    - see how source vdp.addr is affected
      (can it make high source byte inc?)
*/
void dma_copy(void)
{
    int length = (vdp.reg[20] << 8 | vdp.reg[19]) & 0xFFFF;
    int source = (vdp.reg[22] << 8 | vdp.reg[21]) & 0xFFFF;
    if(!length) length = 0x10000;

//fprintf(stderr,"DMA Copy: Line: %d, Length: %d\n",vdp.v_counter,length);

    do {
        uint8 temp = vdp.vram[source];
        vdp.vram[vdp.addr] = temp;
        MARK_BG_DIRTY(vdp.addr);
        source = (source + 1) & 0xFFFF;
        vdp.addr = (vdp.addr + vdp.reg[15]) & 0xFFFF;
    } while (--length);

    vdp.reg[19] = (length >> 0) & 0xFF;
    vdp.reg[20] = (length >> 8) & 0xFF;
}


void dma_vbus(void)
{
    uint32 base, source = ((vdp.reg[23] & 0x7F) << 17 | vdp.reg[22] << 9 | vdp.reg[21] << 1) & 0xFFFFFE;
    uint32 length = (vdp.reg[20] << 8 | vdp.reg[19]) & 0xFFFF;

    if(!length) length = 0x10000;
    base = source;

//fprintf(stderr,"DMA VBus: Line: %d, Length: %d, %x  \n",vdp.v_counter,length, vdp.status);

    //DmaSlow(length);	

    vdp.dma=1;
    do {
        vdp_data_w(vdp_dma_r(source));
        source += 2;
        source = ((base & 0xFE0000) | (source & 0x1FFFF));
    } while (--length);
    vdp.dma=0;

    vdp.reg[19] = (length >> 0) & 0xFF;
    vdp.reg[20] = (length >> 8) & 0xFF;

    vdp.reg[21] = (source >> 1) & 0xFF;
    vdp.reg[22] = (source >> 9) & 0xFF;
    vdp.reg[23] = (vdp.reg[23] & 0x80) | ((source >> 17) & 0x7F);
}


void vdp_test_w(uint16 value)
{
}


