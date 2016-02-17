#include "shared.h"
#include "options.h"

void get_num_val(const char *s, uint32 *par)
{
const char *v;
  v = gtkopts_getvalue(s);
  *par = atoi(v);
}

/*

void get_tz(const char *s, uint32 *par)
{
const char *v;
int i;
  v = gtkopts_getvalue(s);
  *par = 0;
  for (i=0; i<strlen(v); i++) {
	switch (v[i]) {
		case 'a': *par|=INPUT_A; break;
		case 'b': *par|=INPUT_B; break;
		case 'c': *par|=INPUT_C; break;
		case 'x': *par|=INPUT_X; break;
		case 'y': *par|=INPUT_Y; break;
		case 'z': *par|=INPUT_Z; break;
		case 'u': *par|=INPUT_UP; break;
		case 'd': *par|=INPUT_DOWN; break;
		case 'l': *par|=INPUT_LEFT; break;
		case 'r': *par|=INPUT_RIGHT; break;
		};
  };
}

*/

int lget_num_val(const char *s, uint32 *par)
{
const char *v;
  v = locopts_getvalue(s);
  if (stricmp(v,".")==0) return 0;
  *par = atoi(v);
return 1;
}

/*
int lget_tz(const char *s, uint32 *par)
{
const char *v;
int i;
  v = locopts_getvalue(s);
  if (stricmp(v,".")==0) return 0;
  *par = 0;
  for (i=0; i<strlen(v); i++) {
	switch (v[i]) {
		case 'a': *par|=INPUT_A; break;
		case 'b': *par|=INPUT_B; break;
		case 'c': *par|=INPUT_C; break;
		case 'x': *par|=INPUT_X; break;
		case 'y': *par|=INPUT_Y; break;
		case 'z': *par|=INPUT_Z; break;
		case 'u': *par|=INPUT_UP; break;
		case 'd': *par|=INPUT_DOWN; break;
		case 'l': *par|=INPUT_LEFT; break;
		case 'r': *par|=INPUT_RIGHT; break;
		};
  };
return 1;
}
*/

void get_global_opts() {
const char * v;
uint32 temp;

v = gtkopts_getvalue("sound");
    if (stricmp(v,"on")==0) {sound=1;};

v = gtkopts_getvalue("fastsound");
    if (stricmp(v,"true")==0) {fastsound=1;};

v = gtkopts_getvalue("samplerate");
    sound_rate=atoi(v);

v = gtkopts_getvalue("llsfilter");
    if (stricmp(v,"on")==0) {llsfilter=1;} else { llsfilter=0; };

/*
v = gtkopts_getvalue("rotateright");
    if (stricmp(v,"true")==0) {rotateright=1;};
*/
v = gtkopts_getvalue("sound_sem");
    if (stricmp(v,"true")==0) {use_sem=1;};

v = gtkopts_getvalue("show_keys");
    if (stricmp(v,"true")==0) {show_keys=1;} else { show_keys=0; };

/*
v = gtkopts_getvalue("landscape");
    if (stricmp(v,"true")==0) {landscape=1;} else { landscape=0; };

v = gtkopts_getvalue("crop_screen");
    crop_screen=2; //auto
    if (stricmp(v,"true")==0) {crop_screen=1;};
    if (stricmp(v,"false")==0) {crop_screen=0;};
*/
v = gtkopts_getvalue("z80");
    if (stricmp(v,"off")==0) {use_z80=0;};
    if (stricmp(v,"on")==0) {use_z80=1;};

v = gtkopts_getvalue("frameskip");
    if (stricmp(v,"auto")==0) { 
    autofs=1; frameskip=0;
    } else {
    get_num_val("frameskip",&frameskip); autofs=0;
    }

v = gtkopts_getvalue("savefolder");
    sdir=2; //User-defined by default
    if (stricmp(v,"romfolder")==0) {sdir=0;};
    if (stricmp(v,"appfolder")==0) {sdir=1;};

v = gtkopts_getvalue("configfolder");
    cdir=2; //User-defined by default
    if (stricmp(v,"romfolder")==0) {cdir=0;};
    if (stricmp(v,"appfolder")==0) {cdir=1;};

v = gtkopts_getvalue("country");
    country = 0; //autodetect	
    if (stricmp(v,"u")==0) country=4;
    if (stricmp(v,"j")==0) country=1;
    if (stricmp(v,"e")==0) country=8;

v = gtkopts_getvalue("fullscreen");
    if (stricmp(v,"true")==0) {fullscreen=1;} else { fullscreen=0; };

v = gtkopts_getvalue("benchmark");    
    if (stricmp(v,"true")==0) {benchmark=1;} else { benchmark=0; };

v = gtkopts_getvalue("mt");
    mt = 0;	
    if (stricmp(v,"none")==0) mt=0;
    if (stricmp(v,"two")==0) mt=1;
    if (stricmp(v,"four")==0) mt=2;

//
    get_num_val("renderer",&renderer);

    get_num_val("scale",&scale);

v = gtkopts_getvalue("sixbuttonpad");
    if (stricmp(v,"true")==0) {sixbuttonpad=1;} else { sixbuttonpad=0; };

//get_num_val("key_switch",&key_switch);
get_num_val("key_start",&key_start);
get_num_val("key_up",&key_up);
get_num_val("key_down",&key_down);
get_num_val("key_left",&key_left);
get_num_val("key_right",&key_right);
get_num_val("key_up_left",&key_up_left);
get_num_val("key_up_right",&key_up_right);
get_num_val("key_down_right",&key_down_right);
get_num_val("key_down_left",&key_down_left);
get_num_val("key_a",&key_a);
get_num_val("key_b",&key_b);
get_num_val("key_c",&key_c);
get_num_val("key_x",&key_x);
get_num_val("key_y",&key_y);
get_num_val("key_z",&key_z);
get_num_val("key_mode",&key_mode);

/*
get_num_val("tapzones",&temp);
if (temp==6) tmod=2;
if (temp==9) tmod=3;

get_tz("tapzone_1",&tzones[0]);
get_tz("tapzone_2",&tzones[1]);
get_tz("tapzone_3",&tzones[2]);
get_tz("tapzone_4",&tzones[3]);
get_tz("tapzone_5",&tzones[4]);
get_tz("tapzone_6",&tzones[5]);
get_tz("tapzone_7",&tzones[6]);
get_tz("tapzone_8",&tzones[7]);
get_tz("tapzone_9",&tzones[8]);

*/

}

void get_local_opts() {
const char * v;
uint32 temp;

v = locopts_getvalue("sound");
if (stricmp(v,".")!=0)
	if (stricmp(v,"on")==0) {sound=1;} else { sound=0; };

v = locopts_getvalue("fastsound");
if (stricmp(v,".")!=0)
	if (stricmp(v,"true")==0) {fastsound=1;} else { fastsound=0; };

v = locopts_getvalue("crop_screen");
if (stricmp(v,".")!=0)
	if (stricmp(v,"true")==0) {crop_screen=1;} else { crop_screen=0; };

v = locopts_getvalue("landscape");
if (stricmp(v,".")!=0)
	if (stricmp(v,"true")==0) {landscape=1;} else { landscape=0; };

v = locopts_getvalue("samplerate");
if (stricmp(v,".")!=0)
    sound_rate=atoi(v);

v = locopts_getvalue("z80");
if (stricmp(v,".")!=0) {
    if (stricmp(v,"off")==0) {use_z80=0;};
    if (stricmp(v,"on")==0) {use_z80=1;};
}

v = locopts_getvalue("frameskip");
    if (stricmp(v,"auto")==0) {autofs=1; frameskip=0;
    } else {
    lget_num_val("frameskip",&frameskip); autofs=0; }

v = locopts_getvalue("country");
 if (stricmp(v,".")!=0) {
    country = 0; //autodetect	
    if (stricmp(v,"u")==0) country=4;
    if (stricmp(v,"j")==0) country=1;
    if (stricmp(v,"e")==0) country=8;
}

v = locopts_getvalue("sixbuttonpad");
    if (stricmp(v,"true")==0) {sixbuttonpad=1;} else { sixbuttonpad=0; };

//lget_num_val("key_switch",&key_switch);
lget_num_val("key_start",&key_start);
lget_num_val("key_up",&key_up);
lget_num_val("key_down",&key_down);
lget_num_val("key_left",&key_left);
lget_num_val("key_right",&key_right);
lget_num_val("key_up_left",&key_up_left);
lget_num_val("key_up_right",&key_up_right);
lget_num_val("key_down_right",&key_down_right);
lget_num_val("key_down_left",&key_down_left);
lget_num_val("key_a",&key_a);
lget_num_val("key_b",&key_b);
lget_num_val("key_c",&key_c);
lget_num_val("key_x",&key_x);
lget_num_val("key_y",&key_y);
lget_num_val("key_z",&key_z);
lget_num_val("key_mode",&key_mode);

/*
if (lget_num_val("tapzones",&temp)) {
if (temp==6) tmod=2;
if (temp==9) tmod=3;
}

lget_tz("tapzone_1",&tzones[0]);
lget_tz("tapzone_2",&tzones[1]);
lget_tz("tapzone_3",&tzones[2]);
lget_tz("tapzone_4",&tzones[3]);
lget_tz("tapzone_5",&tzones[4]);
lget_tz("tapzone_6",&tzones[5]);
lget_tz("tapzone_7",&tzones[6]);
lget_tz("tapzone_8",&tzones[7]);
lget_tz("tapzone_9",&tzones[8]);
*/

}

void set_local_opts() {
  const char * v;  
  char s[40];
  if (sound) locopts_setvalue("sound","on");	
  if (!sound) locopts_setvalue("sound","off");	
  if (fastsound) locopts_setvalue("fastsound","true");	
  if (!fastsound) locopts_setvalue("fastsound","false");	
  if (use_z80) locopts_setvalue("z80","on");
  if (!use_z80) locopts_setvalue("z80","off");
  if (crop_screen) locopts_setvalue("crop_screen","true");	
  if (!crop_screen) locopts_setvalue("crop_screen","false");	
  if (landscape) locopts_setvalue("landscape","true");	
  if (!landscape) locopts_setvalue("landscape","false");	

if (!autofs) {
  sprintf(s,"%d",frameskip);
  locopts_setvalue("frameskip",s); 
  } else  {
  sprintf(s,"%s","auto");
  locopts_setvalue("frameskip",s); }

  sprintf(s,"%d",sound_rate);
  locopts_setvalue("samplerate",s);

//  if (tmod==2) locopts_setvalue("tapzones","6");
//  if (tmod==3) locopts_setvalue("tapzones","9");

  if (sixbuttonpad) locopts_setvalue("sixbuttonpad","true"); 
               else locopts_setvalue("sixbuttonpad","false");

  if (country==0) locopts_setvalue("country","auto");
  if (country==4) locopts_setvalue("country","u");
  if (country==1) locopts_setvalue("country","j");
  if (country==8) locopts_setvalue("country","e");

/* Copy Keys From Main conf if no local keys set */

#define PCHECK(PAR2) \
		v = locopts_getvalue(PAR2); \
		if (stricmp(v,".")==0) { \
        v=gtkopts_getvalue(PAR2); \
	    locopts_setvalue(PAR2,v); };

PCHECK("key_start");
//PCHECK("key_switch");
PCHECK("key_up");
PCHECK("key_down");
PCHECK("key_left");
PCHECK("key_right");
PCHECK("key_up_left");
PCHECK("key_up_right");
PCHECK("key_down_right");
PCHECK("key_down_left");
PCHECK("key_a");
PCHECK("key_b");
PCHECK("key_c");
PCHECK("key_x");
PCHECK("key_y");
PCHECK("key_z");
PCHECK("key_mode");
/*
PCHECK("tapzone_1");
PCHECK("tapzone_2");
PCHECK("tapzone_3");
PCHECK("tapzone_4");
PCHECK("tapzone_5");
PCHECK("tapzone_6");
PCHECK("tapzone_7");
PCHECK("tapzone_8");
PCHECK("tapzone_9");
*/
}
