# include file for makefiles

NLDIFFOBJS   = nldiff$(OBJ) crc16$(OBJ)
NLCRCOBJS    = crc16$(OBJ) nlcrc$(OBJ)
ULCOBJS      = ulcsort$(OBJ) trail$(OBJ) ulcomp$(OBJ) ulc$(OBJ) string$(OBJ) nldate$(OBJ) julian$(OBJ) nlfind$(OBJ) patmat$(OBJ) ffind$(OBJ) fexist$(OBJ)
NLUPDATEOBJS = nlupdate$(OBJ) trail$(OBJ) string$(OBJ) nldate$(OBJ) julian$(OBJ) nlfind$(OBJ) patmat$(OBJ) ffind$(OBJ) fexist$(OBJ)
OBJS = $(NLDIFFOBJS) $(NLCRCOBJS) $(ULCOBJS) $(NLUPDATEOBJS)
