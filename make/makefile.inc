# include file for makefiles

NLDIFFOBJS   = nldiff$(_OBJ) crc16$(_OBJ)
NLCRCOBJS    = crc16$(_OBJ) nlcrc$(_OBJ)
ULCOBJS      = ulcsort$(_OBJ) trail$(_OBJ) ulcomp$(_OBJ) ulc$(_OBJ) string$(_OBJ) nldate$(_OBJ) julian$(_OBJ) nlfind$(_OBJ)
NLUPDATEOBJS = nlupdate$(_OBJ) trail$(_OBJ) string$(_OBJ) nldate$(_OBJ) julian$(_OBJ) nlfind$(_OBJ)
OBJS = $(NLDIFFOBJS) $(NLCRCOBJS) $(ULCOBJS) $(NLUPDATEOBJS)


commonprogs: nldiff$(_EXE) nlcrc$(_EXE) ulc$(_EXE) nlupdate$(_EXE)

nldiff$(_EXE): $(NLDIFFOBJS)
	$(CC) $(LFLAGS) -o nldiff$(_EXE) $(NLDIFFOBJS) $(LIBS)

nlcrc$(_EXE): $(NLCRCOBJS)
	$(CC) $(LFLAGS) -o nlcrc$(_EXE) $(NLCRCOBJS) $(LIBS)

ulc$(_EXE): $(ULCOBJS)
	$(CC) $(LFLAGS) -o ulc$(_EXE) $(ULCOBJS) $(LIBS)

nlupdate$(_EXE): $(NLUPDATEOBJS)
	$(CC) $(LFLAGS) -o nlupdate$(_EXE) $(NLUPDATEOBJS) $(LIBS)

commonclean:
	-$(RM) *$(_OBJ)

commondistclean: commonclean
	-$(RM) nlcrc$(_EXE)
	-$(RM) nldiff$(_EXE)
	-$(RM) ulc$(_EXE)
	-$(RM) nlupdate$(_EXE)
