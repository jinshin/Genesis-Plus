#ifndef _SHARED_H_
#define _SHARED_H_

#include <stdio.h>
#include <math.h>
#include <zlib.h>
#include <windows.h>

#include "mtypes.h"
#include "macros.h"
#include "genesis.h"
#include "eeprom.h"
#ifdef _WFIX
#include "osd_cpu.h"
#include "z80.h"
#include "m68k.h"
#endif
#include "vdp.h"
#include "render.h"
#include "mem68k.h"
#include "memz80.h"
#include "membnk.h"
#include "memvdp.h"
#include "system.h"
#include "unzip.h"
#include "fileio.h"
#include "loadrom.h"
#include "mio.h"
#include "gtkopts.h"
#include "locopts.h"
#include "sound.h"
#include "fm.h"
#include "sn76496.h"
#include "osd.h"
#include "msg.h"

#include "Drz80.h"

extern uint8 use_cyclone;
extern uint8 use_drz80;
extern uint8 use_z80;
extern uint8 fm_status;
extern uint8 fastsound;
extern uint8 sound;

#define M68K_INT_ACK_AUTOVECTOR    0xffffffff

#undef INLINE
#define INLINE static inline

#define fm_read(a) fm_status

#endif /* _SHARED_H_ */

