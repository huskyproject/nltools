# $Id$
# Makefile for nltools with Husky build enviroment
# Use GNU version of 'make' program

ifeq ($(DEBIAN), 1)
# Every Debian-Source-Paket has one included.
include /usr/share/husky/huskymak.cfg
else
include ../huskymak.cfg
endif

.PHONY: default

all: default

default: nldiff$(EXE) nlcrc$(EXE) ulc$(EXE) nlupdate$(EXE)

ifeq ($(DEBUG), 1)
  CFLAGS= -I$(INCDIR) -Ih $(DEBCFLAGS)
  LFLAGS=$(DEBLFLAGS)
else
  CFLAGS= -I$(INCDIR) -Ih $(OPTCFLAGS)
  LFLAGS=$(OPTLFLAGS)
endif

ifneq ($(DYNLIBS), 1)
  LFLAGS += -static -lc
endif

ifeq ($(SHORTNAME), 1)
  LIBS=-L$(LIBDIR) -lfidoconf -lsmapi
else
  LIBS=-L$(LIBDIR) -lfidoconfig -lsmapi
endif

CDEFS=-D$(OSTYPE) $(ADDCDEFS)

%$(OBJ): src$(DIRSEP)%.c
	$(CC) $(CFLAGS) $(CDEFS) -c $<

nldiff$(EXE): nldiff$(OBJ) crc16$(OBJ)
	$(CC) $(LFLAGS) -o nldiff$(EXE) nldiff$(OBJ) crc16$(OBJ) \
              $(LIBS)

nlcrc$(EXE): crc16$(OBJ) nlcrc$(OBJ)
	$(CC) $(LFLAGS) -o nlcrc$(EXE) crc16$(OBJ) nlcrc$(OBJ) \
              $(LIBS)

ulc$(EXE): ulcsort$(OBJ) ulcomp$(OBJ) ulc$(OBJ) string$(OBJ) \
     nldate$(OBJ) julian$(OBJ) nlfind$(OBJ)
	$(CC) $(LFLAGS) -o ulc$(EXE) ulcsort$(OBJ) ulcomp$(OBJ) ulc$(OBJ) \
          string$(OBJ) nldate$(OBJ) julian$(OBJ) nlfind$(OBJ) \
         $(LIBS)

nlupdate$(EXE): nlupdate$(OBJ) string$(OBJ) nldate$(OBJ) julian$(OBJ) \
          nlfind$(OBJ)
	$(CC) $(LFLAGS) -o nlupdate$(EXE) nlupdate$(OBJ) string$(OBJ) \
          nldate$(OBJ) julian$(OBJ) nlfind$(OBJ) $(LIBS)

clean:
	-$(RM) $(RMOPT) crc16$(OBJ)
	-$(RM) $(RMOPT) nlcrc$(OBJ)
	-$(RM) $(RMOPT) nldiff$(OBJ)
	-$(RM) $(RMOPT) ulc$(OBJ)
	-$(RM) $(RMOPT) ulcomp$(OBJ)
	-$(RM) $(RMOPT) ulcsort$(OBJ)
	-$(RM) $(RMOPT) julian$(OBJ)
	-$(RM) $(RMOPT) nlfind$(OBJ)
	-$(RM) $(RMOPT) nldate$(OBJ)
	-$(RM) $(RMOPT) nlupdate$(OBJ)
	-$(RM) $(RMOPT) string$(OBJ)

distclean: clean
	-$(RM) $(RMOPT) nlcrc$(EXE)
	-$(RM) $(RMOPT) nldiff$(EXE)
	-$(RM) $(RMOPT) ulc$(EXE)
	-$(RM) $(RMOPT) nlupdate$(EXE)

install: ulc$(EXE) nldiff$(EXE) nlcrc$(EXE) nlupdate$(EXE)
	$(INSTALL) $(IBOPT) ulc$(EXE) $(BINDIR)
	$(INSTALL) $(IBOPT) nldiff$(EXE) $(BINDIR)
	$(INSTALL) $(IBOPT) nlcrc$(EXE) $(BINDIR)
	$(INSTALL) $(IBOPT) nlupdate$(EXE) $(BINDIR)

uninstall:
	-$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)ulc$(EXE)
	-$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)nldiff$(EXE)
	-$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)nlcrc$(EXE)
	-$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)nlupdate$(EXE)
