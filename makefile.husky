include ../huskymak.cfg

.PHONY: default

default: nldiff$(EXE) nlcrc$(EXE) ulc$(EXE) nlupdate$(EXE)

ifeq ($(DEBUG), 1)
  CFLAGS= -I$(INCDIR) $(DEBCFLAGS)
  LFLAGS=$(DEBLFLAGS)
else
  CFLAGS= -I$(INCDIR) $(OPTCFLAGS)
  LFLAGS=$(OPTLFLAGS)
endif

ifeq ($(SHORTNAME), 1)
  LIBS=-L$(LIBDIR) -lfidoconf
else
  LIBS=-L$(LIBDIR) -lfidoconfig
endif

CDEFS=-D$(OSTYPE) $(ADDCDEFS)

%$(OBJ): src$(DIRSEP)%.c
	$(CC) $(CFLAGS) $(CDEFS) -c $<

nldiff$(EXE): nldiff$(OBJ) crc16$(OBJ) patmat$(OBJ)
	$(CC) $(LFLAGS) -o nldiff$(EXE) nldiff$(OBJ) crc16$(OBJ) patmat$(OBJ) \
              $(LIBS)

nlcrc$(EXE): crc16$(OBJ) nlcrc$(OBJ) patmat$(OBJ)
	$(CC) $(LFLAGS) -o nlcrc$(EXE) crc16$(OBJ) nlcrc$(OBJ) patmat$(OBJ) \
              $(LIBS)

ulc$(EXE): ulcsort$(OBJ) ulcomp$(OBJ) ulc$(OBJ) nllog$(OBJ) string$(OBJ) \
     nldate$(OBJ) julian$(OBJ) nlfind$(OBJ)  patmat$(OBJ)
	$(CC) $(LFLAGS) -o ulc$(EXE) ulcsort$(OBJ) ulcomp$(OBJ) ulc$(OBJ) \
          nllog$(OBJ) string$(OBJ) nldate$(OBJ) julian$(OBJ) nlfind$(OBJ) \
          patmat$(OBJ) $(LIBS)

nlupdate$(EXE): nlupdate$(OBJ) nllog$(OBJ) string$(OBJ) nldate$(OBJ) julian$(OBJ) \
          nlfind$(OBJ) patmat$(OBJ)
	$(CC) $(LFLAGS) -o nlupdate$(EXE) nlupdate$(OBJ) nllog$(OBJ) string$(OBJ) \
          nldate$(OBJ) julian$(OBJ) nlfind$(OBJ) patmat$(OBJ) $(LIBS)

clean:
	-$(RM) crc16$(OBJ)
	-$(RM) nlcrc$(OBJ)
	-$(RM) nldiff$(OBJ)
	-$(RM) ulc$(OBJ)
	-$(RM) ulcomp$(OBJ)
	-$(RM) ulcsort$(OBJ)
	-$(RM) nllog$(OBJ)
	-$(RM) julian$(OBJ)
	-$(RM) nlfind$(OBJ)
	-$(RM) nldate$(OBJ)
	-$(RM) nlupdate$(OBJ)
	-$(RM) string$(OBJ)
	-$(RM) patmat$(OBJ)

distclean: clean
	-$(RM) nlcrc$(EXE)
	-$(RM) nldiff$(EXE)
	-$(RM) ulc$(EXE)
	-$(RM) nlupdate$(EXE)

install:
	$(INSTALL) $(IBOPT) ulc$(EXE) $(BINDIR)
	$(INSTALL) $(IBOPT) nldiff$(EXE) $(BINDIR)
	$(INSTALL) $(IBOPT) nlcrc$(EXE) $(BINDIR)
	$(INSTALL) $(IBOPT) nlupdate$(EXE) $(BINDIR)

