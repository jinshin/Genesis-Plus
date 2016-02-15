/*
    sound.c
    YM2612 and SN76489 emulation
*/

#include "shared.h"

/* YM2612 data */
//uint8 fm_reg[2][0x100];     /* Register arrays (2x256) */
//uint8 fm_latch[2];          /* Register latches */
//uint8 fm_status;            /* Read-only status flags */
//t_timer timer[2];           /* Timers A and B */

uint8	fm_regs[0x200];
uint8	fm_status;
uint8	fm_addr;
uint32	fm_mode;
int32	fm_ta;   //index
int32	fm_tac;  //base
int32	fm_tat;  //count
uint8	fm_tb;   //index
int32	fm_tbc;  //base
int32	fm_tbt;  //count

void fm_write(int address, int data)
{
if(snd.enabled)
    {

extern uint8 fastsound;

  if (!fastsound) {
        if(snd.fm.curStage - snd.fm.lastStage > 1)
        {
            int16 *tempBuffer;       
            tempBuffer = snd.fm.buffer + (snd.fm.lastStage<<1);
            YM2612UpdateOne(tempBuffer, snd.fm.curStage - snd.fm.lastStage);
            snd.fm.lastStage = snd.fm.curStage;
        }
  }

  YM2612Write(address, data);

    }
}

void psg_write(int data)
{
    if(snd.enabled)
    {
extern uint8 fastsound;

if (!fastsound) 
        if(snd.psg.curStage - snd.psg.lastStage > 1)
        {
            int16 *tempBuffer;
            tempBuffer = snd.psg.buffer + snd.psg.lastStage;
            SN76496Update(0, tempBuffer, snd.psg.curStage - snd.psg.lastStage);
            snd.psg.lastStage = snd.psg.curStage;
        }

        SN76496Write(0, data);
    }
}

