#ifndef NLFIND_H
#define NLFIND_H

#include "fidoconfig.h"
#include "dirlayer.h"

typedef struct s_nlist
{
    char **matches;
    int n, nmax;
} nlist;

nlist *find_nodelistfiles(char *path, char *base, int allowarc);
void free_nlist(nlist *pnl);

char *findNodelist(s_fidoconfig *config, int i); /* find the raw nodelist */

#endif
