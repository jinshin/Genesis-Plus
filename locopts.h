int locopts_load(const char *file);
int locopts_save(const char *file);
const char *locopts_getvalue(const char *key);
int locopts_setvalue(const char *key, const char *val);
#ifndef T_CONF  
#define T_CONF
typedef struct _t_conf {
  struct _t_conf *next;
  char *key;
  char *value;
} t_conf;
#endif
extern t_conf *locopts_conf;
