
#ifndef _MACROS_H_
#define _MACROS_H_


#define READ_BYTE(ADDR) *(uint8*)((uint32)(ADDR)^1)
#define READ_WORD(ADDR) *(uint16 *)(ADDR)

#define WRITE_BYTE(ADDR, VAL) *(uint8*)((uint32)(ADDR)^1) = (VAL)&0xff
#define WRITE_WORD(ADDR, VAL) *(uint16 *)(ADDR)=VAL

#endif /* _MACROS_H_ */

