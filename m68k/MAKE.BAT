gcc m68kmake.c -o m68kmake.exe
m68kmake
del *.o
del *.oa
gcc -c m68kcpu.c  -o m68kcpu.o  -Ofast -march=i686 -DGENESIS_HACKS="1"
gcc -c m68kops.c  -o m68kops.o  -Ofast -march=i686 -DGENESIS_HACKS="1"
gcc -c m68kopac.c -o m68kopac.o -Ofast -march=i686 -DGENESIS_HACKS="1"
gcc -c m68kopdm.c -o m68kopdm.o -Ofast -march=i686 -DGENESIS_HACKS="1"
gcc -c m68kopnz.c -o m68kopnz.o -Ofast -march=i686 -DGENESIS_HACKS="1"
ar rs m68k.oa m68kcpu.o m68kops.o m68kopac.o m68kopdm.o m68kopnz.o
