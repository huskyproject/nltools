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
char *findNodelist(s_fidoconfig *config, int i);

#endif
