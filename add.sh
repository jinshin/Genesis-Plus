arm-wince-mingw32ce-gcc -c sound/fm.c -o obj/fm.o -I. -Iwm -Isound -I/usr/local/include -O2 -mcpu=strongarm -fomit-frame-pointer -DUSE_CYCLONE -DUSE_DRZ80 -D_CE_HACK
arm-wince-mingw32ce-gcc -c vdp.c -o obj/vdp.o -I. -Iwm -Isound -I/usr/local/include -O2 -mcpu=strongarm -fomit-frame-pointer -DUSE_CYCLONE -DUSE_DRZ80 -D_CE_HACK
arm-wince-mingw32ce-gcc -c memvdp.c -o obj/memvdp.o -I. -Iwm -Isound -I/usr/local/include -O2 -mcpu=strongarm -fomit-frame-pointer -DUSE_CYCLONE -DUSE_DRZ80 -D_CE_HACK
arm-wince-mingw32ce-gcc -c membnk.c -o obj/membnk.o -I. -Iwm -Isound -I/usr/local/include -O2 -mcpu=strongarm -fomit-frame-pointer -DUSE_CYCLONE -DUSE_DRZ80 -D_CE_HACK
arm-wince-mingw32ce-gcc -c memz80.c -o obj/memz80.o -I. -Iwm -Isound -I/usr/local/include -O2 -mcpu=strongarm -fomit-frame-pointer -DUSE_CYCLONE -DUSE_DRZ80 -D_CE_HACK
arm-wince-mingw32ce-gcc -c mem68k_c.c -o obj/mem68k_c.o -I. -Iwm -Isound -I/usr/local/include -O2 -mcpu=strongarm -fomit-frame-pointer -DUSE_CYCLONE -DUSE_DRZ80 -D_CE_HACK
arm-wince-mingw32ce-gcc -c system.c -o obj/system.o -I. -Iwm -Isound -I/usr/local/include -O2 -mcpu=strongarm -fomit-frame-pointer -DUSE_CYCLONE -DUSE_DRZ80 -D_CE_HACK
arm-wince-mingw32ce-gcc -c genesis.c -o obj/genesis.o -I. -Iwm -Isound -I/usr/local/include -O2 -mcpu=strongarm -fomit-frame-pointer -DUSE_CYCLONE -DUSE_DRZ80 -D_CE_HACK
