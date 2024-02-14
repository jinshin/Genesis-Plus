# Genesis Plus - Sega Mega Drive emulator
# (c) 1999, 2000, 2001, 2002, 2003  Charles MacDonald

# -DLSB_FIRST - Leave undefined for big-endian processors.
# -DDEBUG     - Enable debugging code
# -DX86_ASM   - Enable inline x86 assembly code in Z80 emulator

CC	=	i686-w64-mingw32-gcc
AS	=	nasm -f coff
LDFLAGS	=	--enable-stdcall-fixup
FLAGS	=	-I. -Icpu -Iwin -Isound -I/usr/local/cross-tools/i386-mingw32/include -I/usr/local/cross-tools/i386-mingw32/include/SDL \
		 \
		 \
		-Ofast -march=i686 -D_WFIX -s

LIBS	=	m68k/m68k.oa -s -lz -lm -lSDL -lwinmm -lcomdlg32 -lgdi32 -lkernel32 -lpthread -static -Wl,-Map=genmap.txt -L/usr/local/cross-tools/i386-mingw32/lib

#-Wl,-Map=1.txt

OBJ	=       obj/z80.oa	\
		obj/genesis.o	\
		obj/vdp.o	\
		obj/render.o	\
		obj/system.o    \
		obj/unzip.o     \
		obj/fileio.o	\
		obj/loadrom.o	\
		obj/mio.o	\
		obj/mem68k.o	\
		obj/mem68k_c.o	\
		obj/memz80.o	\
		obj/membnk.o	\
		obj/memvdp.o	\
		obj/state.o	\
		obj/gtkopts.o	\
		obj/locopts.o	\
		obj/eeprom.o	\
		obj/icon.o
		
OBJ	+=      obj/sound.o	\
		obj/fm.o	\
		obj/sn76496.o
		
OBJ	+=	obj/main.o	\
		obj/error.o	\
		obj/msg.o	\
		obj/options.o   \
		obj/xBRZ.o

EXE	=	genpp.exe

all	:	$(EXE)

$(EXE)	:	$(OBJ) shared.h
		$(CC) -o $(EXE) $(OBJ) $(LIBS) $(LDFLAGS)
	        
obj/%.oa :	cpu/%.c cpu/%.h
		$(CC) -c $< -o $@ $(FLAGS)

obj/%.o : 	%.c %.h
		$(CC) -c $< -o $@ $(FLAGS)

obj/%.o : 	%.s 
		$(CC) -c $< -o $@

obj/%.o : 	%.rc 
		i686-w64-mingw32-windres $< -o $@
	        
obj/%.o :	win/%.s
		$(AS) $< -o $@
	        
obj/%.o :	sound/%.c sound/%.h	        
		$(CC) -c $< -o $@ $(FLAGS)
	        
obj/%.o :	cpu/%.c cpu/%.h	        
		$(CC) -c $< -o $@ $(FLAGS)

obj/%.o :	win/%.c win/%.h	        
		$(CC) -c $< -o $@ $(FLAGS)

obj/%.o :	%.cpp %.h	        
		$(CC) -c $< -o $@ $(FLAGS)
	        
pack	:
		i686-w64-mingw32-strip $(EXE)
		upx -9 $(EXE)	        

clean	:	        
		rm -f obj/*.o
		rm -f obj/*.oa
		rm -f *.bak
		rm -f *.log
		rm -f *.wav
		rm -f *.zip
cleancpu :		
		rm -f obj/z80.oa

makedir :
		mkdir obj
	        
archive:	        
		pk -add -max -excl=bak -excl=out \
		-excl=test -excl=obj mdsrc.zip -dir=current
	        
#
# end of makefile
#
