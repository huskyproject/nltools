#ifndef NLFIND_H
#define NLFIND_H

#include <fidoconf/fidoconf.h>
#include <fidoconf/dirlayer.h>

typedef struct s_nlist
{
    char **matches;
    long *julians;
    int n, nmax;
} nlist;

nlist *find_nodelistfiles(char *path, char *base, int allowarc);
void free_nlist(nlist *pnl);

char *findNodelist(s_fidoconfig *config, int i); /* find the raw nodelist */

#endif
