#ifndef _STATE_H_
#define _STATE_H_

#define STREAM gzFile 
#define OPEN_STREAM gzopen
#define CLOSE_STREAM gzclose

#define READ(val, size) gzread(file,val,size)
#define WRITE(val, size) gzwrite(file,val,size)

int state_load(int num,int onlytest);
int state_save(int num);
void save_sram(void);
void load_sram(void);

#endif
