/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

/* gtk options loading/saving */

#define _GNU_SOURCE 1
#define _BSD_SOURCE 1
#define __EXTENSIONS__ 1

#define PACKAGE "Genesis Plus/PC"

#include <stdio.h>
#include <string.h>
#ifndef NO_ERRNO_H
#include <errno.h>
#endif
#include <stdlib.h>

#ifdef _MSC_VER
#define strerror(errno) 0
#define strcasecmp _stricmp
#endif

#include "gtkopts.h"

t_conf *gtkopts_conf = NULL;

static const char *gtkopts_default(const char *key);

#define CONFLINELEN 1024

#ifndef _WFIX
#define HEADER "# Genesis Plus/Pocket PC main configuration file\r\n\r\n"
#define DESC   "# %s\r\n# values: %s\r\n"
#define VALUE  "%s = %s\r\n\r\n"
#else
#define HEADER "# Genesis Plus/PC main configuration file\n\n"
#define DESC   "# %s\n# values: %s\n"
#define VALUE  "%s = %s\n\n"
#endif

typedef struct {
  char *key;
  char *vals;
  char *def;
  char *desc;
} t_opts;

/* *INDENT-OFF* */

static t_opts gtkopts_opts[] = {
  { "scale","2, 3, 4", "2", "Scale output - 2X, 3X, 4X. Toggle with F4." },
  { "mt","none, two, four", "0", "Multihreaded xBRZ filter. none - Single thread, two - Two threads, four - Four threads." },
  { "renderer", "0..8", "0", "0:Plain, 1:TV, 2:Phosph, 3:TV+Phosph, 4:LCD, 5:xBRZ, 6:TV+xBRZ, 7:Phosph+xBRZ, 8:TV+Phosph+xBRZ, Toggle with F3." },
  { "fullscreen", "true/false", "false", "Start fullscreen. Toggle with ESC." },
  { "benchmark","true/false", "false", "Benchmark mode. Print fps to console, no sound, no sync." },
  { "show_keys", "true/false",	"true",	"Show pressed and not binded keys" },
  { "sound",	 "on/off",	"on",	"Sound on/off" },
  { "fastsound", "true/false",	"true",	"Update FM only once per frame - disable if music seem to be incorrect" },
  { "sound_sem", "true/false",	"true",	"Alternate sound buffer wait routine. Works better on newer devices, on my h2210 sound is better with old one. So choose yourself." },
  { "samplerate","8000, 11025, 22050, 44100","8000", "Samplerate to use for audio" }, 
  { "llsfilter", "on/off",	 "off", "Lame lowpass sound filter" }, 
  { "z80",	 "on/off",	 "on",	"Z80 emulation -  on or off"},
  { "country",	 "u/j/e or auto",	"auto",	        "Country code: u-USA, j-Japan, e-Europe or auto-Autodetect"},
  { "frameskip", "auto or 0..9",	"auto",	"Frameskip"},
//  { "rotateright", "true/false", "false","Rotate display right"},
//  { "landscape", "true/false", "true","Landscape mode"},
//  { "crop_screen", "auto/true/false", "false","Crop or shrink screen when in portrait mode or on square-screen device."}, 
  { "romfolder", "Auto-Created", "\\","ROM's folder"},
  { "savefolder", "User-defined folder or 'romfolder','appfolder'", "romfolder","Where to place ROM's saves"},
  { "configfolder", "User-defined folder or 'romfolder','appfolder'", "romfolder","Where to place ROM's config files"},
  { "sixbuttonpad","true/false","false","Whether to emulate six or three button pad" },
//  { "key_switch","SDL keysym",	"193",	"Switch toolbar button" },
  { "key_start", "SDL keysym",	"13",	"Start button" },
  { "key_mode",	 "SDL keysym",	"197",	"Mode button" },
  { "key_up",	 "SDL keysym",	"273",	"Up" },
  { "key_down",	 "SDL keysym",	"274",	"Down" },
  { "key_left",	 "SDL keysym",	"276",	"Left" },
  { "key_right", "SDL keysym",	"275",	"Right" },
  { "key_up_left",       "SDL keysym",	"278",	"Up and Left" },
  { "key_up_right",	 "SDL keysym",	"280",	"Up and Right" },
  { "key_down_right",	 "SDL keysym",	"281",	"Down and Right" },
  { "key_down_left",     "SDL keysym",	"279",	"Down and Left" },
  { "key_a",	 "SDL keysym",	"194",	"'A' button" },
  { "key_b",	 "SDL keysym",	"195",	"'B' button" },
  { "key_c",	 "SDL keysym",	"196",	"'C' button" },
  { "key_x",	 "SDL keysym",	"197",	"'X' button" },
  { "key_y",	 "SDL keysym",	"198",	"'Y' button" },
  { "key_z",	 "SDL keysym",	"199",	"'Z' button" },
/*
  { "tapzones",	 "6 or 9",	"6",	"Number of tap-zones" },
  { "tapzone_1","a,b,c,x,y,z,u,d,l,r",	"abc",	"TapZone 1" },
  { "tapzone_2","a,b,c,x,y,z,u,d,l,r",	"ab",	"TapZone 2" },
  { "tapzone_3","a,b,c,x,y,z,u,d,l,r",	"bc",	"TapZone 3" },
  { "tapzone_4","a,b,c,x,y,z,u,d,l,r",	"a",	"TapZone 4" },
  { "tapzone_5","a,b,c,x,y,z,u,d,l,r",	"b",	"TapZone 5" },
  { "tapzone_6","a,b,c,x,y,z,u,d,l,r",	"c",	"TapZone 6" },
  { "tapzone_7","a,b,c,x,y,z,u,d,l,r",	"a",	"TapZone 7" },
  { "tapzone_8","a,b,c,x,y,z,u,d,l,r",	"b",	"TapZone 8" },
  { "tapzone_9","a,b,c,x,y,z,u,d,l,r",	"c",	"TapZone 9" },
*/
  { NULL, NULL, NULL, NULL }
};

/* *INDENT-ON* */

int gtkopts_load(const char *file)
{
  char buffer[CONFLINELEN];
  FILE *fd;
  char *a, *p, *q, *t;
  int line = 0;
  int i;
  t_conf *conf, *confi;

  if ((fd = fopen(file, "r")) == NULL) {
    fprintf(stderr, "%s: unable to open conf '%s' for reading: %s\n",
            PACKAGE, file, strerror(errno));
    return -1;
  }
  while (fgets(buffer, CONFLINELEN, fd)) {
    line++;
    if (buffer[strlen(buffer) - 1] != '\n') {
      fprintf(stderr, "%s: line %d too long in conf file, ignoring.\n",
              PACKAGE, line);
      while ((a = fgets(buffer, CONFLINELEN, fd))) {
        if (buffer[strlen(buffer) - 1] == '\n')
          continue;
      }
      if (!a)
        goto finished;
      continue;
    }
    for (i=0; i<strlen(buffer); i++) if (buffer[i] == '\r') buffer[i] = ' ';
    buffer[strlen(buffer) - 1] = '\0';
    /* remove whitespace off start and end */
    p = buffer + strlen(buffer) - 1;
    while (p > buffer && *p == ' ')
      *p-- = '\0';
    p = buffer;
    while (*p == ' ')
      p++;
    /* check for comment */
    if (!*p || *p == '#' || *p == ';')
      continue;
    if ((conf = malloc(sizeof(t_conf))) == NULL) {
      fprintf(stderr, "%s: Out of memory adding to conf\n", PACKAGE);
      goto error;
    }
    q = strchr(p, '=');
    if (q == NULL) {
      fprintf(stderr, "%s: line %d not understood in conf file\n", PACKAGE,
              line);
      goto error;
    }
    *q++ = '\0';
    /* remove whitespace off end of p and start of q */
    t = p + strlen(p) - 1;
    while (t > p && *t == ' ')
      *t-- = '\0';
    while (*q == ' ')
      q++;
    if ((conf->key = malloc(strlen(p) + 1)) == NULL ||
        (conf->value = malloc(strlen(q) + 1)) == NULL) {
      fprintf(stderr, "%s: Out of memory building conf\n", PACKAGE);
      goto error;
    }
    strcpy(conf->key, p);
    strcpy(conf->value, q);
    conf->next = NULL;
    for (confi = gtkopts_conf; confi && confi->next; confi = confi->next);
    if (!confi)
      gtkopts_conf = conf;
    else
      confi->next = conf;
  }
finished:
  if (ferror(fd) || fclose(fd)) {
    fprintf(stderr, "%s: error whilst reading conf: %s\n", PACKAGE,
            strerror(errno));
    return -1;
  }
  return 0;
error:
  fclose(fd);
  return -1;
}

const char *gtkopts_getvalue(const char *key)
{
  t_conf *c;
  const char *v;

  for (c = gtkopts_conf; c; c = c->next) {
    if (!strcasecmp(key, c->key))
      return c->value;
  }
  v = gtkopts_default(key);
  if (!v)
    printf("Warning: invalid option '%s' requested.\n", key);
  return v;
}

int gtkopts_setvalue(const char *key, const char *value)
{
  t_conf *c;
  char *n;

  for (c = gtkopts_conf; c; c = c->next) {
    if (!strcasecmp(key, c->key)) {
      if ((n = malloc(strlen(value) + 1)) == NULL)
        return -1;
      strcpy(n, value);
      free(c->value);
      c->value = n;
      return 0;
    }
  }
  if ((c = malloc(sizeof(t_conf))) == NULL ||
      (c->key = malloc(strlen(key) + 1)) == NULL ||
      (c->value = malloc(strlen(value) + 1)) == NULL)
    return -1;
  strcpy(c->key, key);
  strcpy(c->value, value);
  c->next = gtkopts_conf;
  gtkopts_conf = c;
  return 0;
}

int gtkopts_save(const char *file)
{
  FILE *fd;
  t_opts *o;

  if ((fd = fopen(file, "w")) == NULL) {
    fprintf(stderr, "%s: unable to open conf '%s' for writing: %s\n",
            PACKAGE, file, strerror(errno));
    return -1;
  }
  fprintf(fd, HEADER);
  for (o = gtkopts_opts; o->key; o++) {
    fprintf(fd, DESC, o->desc, o->vals);
    fprintf(fd, VALUE, o->key, gtkopts_getvalue(o->key));
  }
  if (fclose(fd)) {
    fprintf(stderr, "%s: error whilst writing conf: %s\n", PACKAGE,
            strerror(errno));
    return -1;
  }
  return 0;
}

static const char *gtkopts_default(const char *key)
{
  t_opts *o;

  for (o = gtkopts_opts; o->key; o++) {
    if (!strcasecmp(key, o->key))
      return o->def;
  }
  return NULL;
}
