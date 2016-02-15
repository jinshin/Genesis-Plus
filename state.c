// state.c

#include "shared.h"
#include "genesis.h"
#include "cyclone.h"
#include "state.h"
#include "vdp.h"
#include "sn76496.h"

extern void AddMessage (char * string, int ttl);

#define SIG	'GP'

extern char SaveDir[260];
extern char RomName[260];

void load_sram(void)
{
	STREAM file;
	char chaine[256];  	
	sprintf(chaine,"%s\\%s%s",SaveDir,RomName,".srm");
	file=OPEN_STREAM(chaine,"rb");
	if (!file) return;
	READ(sram.mem,sram.len);
	CLOSE_STREAM(file);
}

void save_sram(void)
{
	STREAM file;
	char chaine[256];  	
	sprintf(chaine,"%s\\%s%s",SaveDir,RomName,".srm");
	file=OPEN_STREAM(chaine,"wb7");
	if (!file) return;
	WRITE(sram.mem,sram.len);
	CLOSE_STREAM(file);
}

void GetStateFileName(int num,char *str)
{
	sprintf(str,"%s\\%s%s%d",SaveDir,RomName,".gs",num);
}

void state_delete(int num)
{
	char savename[255];
	GetStateFileName(num,savename);
#ifndef _MSC_VER
	unlink(savename);
#endif
}

uint32 dummy; //For compability

uint8 cyc_state[0x80];

extern uint8	fm_regs[0x200];
extern uint8	fm_status;
extern uint8	fm_addr;
extern uint32	fm_mode;
extern int32	fm_ta;   //index
extern int32	fm_tac;  //base
extern int32	fm_tat;  //count
extern uint8	fm_tb;   //index
extern int32	fm_tbc;  //base
extern int32	fm_tbt;  //count


int state_load(int num,int onlytest)
{
	uint32	tmp32;
	STREAM	file;
	uint32	sig;
	int i;
	int		height, width;
	char savename[255];
	char str[80];
	
	GetStateFileName(num,savename);

	file = OPEN_STREAM(savename, "rb");
	if (file == NULL)
	{

		return -1;
	}
	
	READ(&sig, 4);
	if (sig != SIG)
	{
		CLOSE_STREAM(file);
		return -1;
	}
	
	if (onlytest)
	{
		CLOSE_STREAM(file);
		return 0;
	}

	// gen.c stuff
	READ(work_ram, 0x10000);
	READ(zram, 0x2000);
	READ(&zbusreq, 1);
	READ(&zreset, 1);
	READ(&zbusack, 1);
	READ(&zirq, 1);
	READ(&zbank, 4);

	// io.c stuff
	READ(io_reg, 0x10);
	
	// vdp.c stuff
	READ(vdp.sat, 0x400);
	READ(vdp.vram, 0x10000);
	READ(vdp.cram, 0x80);
	READ(vdp.vsram, 0x80);
	READ(vdp.reg, 0x20);
	READ(&vdp.addr, 2);
	READ(&vdp.addr_latch, 2);
	READ(&vdp.code, 1);
	READ(&vdp.pending, 1);
	READ(&vdp.buffer, 2);
	READ(&vdp.status, 2);
	READ(&vdp.ntab, 2);
	READ(&vdp.ntbb, 2);
	READ(&vdp.ntwb, 2);
	READ(&vdp.satb, 2);
	READ(&vdp.hscb, 2);
	READ(&vdp.sat_base_mask, 2);
	READ(&vdp.sat_addr_mask, 2);
	READ(&vdp.border, 1);
	READ(&vdp.playfield_shift, 1);
	READ(&vdp.playfield_col_mask, 1);
	READ(&vdp.playfield_row_mask, 2);
	READ(&vdp.y_mask, 4);
	READ(&vdp.hint_pending, 4);
	READ(&vdp.vint_pending, 4);
	READ(&dummy, 4);
	READ(&vdp.dma_fill, 4);
	READ(&vdp.im2_flag, 4);
	READ(&vdp.frame_end, 4);
	READ(&vdp.v_counter, 4);
	READ(&vdp.v_update, 4);

//extern uint8	fm_regs[0x200];
//extern uint8	fm_status;
//extern uint8	fm_addr;
//extern uint32	fm_mode;
//extern int32	fm_ta;   //index
//extern int32	fm_tac;  //base
//extern int32	fm_tat;  //count
//extern uint8	fm_tb;   //index
//extern int32	fm_tbc;  //base
//extern int32	fm_tbt;  //count
	
	// sound.c stuff
	//READ(fm_reg[0], 0x100);
	//READ(fm_reg[1], 0x100);

	READ(&fm_regs[0],0x200);

	//READ(fm_latch, 2);

	READ(&fm_addr,1);
	READ(&dummy,1);

	READ(&fm_status, 1);
	READ(&dummy,1);

	READ(&fm_mode, sizeof(int));
	READ(&dummy, sizeof(int));
	READ(&fm_tat, sizeof(int));
	READ(&fm_tac, sizeof(int));
	READ(&fm_ta, sizeof(int));

	READ(&dummy, sizeof(int));
	READ(&dummy, sizeof(int));
	READ(&fm_tbt, sizeof(int));
	READ(&fm_tbc, sizeof(int));
	READ(&fm_tb, 1);
	READ(&dummy, 3);
	
	// Window size
	READ(&height, sizeof(int));
	READ(&width, sizeof(int));
	if (height != bitmap.viewport.h)
	{
		bitmap.viewport.oh = bitmap.viewport.h;
		bitmap.viewport.h = height;
		bitmap.viewport.changed = 1;
	}
	if (width != bitmap.viewport.w)
	{
		bitmap.viewport.ow = bitmap.viewport.w;
		bitmap.viewport.w = width;
		bitmap.viewport.changed = 1;
	}

	// 68000 CPU

#ifdef USE_CYCLONE
	{

	READ(&cyc_state, 0x80);
	CycloneUnpack(&CycloneCPU,&cyc_state);

	}
#else

	{
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_D0, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_D1, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_D2, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_D3, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_D4, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_D5, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_D6, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_D7, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_A0, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_A1, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_A2, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_A3, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_A4, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_A5, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_A6, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_A7, tmp32);
	READ(&tmp32, 4); m68k_set_reg(M68K_REG_PC, tmp32);	
	READ(&tmp32, 2); m68k_set_reg(M68K_REG_SR, tmp32);
	}
#endif

#ifdef USE_DRZ80 
/*
  unsigned int Z80A;            // 0x00 - A Register:   0xAA------
  unsigned int Z80F;            // 0x04 - F Register:   0x------FF
  unsigned int Z80BC;           // 0x08 - BC Registers: 0xBBCC----
  unsigned int Z80DE;           // 0x0C - DE Registers: 0xDDEE----
  unsigned int Z80HL;           // 0x10 - HL Registers: 0xHHLL----
  unsigned int Z80PC;           // 0x14 - PC Program Counter (Memory Base + PC)
  unsigned int Z80PC_BASE;      // 0x18 - PC Program Counter (Memory Base)
  unsigned int Z80SP;           // 0x1C - SP Stack Pointer (Memory Base + PC)
  unsigned int Z80SP_BASE;      // 0x20 - SP Stack Pointer (Memory Base)
  unsigned int Z80IX;           // 0x24 - IX Index Register
  unsigned int Z80IY;           // 0x28 - IY Index Register
  unsigned int Z80I;            // 0x2C - I Interrupt Register
  unsigned int Z80A2;           // 0x30 - A' Register:    0xAA------
  unsigned int Z80F2;           // 0x34 - F' Register:    0x------FF
  unsigned int Z80BC2;          // 0x38 - B'C' Registers: 0xBBCC----
  unsigned int Z80DE2;          // 0x3C - D'E' Registers: 0xDDEE----
  unsigned int Z80HL2;          // 0x40 - H'L' Registers: 0xHHLL----
  unsigned char Z80_IRQ;        // 0x44 - Set IRQ Number
  unsigned char Z80IF;          // 0x45 - Interrupt Flags:  bit1=_IFF1, bit2=_IFF2, bit3=_HALT
  unsigned char Z80IM;          // 0x46 - Set IRQ Mode
*/
	READ(&tmp32, 4); //z80_set_reg(Z80_PC, tmp32);
	DrZ80CPU.Z80PC = DrZ80CPU.z80_rebasePC(tmp32);
	READ(&tmp32, 4); //z80_set_reg(Z80_SP, tmp32);
	DrZ80CPU.Z80SP = DrZ80CPU.z80_rebaseSP(tmp32);
	READ(&tmp32, 4); //z80_set_reg(Z80_AF, tmp32);
        DrZ80CPU.Z80A = (tmp32<<16)&0xFF000000;
        DrZ80CPU.Z80F = tmp32&0x000000FF;
	READ(&tmp32, 4); //z80_set_reg(Z80_BC, tmp32);
	DrZ80CPU.Z80BC = (tmp32<<16);
	READ(&tmp32, 4); //z80_set_reg(Z80_DE, tmp32);
	DrZ80CPU.Z80DE = (tmp32<<16);
	READ(&tmp32, 4); //z80_set_reg(Z80_HL, tmp32);
	DrZ80CPU.Z80HL = (tmp32<<16);
	READ(&tmp32, 4); //z80_set_reg(Z80_IX, tmp32);
	DrZ80CPU.Z80IX = tmp32;
	READ(&tmp32, 4); //z80_set_reg(Z80_IY, tmp32);
	DrZ80CPU.Z80IY = tmp32;
	READ(&tmp32, 4); //z80_set_reg(Z80_R, tmp32);
	//Nothing?
	READ(&tmp32, 4); //z80_set_reg(Z80_I, tmp32);
	DrZ80CPU.Z80I = tmp32;
	READ(&tmp32, 4); //z80_set_reg(Z80_AF2, tmp32);
        DrZ80CPU.Z80A2 = (tmp32<<16)&0xFF000000;
        DrZ80CPU.Z80F2 = tmp32&0x000000FF;
	READ(&tmp32, 4); //z80_set_reg(Z80_BC2, tmp32);
	DrZ80CPU.Z80BC2 = (tmp32<<16);
	READ(&tmp32, 4); //z80_set_reg(Z80_DE2, tmp32);
	DrZ80CPU.Z80DE2 = (tmp32<<16);
	READ(&tmp32, 4); //z80_set_reg(Z80_HL2, tmp32);
	DrZ80CPU.Z80HL2 = (tmp32<<16);
	READ(&tmp32, 4); //z80_set_reg(Z80_IM, tmp32);
        DrZ80CPU.Z80IM = tmp32;
	READ(&tmp32, 4); //z80_set_reg(Z80_IFF1, tmp32);
	if (tmp32) DrZ80CPU.Z80IF=1; else DrZ80CPU.Z80IF=0;
	READ(&tmp32, 4); //z80_set_reg(Z80_IFF2, tmp32);
	if (tmp32) DrZ80CPU.Z80IF|=2; 
	READ(&tmp32, 4); //z80_set_reg(Z80_HALT, tmp32);
	if (tmp32) DrZ80CPU.Z80IF|=4; 
	READ(&tmp32, 4); //z80_set_reg(Z80_NMI_STATE, tmp32);
	//Nothing?
	READ(&tmp32, 4); //z80_set_reg(Z80_IRQ_STATE, tmp32);
        DrZ80CPU.Z80_IRQ = tmp32;
	READ(&tmp32, 4); 
	READ(&tmp32, 4); 
	READ(&tmp32, 4); 
	READ(&tmp32, 4); 
#else
	// MAME Z80 CPU
	READ(&tmp32, 4); z80_set_reg(Z80_PC, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_SP, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_AF, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_BC, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_DE, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_HL, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_IX, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_IY, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_R, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_I, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_AF2, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_BC2, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_DE2, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_HL2, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_IM, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_IFF1, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_IFF2, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_HALT, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_NMI_STATE, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_IRQ_STATE, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_DC0, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_DC1, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_DC2, tmp32);
	READ(&tmp32, 4); z80_set_reg(Z80_DC3, tmp32);
#endif

        if (READ(&tmp32, 4)==4) {
#ifdef USE_CYCLONE
        //CycloneCPU.osp=tmp32;
#else
	m68k_set_reg(M68K_REG_USP, tmp32);
#endif
        }

        READ(&sn[0],sizeof(sn[0]));

	// Close it
	CLOSE_STREAM(file);
	
	// Remake cache
	/*make_bg_pattern_cache();
	vdp_restore_colours();*/
	for (i=0;i<0x800;i++) 
    {
	    bg.bg_name_list[i]=i;
	    bg.bg_name_dirty[i]=0xFF;
    }
    bg.bg_list_index=0x800;

#define color_update color_update_32

    {
    	for(i = 0; i < 0x40; i += 1)
        {
        	color_update(i, *(uint16 *)&vdp.cram[i << 1]);                    
        }
        color_update(0x00, *(uint16 *)&vdp.cram[vdp.border << 1]);
        color_update(0x40, *(uint16 *)&vdp.cram[vdp.border << 1]);
        color_update(0x80, *(uint16 *)&vdp.cram[vdp.border << 1]);
    }    					
		
	// Flush fm
	//fm_flush();

extern void YM2612Load(void);

	if (snd.enabled) YM2612Load();
	
	// Success
        if (num!=0) {
        sprintf(str,"Loaded from %s",savename);
	AddMessage(str,40); }

	return 0;
}

int state_save(int num)
{
	uint32	tmp32;
	STREAM	file;
	uint32	sig;	
	char savename[255];
	char str[80];
	
	GetStateFileName(num,savename);
	
	fprintf(stderr,"%s\n",savename);

	file = OPEN_STREAM(savename, "wb7");
	if (file == NULL)
	{
		fprintf(stderr,"ErrNo: %d\n",errno);
		return -1;
	}
	
	fprintf(stderr,"Saving\n");

extern void YM2612Save(void);

if (snd.enabled) YM2612Save();

	// Header
	sig = SIG;
	WRITE(&sig, 4);

	// gen.c stuff
	WRITE(work_ram, 0x10000);
	WRITE(zram, 0x2000);
	WRITE(&zbusreq, 1);
	WRITE(&zreset, 1);
	WRITE(&zbusack, 1);
	WRITE(&zirq, 1);
	WRITE(&zbank, 4);

	// io.c stuff
	WRITE(io_reg, 0x10);
	
	// vdp.c stuff
	WRITE(vdp.sat, 0x400);
	WRITE(vdp.vram, 0x10000);
	WRITE(vdp.cram, 0x80);
	WRITE(vdp.vsram, 0x80);
	WRITE(vdp.reg, 0x20);
	WRITE(&vdp.addr, 2);
	WRITE(&vdp.addr_latch, 2);
	WRITE(&vdp.code, 1);
	WRITE(&vdp.pending, 1);
	WRITE(&vdp.buffer, 2);
	WRITE(&vdp.status, 2);
	WRITE(&vdp.ntab, 2);
	WRITE(&vdp.ntbb, 2);
	WRITE(&vdp.ntwb, 2);
	WRITE(&vdp.satb, 2);
	WRITE(&vdp.hscb, 2);
	WRITE(&vdp.sat_base_mask, 2);
	WRITE(&vdp.sat_addr_mask, 2);
	WRITE(&vdp.border, 1);
	WRITE(&vdp.playfield_shift, 1);
	WRITE(&vdp.playfield_col_mask, 1);
	WRITE(&vdp.playfield_row_mask, 2);
	WRITE(&vdp.y_mask, 4);
	WRITE(&vdp.hint_pending, 4);
	WRITE(&vdp.vint_pending, 4);
	WRITE(&dummy, 4);
	WRITE(&vdp.dma_fill, 4);
	WRITE(&vdp.im2_flag, 4);
	WRITE(&vdp.frame_end, 4);
	WRITE(&vdp.v_counter, 4);
	WRITE(&vdp.v_update, 4);

	// sound.c stuff

	// sound.c stuff
	//READ(fm_reg[0], 0x100);
	//READ(fm_reg[1], 0x100);

	WRITE(&fm_regs[0],0x200);

	//READ(fm_latch, 2);

	WRITE(&fm_addr,1);
	WRITE(&dummy,1);

	WRITE(&fm_status, 1);
	WRITE(&dummy,1);

	WRITE(&fm_mode, sizeof(int));
	WRITE(&dummy, sizeof(int));
	WRITE(&fm_tat, sizeof(int));
	WRITE(&fm_tac, sizeof(int));
	WRITE(&fm_ta, sizeof(int));

	WRITE(&dummy, sizeof(int));
	WRITE(&dummy, sizeof(int));
	WRITE(&fm_tbt, sizeof(int));
	WRITE(&fm_tbc, sizeof(int));
	WRITE(&fm_tb, 1);
	WRITE(&dummy, 3);

/*
	WRITE(fm_reg[0], 0x100);
	WRITE(fm_reg[1], 0x100);
	WRITE(fm_latch, 2);
	WRITE(&fm_status, 1);
	WRITE(&dummy,1)
	WRITE(&timer[0].running, sizeof(int));
	WRITE(&timer[0].enable, sizeof(int));
	WRITE(&timer[0].count, sizeof(int));
	WRITE(&timer[0].base, sizeof(int));
	WRITE(&timer[0].index, sizeof(int));
	WRITE(&timer[1].running, sizeof(int));
	WRITE(&timer[1].enable, sizeof(int));
	WRITE(&timer[1].count, sizeof(int));
	WRITE(&timer[1].base, sizeof(int));
	WRITE(&timer[1].index, sizeof(int));
*/
	
	// Window size
	WRITE(&bitmap.viewport.h, sizeof(int));
	WRITE(&bitmap.viewport.w, sizeof(int));
	
	// 68000 CPU
#ifdef USE_CYCLONE
	{

	CyclonePack(&CycloneCPU,&cyc_state);
	WRITE(&cyc_state, 0x80);
	
	}
#else

	{
	tmp32 =m68k_get_reg(NULL, M68K_REG_D0);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_D1); 	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_D2);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_D3);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_D4);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_D5);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_D6);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_D7);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_A0);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_A1);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_A2);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_A3);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_A4);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_A5);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_A6);	WRITE(&tmp32, 4);
	tmp32 =	m68k_get_reg(NULL, M68K_REG_A7);	WRITE(&tmp32, 4);		
	tmp32 = m68k_get_reg(NULL, M68K_REG_PC);	WRITE(&tmp32, 4);
	tmp32 = m68k_get_reg(NULL, M68K_REG_SR);	WRITE(&tmp32, 2); 
	}
#endif
  	
	// Z80 CPU
#ifdef USE_DRZ80
        tmp32 = DrZ80CPU.Z80PC - DrZ80CPU.Z80PC_BASE; /*z80_get_reg(Z80_PC);*/	WRITE(&tmp32, 4);
	tmp32 = DrZ80CPU.Z80SP - DrZ80CPU.Z80SP_BASE; /*z80_get_reg(Z80_SP);*/	WRITE(&tmp32, 4);
        tmp32 = ((DrZ80CPU.Z80A>>16)&0xFF00)|(DrZ80CPU.Z80F&0xFF); /*z80_get_reg(Z80_AF);*/	WRITE(&tmp32, 4);
	tmp32 = (DrZ80CPU.Z80BC>>16); /*z80_get_reg(Z80_BC);*/			WRITE(&tmp32, 4);
	tmp32 = (DrZ80CPU.Z80DE>>16); /*z80_get_reg(Z80_DE);*/			WRITE(&tmp32, 4);
	tmp32 = (DrZ80CPU.Z80HL>>16); /*z80_get_reg(Z80_HL);*/			WRITE(&tmp32, 4);
	tmp32 = DrZ80CPU.Z80IX; 	/*z80_get_reg(Z80_IX);*/			WRITE(&tmp32, 4);
	tmp32 = DrZ80CPU.Z80IY; 	/*z80_get_reg(Z80_IY);*/			WRITE(&tmp32, 4);
	tmp32 = 0;		/*z80_get_reg(Z80_R);*/				WRITE(&tmp32, 4);
	tmp32 = DrZ80CPU.Z80I; 	/*z80_get_reg(Z80_I);*/			WRITE(&tmp32, 4);
        tmp32 = ((DrZ80CPU.Z80A2>>16)&0xFF00)|(DrZ80CPU.Z80F2&0xFF); /*z80_get_reg(Z80_AF2);*/ WRITE(&tmp32, 4);
	tmp32 = (DrZ80CPU.Z80BC2>>16); /*z80_get_reg(Z80_BC2);*/			WRITE(&tmp32, 4);
	tmp32 = (DrZ80CPU.Z80DE2>>16); /*z80_get_reg(Z80_DE2);*/			WRITE(&tmp32, 4);
	tmp32 = (DrZ80CPU.Z80HL2>>16); /*z80_get_reg(Z80_HL2);*/			WRITE(&tmp32, 4);
	tmp32 = DrZ80CPU.Z80IM; 	/*z80_get_reg(Z80_IM);*/			WRITE(&tmp32, 4);
	if (DrZ80CPU.Z80IF&1) tmp32 = 1; else tmp32 = 0;
        									WRITE(&tmp32, 4);
	if (DrZ80CPU.Z80IF&2) tmp32 = 1; else tmp32 = 0;
        									WRITE(&tmp32, 4);
	if (DrZ80CPU.Z80IF&4) tmp32 = 1; else tmp32 = 0;
        									WRITE(&tmp32, 4);
	tmp32 = 0; /*z80_get_reg(Z80_NMI_STATE);*/				WRITE(&tmp32, 4);
	tmp32 = DrZ80CPU.Z80_IRQ; /*z80_get_reg(Z80_IRQ_STATE);*/		WRITE(&tmp32, 4);
	tmp32 = 0;
										WRITE(&tmp32, 4);
										WRITE(&tmp32, 4);
										WRITE(&tmp32, 4);
										WRITE(&tmp32, 4);
#else
	tmp32 = z80_get_reg(Z80_PC);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_SP);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_AF);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_BC);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_DE);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_HL);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_IX);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_IY);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_R);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_I);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_AF2);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_BC2);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_DE2);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_HL2);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_IM);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_IFF1);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_IFF2);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_HALT);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_NMI_STATE);		WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_IRQ_STATE);		WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_DC0);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_DC1);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_DC2);			WRITE(&tmp32, 4);
	tmp32 = z80_get_reg(Z80_DC3);			WRITE(&tmp32, 4);
#endif
	
#ifdef USE_CYCLONE
//      tmp32 = CycloneCPU.osp;
#else
	tmp32 = m68k_get_reg(NULL, M68K_REG_USP);
#endif
        WRITE(&tmp32, 4);

        WRITE(&sn[0],sizeof(sn[0]));

	// Close it
	CLOSE_STREAM(file);
        if (num!=0) {
        sprintf(str,"Saved to %s",savename);
	AddMessage(str,40); }

	return 0;
}
