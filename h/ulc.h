/* $Id$
   Written 1999 by Tobias Ernst and released do the Public Domain.
   This file is part of NLTOOLS, the nodelist processor of the Husky fidonet
   software project.

*/
#ifndef ULC_H
#define ULC_H

#define LOGNAME "nltools.log"

enum { F_NODELIST, F_POINTS24, F_POINTS4D };

int ul_compile (FILE *fin, FILE *fout, int type, int defzone);
int ul_sort    (FILE *);

#endif
