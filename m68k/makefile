CFLAGS	=	-I. -DGENESIS_HACKS="1" -Ofast -march=i686
OBJ	=       m68kopac.o m68kcpu.o m68kops.o m68kopdm.o m68kopnz.o
all	:	$(OBJ)
		ar rs m68k.oa m68kcpu.o m68kops.o m68kopac.o m68kopdm.o m68kopnz.o	
		ranlib m68k.oa
.c.o:
	gcc  -s -c $(CFLAGS) -o $@ $<

