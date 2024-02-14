CFLAGS	=	-I. -DGENESIS_HACKS="1" -Ofast -march=i686
OBJ	=       m68kopac.o m68kcpu.o m68kops.o m68kopdm.o m68kopnz.o
all	:	$(OBJ)
		i686-w64-mingw32-ar rs m68k.oa m68kcpu.o m68kops.o m68kopac.o m68kopdm.o m68kopnz.o	
		i686-w64-mingw32-ranlib m68k.oa
.c.o:
	i686-w64-mingw32-gcc  -s -c $(CFLAGS) -o $@ $<

