# $Id$
#
# Common makefiles part (include-file) for nltools

# Use defines before including this file:
# _OBJ	object files suffix begins with point (".o", ".obj", ... )
# _LIB	library files suffix begins with point (".a", ".lib", ... )
# _DLL	dinamic linking library files suffix begins with point (".dll", ".so", ...)
# LIBPREFIX  library filename prefix ("lib", "", ...)
# LIBSUFFIX  library filename suffix (compiler ID, three characters, somewhere empty)
# DLLPREFIX  dinamic linking library prefix (usually empty)
# DLLSUFFIX  dinamic linking library suffix (compiler ID or empty)
# O, B, L, H prefixes to use with names of objects, binaries, libraries and headers respectively

# Module name
LIBNAME = nltools

# Version
VERMAJOR= 1
VERMINOR= 9
VERPATCH= 0
VERH	= $(VERMAJOR).$(VERMINOR)
VER	= $(VERH).$(VERPATCH)

# Sources directory
_SRC_DIR= ../src/
# Headers directory
H_DIR	= ../h/

# Programms name
NLDIFF	= $(B)nldiff$(_EXE)
NLCRC	= $(B)nlcrc$(_EXE)
ULC	= $(B)ulc$(_EXE)
NLUPDATE= $(B)nlupdate$(_EXE)

HEADERS	= $(H)ccitttab.h $(H)crc16.h $(H)julian.h $(H)nldate.h $(H)nlfind.h \
	$(H)nlstring.h $(H)ulc.h $(H)version.h $(H)cvsdate.h

# Object files: programs
NLDIFFOBJS   = $(O)nldiff$(_OBJ) $(O)crc16$(_OBJ)
NLCRCOBJS    = $(O)crc16$(_OBJ) $(O)nlcrc$(_OBJ)
ULCOBJS      = $(O)ulcsort$(_OBJ) $(O)trail$(_OBJ) $(O)ulcomp$(_OBJ) $(O)ulc$(_OBJ) \
	$(O)string$(_OBJ) $(O)nldate$(_OBJ) $(O)julian$(_OBJ) $(O)nlfind$(_OBJ)
NLUPDATEOBJS = $(O)nlupdate$(_OBJ) $(O)trail$(_OBJ) $(O)string$(_OBJ) $(O)nldate$(_OBJ) \
	$(O)julian$(_OBJ) $(O)nlfind$(_OBJ)

PRGOBJS      = $(NLDIFFOBJS) $(NLCRCOBJS) $(ULCOBJS) $(NLUPDATEOBJS)
# Header files

# Binary files(s) to bild from sources
PROGRAMS  = $(NLDIFF) $(NLCRC) $(ULC) $(NLUPDATE)

# List of FIDOCONFIG libraries required to build binary file(s)
LIBS	=
