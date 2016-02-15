/*
    Copyright (C) 1999, 2000, 2001, 2002, 2003  Charles MacDonald

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

t_bitmap bitmap;
t_input input;
t_snd snd;
static int sound_tbl[400];

extern uint8 vdp_rate;
extern uint8 llsfilter;
extern uint8 sixbuttonpad;
extern uint8 cheat;
extern int buf_frames;

extern SDL_sem *sound_sem;

int aim_m68k = 0;
int frame_m68k = 0;
uint32 total_m68k = 0;
int line_cycles = 0;

extern int num_blocks;
extern double s_t;

#define SND_SIZE (snd.buffer_size * sizeof(int16))

#define drz80_execute(x) DrZ80Run(&DrZ80CPU, x)

extern int32   y_dacen;
extern int16   y_dacout;

//Dynamic buffers work unstable!!!
int16   pcmbuffer[2000];
int16   sndbuffer[4000];
int16   sndfmbuffer[4000];
int16   sndpsgbuffer[2000];
int32   pcmstep;

int 	lines_per_frame;

int audio_init(int rate)
{
    int i;

    /* 68000 and YM2612 clock */
    float vclk = 53693175.0 / 7;

    /* Z80 and SN76489 clock */
    float zclk = 3579545.0;

    /* Clear the sound data context */
    memset(&snd, 0, sizeof(snd));

    /* Make sure the requested sample rate is valid */
    if(!rate || ((rate < 8000) | (rate > 44100)))
    {
        return (0);
    }

    /* Calculate the sound buffer size */
    snd.buffer_size = (rate / vdp_rate);
    snd.sample_rate = rate;
    snd.block_size = 0;
    snd.block_offset = 0;
    num_blocks=0;	

    /* Allocate sound buffers */
    snd.buffer = &sndbuffer[0]; //malloc(snd.buffer_size * sizeof(int16) * 2);
    snd.fm.buffer = &sndfmbuffer[0]; //malloc(snd.buffer_size * sizeof(int16) * 2 );
    snd.psg.buffer = &sndpsgbuffer[0]; //malloc(snd.buffer_size * sizeof(int16) );

// n0p - dac code
    //pcmbuffer = malloc(snd.buffer_size * sizeof(int16) + snd.buffer_size );

    /* Make sure we could allocate everything */
/*
    if(!snd.fm.buffer || !snd.psg.buffer)
    {
	fprintf(stderr,"Error allocating sound buffers.\nSound disabled.\n");
        return (0);
    }
*/

    /* Initialize sound chip emulation */
    SN76496_sh_start(zclk, 100, rate);

//    SN76496_init(zclk, rate);

    YM2612Init(vclk, rate);

    /* Set audio enable flag */
    snd.enabled = 1;

    /* Make sound table */
    for (i = 0; i < (lines_per_frame+1); i++)
    {
        float p = snd.buffer_size * i;
        p = p / lines_per_frame;
    	sound_tbl[i] = p;
    }    	

    return (0);    
}

int audio_reinit(int rate)
{
    int i;

    /* 68000 and YM2612 clock */
    float vclk = 53693175.0 / 7;

    /* Z80 and SN76489 clock */
    float zclk = 3579545.0;

    /* Make sure the requested sample rate is valid */
    if(!rate || ((rate < 8000) | (rate > 44100)))
    {
        return (0);
    }

    /* Calculate the sound buffer size */
    snd.buffer_size = (rate / vdp_rate);
    snd.sample_rate = rate;
    snd.block_size = 0;
    snd.block_offset = 0;
    num_blocks=0;	

    /* Initialize sound chip emulation */
    SN76496_sh_start(zclk, 100, rate);

    YM2612Init(vclk, rate);

    /* Set audio enable flag */
    snd.enabled = 1;

    /* Make sound table */
    for (i = 0; i < (lines_per_frame+1); i++)
    {
        float p = snd.buffer_size * i;
        p = p / lines_per_frame;
    	sound_tbl[i] = p;
    }    	

    return (0);    
}


void audio_shutdown(void)
{
    if (snd.enabled)
    {
    YM2612Shutdown();
    snd.enabled=0;
    }
}

void system_init(void)
{
    gen_init();
    vdp_init();
    render_init();
    char c[80];
    sprintf(c,"VDP Rate: %dHz",vdp_rate);
    AddMessage(c,80);
}

void system_reset(void)
{
    gen_reset();
    vdp_reset();
    render_reset();

    aim_m68k = frame_m68k = total_m68k = 0;

    if(snd.enabled)
    {
        YM2612ResetChip();
        memset(snd.buffer, 0, SND_SIZE*2);
        memset(snd.fm.buffer, 0, SND_SIZE);
        memset(snd.psg.buffer, 0, SND_SIZE);
        memset(pcmbuffer, 0, SND_SIZE);
    }
    AddMessage("System reset",60);
}

void system_shutdown(void)
{
    gen_shutdown();
    vdp_shutdown();
    render_shutdown();
}

extern int m68ki_remaining_cycles;

INLINE void m68k_run (int cyc) 
{
  int cyc_do;
  aim_m68k+=cyc;
  line_cycles+=cyc;
  if((cyc_do=aim_m68k-frame_m68k) < 0) return;
#ifdef USE_CYCLONE
    CycloneCPU.cycles=cyc_do;
    CycloneRun(&CycloneCPU);
    frame_m68k+=cyc_do-CycloneCPU.cycles;
#else
    frame_m68k+=cyc_do;
    m68k_execute(cyc_do);
#endif
}

void m68k_endrun(after)
{
int cycles_left;
#ifdef USE_CYCLONE
    cycles_left=CycloneCPU.cycles; 
    frame_m68k -= cycles_left - after;
    if(frame_m68k < 0) frame_m68k = 0;
    CycloneCPU.cycles=after;
#else
    cycles_left=m68k_cycles_remaining();
    frame_m68k -= cycles_left - after;
    if(frame_m68k < 0) frame_m68k = 0;
    m68ki_remaining_cycles = after;
#endif
}

int m68k_total_cycles()
{
int cycles_left;
#ifdef USE_CYCLONE
    cycles_left=CycloneCPU.cycles; 
#else
    cycles_left=m68k_cycles_remaining();
#endif
return total_m68k+aim_m68k-cycles_left;
}

INLINE z80_run(x)
{
      	  if (use_z80) {
#ifdef USE_DRZ80
           drz80_execute(x);
#else
 	   z80_execute(x);
#endif
          }
}

INLINE UPDATE_FM (line)
{
 if (snd.enabled)
  if (fastsound) {
	int16 acc;
	int16 * tpcm = pcmbuffer + sound_tbl[line];
	int32 j = (sound_tbl[line+1]-sound_tbl[line])+1;
	if (y_dacen) acc = y_dacout; else acc = 0;
	if (j>0)
	do {
	  *tpcm++=acc;
	   } while (j--);
  }
        YM2612TimerTick();
     
      snd.fm.curStage = sound_tbl[line];
      snd.psg.curStage = sound_tbl[line];
}

#define FIX_J_AND_L \
	line_cycles = 0; \
        if (sixbuttonpad) if(input.pad[0].delay++>25) input.pad[0].phase=0;

int system_frame_gens (int do_skip)
{
#define VIS_LINES 224
#define HC 65

  int line, hcounter;

  total_m68k+=frame_m68k;
  aim_m68k = frame_m68k = 0;

  /* Clear V-Blank flag */
  vdp.status &= 0xFFF7; // Clear V Blank
       
  /* Toggle even/odd flag (IM2 only) */
  if (vdp.im2_flag) vdp.status ^= 0x0010;

  /* Load H-Counter */
  hcounter = vdp.reg[10];
	
  /* Point to start of sound buffer */
  snd.fm.lastStage = snd.fm.curStage = 0;
  snd.psg.lastStage = snd.psg.curStage = 0;

  /* Parse sprites for line 0 (done on line 261) */
  parse_satb (0x80);

  for (line = 0; line<vdp.frame_end; line++)
  {

      FIX_J_AND_L;

      /* Used by HV counter */
      vdp.v_counter = line;

      vdp.status |= 0x0004;					// HBlank = 1
      m68k_run(HC);
      vdp.status &= 0xFFFB;					// HBlank = 0

  if (--hcounter < 0)
  {
      hcounter = vdp.reg[10];
      vdp.hint_pending = 1;
      if (vdp.reg[0] & 0x10)
	  {
#ifdef USE_CYCLONE
	CycloneInterrupt(4);
#else
	m68k_set_irq(4);
#endif
	  }
  }

	/* Render a line of the display */
      if (do_skip == 0)
	  {
	    if (line < VIS_LINES) render_line (line);
	    if (line < VIS_LINES-1) parse_satb (0x81 + line);
	  }

      m68k_run(487-HC);

      /* Run Z80 emulation (if enabled) */
      if (zreset == 1 && zbusreq == 0) z80_run(228);
      UPDATE_FM(line);

  }

  FIX_J_AND_L;

  /* last visible frame */
  vdp.v_counter = vdp.frame_end;

  if (--hcounter < 0)
  {
      vdp.hint_pending = 1;
      if (vdp.reg[0] & 0x10)
	  {
#ifdef USE_CYCLONE
	CycloneInterrupt(4);
#else
	m68k_set_irq(4);
#endif
	  }
  }

  vdp.status |= (0x000C | 0x0088);
  m68k_run(128);

  if (zreset == 1 && zbusreq == 0)  z80_run(60);

  vdp.status &= 0xFFFB;			// HBlank = 0
  vdp.status |= 0x0080;			// V Int happened

  //Raise VINT
  vdp.vint_pending = 1;
  if (vdp.reg[1] & 0x20) 
#ifdef USE_CYCLONE
	CycloneInterrupt(6);
#else
	m68k_set_irq(6);
#endif

  //Raise Z80 irq
#ifdef USE_DRZ80
	DrZ80CPU.z80irqvector = 0xFF; DrZ80CPU.Z80_IRQ = 1;
#else
	z80_set_irq_line(0, ASSERT_LINE);
#endif
        zirq = 1;

  m68k_run(487-128);

  if (zreset == 1 && zbusreq == 0) z80_run(228-60);
  UPDATE_FM(line);

  for (line++; line<lines_per_frame; line++)
  {

      FIX_J_AND_L;

      /* Used by HV counter */
      vdp.v_counter = line;

      vdp.status |= 0x0004;					// HBlank = 1
      m68k_run(HC);
      vdp.status &= 0xFFFB;					// HBlank = 0

      m68k_run(487-HC);

      if (zreset == 1 && zbusreq == 0) z80_run(228);
      UPDATE_FM(line);
  }

  if (snd.enabled)
  {
      if (!cheat) if (llsfilter) audio_update_f(); else audio_update();
  }

  return gen_running;
}

extern uint8 use_sem;

void audio_update(void)
{
    int i;
    int16 acc;
    int16 *tempBuffer;
    int16 * fm0 = snd.fm.buffer;
    int16 * psg = snd.psg.buffer;
    int16 * pcm = pcmbuffer;
    int16 * tmpbuf;
    int cycles;
    int offset;

    tempBuffer = snd.fm.buffer + (snd.fm.lastStage<<1);
    YM2612UpdateOne(tempBuffer, snd.buffer_size - snd.fm.lastStage);

    tempBuffer = snd.psg.buffer + snd.psg.lastStage;
    SN76496Update(0, tempBuffer, snd.buffer_size - snd.psg.lastStage);

if (use_sem)
    { if (num_blocks>=buf_frames) { SDL_SemWait(sound_sem); s_t = 0; }; }
else
    { while (num_blocks>=buf_frames) { Sleep(0); s_t = 0; }; };

SDL_LockAudio();

    offset = num_blocks*snd.buffer_size;
    tmpbuf = offset*2 + snd.buffer;

    if (offset+snd.buffer_size>snd.block_size) cycles=snd.block_size-offset; else cycles=snd.buffer_size;

  if (cycles<=snd.buffer_size)
    if (cycles>0)
    do
     {
        int16 psgv = ((*psg++) >> 1) + *pcm++;
        *tmpbuf++ = psgv+*fm0++;
        *tmpbuf++ = psgv+*fm0++;

     } 
	while (--cycles);

SDL_UnlockAudio(); 

    num_blocks++;
} 


#define FILTER(a,v) (a>>1)+(a>>2)+(v>>2)

//#define PSGFILTER(a,v) (a>>1)+(a>>2)+((int)(v)<<16)>>18

#define PSGFILTER(a,v) (a)+((int)(v)<<16)>>18

int32 la,ra;

void audio_update_f(void)
{
    int i;
    int16 acc;
    int16 *tempBuffer;
    int16 * fm0 = snd.fm.buffer;
    int16 * psg = snd.psg.buffer;
    int16 * pcm = pcmbuffer;
    int16 * tmpbuf;
    int cycles;
    int offset;

    tempBuffer = snd.fm.buffer + (snd.fm.lastStage<<1);
    YM2612UpdateOne(tempBuffer, snd.buffer_size - snd.fm.lastStage);

    tempBuffer = snd.psg.buffer + snd.psg.lastStage;
    SN76496Update(0, tempBuffer, snd.buffer_size - snd.psg.lastStage);

if (use_sem)
    { if (num_blocks>=buf_frames) { SDL_SemWait(sound_sem); s_t = 0; }; }
else
    { while (num_blocks>=buf_frames) { Sleep(0); s_t = 0; }; };

SDL_LockAudio();

    offset = num_blocks*snd.buffer_size;
    tmpbuf = offset*2 + snd.buffer;

    if (offset+snd.buffer_size>snd.block_size) cycles=snd.block_size-offset; else cycles=snd.buffer_size;

  if (cycles<=snd.buffer_size)
    if (cycles>0)
    do
     {
        int16 psgv = ((*psg++) >> 1) + *pcm++;

        la = PSGFILTER(la,psgv+*fm0++);

        *tmpbuf++ = la;

        ra = PSGFILTER(ra,psgv+*fm0++);

        *tmpbuf++ = ra;
     } 
	while (--cycles); 

SDL_UnlockAudio();

    num_blocks++;
} 
