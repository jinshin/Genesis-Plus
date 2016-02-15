int gtkopts_load(const char *file);
int gtkopts_save(const char *file);
const char *gtkopts_getvalue(const char *key);
int gtkopts_setvalue(const char *key, const char *val);
#ifndef T_CONF
#define T_CONF  
typedef struct _t_conf {
  struct _t_conf *next;
  char *key;
  char *value;
} t_conf;
#endif
extern t_conf *gtkopts_conf;
