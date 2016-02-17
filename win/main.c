#include <SDL.h>
#include "shared.h"
#include "wtypes.h"
#include "commdlg.h"
#include "state.h"

#ifndef _WFIX
#include "devmode.h"
#endif

int so=0;

int timer_count = 0;
int old_timer_count = 0;
int paused = 0;
int frame_count = 0;
int rendered_frames = 0;
int running = 1;

int FSPosX = 0;
int FSPosY = 0;
int FRPosX = 0;
int FRPosY = 0;

SDL_Surface *screen;

//Various Paths
char RomPath[260];
char RomName[260];
char RomDir[260];
char SaveDir[260];
char ConfigDir[260];
char tempname[260];

int  sdir = 0; //0 - rom path, 1 - genpp path, 2 - user-defined
int  cdir = 0; //0 - rom path, 1 - genpp path, 2 - user-defined

//Options

int  scale = 3;

uint8  benchmark = 0;
uint8  renderer = 0;
uint8  fullscreen = 0;
uint8  rotateright = 0;
uint8  show_keys = 1;
uint8  landscape = 1;
uint8  square_screen = 0;
uint8  crop_screen = 0;
uint8  fastsound = 0;
uint8  sound = 0;
uint8  use_z80 = 1;
uint8  use_sem = 0;
uint8  sixbuttonpad = 0;
uint8  llsfilter = 0;
uint8  fast_vid = 1;
uint8  cheat = 0;
uint8  xbrz = 0;

uint8  NeedMessage = 1;
uint8  UncachedRender = 0;

uint32 sound_rate = 22050;
uint32 buf_frames = 4;
int    frameskip = 3;
int    autofs = 0;

//Keys
uint32 key_start = 13;
uint32 key_mode = 0xC5;
uint32 key_a = 0xC2;
uint32 key_b = 0xC3;
uint32 key_c = 0xC4;
uint32 key_x = 0xC2;
uint32 key_y = 0xC3;
uint32 key_z = 0xC4;
uint32 key_switch = 0xC1;
uint32 key_up = 273;
uint32 key_down = 274;
uint32 key_left = 276;
uint32 key_right = 275;
uint32 key_up_left =     278;
uint32 key_up_right =    280;
uint32 key_down_right =  281;
uint32 key_down_left =   279;

uint32 temp_key_start = 13;
uint32 temp_key_mode = 0xC5;
uint32 temp_key_a = 0xC2;
uint32 temp_key_b = 0xC3;
uint32 temp_key_c = 0xC4;
uint32 temp_key_x = 0xC2;
uint32 temp_key_y = 0xC3;
uint32 temp_key_z = 0xC4;
uint32 temp_key_switch = 0xC1;
uint32 temp_key_up = 273;
uint32 temp_key_down = 274;
uint32 temp_key_left = 276;
uint32 temp_key_right = 275;
uint32 temp_key_up_left =     278;
uint32 temp_key_up_right =    280;
uint32 temp_key_down_right =  281;
uint32 temp_key_down_left =   279;

uint16 ScreenCache[320*320];

uint16 Toolbar[320*240];
uint16 AltToolbar[320*240];
uint8  Map[320*240];
uint32 ToolbarHeight;
uint8  ToolbarState = 0;

uint8   need_reinit = 0;
uint8   need_sound = 0;

unsigned short sfiles[5];

uint32 tmod = 2; //2 - 6 tap zones, 3 - 9
uint32 tzones[9];

uint32 temp_tzones[9];
int32  ActiveZone = -1;
int32  ActiveToolbar=1;

int num_blocks;

SDL_sem *sound_sem;

#define True 1
#define False 0
#define BMPWIDTH 320
#define BMPHEIGHT 240

float int_ft; //Time in ms per frame
float acc_ft; //Elapsed frame time
int min_delay = 2; //min allowed system sleep time
int max_delay = 2000; //max allowed system sleep time
int skipnext = 0;
double s_t; //Start Time
double e_t; //End Time
double t_res; //Res
long long p_c; //Performance counter


//double frames_rendered = 0;

uint32 z80_counter_8 = 0;
uint32 z80_counter_16 = 0;

extern uint8 country;

char     filename[260]= "\0";
char     cfgfilename[260]= "\0";
wchar_t  szFile[260] = L"\0";
char	 mypath[260]="\0";

#define GET_PCR \
	{ QueryPerformanceFrequency(&p_c); \
	t_res = (double)(p_c); }

#define GET_T(x) \
	{ QueryPerformanceCounter(&p_c); \
	x = ((double)(p_c)*1000)/t_res; }

#define RESYNC \
	if (s_t == 0) { \
	GET_T(s_t); \
	acc_ft = int_ft; }

#if _MSC_VER
#define stricmp _stricmp
#endif

#ifdef _WFIX
unsigned char * _GetGAPIBuffer(void);
void _FreeGAPIBuffer(void);
#endif

extern void DIB_ShowTaskBar(BOOL taskBarShown);

OPENFILENAME   ofn;

extern 
struct {
  mymessage msg[32];
  int	    num;
} messages;

void Pause() {
paused=1;
if (sound) SDL_PauseAudio(1);
}

void UnPause() {
paused=0;
s_t = 0;
if (sound) SDL_PauseAudio(0);
}

//HANDLERS

void (*PrepareToolbar) (int CheckFiles);
void (*UpdateToolbar) ();
void (*HandleMouseButton) (SDL_MouseButtonEvent * button, int isdown);
void (*DrawLine) (uint16 *src, int line, int offset, uint16 *dst, int length);

#define PMTH 80 //Portrait Mini-toolbar height
#define LMTH 16
#define SMTH 16

#define PMW  240 //portrait Mode Width
#define PMH  320 //portrait Mode Width
#define LMW  320
#define LMH  240

#define SMW  240
#define SMH  240


#define COMMON_STUFF \
        case GENPP_H1: if (sfiles[0]) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_H2: if (sfiles[1]) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_H3: if (sfiles[2]) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_H4: if (sfiles[3]) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_H5: if (sfiles[4]) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_Z80:  if (use_z80)  AltToolbar[i]^=0xFFFF; break; \
	case GENPP_SOUND:  if (sound)  AltToolbar[i]^=0xFFFF; break; \
	case GENPP_RR: if (rotateright) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_FASTSOUND: if (fastsound) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_LLS: if (llsfilter) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_CS: if (crop_screen==1) AltToolbar[i]^=0xFFFF; if (crop_screen==2) AltToolbar[i]^=0xCCCC; break; \
	case GENPP_SIXBUTTON: if (sixbuttonpad) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_NINE: if (tmod==3) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_SHOW: if (show_keys) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_CHEAT: if (cheat) AltToolbar[i]^=0xFFFF; break; 
	

void UpdateToolbarNull () {
   return 0;
};


Uint32 my_callbackfunc(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    /* In this example, our callback pushes an SDL_USEREVENT event
    into the queue, and causes ourself to be called again at the
    same interval: */

    if (!paused) {
    fprintf(stderr,"%d fps\n",(rendered_frames/10));
    rendered_frames=0;
    }

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return(interval);
}

void PrepareToolbarLS(int CheckFiles) {
int i;
char v[10];
memcpy(AltToolbar,Toolbar,ToolbarHeight*2*LMW);
FSPosX=FSPosY=0;
FRPosX=FRPosY=0;
if (CheckFiles) {
   for (i=0; i<5; i++) {
	if (state_load(i+1,1)==0) sfiles[i]=1; else sfiles[i]=0;
	}
}
   for (i=LMW*LMTH; i<LMW*(ToolbarHeight); i++) {
        switch (Map[i]) {

	COMMON_STUFF;

	case GENPP_FS: if ((!FSPosX)|(!FSPosY)) {
						FSPosY=(i/LMW); FSPosX=(i%LMW);
						  if (!autofs) {
						  sprintf(v,"%d",frameskip);
						  draw_text((uint8*)AltToolbar,FSPosX,FSPosY+1,v,320*scale); } else {
						  sprintf(v,"%s","auto");
						  draw_text((uint8*)AltToolbar,FSPosX-6,FSPosY+1,v,320*scale); }
						};
						break;
	case GENPP_FR: if ((!FRPosX)|(!FRPosY)) {
						FRPosY=(i/LMW); FRPosX=(i%LMW);
						  sprintf(v,"%d",sound_rate);
						  draw_text((uint8*)AltToolbar,FRPosX-(3*2),FRPosY+1,v,320*scale);
						};
						break;
	}
   }
}

void UpdateConfigBar() {
int i;
char v[10];
memcpy(AltToolbar,Toolbar,ToolbarHeight*2*240);

   for (i=0; i<(240*ToolbarHeight); i++) {

        switch (Map[i]) {




	}
   }
}

#ifndef _WFIX

void PrepareToolbarPT(int CheckFiles) {
int i;
char v[10];
memcpy(AltToolbar,Toolbar,ToolbarHeight*2*240);
FSPosX=FSPosY=0;
FRPosX=FRPosY=0;
if (CheckFiles) {
   for (i=0; i<5; i++) {
	if (state_load(i+1,1)==0) sfiles[i]=1; else sfiles[i]=0;
	}
}
   for (i=PMW*PMTH; i<PMW*(ToolbarHeight); i++) {
        switch (Map[i]) {

	COMMON_STUFF;

	case GENPP_FS: if ((!FSPosX)|(!FSPosY)) {
						FSPosY=(i/PMW); FSPosX=(i%PMW);
						  if (!autofs) {
						  sprintf(v,"%d",frameskip);
						  draw_text ((uint8*)AltToolbar,FSPosX,FSPosY+1,v,240*scale); } else {
						  sprintf(v,"%s","auto");
						  draw_text ((uint8*)AltToolbar,FSPosX-6,FSPosY+1,v,240*scale); }
						};
						break;
	case GENPP_FR: if ((!FRPosX)|(!FRPosY)) {
						FRPosY=(i/PMW); FRPosX=(i%PMW);
						  sprintf(v,"%d",sound_rate);
						  draw_text((uint8*)AltToolbar,FRPosX-(3*2),FRPosY+1,v,240*scale);
						};
						break;
	}	
   }
}


void PrepareToolbarSQ(int CheckFiles) {
int i;
char v[10];
memcpy(AltToolbar,Toolbar,ToolbarHeight*2*240);
FSPosX=FSPosY=0;
FRPosX=FRPosY=0;
if (CheckFiles) {
   for (i=0; i<5; i++) {
	if (state_load(i+1,1)==0) sfiles[i]=1; else sfiles[i]=0;
	}
}
   for (i=SMW*SMTH; i<SMW*(ToolbarHeight); i++) {
        switch (Map[i]) {

	COMMON_STUFF;

	case GENPP_FS: if ((!FSPosX)|(!FSPosY)) {
						FSPosY=(i/SMW); FSPosX=(i%SMW);
						  if (!autofs) {
						  sprintf(v,"%d",frameskip);
						  draw_text ((uint8*)AltToolbar,FSPosX,FSPosY+1,v,240*scale); } else {
						  sprintf(v,"%s","auto");
						  draw_text ((uint8*)AltToolbar,FSPosX-6,FSPosY+1,v,240*scale); }
						};
					break;
	case GENPP_FR: if ((!FRPosX)|(!FRPosY)) {
						FRPosY=(i/SMW); FRPosX=(i%SMW);
						  sprintf(v,"%d",sound_rate);
						  draw_text((uint8*)AltToolbar,FRPosX-(3*2),FRPosY+1,v,240*scale);
						};
						break;
	}
   }
}

void UpdateToolbarPT() {
uint16* buffer;
int i,j,line;
j=0;
buffer=_GetGAPIBuffer();
if (!ToolbarState) {
   i=PMTH;
   line=PMH-i;
     do {
       DrawLine(&Toolbar[j],line++,0,buffer,PMW);
       j+=PMW;
     } while (--i);
} else {
   i=(ToolbarHeight-PMTH);
   line=PMH-i;
     do {
       DrawLine(&AltToolbar[j]+PMTH*PMW,line++,0,buffer,PMW);
       j+=PMW;
     } while (--i);
}
_FreeGAPIBuffer();
}

void UpdateToolbarSQ() {
uint16* buffer;
int i,j,line;
j=0;
buffer=_GetGAPIBuffer();
if (!ToolbarState) {
   i=SMTH;
   line=SMH-i;
     do {
       DrawLine(&Toolbar[j],line++,0,buffer,SMW);
       j+=SMW;
     } while (--i);
} else {
   i=(ToolbarHeight-SMTH);
   line=SMH-i;
     do {
       DrawLine(&AltToolbar[j]+SMTH*SMW,line++,0,buffer,SMW);
       j+=SMW;
     } while (--i);
}
_FreeGAPIBuffer();
}

void UpdateToolbarLS() {
uint16* buffer;
int i,j,line;
j=0;
buffer=_GetGAPIBuffer();
if (!ToolbarState) {
   i=LMTH;
   line=LMH-i;
     do {
       DrawLine(&Toolbar[j],line++,0,buffer,LMW);
       j+=LMW;
     } while (--i);
} else {
   i=(ToolbarHeight-LMTH);
   line=LMH-i;
     do {
       DrawLine(&AltToolbar[j]+LMTH*LMW,line++,0,buffer,LMW);
       j+=LMW;
     } while (--i);
}
_FreeGAPIBuffer();
}

#else

void UpdateToolbarLS() {
if (!ToolbarState) {
    memcpy((Uint8*)screen->pixels+(LMW*(LMH-LMTH)*2),(Uint8*)Toolbar,LMW*LMTH*2); 
    SDL_UpdateRect(screen,0,(LMH-LMTH),LMW,LMTH); 
} else {
    memcpy((Uint8*)screen->pixels+(LMW*(LMH-(ToolbarHeight-LMTH))*2),(Uint8*)AltToolbar + (LMW*LMTH*2),LMW*(ToolbarHeight-LMTH)*2); 
    SDL_UpdateRect(screen,0,(LMH-(ToolbarHeight-LMTH)),LMW,ToolbarHeight-LMTH); 
}
}

#endif

void HandleMiniToolbar(SDL_MouseButtonEvent *b, int isdown) {
int ypos,xpos;
Uint8 action;

#ifndef _WFIX

if (square_screen) {
   ypos=((b->y)-(SMH-SMTH)); xpos=(b->x);
   action=Map[xpos+ypos*SMW];
} else {
  if (landscape) {
     ypos=((b->y)-(LMH-LMTH)); xpos=(b->x);
     action=Map[xpos+ypos*LMW];
  } else {
     ypos=((b->y)-(PMH-PMTH)); xpos=(b->x);
     action=Map[xpos+ypos*PMW];
  }
}

#else
     ypos=((b->y)-(LMH-LMTH)); xpos=(b->x);
     action=Map[xpos+ypos*LMW];
#endif

   if (action==0) return;
     if (isdown) {
        switch (action) {
	case GENPP_START: input.pad[0].data |= INPUT_START; break;
	case GENPP_MODE: input.pad[0].data |= INPUT_MODE; break;
	case GENPP_A: input.pad[0].data |= INPUT_A; break;
	case GENPP_B: input.pad[0].data |= INPUT_B; break;	
	case GENPP_C: input.pad[0].data |= INPUT_C; break;	
	case GENPP_X: input.pad[0].data |= INPUT_X; break;
	case GENPP_Y: input.pad[0].data |= INPUT_Y; break;	
	case GENPP_Z: input.pad[0].data |= INPUT_Z; break;	
	case GENPP_EXIT: running = 0; break;
	case GENPP_RESET: system_reset(); break;
	case GENPP_NEXT: Pause(); ToolbarState=1; PrepareToolbar(1); UpdateToolbar(); break;

	case GENPP_S1: Pause(); state_save(1); UnPause(); break;
	case GENPP_S2: Pause(); state_save(2); UnPause(); break;
	case GENPP_S3: Pause(); state_save(3); UnPause(); break;
	case GENPP_S4: Pause(); state_save(4); UnPause(); break;
	case GENPP_S5: Pause(); state_save(5); UnPause(); break;

	case GENPP_L1: Pause(); state_load(1,0); UnPause(); break;
	case GENPP_L2: Pause(); state_load(2,0); UnPause(); break;
	case GENPP_L3: Pause(); state_load(3,0); UnPause(); break;
	case GENPP_L4: Pause(); state_load(4,0); UnPause(); break;
	case GENPP_L5: Pause(); state_load(5,0); UnPause(); break;
	}
     } else {        
        switch (action) {
	case GENPP_START: input.pad[0].data &=~ INPUT_START; break;
	case GENPP_MODE: input.pad[0].data &=~ INPUT_MODE; break;
	case GENPP_A: input.pad[0].data &=~ INPUT_A; break;
	case GENPP_B: input.pad[0].data &=~ INPUT_B; break;	
	case GENPP_C: input.pad[0].data &=~ INPUT_C; break;	
	case GENPP_X: input.pad[0].data &=~ INPUT_X; break;
	case GENPP_Y: input.pad[0].data &=~ INPUT_Y; break;	
	case GENPP_Z: input.pad[0].data &=~ INPUT_Z; break;	
	}
     }
return;
}

void ClearAll();

#define EXIT_TOOLBAR 	ClearAll(); \
			ToolbarState=0; \
			UpdateToolbar(); \
			UnPause();

void HandleToolbar(SDL_MouseButtonEvent *b, int isdown) {
int ypos,xpos;
Uint8 action;
char filename[1000];
int resound=0;

if (square_screen) {
    ypos=((b->y)-(SMH-(ToolbarHeight-SMTH))); xpos=(b->x);
    action=Map[xpos+(ypos+SMTH)*SMW];
} else {
  if (landscape) {
    ypos=((b->y)-(LMH-(ToolbarHeight-LMTH))); xpos=(b->x);
    action=Map[xpos+(ypos+LMTH)*LMW];
  } else {
   ypos=((b->y)-(PMH-(ToolbarHeight-PMTH))); xpos=(b->x);
   action=Map[xpos+(ypos+PMTH)*PMW];
  }
}

   if (action==0) return;
     if (isdown) {
        switch (action) {
	case GENPP_EXIT: running = 0; break;
	case GENPP_RESET: system_reset(); EXIT_TOOLBAR; break;
	case GENPP_NEXT: EXIT_TOOLBAR; break;

	case GENPP_S1: state_save(1); EXIT_TOOLBAR; break;
	case GENPP_S2: state_save(2); EXIT_TOOLBAR; break;
	case GENPP_S3: state_save(3); EXIT_TOOLBAR; break;
	case GENPP_S4: state_save(4); EXIT_TOOLBAR; break;
	case GENPP_S5: state_save(5); EXIT_TOOLBAR; break;
                     
	case GENPP_L1: state_load(1,0); EXIT_TOOLBAR; break;
	case GENPP_L2: state_load(2,0); EXIT_TOOLBAR; break;
	case GENPP_L3: state_load(3,0); EXIT_TOOLBAR; break;
	case GENPP_L4: state_load(4,0); EXIT_TOOLBAR; break;
	case GENPP_L5: state_load(5,0); EXIT_TOOLBAR; break;

#ifndef _WFIX
        case GENPP_PT: landscape=0; need_reinit=1; EXIT_TOOLBAR; break;
        case GENPP_LS: landscape=1; need_reinit=1; EXIT_TOOLBAR; break;
        case GENPP_RR: rotateright^=1; landscape=1; need_reinit=1; EXIT_TOOLBAR; break;
        case GENPP_CS: crop_screen++; if (crop_screen>2) crop_screen=0; landscape=0; need_reinit=1; EXIT_TOOLBAR; break;
#endif

	case GENPP_FASTSOUND: fastsound^=1; PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_LLS: llsfilter^=1; PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_SIXBUTTON: sixbuttonpad^=1; io_reset(); PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_NINE: if (tmod==3) tmod=2; else tmod=3; PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_SHOW: show_keys^=1; PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_CHEAT: cheat^=1; if (cheat) int_ft*=2; else int_ft/=2; PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_FS_UP: if (!autofs) { frameskip++;} else {frameskip=0;}; autofs=0; if (frameskip>9) frameskip=9; PrepareToolbar(0); UpdateToolbar(); break;
	case GENPP_FS_DOWN: frameskip--; autofs=0; if (frameskip<0) {frameskip=0; autofs=1;} PrepareToolbar(0); UpdateToolbar(); break;

	case GENPP_FR_UP: 
			if (sound_rate==22050) {sound_rate=44100; resound=1;}; 	
 			if (sound_rate==11025) {sound_rate=22050; resound=1;};
			if (sound_rate==8000) {sound_rate=11025; resound=1;}; 	  
			  if (resound) { resound=0;
			     if (sound) {
			        YM2612Save();
				SDL_LockAudio();
			         sdl_soundstop();
			         audio_shutdown();
				 audio_reinit(sound_rate);
				 sdl_soundstart();
				SDL_PauseAudio(1);
				SDL_UnlockAudio();
 				YM2612Load();
				 };
				PrepareToolbar(0); UpdateToolbar();
			     };
			break;

	case GENPP_FR_DOWN: 
			  if (sound_rate==11025) {sound_rate=8000; resound=1;} ;
 			  if (sound_rate==22050) {sound_rate=11025; resound=1;} ;
			  if (sound_rate==44100) {sound_rate=22050; resound=1;} ;
			  if (resound) { resound=0;
			     if (sound) {
				YM2612Save();
				SDL_LockAudio();
			         sdl_soundstop();
			         audio_shutdown();
				 audio_reinit(sound_rate);
				 sdl_soundstart();
				SDL_PauseAudio(1);
				SDL_UnlockAudio();
				YM2612Load();
				 };
				PrepareToolbar(0); UpdateToolbar();
			     };
			break;

	case GENPP_Z80: use_z80^=1; 
	                if (!use_z80) {
			
			} else {
#ifdef USE_DRZ80
                        drz80_init();
                        drz80_reset();
#else
	                z80_reset(0);
			z80_set_irq_callback(z80_irq_callback);
#endif
			}
                        PrepareToolbar(0); UpdateToolbar(); break;

	case GENPP_SOUND: sound^=1;  
                          	if (sound) {
                                  audio_init(sound_rate);
	    			  if (snd.enabled) { sdl_soundstart(); SDL_PauseAudio(1); } else sound=0;
				} else {
                                  if (snd.enabled) sdl_soundstop();
                                  audio_shutdown();
				}
			          PrepareToolbar(0); UpdateToolbar(); break;

	case GENPP_SAVE: 
	                	set_local_opts();
    				sprintf(filename,"%s\\%s%s",ConfigDir,RomName,".cfg");
					locopts_save(filename);	
    				sprintf(filename,"Options saved to %s\\%s%s",ConfigDir,RomName,".cfg");
					AddMessage(filename,60);	
					EXIT_TOOLBAR;
					break;

	} //end of switch
}
return;
}

void CopyToTemp() {
temp_key_start      =  key_start;
temp_key_mode       =  key_mode;
temp_key_a          =  key_a;
temp_key_b          =  key_b;
temp_key_c          =  key_c;
temp_key_x          =  key_x;
temp_key_y          =  key_y;
temp_key_z          =  key_z;
temp_key_switch     =  key_switch;
temp_key_up         =  key_up;
temp_key_down       =  key_down;
temp_key_left       =  key_left;
temp_key_right      =  key_right;
temp_key_up_left    =  key_up_left;
temp_key_up_right   =  key_up_right;
temp_key_down_right =  key_down_right;
temp_key_down_left  =  key_down_left;
memcpy(temp_tzones,tzones,sizeof(tzones));
}

void CopyFromTemp() {
key_start      =  temp_key_start;
key_mode       =  temp_key_mode;
key_a          =  temp_key_a;
key_b          =  temp_key_b;
key_c          =  temp_key_c;
key_x          =  temp_key_x;
key_y          =  temp_key_y;
key_z          =  temp_key_z;
key_switch     =  temp_key_switch;
key_up         =  temp_key_up;
key_down       =  temp_key_down;
key_left       =  temp_key_left;
key_right      =  temp_key_right;
key_up_left    =  temp_key_up_left;
key_up_right   =  temp_key_up_right;
key_down_right =  temp_key_down_right;
key_down_left  =  temp_key_down_left;
memcpy(tzones,temp_tzones,sizeof(tzones));
}

void LoadConfigBar() {
gzFile 	*g;
     if (ToolbarState==2) {
            sprintf(filename,"%s/%s",mypath,"genpp.key");
     } else {
            sprintf(filename,"%s/%s",mypath,"genpp.tz");
     }
	    g = gzopen(filename,"rb");
	    gzread(g,&ToolbarHeight,4);
	    gzread(g,Toolbar,240*ToolbarHeight*2);
            memset(Map, 0, sizeof(Map));
	    gzread(g,Map,240*ToolbarHeight);
	    gzclose(g);
}

void HandleKeyConfig(SDL_MouseButtonEvent *b, int isdown) {
int ypos,xpos;
Uint8 action;

if (square_screen) {
    ypos=(b->y); xpos=(b->x);
    action=Map[xpos+ypos*240];
} else {
  if (landscape) {
    ypos=(b->y); xpos=(b->x);
    if ((xpos<40)|(xpos>280)) return;
    action=Map[(xpos-40)+ypos*240];
  } else {
   ypos=(b->y); xpos=(b->x);
   action=Map[xpos+ypos*240];
  }
}

   if (action==0) return;
     if (isdown) {
        switch (action) {

	} //end of switch
}
return;
}


void HandleTzConfig(SDL_MouseButtonEvent *b, int isdown) {
int ypos,xpos;
Uint8 action;

if (square_screen) {
    ypos=(b->y); xpos=(b->x);
    if (ypos>100) ypos+=100;
    action=Map[xpos+ypos*240];
} else {
  if (landscape) {
    ypos=(b->y); xpos=(b->x);
    if (ypos>100) ypos+=100;
    if ((xpos<40)|(xpos>280)) return;
    action=Map[(xpos-40)+ypos*240];
  } else {
   ypos=(b->y); xpos=(b->x);
    if (ypos>100) ypos+=100;
   action=Map[xpos+ypos*240];
  }
}

   if (action==0) return;
     if (isdown) {
        switch (action) {

	} //end of switch
}
return;
}


void HandleMouseButtonSQ(SDL_MouseButtonEvent * button, int isdown) {
int zone;

	if ((button->y>=(SMH-SMTH))&(!ToolbarState)) { HandleMiniToolbar(button,isdown); return; }
if (ToolbarState) {
	if ((button->y>=(SMH-(ToolbarHeight-SMTH)))&(ToolbarState==1)) { HandleToolbar(button,isdown); return; }
	if ((ToolbarState==2)&(button->y<ToolbarHeight)) { HandleKeyConfig(button,isdown); return; }
	if ((ToolbarState==3)&(button->y<(ToolbarHeight-100))) { HandleTzConfig(button,isdown); return; }
}
	int xzone = (button->x*tmod)/SMW;
	zone = (button->y/75) + xzone*3;
	if (zone>8) zone = 8; //Shouldn't happen

if (isdown) input.pad[0].data |= tzones[zone];
	else	input.pad[0].data &=~ tzones[zone];
	
}

void HandleMouseButtonPT(SDL_MouseButtonEvent * button, int isdown) {
int zone;

	if ((button->y>=(PMH-PMTH))&(!ToolbarState)) { HandleMiniToolbar(button,isdown); return; }
if (ToolbarState) {
	if ((button->y>=(PMH-(ToolbarHeight-PMTH)))&(ToolbarState==1)) { HandleToolbar(button,isdown); return; }
	if ((ToolbarState==2)&(button->y<ToolbarHeight)) { HandleKeyConfig(button,isdown); return; }
	if ((ToolbarState==3)&(button->y<(ToolbarHeight-100))) { HandleTzConfig(button,isdown); return; }
}

	int xzone = (button->x*tmod)/PMW;
	zone = (button->y/75) + xzone*3;
	if (zone>8) zone = 8; //Shouldn't happen

if (isdown) input.pad[0].data |= tzones[zone];
	else	input.pad[0].data &=~ tzones[zone];
	
}

void HandleMouseButtonLS(SDL_MouseButtonEvent * button, int isdown) {
int zone;

	if ((button->y>=(LMH-LMTH))&(!ToolbarState)) { HandleMiniToolbar(button,isdown); return; }
if (ToolbarState) {
	if ((button->y>=(LMH-(ToolbarHeight-LMTH)))&(ToolbarState==1)) { HandleToolbar(button,isdown); return; }
	if ((ToolbarState==2)&(button->y<ToolbarHeight)) { HandleKeyConfig(button,isdown); return; }
	if ((ToolbarState==3)&(button->y<(ToolbarHeight-100))) { HandleTzConfig(button,isdown); return; }
}
	int xzone = (button->x*tmod)/LMW;
	zone = (button->y/75) + xzone*3;
	if (zone>8) zone = 8; //Shouldn't happen

if (isdown) input.pad[0].data |= tzones[zone];
	else	input.pad[0].data &=~ tzones[zone];
	
}

char * b2s(int val) {
if (val) return("On"); else return("Off");
}

#if _MSC_VER
char cwd[MAX_PATH+1] = "";
char *getcwd(char *buffer, int maxlen)
{
	TCHAR fileUnc[MAX_PATH+1];
	char* plast;
	
	if(cwd[0] == 0)
	{
		GetModuleFileName(NULL, fileUnc, MAX_PATH);
		WideCharToMultiByte(CP_ACP, 0, fileUnc, -1, cwd, MAX_PATH, NULL, NULL);
		plast = strrchr(cwd, '\\');
		if(plast)
			*plast = 0;
		/* Special trick to keep start menu clean... */
		if(_stricmp(cwd, "\\windows\\start menu") == 0)
			strcpy(cwd, "\\Apps");
	}
	if(buffer)
		strncpy(buffer, cwd, maxlen);
	return cwd;
}
#endif

#ifndef _WFIX
void ClearAll() {		
		int cnt, ys;		
		uint32 *outW;
		uint32 *outC;
		if (square_screen) ys=240; else ys=320;
		outW = (uint32 *)_GetGAPIBuffer();
		outC = (uint32 *)ScreenCache;
		cnt = ys*240/2;
		do {
		*outW++ = 0;
		*outC++ = 0; 
		} while (--cnt);
		_FreeGAPIBuffer();
}

void SetHandlers() {
gzFile 	*g;
	if (square_screen) {
		PrepareToolbar=PrepareToolbarSQ;
		//UpdateToolbar=UpdateToolbarSQ;
		HandleMouseButton=HandleMouseButtonSQ;
		DrawLine=DrawLine_P;
	        sprintf(filename,"%s/%s",mypath,"genpp.sq");
	    g = gzopen(filename,"rb");
	    gzread(g,&ToolbarHeight,4);
	    gzread(g,Toolbar,240*ToolbarHeight*2);
	    gzread(g,Map,240*ToolbarHeight);
	    gzclose(g);
	} else {
	if (landscape) {
		PrepareToolbar=PrepareToolbarLS;
		//UpdateToolbar=UpdateToolbarLS;
		HandleMouseButton=HandleMouseButtonLS;
	        sprintf(filename,"%s/%s",mypath,"genpp.ls");
		if (rotateright) DrawLine=DrawLine_RR; else DrawLine=DrawLine_RL; 
	    g = gzopen(filename,"rb");
	    gzread(g,&ToolbarHeight,4);
	    gzread(g,Toolbar,320*ToolbarHeight*2);
	    gzread(g,Map,320*ToolbarHeight);
	    gzclose(g);
		} else {
		PrepareToolbar=PrepareToolbarPT;
		//UpdateToolbar=UpdateToolbarPT;
		HandleMouseButton=HandleMouseButtonPT;
		DrawLine=DrawLine_P;
	        sprintf(filename,"%s/%s",mypath,"genpp.pt"); 
	    g = gzopen(filename,"rb");
	    gzread(g,&ToolbarHeight,4);
	    gzread(g,Toolbar,240*ToolbarHeight*2);
	    gzread(g,Map,240*ToolbarHeight);
	    gzclose(g);
	    }
	}
}


void ReinitScreen()
{
Pause();

SDL_QuitSubSystem(SDL_INIT_VIDEO);


if (square_screen) {

screen = SDL_SetVideoMode(240, 240, 16, SDL_SWSURFACE|SDL_FULLSCREEN);

} else {

if (landscape) {
if (rotateright) screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE|SDL_FULLSCREEN|SDL_LEFTHAND); \
    else screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE|SDL_FULLSCREEN);
} else { screen = SDL_SetVideoMode(240, 320, 16, SDL_SWSURFACE|SDL_FULLSCREEN); }

}

    SetHandlers();
    set_renderer();
    ToolbarState=0;
    UpdateToolbar();

UnPause();

}


#else
void ClearAll() {
		int cnt, ys;		
		uint32 *outW;
		uint32 *outC;
		outW = (Uint32 *)screen->pixels;
		outC = (Uint32 *)ScreenCache;
		cnt = 320*240/2;
		do {
		*outW++ = 0;
		*outC++ = 0; //*outC ^ 0x11111111;
		} while (--cnt);
}


void SetHandlers() {
gzFile 	*g;
		PrepareToolbar=PrepareToolbarLS;

		UpdateToolbar=UpdateToolbarNull;;

		HandleMouseButton=HandleMouseButtonLS;
	        sprintf(filename,"%s/%s",mypath,"genpp.ls");
		DrawLine=DrawLine_X; 
	    g = gzopen(filename,"rb");
	    gzread(g,&ToolbarHeight,4);
	    gzread(g,Toolbar,320*ToolbarHeight*2);
	    gzread(g,Map,320*ToolbarHeight);
	    gzclose(g);
}


void ReinitScreen()
{
Pause();

SDL_QuitSubSystem(SDL_INIT_VIDEO);
uint32 addflag = 0;
if (fullscreen) addflag|=SDL_FULLSCREEN;
screen = SDL_SetVideoMode(320*scale, 240*scale, 32, SDL_HWSURFACE|addflag);

fprintf (stderr,"Videomode: %dx%dx%d pitch %d\n",screen->w,screen->h,screen->format->BitsPerPixel, (screen->pitch>>2) );
bitmap.pitch = (screen->pitch>>2);

extern HWND SDL_Window;
SetWindowTextW(SDL_Window,L"Genesis Plus/PC");

    SetHandlers();
    set_renderer();
    ToolbarState=0;
    UpdateToolbar();

UnPause();

}


#endif


void ReinitRender()
{
    set_renderer();
    ToolbarState=0;
    UpdateToolbar();
}


#ifndef _WFIX
void RestoreOrientation()
{
DEVMODE4 devmode = {0};
  devmode.dmSize = sizeof(DEVMODE4);
  devmode.dmDisplayOrientation = so; 
  devmode.dmFields = DM_DISPLAYORIENTATION; 
  ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL); 
}

void SetOrientation()
{
DEVMODE4 devmode = {0};
devmode.dmSize = sizeof(DEVMODE4);
devmode.dmFields = DM_DISPLAYORIENTATION;
ChangeDisplaySettingsEx(NULL, &devmode, 0, CDS_TEST, NULL); 
so=devmode.dmDisplayOrientation;

if (so!=0) {
devmode.dmSize = sizeof(DEVMODE4);
devmode.dmDisplayOrientation = DMDO_0; 
devmode.dmFields = DM_DISPLAYORIENTATION; 
ChangeDisplaySettingsEx(NULL, &devmode, NULL, 0, NULL); 
}
}

volatile int  ScreenMemSpeed(void)
{
		int cnt;
		double tb,te;
		uint16 *outW, s;
		outW = (uint16 *)_GetGAPIBuffer();
		s = outW;
		
GET_PCR;
GET_T(tb);
		cnt = 240*240;
		do {
		*outW++ = 1; }
                while (--cnt);

		cnt = 240*240;
		do {
		*outW-- = 0; }
                while (--cnt);

GET_T(te);
                _FreeGAPIBuffer();
cnt = te-tb;
return cnt;
}               

volatile int  DeviceMemSpeed(void)
{
		int cnt;
		double tb,te;
		uint16 *outW;
		outW = (uint16 *)ScreenCache;
		
GET_PCR;
GET_T(tb);
		cnt = 240*240;
		do {
		*outW++ = 1; }
                while (--cnt);
		cnt = 240*240;
		do {
		*outW-- = 0; }
                while (--cnt);

GET_T(te);
cnt = te-tb;
return cnt;
}       

void ExitApp()
{
        RestoreOrientation();
        exit(1);
}

#else
void ExitApp()
{
        exit(1);
}
#endif        


#undef main
//int main (int argc, char **argv)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{

gzFile 	*g;
int 	i,xs,ys;
char 	c[80];
SDL_Event event;
FILE *mylog;

    atexit(SDL_Quit); 

    getcwd(mypath,MAX_PATH);

#ifdef DEBUG
    mylog = fopen("GenPP.log","w+");
    *stderr = *mylog;
    fprintf (stderr,"Genesis Plus/PC\n");
#endif

    sprintf(cfgfilename,"%s/%s",mypath,"genpp.cfg");
    if (gtkopts_load(cfgfilename)==(-1)) gtkopts_save(cfgfilename);
    get_global_opts();

#ifndef _WFIX
    SetOrientation();

    xs=GetSystemMetrics(SM_CXSCREEN);
    ys=GetSystemMetrics(SM_CYSCREEN);
    
    if (ys==240)
     if (xs==ys) { square_screen=1; landscape=0; ys=240; } else { square_screen=0; ys=320; };

    screen = SDL_SetVideoMode(240, ys, 16, SDL_SWSURFACE|SDL_FULLSCREEN);	
    sprintf(filename,"%s/%s",mypath,"genpp.logo");
    g = gzopen(filename,"rb");
    gzread(g,screen->pixels,ys*240*2);
    gzclose(g);
    SDL_Flip(screen);
#else
    screen = SDL_SetVideoMode(240, 320, 16, SDL_SWSURFACE);
    sprintf(filename,"%s/%s",mypath,"genpp.logo");
    g = gzopen(filename,"rb");
    gzread(g,screen->pixels,320*240*2);
    gzclose(g);
    SDL_Flip(screen);
#endif

extern HWND SDL_Window;

SetWindowTextW(SDL_Window,L"Genesis Plus/PC");

extern gsGetOpenFileName(OPENFILENAME * ofn);

/* allow open by cmd line */
//wcstombs(filename,lpCmdLine,MAX_PATH);
if (strlen(lpCmdLine)>0) {
    sprintf(RomPath,"%s",filename);

} else {
 
#ifndef _WFIX
wchar_t rp[MAX_PATH];
#else
char rp[MAX_PATH];
#endif

   memset( &(ofn), 0, sizeof(ofn));
   ofn.lStructSize   = sizeof(ofn);
   ofn.hwndOwner = SDL_Window;
   ofn.lpstrFile = szFile;
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrFilter = TEXT("Genesis/Megadrive ROMs\0*.bin;*.smd;*.gen;*.zip\0");
   ofn.lpstrTitle = TEXT("GenPP - Select ROM");

const char * v1 = gtkopts_getvalue("romfolder");

#ifndef _WFIX
   mbstowcs(rp,v1,MAX_PATH);
#else
   sprintf(rp,"%s",v1);
#endif
   ofn.lpstrInitialDir = rp;

   ofn.Flags = OFN_EXPLORER;
#ifndef _WFIX
   if (gsGetOpenFileName(&ofn)) {
   wcstombs(filename,szFile,MAX_PATH);
#else
   if (GetOpenFileNameA(&ofn)) {
   sprintf(filename,"%s",szFile);
#endif
   sprintf(RomPath,"%s",filename);
   } else { ExitApp(); };

}

//Parsing directories

#ifdef DEBUG
    fprintf (stderr,"filename: %s\n",filename);
    fprintf (stderr,"RomPath: %s\n",RomPath);
#endif

//Clear extension
   i=strlen(RomPath);
   do {
   if (RomPath[i]=='.') { RomPath[i]='\0'; break; }
   if (RomPath[i]=='\\') break;
   if (RomPath[i]=='/') break;
   } while (--i);

#ifdef DEBUG
    fprintf (stderr,"RomPath: %s\n",RomPath);
#endif

//Get RomName
   char TRN[260];
   TRN[0]='\0';
   i=strlen(RomPath);
   int j=0; 
   do {
   if (RomPath[i]=='\\') { RomPath[i]='\0'; RomPath[i+1]='\0'; break; };
   if (RomPath[i]=='/')  { RomPath[i]='\0'; RomPath[i+1]='\0'; break; };
   TRN[j++]=RomPath[i];
   } while (i--);

   i=0;
   if (j>0) {
     do {
     RomName[i++]=TRN[j-1];
     } while (--j);   
   } else { RomName[0]='\0'; };
   RomName[i]='\0';

//Get RomFolder
   if (strlen(RomPath)==0) { RomPath[0]='\\'; RomPath[1]='\0'; }

   sprintf(RomDir,"%s",RomPath);

   sprintf(SaveDir,"%s",RomDir);
   sprintf(ConfigDir,"%s",RomDir);

   const char * v;

   switch (sdir) {
       case 0: sprintf(SaveDir,"%s",RomDir); break;
       case 1: sprintf(SaveDir,"%s",mypath); break;
       case 2: {v = gtkopts_getvalue("savefolder"); sprintf(SaveDir,"%s",v); } break; 
   };

   switch (cdir) {
       case 0: sprintf(ConfigDir,"%s",RomDir); break;
       case 1: sprintf(ConfigDir,"%s",mypath); break;
       case 2: {v = gtkopts_getvalue("configfolder"); sprintf(ConfigDir,"%s",v); } break; 
   };

   gtkopts_setvalue("romfolder",RomDir);

   gtkopts_save(cfgfilename);

   SDL_Flip(screen);

  sprintf(tempname,"%s\\%s%s",ConfigDir,RomName,".cfg");

    if (locopts_load(tempname)!=(-1)) {
	get_local_opts();
	sprintf(tempname,"Using options from %s\\%s%s",ConfigDir,RomName,".cfg");
	AddMessage(tempname,80); };
	
   if(!load_rom(filename))
	{
	  ExitApp();
	}  

SetWindowText(SDL_Window,L"Genesis Plus/PC");

    if (sram.mem) { load_sram(); }

    if (!use_z80) AddMessage("Z80: Off",100); else AddMessage("Z80: On",100);

    if (llsfilter) AddMessage("Lame sound filter used",100);

    //sprintf(c,"%d",so);
    //AddMessage(c,200);

#ifndef _WFIX
    if ( ScreenMemSpeed()>>1 >= DeviceMemSpeed() ) fast_vid = 0; else fast_vid = 1;
    if (fast_vid) AddMessage("Fast video",100); else AddMessage("Slow video",100);
#endif

    ReinitScreen(); 

    memset(&bitmap, 0, sizeof(t_bitmap));
    bitmap.width  = 320;
    bitmap.height = 240;
    bitmap.depth  = 16;
    bitmap.granularity = 2;
    bitmap.pitch = screen->pitch>>2;
    bitmap.data   = NULL;
    bitmap.viewport.w = 256;
    bitmap.viewport.h = 224;
    bitmap.viewport.x = 0x20;
    bitmap.viewport.y = 0x00;
    bitmap.remap = 1;

    system_init();

    xBRZScale_Init();

SDL_InitSubSystem(SDL_INIT_JOYSTICK);
SDL_InitSubSystem(SDL_INIT_TIMER);

SDL_TimerID SDL_fps = SDL_AddTimer(10000, my_callbackfunc, 0);

    //sprintf(c,"VidMem: %d, DevMem: %d", ScreenMemSpeed(), DeviceMemSpeed());
    //AddMessage(c,100);

    printf("%i gamepads were found.\n\n", SDL_NumJoysticks() );
    printf("The names of the gamepads are:\n");
		
    for( int i=0; i < SDL_NumJoysticks(); i++ ) 
    {
        printf("  %s\n", SDL_JoystickName(i));
    }

    SDL_Joystick *joystick;

    SDL_JoystickEventState(SDL_ENABLE);
    joystick = SDL_JoystickOpen(0);

    fprintf(stderr,"VDP Rate: %d\n",vdp_rate);

    //TimeSync code
    int_ft = 1000 / vdp_rate;
    //if (vdp_rate==50) int_ft--; //ugly fixup
    s_t = 0;
    GET_PCR;

    if (benchmark) sound=0;

if (sound) {
    audio_init(sound_rate);
    if (snd.enabled) sdl_soundstart(); else sound=0;
}
    sprintf(c,"Sound: %s @ %dHz",b2s(sound),sound_rate);
    AddMessage(c,100);

    system_reset();

	while(running)
	{

		running = 1; //update_input();

		while (SDL_PollEvent(&event)) 
		{
			switch(event.type) 
			{

				case SDL_QUIT: /* Windows was closed */
					running = 0;
					break;

				case SDL_ACTIVEEVENT:
					if(event.active.state & (0x10))
					{
#ifndef _WFIX
						if (!event.active.gain) { Pause(); ToolbarState=1; PrepareToolbar(1); UpdateToolbar(); _GAPISuspend(); } else { _GAPIResume(); UpdateToolbar(); SetForegroundWindow(SDL_Window); };
#else
						if (!event.active.gain) { Pause(); ToolbarState=1; PrepareToolbar(1); UpdateToolbar(); } else { UpdateToolbar(); SetForegroundWindow(SDL_Window); };
#endif
					}
					break;

                		case SDL_MOUSEBUTTONDOWN:
                        		HandleMouseButton(&event.button, 1);
                        	break;

                		case SDL_MOUSEBUTTONUP:
	                        	HandleMouseButton(&event.button, 0);
	                        break;

				case SDL_JOYAXISMOTION:
					if (event.jaxis.axis==0) {
						int LeftRight = event.jaxis.value;
						input.pad[event.jaxis.which].data &=~ (INPUT_LEFT|INPUT_RIGHT);
						if (event.jaxis.value<(-10000)) input.pad[event.jaxis.which].data |= INPUT_LEFT;
						if (event.jaxis.value>(10000)) input.pad[event.jaxis.which].data |= INPUT_RIGHT;
						}
					if (event.jaxis.axis==1) {
						int UpDown = event.jaxis.value;
						input.pad[event.jaxis.which].data &=~ (INPUT_UP|INPUT_DOWN);
						if (event.jaxis.value<(-10000)) input.pad[event.jaxis.which].data |= INPUT_UP;
						if (event.jaxis.value>(10000)) input.pad[event.jaxis.which].data |= INPUT_DOWN;
						}
					//fprintf(stderr,"Gamepad %d Axis %d:%d\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
				break;

#define btn_a 0
#define btn_b 1
#define btn_c 2
#define btn_x 3
#define btn_y 4
#define btn_z 5
#define btn_mode 6
#define btn_start 7

				case SDL_JOYBUTTONDOWN:
					if(event.jbutton.button==btn_a)     { input.pad[event.jbutton.which].data |= INPUT_A; break;}
    					if(event.jbutton.button==btn_b)     { input.pad[event.jbutton.which].data |= INPUT_B; break;}
    					if(event.jbutton.button==btn_c)     { input.pad[event.jbutton.which].data |= INPUT_C; break;}
    					if(event.jbutton.button==btn_x)     { input.pad[event.jbutton.which].data |= INPUT_X; break;}
    					if(event.jbutton.button==btn_y)     { input.pad[event.jbutton.which].data |= INPUT_Y; break;}
    					if(event.jbutton.button==btn_z)     { input.pad[event.jbutton.which].data |= INPUT_Z; break;}
    					if(event.jbutton.button==btn_start) { input.pad[event.jbutton.which].data |= INPUT_START; break;}
    					if(event.jbutton.button==btn_mode)  { input.pad[event.jbutton.which].data |= INPUT_MODE; break;}
					//fprintf(stderr,"Gamepad %d BtnDown %d:%d\n", event.jbutton.which, event.jbutton.button, event.jbutton.state);
				break;

				case SDL_JOYBUTTONUP:
					if(event.jbutton.button==btn_a)     { input.pad[event.jbutton.which].data &=~ INPUT_A; break;}
    					if(event.jbutton.button==btn_b)     { input.pad[event.jbutton.which].data &=~ INPUT_B; break;}
    					if(event.jbutton.button==btn_c)     { input.pad[event.jbutton.which].data &=~ INPUT_C; break;}
    					if(event.jbutton.button==btn_x)     { input.pad[event.jbutton.which].data &=~ INPUT_X; break;}
    					if(event.jbutton.button==btn_y)     { input.pad[event.jbutton.which].data &=~ INPUT_Y; break;}
    					if(event.jbutton.button==btn_z)     { input.pad[event.jbutton.which].data &=~ INPUT_Z; break;}
    					if(event.jbutton.button==btn_start) { input.pad[event.jbutton.which].data &=~ INPUT_START; break;}
    					if(event.jbutton.button==btn_mode)  { input.pad[event.jbutton.which].data &=~ INPUT_MODE; break;}
					//fprintf(stderr,"Gamepad %d BtnUp %d:%d\n", event.jbutton.which, event.jbutton.button, event.jbutton.state);
				break;

				case SDL_JOYHATMOTION:
                                        fprintf(stderr,"Gamepad %d Hat %d:%d\n", event.jhat.which, event.jhat.hat, event.jhat.value);
				break;

				case SDL_KEYDOWN:

					if(event.key.keysym.sym==key_switch) {
					if (!ToolbarState) {
					Pause(); ToolbarState=1; PrepareToolbar(1); UpdateToolbar(); 
 					} else { EXIT_TOOLBAR; };
					break;
					}

    if(event.key.keysym.sym==SDLK_ESCAPE) { fullscreen^=1; need_reinit=1; break;  }
    if(event.key.keysym.sym==SDLK_F5) { state_save(1); break; }
    if(event.key.keysym.sym==SDLK_F9) { state_load(1,0); break; }	
    if(event.key.keysym.sym==SDLK_F3) { renderer++; if (renderer>8) renderer=0; ReinitRender(); break; }

    if(event.key.keysym.sym==key_up) 	{ input.pad[0].data |= INPUT_UP; break; }
    if(event.key.keysym.sym==key_down)  { input.pad[0].data |= INPUT_DOWN; break; }
    if(event.key.keysym.sym==key_left)  { input.pad[0].data |= INPUT_LEFT; break; }
    if(event.key.keysym.sym==key_right) { input.pad[0].data |= INPUT_RIGHT; break; }

    if(event.key.keysym.sym==key_up_left) 	{ input.pad[0].data |= INPUT_UP; input.pad[0].data |= INPUT_LEFT; break; }
    if(event.key.keysym.sym==key_up_right)      { input.pad[0].data |= INPUT_UP; input.pad[0].data |= INPUT_RIGHT; break; }
    if(event.key.keysym.sym==key_down_right)    { input.pad[0].data |= INPUT_DOWN; input.pad[0].data |= INPUT_RIGHT; break; }
    if(event.key.keysym.sym==key_down_left)     { input.pad[0].data |= INPUT_DOWN; input.pad[0].data |= INPUT_LEFT; break; }

    if(event.key.keysym.sym==key_a)     { input.pad[0].data |= INPUT_A; break;}
    if(event.key.keysym.sym==key_b)     { input.pad[0].data |= INPUT_B; break;}
    if(event.key.keysym.sym==key_c)     { input.pad[0].data |= INPUT_C; break;}
    if(event.key.keysym.sym==key_x)     { input.pad[0].data |= INPUT_X; break;}
    if(event.key.keysym.sym==key_y)     { input.pad[0].data |= INPUT_Y; break;}
    if(event.key.keysym.sym==key_z)     { input.pad[0].data |= INPUT_Z; break;}
    if(event.key.keysym.sym==key_start) { input.pad[0].data |= INPUT_START; break;}
    if(event.key.keysym.sym==key_mode)  { input.pad[0].data |= INPUT_MODE; break;}

if (show_keys) {
    sprintf(c,"Key press: %d",event.key.keysym.sym);
    AddMessage(c,100);
    }
    break;

                                case SDL_KEYUP:

    if(event.key.keysym.sym==key_up) 	{  input.pad[0].data &=~ INPUT_UP; break; }
    if(event.key.keysym.sym==key_down)  {  input.pad[0].data &=~ INPUT_DOWN; break; }
    if(event.key.keysym.sym==key_left)  {  input.pad[0].data &=~ INPUT_LEFT; break; }
    if(event.key.keysym.sym==key_right) {  input.pad[0].data &=~ INPUT_RIGHT; break; }

    if(event.key.keysym.sym==key_up_left) 	{ input.pad[0].data &=~ INPUT_UP; input.pad[0].data &=~ INPUT_LEFT; break; }
    if(event.key.keysym.sym==key_up_right)      { input.pad[0].data &=~ INPUT_UP; input.pad[0].data &=~ INPUT_RIGHT; break; }
    if(event.key.keysym.sym==key_down_right)    { input.pad[0].data &=~ INPUT_DOWN; input.pad[0].data &=~ INPUT_RIGHT; break; }
    if(event.key.keysym.sym==key_down_left)     { input.pad[0].data &=~ INPUT_DOWN; input.pad[0].data &=~ INPUT_LEFT; break; }

    if(event.key.keysym.sym==key_a)     {  input.pad[0].data &=~ INPUT_A; break; }
    if(event.key.keysym.sym==key_b)     {  input.pad[0].data &=~ INPUT_B; break; }
    if(event.key.keysym.sym==key_c)     {  input.pad[0].data &=~ INPUT_C; break; }
    if(event.key.keysym.sym==key_x)     {  input.pad[0].data &=~ INPUT_X; break; }
    if(event.key.keysym.sym==key_y)     {  input.pad[0].data &=~ INPUT_Y; break; }
    if(event.key.keysym.sym==key_z)     {  input.pad[0].data &=~ INPUT_Z; break; }
    if(event.key.keysym.sym==key_start) {  input.pad[0].data &=~ INPUT_START; break; }
    if(event.key.keysym.sym==key_mode)  {  input.pad[0].data &=~ INPUT_MODE; break; }


if (show_keys) {
    sprintf(c,"Key release: %d",event.key.keysym.sym);
    AddMessage(c,100);
}

    break;

				default:
					break;
			}
		}

if (need_reinit) {
        if (square_screen) { landscape=0; rotateright=0; };
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	ReinitScreen(); 
	bitmap.viewport.changed = 1;	
	need_reinit=0;
}


if(!paused)
	{
	frame_count++;          				

	//TimeSync code
	RESYNC;


	if ((frame_count > frameskip )&(!skipnext)) {
            		if(bitmap.viewport.changed)
            		{
                	bitmap.viewport.changed = 0;
    			bitmap.viewport.x = (320-bitmap.viewport.w)>>1;
			ClearAll();
			ReinitRender();
            		}

	 		frame_count=0;
        	 	bitmap.data = _GetGAPIBuffer();
         		if(!system_frame_gens(0)) system_reset();
         		rendered_frames++;
#ifndef _WFIX
         		if (fast_vid)
         		      {
				if ((!NeedMessage)&(!UncachedRender)) set_renderer();
         			if ((NeedMessage)&(UncachedRender)) { UpdateCache(); set_renderer(); }
         		      };	
         		ProcessMessages();
#endif
			if (xbrz) {
				if (scale==2) xBRZ_2X_MT(0,0);
				if (scale==3) xBRZ_3X_MT(0,0);
				if (scale==4) xBRZ_4X_MT(0,0);
			}
         		_FreeGAPIBuffer();

	} else { system_frame_gens(1); }

if (!benchmark) {
//TimeSync begin
	skipnext = 0;
		if (s_t!=0) {
	    		GET_T(e_t);
	    		int diff_ft = e_t - s_t;
	    		if ((acc_ft - diff_ft) > min_delay) {
	        		if ((acc_ft - diff_ft) < max_delay) Sleep(acc_ft - diff_ft - 1);
	        		s_t = 0;
			} else {       
				skipnext = ((diff_ft>int_ft)&(autofs));
			}
            		acc_ft += int_ft;
	    	}
//TimeSync end;
}

         } else
         //if paused, why waste CPU?
          { Sleep(2); };

  //if not running
  }

  if (sram.mem) save_sram();

  sdl_soundstop();

  system_shutdown();

  fclose (mylog);

  ExitApp();

}

void sdl_soundcallback(void *userdata, uint8 *stream, int len)
{
snd.block_size=len>>2;
snd.buffer=(int16*)stream;
num_blocks=0;
if (use_sem) {if (SDL_SemValue(sound_sem)==0) SDL_SemPost(sound_sem);}
}

int sdl_soundstart(void)
{
  SDL_AudioSpec as;

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
      fprintf(stderr,("SDL_InitSubSystem failed"));
      return 1;
    }

  as.userdata = snd.buffer;
  as.freq = snd.sample_rate;
  as.format = AUDIO_S16;
  as.channels = 2;
  buf_frames = (vdp_rate/4 + 1);
  as.samples = snd.buffer_size*buf_frames;
  as.callback = sdl_soundcallback;	

  if (SDL_OpenAudio(&as, NULL) == -1) {	
    fprintf(stderr,("SDL_OpenAudio failed: %s", SDL_GetError()));
	SDL_Delay(500);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return 1;
  }

  if (use_sem) sound_sem = SDL_CreateSemaphore(1);

  SDL_PauseAudio(0);

  return 0;
}

int sdl_soundstop(void)
{
  if (use_sem) SDL_DestroySemaphore(sound_sem);
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

#ifdef _WFIX
unsigned char * _GetGAPIBuffer(void)
{
	return screen->pixels;
}

void _FreeGAPIBuffer(void)
{
	 SDL_Flip(screen);
}

#endif
