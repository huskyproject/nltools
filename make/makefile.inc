# include file for makefiles

NLDIFFOBJS   = nldiff$(OBJ) crc16$(OBJ)
NLCRCOBJS    = crc16$(OBJ) nlcrc$(OBJ)
ULCOBJS      = ulcsort$(OBJ) trail$(OBJ) ulcomp$(OBJ) ulc$(OBJ) string$(OBJ) nldate$(OBJ) julian$(OBJ) nlfind$(OBJ)
NLUPDATEOBJS = nlupdate$(OBJ) trail$(OBJ) string$(OBJ) nldate$(OBJ) julian$(OBJ) nlfind$(OBJ)
OBJS = $(NLDIFFOBJS) $(NLCRCOBJS) $(ULCOBJS) $(NLUPDATEOBJS)


commonprogs: nldiff$(EXE) nlcrc$(EXE) ulc$(EXE) nlupdate$(EXE)

nldiff$(EXE): $(NLDIFFOBJS)
	$(CC) $(LFLAGS) -o nldiff$(EXE) $(NLDIFFOBJS) $(LIBS)

nlcrc$(EXE): $(NLCRCOBJS)
	$(CC) $(LFLAGS) -o nlcrc$(EXE) $(NLCRCOBJS) $(LIBS)

ulc$(EXE): $(ULCOBJS)
	$(CC) $(LFLAGS) -o ulc$(EXE) $(ULCOBJS) $(LIBS)

nlupdate$(EXE): $(NLUPDATEOBJS)
	$(CC) $(LFLAGS) -o nlupdate$(EXE) $(NLUPDATEOBJS) $(LIBS)

commonclean:
	-$(RM) *$(OBJ)

commondistclean: commonclean
	-$(RM) nlcrc$(EXE)
	-$(RM) nldiff$(EXE)
	-$(RM) ulc$(EXE)
	-$(RM) nlupdate$(EXE)
