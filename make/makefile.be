# Legacy makefile for BeOS
#
# Do not support husky build enviroment (makefile.cfg)

.PHONY: default

SRC_DIR=../src/
OBJ=.o
CC=gcc
RM=rm
EXE=
CFLAGS=-O3 -I../h -I../.. -DUNIX
LFLAGS=-s -L../../fidoconf -L../../smapi
LIBS= -lfidoconfigbe -lsmapibe

{$(SRC_DIR)}.c$(OBJ):
	$(CC) $(CFLAGS) -c $<

include makefile.inc

all: commonprogs

default: all

clean: commonclean

distclean: clean commondistclean
