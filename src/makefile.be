.PHONY: default


default: nldiff nlcrc ulc nlupdate
all: default

OBJ=.o
CC=gcc
CFLAGS=-O3 -I../.. -DUNIX
LFLAGS=-s -L../../fidoconf -L../../smapi
LIBS= -lfidoconfigbe -lsmapibe

include makefile.inc

.c.o:
	$(CC) $(CFLAGS) -c $<

nldiff: $(NLDIFFOBJS)
	$(CC) $(LFLAGS) -o nldiff $(NLDIFFOBJS)

nlcrc: $(NLCRCOBJS)
	$(CC) $(LFLAGS) -o nlcrc $(NLCRCOBJS)

ulc: $(ULCOBJS)
	$(CC) $(LFLAGS) -o ulc $(ULCOBJS) $(LIBS)

nlupdate: $(NLUPDATEOBJS)
	$(CC) $(LFLAGS) -o nlupdate $(NLUPDATEOBJS) $(LIBS)

clean:
	-rm $(OBJS)

distclean: clean
	-rm nlcrc
	-rm nldiff
	-rm ulc
	-rm nlupdate
