.PHONY: default

INSTDIR = /home/fido/sbin
LIBDIR = /home/fido/lib

default: nldiff nlcrc ulc

CC=cc
CFLAGS=-g -I../../smapi -I../../fidoconfig -DUNIX
LFLAGS=-g -L$(LIBDIR)

.c.o: 
	$(CC) $(CFLAGS) -c $<

nldiff: nldiff.o crc16.o
	$(CC) $(LFLAGS) -o nldiff ndiff.o crc16.o

nlcrc: crc16.o nlcrc.o
	$(CC) $(LFLAGS) -o nlcrc crc16.o nlcrc.o

ulc: ulcsort.o ulcomp.o ulc.o nllog.o string.o
	$(CC) $(LFLAGS) -o ulc ulcsort.o ulcomp.o ulc.o nllog.o string.obj -lfidoconfig

clean:
	-rm crc16.o nlcrc.o nldiff.o ulc.o ulcomp.o ulcsort.o nllog.o
	-rm nlcrc
	-rm nldiff
	-rm ulc

install:
	cp ulc $(INSTDIR)/ulc
	cp nldiff $(INSTDIR)/nldiff
	cp nlcrc $(INSTDIR)/nlcrc



