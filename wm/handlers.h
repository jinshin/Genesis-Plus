void (*PrepareToolbar) (int CheckFiles);
void (*UpdateToolbar) ();
void (*HandleMouseButton) (SDL_MouseButtonEvent * button, int isdown);


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
	case GENPP_SIXBUTTON: if (sixbuttonpad) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_NINE: if (tmod==3) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_SHOW: if (show_keys) AltToolbar[i]^=0xFFFF; break; \
	case GENPP_CHEAT: if (cheat) AltToolbar[i]^=0xFFFF; break; 
	

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
						  draw_text((uint8*)AltToolbar,FSPosX,FSPosY+1,v,320); } else {
						  sprintf(v,"%s","auto");
						  draw_text((uint8*)AltToolbar,FSPosX-6,FSPosY+1,v,320); }
						};
						break;
	case GENPP_FR: if ((!FRPosX)|(!FRPosY)) {
						FRPosY=(i/LMW); FRPosX=(i%LMW);
						  sprintf(v,"%d",sound_rate);
						  draw_text((uint8*)AltToolbar,FRPosX-(3*2),FRPosY+1,v,320);
						};
						break;
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
						  draw_text ((uint8*)AltToolbar,FSPosX,FSPosY+1,v,240); } else {
						  sprintf(v,"%s","auto");
						  draw_text ((uint8*)AltToolbar,FSPosX-6,FSPosY+1,v,240); }
						};
						break;
	case GENPP_FR: if ((!FRPosX)|(!FRPosY)) {
						FRPosY=(i/PMW); FRPosX=(i%PMW);
						  sprintf(v,"%d",sound_rate);
						  draw_text((uint8*)AltToolbar,FRPosX-(3*2),FRPosY+1,v,240);
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
						  draw_text ((uint8*)AltToolbar,FSPosX,FSPosY+1,v,240); } else {
						  sprintf(v,"%s","auto");
						  draw_text ((uint8*)AltToolbar,FSPosX-6,FSPosY+1,v,240); }
						};
					break;
	case GENPP_FR: if ((!FRPosX)|(!FRPosY)) {
						FRPosY=(i/SMW); FRPosX=(i%SMW);
						  sprintf(v,"%d",sound_rate);
						  draw_text((uint8*)AltToolbar,FRPosX-(3*2),FRPosY+1,v,240);
						};
						break;
	}	
   }

}

void UpdateToolbarPT() {
if (!ToolbarState) {
    memcpy((Uint8*)screen->pixels+(PMW*(PMH-PMTH)*2),(Uint8*)Toolbar,PMW*PMTH*2); 
    SDL_UpdateRect(screen,0,PMW,(PMH-PMTH),PMTH); 
} else {
    memcpy((Uint8*)screen->pixels+(PMW*(PMH-(ToolbarHeight-PMTH))*2),(Uint8*)AltToolbar + (PMW*PMTH*2),PMW*(ToolbarHeight-PMTH)*2); 
    SDL_UpdateRect(screen,0,(PMH-(ToolbarHeight-PMTH)),PMW,ToolbarHeight-PMTH); 
}
}

void UpdateToolbarSQ() {
if (!ToolbarState) {
    memcpy((Uint8*)screen->pixels+(SMW*(SMH-SMTH)*2),(Uint8*)Toolbar,SMW*SMTH*2); 
    SDL_UpdateRect(screen,0,(SMH-SMTH),SMW,SMTH); 
} else {
    memcpy((Uint8*)screen->pixels+(SMW*(SMH-(ToolbarHeight-SMTH))*2),(Uint8*)AltToolbar + (SMW*SMTH*2),SMW*(ToolbarHeight-SMTH)*2); 
    SDL_UpdateRect(screen,0,(SMH-(ToolbarHeight-SMTH)),SMW,ToolbarHeight-SMTH); 
}
}

#endif

void UpdateToolbarLS() {
if (!ToolbarState) {
    memcpy((Uint8*)screen->pixels+(LMW*(LMH-LMTH)*2),(Uint8*)Toolbar,LMW*LMTH*2); 
    SDL_UpdateRect(screen,0,(LMH-LMTH),LMW,LMTH); 
} else {
    memcpy((Uint8*)screen->pixels+(LMW*(LMH-(ToolbarHeight-LMTH))*2),(Uint8*)AltToolbar + (LMW*LMTH*2),LMW*(ToolbarHeight-LMTH)*2); 
    SDL_UpdateRect(screen,0,(LMH-(ToolbarHeight-LMTH)),LMW,ToolbarHeight-LMTH); 
}
}

void HandleMiniToolbar(SDL_MouseButtonEvent *b, int isdown) {
int ypos,xpos;
Uint8 action;

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

        case GENPP_PT: landscape=0; need_reinit=1; EXIT_TOOLBAR; break;
        case GENPP_LS: landscape=1; need_reinit=1; EXIT_TOOLBAR; break;
        case GENPP_RR: rotateright^=1; landscape=1; need_reinit=1; EXIT_TOOLBAR; break;
        case GENPP_CS: crop_screen^=1; landscape=0; need_reinit=1; EXIT_TOOLBAR; break;

	case GENPP_FASTSOUND: fastsound^=1; PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_LLS: llsfilter^=1; PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_SIXBUTTON: sixbuttonpad^=1; io_reset(); PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_NINE: if (tmod==3) tmod=2; else tmod=3; PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_SHOW: show_keys^=1; PrepareToolbar(0); UpdateToolbar(); break; 
	case GENPP_CHEAT: cheat^=1; if (cheat) int_ft<<=1; else int_ft>>=1; PrepareToolbar(0); UpdateToolbar(); break; 

	case GENPP_FS_UP: if (!autofs) { frameskip++;} else {frameskip=0;}; autofs=0; if (frameskip>9) frameskip=9; PrepareToolbar(0); UpdateToolbar(); break;
	case GENPP_FS_DOWN: frameskip--; autofs=0; if (frameskip<0) {frameskip=0; autofs=1;} PrepareToolbar(0); UpdateToolbar(); break;

	case GENPP_FR_UP: 
			if (sound_rate==22050) {sound_rate=44100; resound=1;} 	
 			if (sound_rate==11025) {sound_rate=22050; resound=1;}
			if (sound_rate==8000) {sound_rate=11025; resound=1;} 	  
			  if (resound) {
			     if (sound) {
				SDL_LockAudio();
			         sdl_soundstop();
			         audio_shutdown();
				 audio_init(sound_rate);
				 sdl_soundstart();
				SDL_PauseAudio(1);
				SDL_UnlockAudio();
				 };
				PrepareToolbar(0); UpdateToolbar();
			     };
			break;

	case GENPP_FR_DOWN: 
			  if (sound_rate==11025) {sound_rate=8000; resound=1;} 
 			  if (sound_rate==22050) {sound_rate=11025; resound=1;} 
			  if (sound_rate==44100) {sound_rate=22050; resound=1;} 
			  if (resound) {
			     if (sound) {
				SDL_LockAudio();
			         sdl_soundstop();
			         audio_shutdown();
				 audio_init(sound_rate);
				 sdl_soundstart();
				SDL_PauseAudio(1);
				SDL_UnlockAudio();
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

void HandleMouseButtonSQ(SDL_MouseButtonEvent * button, int isdown) {
int zone;

	if ((button->y>=(SMH-SMTH))&(!ToolbarState)) { HandleMiniToolbar(button,isdown); return; }
	if ((button->y>=(SMH-(ToolbarHeight-SMTH)))&(ToolbarState)) { HandleToolbar(button,isdown); return; }

	int xzone = (button->x*tmod)/SMW;
	zone = (button->y/75) + xzone*3;
	if (zone>8) zone = 8; //Shouldn't happen

if (isdown) input.pad[0].data |= tzones[zone];
	else	input.pad[0].data &=~ tzones[zone];
	
}

void HandleMouseButtonPT(SDL_MouseButtonEvent * button, int isdown) {
int zone;

	if ((button->y>=(PMH-PMTH))&(!ToolbarState)) { HandleMiniToolbar(button,isdown); return; }
	if ((button->y>=(PMH-(ToolbarHeight-PMTH)))&(ToolbarState)) { HandleToolbar(button,isdown); return; }

	int xzone = (button->x*tmod)/PMW;
	zone = (button->y/75) + xzone*3;
	if (zone>8) zone = 8; //Shouldn't happen

if (isdown) input.pad[0].data |= tzones[zone];
	else	input.pad[0].data &=~ tzones[zone];
	
}

void HandleMouseButtonLS(SDL_MouseButtonEvent * button, int isdown) {
int zone;

	if ((button->y>=(LMH-LMTH))&(!ToolbarState)) { HandleMiniToolbar(button,isdown); return; }
	if ((button->y>=(LMH-(ToolbarHeight-LMTH)))&(ToolbarState)) { HandleToolbar(button,isdown); return; }

	int xzone = (button->x*tmod)/LMW;
	zone = (button->y/75) + xzone*3;
	if (zone>8) zone = 8; //Shouldn't happen

if (isdown) input.pad[0].data |= tzones[zone];
	else	input.pad[0].data &=~ tzones[zone];
	
}
