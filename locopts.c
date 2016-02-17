/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

/* gtk options loading/saving */

#define _GNU_SOURCE 1
#define _BSD_SOURCE 1
#define __EXTENSIONS__ 1

#define PACKAGE "Genesis Plus/PocketPC"

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

#include "locopts.h"

t_conf *locopts_conf = NULL;

static const char *locopts_default(const char *key);

#define CONFLINELEN 1024

#ifndef _WFIX
#define HEADER "# Genesis Plus/PocketPC ROM configuration file\r\n\r\n"
#define DESC   "# %s\r\n# values: %s\r\n"
#define VALUE  "%s = %s\r\n"
#else
#define HEADER "# Genesis Plus/PC ROM configuration file\n\n"
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

static t_opts locopts_opts[] = {
  { "sound",	 NULL,".",NULL},
  { "fastsound", NULL,".",NULL},
  { "samplerate",NULL,".",NULL},
  { "z80",	 NULL,".",NULL},
  { "frameskip", NULL,".",NULL},
//  { "landscape", NULL,".",NULL}, 
  { "country",   NULL,".",NULL},
  { "sixbuttonpad",NULL,".",NULL},
//  { "crop_screen", NULL,".",NULL}, 
//  { "key_switch",NULL,".",NULL},
  { "key_start", NULL,".",NULL},
  { "key_mode",	 NULL,".",NULL},
  { "key_up",	 NULL,".",NULL},
  { "key_down",	 NULL,".",NULL},
  { "key_left",	 NULL,".",NULL},
  { "key_right", NULL,".",NULL},
  { "key_up_left",     NULL,".",NULL},
  { "key_up_right",    NULL,".",NULL},
  { "key_down_right",  NULL,".",NULL},
  { "key_down_left",   NULL,".",NULL},
  { "key_a",	 NULL,".",NULL},
  { "key_b",	 NULL,".",NULL},
  { "key_c",	 NULL,".",NULL},
  { "key_x",	 NULL,".",NULL},
  { "key_y",	 NULL,".",NULL},
  { "key_z",	 NULL,".",NULL},
/*
  { "tapzones",	NULL,".",NULL},
  { "tapzone_1",NULL,".",NULL},
  { "tapzone_2",NULL,".",NULL},
  { "tapzone_3",NULL,".",NULL},
  { "tapzone_4",NULL,".",NULL},
  { "tapzone_5",NULL,".",NULL},
  { "tapzone_6",NULL,".",NULL},
  { "tapzone_7",NULL,".",NULL},
  { "tapzone_8",NULL,".",NULL},
  { "tapzone_9",NULL,".",NULL},
*/
  { NULL, NULL, NULL, NULL }
};

/* *INDENT-ON* */

int locopts_load(const char *file)
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

  fprintf(stderr, "Reading ROM config...\n");

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
    for (confi = locopts_conf; confi && confi->next; confi = confi->next);
    if (!confi)
      locopts_conf = conf;
    else
      confi->next = conf;
  }
finished:
  if (ferror(fd) || fclose(fd)) {
    fprintf(stderr, "%s: error whilst reading conf: %s\n", PACKAGE,
            strerror(errno));
    return -1;
  }
  fprintf(stderr, "Done.\n");
  return 0;
error:
  fclose(fd);
  fprintf(stderr, "Error.\n");
  return -1;
}

const char *locopts_getvalue(const char *key)
{
  t_conf *c;
  const char *v;

  for (c = locopts_conf; c; c = c->next) {
    if (!strcasecmp(key, c->key))
      return c->value;
  }
  v = locopts_default(key);
  if (!v)
    printf("Warning: invalid option '%s' requested.\n", key);
  return v;
}

int locopts_setvalue(const char *key, const char *value)
{
  t_conf *c;
  char *n;

  for (c = locopts_conf; c; c = c->next) {
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
  c->next = locopts_conf;
  locopts_conf = c;
  return 0;
}

int locopts_save(const char *file)
{
  FILE *fd;
  t_opts *o;

  if ((fd = fopen(file, "w")) == NULL) {
    fprintf(stderr, "%s: unable to open conf '%s' for writing: %s\n",
            PACKAGE, file, strerror(errno));
    return -1;
  }
  fprintf(fd, HEADER);
  for (o = locopts_opts; o->key; o++) {
    fprintf(fd, VALUE, o->key, locopts_getvalue(o->key));
  }
  if (fclose(fd)) {
    fprintf(stderr, "%s: error whilst writing conf: %s\n", PACKAGE,
            strerror(errno));
    return -1;
  }
  return 0;
}

static const char *locopts_default(const char *key)
{
  t_opts *o;

  for (o = locopts_opts; o->key; o++) {
    if (!strcasecmp(key, o->key))
      return o->def;
  }
  return NULL;
}
