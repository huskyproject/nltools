# Legacy makefile for BeOS
#
# Do not support husky build enviroment (makefile.cfg)

.PHONY: default

OBJ=.o
CC=gcc
RM=rm
EXE=
CFLAGS=-O3 -I../h -DUNIX
LFLAGS=-s -L../../fidoconf -L../../smapi
LIBS= -lfidoconfigbe -lsmapibe

.c.o:
	$(CC) $(CFLAGS) -c $<

include makefile.inc

all: commonprogs

default: all

clean: commonclean

distclean: clean commondistclean
