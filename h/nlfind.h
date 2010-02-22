/* $Id$
   Written 1999 by Tobias Ernst and released do the Public Domain.
   This file is part of NLTOOLS, the nodelist processor of the Husky fidonet
   software project.

*/
#ifndef NLFIND_H
#define NLFIND_H

#include <fidoconf/fidoconf.h>
#include <huskylib/dirlayer.h>

typedef struct s_nlist
{
    char **matches;
    int *applied;
    long *julians;
    int n, nmax;
} nlist;

/* find nodelist or nodediff : base.NNN or base.CNN (C - char, N - digit)
 */
nlist *find_nodelistfiles(char *path, char *base, int allowarc);
/* release 'pnl' list (free memory)
 */
void free_nlist(nlist *pnl);
/* Store 'match' into 'pnl' list
 */
int add_match(nlist *pnl, char *match);

/* find the raw nodelist (look in nodelistdir)
 */
char *findNodelist(s_fidoconfig *config, unsigned int i);

#endif
