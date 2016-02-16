#include "osdfont.h"
#include "shared.h"

extern uint8 rotateright;
extern uint8 landscape;
extern uint8 fast_vid;
extern uint8 NeedMessage;

#define MUL320(x) ((x<<8)+(x<<6))
#define MUL240(x) ((x<<8)-(x<<4))

#ifndef _WFIX
void DrawLine_RL (uint16 *src, int line, int offset, uint16 *dst, int length);
void DrawLine_RR (uint16 *src, int line, int offset, uint16 *dst, int length);
void DrawLine_P  (uint16 *src, int line, int offset, uint16 *dst, int length);
#endif
void DrawLine_X  (uint16 *src, int line, int offset, uint16 *dst, int length);

#ifndef _WFIX
void DrawLine_RL (uint16 *src, int line, int offset, uint16 *dst, int length)
{
#undef  OUTPIXELPITCH
#define OUTPIXELPITCH (-240)
uint16 * outWrite;
uint8 * tmp;

tmp = (uint8*)dst + ((MUL240(319-offset)+line)<<1);
outWrite = (uint16*)tmp;
	do {
		*outWrite = *src++;			
		outWrite+=OUTPIXELPITCH;
	} while (--length);
}


void DrawLine_RR (uint16 *src, int line, int offset, uint16 *dst, int length)
{
#undef  OUTPIXELPITCH
#define OUTPIXELPITCH (240)
uint16 * outWrite;
uint8 * tmp;
tmp = (uint8*)dst + ((MUL240(offset)-line+239)<<1);
outWrite = (uint16*)tmp;
	do {
                *outWrite = *src++;			
		outWrite+=OUTPIXELPITCH;
	} while (--length);
}

#undef OUTPIXELPITCH

void DrawLine_P (uint16 *src, int line, int offset, uint16 *dst, int length)
{
uint16 * outWrite;
(uint8*) outWrite = (uint8*)dst + ((offset+line*240)<<1);
	do {
		*outWrite++ = *src++;
	} while (--length);
}
#endif

void DrawLine_X (uint16 *src, int line, int offset, uint16 *dst, int length)
{
uint16 * outWrite;
uint8 * tmp;
tmp =  (uint8*)dst + ((offset+line*320)<<1);
outWrite = (uint8*)tmp;
	do {
		*outWrite++ = *src++;
	} while (--length);
}



struct {
  mymessage msg[32];
  int	    num;
} messages;

extern uint16 ScreenCache[320*240];
uint16 MsgBuf[320*240];

extern int frameskip;

int NeedUpdate = 1;

void AddMessage (char * string, int ttl) {

    return 0;

    NeedUpdate = 1;	
    strcpy(messages.msg[messages.num].str,string);
    messages.msg[messages.num].ttl=ttl;
    messages.num++;
    NeedMessage=1;

    /* fprintf (stderr,"MessagesA: %d\n", messages.num); */
}

void ShiftMessages (int From) {
int i;
NeedUpdate = 1;
if (messages.num==1) { messages.num--; NeedMessage=0; /* fprintf (stderr,"MessagesS: %d\n", messages.num); */ return; };
for (i=From; i<messages.num; i++) {
  strcpy(messages.msg[i].str,messages.msg[i+1].str);
  messages.msg[i].ttl=messages.msg[i+1].ttl;
  }
if (messages.num>0) messages.num--; else NeedMessage=0;

/* fprintf (stderr,"MessagesS: %d\n", messages.num); */

}

extern int scale;

void ProcessMessages () {

int i;

//if (NeedUpdate) { memset(&MsgBuf[0],0,320*240*2); UpdateFromCache(); }; //4ms

if (messages.num==0) { NeedUpdate=0; return; };

for (i=0; i<messages.num; i++) {
    if (NeedUpdate) draw_text(&MsgBuf[0], 5, i*10+5, messages.msg[i].str, 320*scale);
    messages.msg[i].ttl-=(frameskip+1);
    }

NeedUpdate = 0;

//PutBuffer();

for (i=0; i<messages.num; i++) {
    if (messages.msg[i].ttl<=0) ShiftMessages(i);
    }
}


void draw_text (uint8 * buffer, int x, int y, char * string, int bwidth) {
int i,j;
int sl;
unsigned char a;
unsigned char b;
unsigned char * mchar;
int tshift;
buffer = buffer + (((y*bwidth*scale)+x)<<1);
sl=strlen(string);
do {
    a=*string;
    tshift=0;
    mchar = font + (a<<3);
    for (i=0; i<8; i++) {       
        b=*mchar;
        if (b!=0) {
          for (j=0; j<8; j+=2) {
	     if (b&0x80) {
	     *(uint16*)(buffer + tshift + j) = 0xFFFF;
	     *(uint16*)(buffer + tshift + (bwidth<<1) + j + 2) = 0x0001;
	     };
	     if (b==0) {
	     break;
	     }	
	     b=(b<<1);
	  }
	}
	mchar++;
    tshift+=(bwidth<<1)*scale;
    }
    buffer+=(4<<1);
    string++;
   } while (--sl);
return;
}

void PutBuffer() {

#define CYCLE \
		do { \
			tmpWrite=outWrite; \
			tmpRead=inRead; \
			x = 320; \
			do { \
				uint16 val = *(uint16 *)tmpRead; \
				if (val!=0) *(uint16 *)tmpWrite = val; \
				tmpRead+=2; \
				tmpWrite+=OUTPIXELPITCH; \
			} while (--x); \
			outWrite+=OUTLINEPITCH; \
			inRead+=(320*2); \
		} while (--y);

uint8 * outWrite;
uint8 * inRead;
uint8 * tmpWrite;
uint8 * tmpRead;

	inRead = (uint8 *)MsgBuf;
	outWrite = bitmap.data;
	int y=messages.num*10+5;
	if (y>220) y = 220;
	int x;

#ifndef _WFIX

if (!landscape) {
#define OUTLINEPITCH (240*2)
#define OUTPIXELPITCH (2)	
	CYCLE;
#undef OUTPIXELPITCH
#undef OUTLINEPITCH
return;
}

if (rotateright) {
	outWrite = outWrite + (((239))<<1);
	#define OUTLINEPITCH -2
	#define OUTPIXELPITCH (240*2)	
	CYCLE;
	#undef OUTPIXELPITCH
	#undef OUTLINEPITCH
	}
	else {
	outWrite = outWrite + (((319)*240)<<1);
	#define OUTLINEPITCH 2
	#define OUTPIXELPITCH (-240*2)		
	CYCLE;
	#undef OUTPIXELPITCH
	#undef OUTLINEPITCH
	}

#else

#define OUTLINEPITCH (320*2)
#define OUTPIXELPITCH (2)	
	CYCLE;
#undef OUTPIXELPITCH
#undef OUTLINEPITCH
        return;

#endif 
}

void UpdateFromCache() {
#undef CYCLE
#define CYCLE(XX) \
		do { \
			tmpWrite=outWrite; \
			tmpRead=inRead; \
			x = XX; \
			do { \
				uint16 val = *(uint16 *)tmpRead; \
				*(uint16 *)tmpWrite = val; \
				tmpRead+=2; \
				tmpWrite+=OUTPIXELPITCH; \
			} while (--x); \
			outWrite+=OUTLINEPITCH; \
			inRead+=(XX*2); \
		} while (--y);

uint8 * outWrite;
uint8 * inRead;
uint8 * tmpWrite;
uint8 * tmpRead;
	inRead = (uint8 *)ScreenCache;
	outWrite = bitmap.data;
	int y=220;
	int x;

#ifndef _WFIX

if (!landscape) {
#define OUTLINEPITCH (240*2)
#define OUTPIXELPITCH (2)	
	CYCLE(240);
#undef OUTPIXELPITCH
#undef OUTLINEPITCH
return;
}

if (rotateright) {
	outWrite = outWrite + (((239))<<1);
	#define OUTLINEPITCH -2
	#define OUTPIXELPITCH (240*2)	
	CYCLE(320);
	#undef OUTPIXELPITCH
	#undef OUTLINEPITCH
	}
	else {
	outWrite = outWrite + (((319)*240)<<1);
	#define OUTLINEPITCH 2
	#define OUTPIXELPITCH (-240*2)		
	CYCLE(320);
	#undef OUTPIXELPITCH
	#undef OUTLINEPITCH
	}

#else

#define OUTLINEPITCH (320*2)
#define OUTPIXELPITCH (2)	
	CYCLE(320);
#undef OUTPIXELPITCH
#undef OUTLINEPITCH
return;

#endif
}

void UpdateCache() {
#undef CYCLE
#define CYCLE(XX) \
		do { \
			tmpWrite=outWrite; \
			tmpRead=inRead; \
			x = XX; \
			do { \
				uint16 val = *(uint16 *)tmpRead; \
				*(uint16 *)tmpWrite = val; \
				tmpRead+=2; \
				tmpWrite+=OUTPIXELPITCH; \
			} while (--x); \
			outWrite+=OUTLINEPITCH; \
			inRead+=(XX*2); \
		} while (--y);

uint8 * outWrite;
uint8 * inRead;
uint8 * tmpWrite;
uint8 * tmpRead;
	inRead = (uint8 *)bitmap.data;
	outWrite = ScreenCache;
	int y=220;
	int x;

#ifndef _WFIX

if (!landscape) {
#define OUTLINEPITCH (240*2)
#define OUTPIXELPITCH (2)	
	CYCLE(240);
#undef OUTPIXELPITCH
#undef OUTLINEPITCH
return;
}

if (rotateright) {
	outWrite = outWrite + (((239))<<1);
	#define OUTLINEPITCH -2
	#define OUTPIXELPITCH (240*2)	
	CYCLE(320);
	#undef OUTPIXELPITCH
	#undef OUTLINEPITCH
	}
	else {
	outWrite = outWrite + (((319)*240)<<1);
	#define OUTLINEPITCH 2
	#define OUTPIXELPITCH (-240*2)		
	CYCLE(320);
	#undef OUTPIXELPITCH
	#undef OUTLINEPITCH
	}

#else
#define OUTLINEPITCH (320*2)
#define OUTPIXELPITCH (2)	
	CYCLE(320);
#undef OUTPIXELPITCH
#undef OUTLINEPITCH
#endif


}
