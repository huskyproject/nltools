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
# _SRC_DIR  sources dir

LIBNAME = nltools

# Version
VERMAJOR= 1
VERMINOR= 9
VERPATCH= 0
VERH	= $(VERMAJOR).$(VERMINOR)
VER	= $(VERH).$(VERPATCH)

# Programms name
NLDIFF	= nldiff$(_EXE)
NLCRC	= nlcrc$(_EXE)
ULC	= ulc$(_EXE)
NLUPDATE= nlupdate$(_EXE)

# Object files: programs
NLDIFFOBJS   = nldiff$(_OBJ) crc16$(_OBJ)
NLCRCOBJS    = crc16$(_OBJ) nlcrc$(_OBJ)
ULCOBJS      = ulcsort$(_OBJ) trail$(_OBJ) ulcomp$(_OBJ) ulc$(_OBJ) string$(_OBJ) nldate$(_OBJ) julian$(_OBJ) nlfind$(_OBJ)
NLUPDATEOBJS = nlupdate$(_OBJ) trail$(_OBJ) string$(_OBJ) nldate$(_OBJ) julian$(_OBJ) nlfind$(_OBJ)

# Header files
HEADERS	= ccitttab.h crc16.h julian.h nldate.h nlfind.h nlstring.h ulc.h \
	  version.h cvsdate.h

# Sources directory
_SRC_DIR= ../src/

# Headers directory
H_DIR	= ../h/

# Binary files(s) to bild from sources
PROGRAMS  = $(NLDIFF) $(NLCRC) $(ULC) $(NLUPDATE)

# List of FIDOCONFIG libraries required to build binary file(s)
LIBS	=
