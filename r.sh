arm-wince-pe-gcc render.c -Wall -I. -Iwm -Isound -I/usr/local/include/SDL -O3 -mcpu=strongarm -DDETECT_COLLISION -D_CE_HACK -DUSE_CYCLONE -DUSE_DRZ80 -S
arm-wince-pe-gcc vdp.c -Wall -I. -Iwm -Isound -I/usr/local/include/SDL -O3 -mcpu=strongarm -DDETECT_COLLISION -D_CE_HACK -DUSE_CYCLONE -DUSE_DRZ80 -S