#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "fidoconfig.h"
#include "nlstring.h"
#include "julian.h"
#include "nlfind.h"
#include "nllog.h"
#include "nldate.h"

void free_nlist(nlist *pnl)
{
    int i;

    for (i = 0; i < pnl->n; i++)
    {
        free(pnl->matches[i]);
    }
    if (pnl->matches != NULL)
    {
        free (pnl->matches);
    }
    free(pnl);
}

static nlist *make_nlist(void)
{
    nlist *res = malloc(sizeof(nlist));

    if (res == NULL)
    {
        logentry(LOG_ERROR, "Out of memory.");
        return NULL;
    }

    res->n = 0; res->nmax = 5;
    res->matches = malloc(res->nmax*sizeof(char *));

    if (res->matches == NULL)
    {
        logentry(LOG_ERROR, "Out of memory.");
        free(res);
        return NULL;
    }

    return res;
}

static int add_match(nlist *pnl, char *match)
{
    char *cp = malloc(strlen(match) + 1);
    char **new;

    if (cp == NULL)
    {
        logentry(LOG_ERROR, "Out of memory.");
        return 0;
    }
    if (pnl->n == pnl->nmax)
    {
        if ((new = realloc(pnl->matches, pnl->nmax + 5)) == NULL)
        {
            logentry(LOG_ERROR, "Out of memory.");
            return 0;
        }
        pnl->matches = new;
        pnl->nmax += 5;
    }

    strcpy(cp, match);
    pnl->matches[(pnl->n)++] = cp;
    return 1;
}

nlist *find_nodelistfiles(char *path, char *base, int allowarc)
{
    struct dirent *dp;
    DIR *hdir;
    nlist *pnl = make_nlist();
    size_t l, l2;
    
    if (pnl == NULL)
    {
        return pnl;
    }
        
    hdir = opendir(path);

    if (hdir == NULL)
    {
        logentry(LOG_ERROR, "cannot read directory %s", path);
        free_nlist(pnl);
        return NULL;
    }

    l = strlen(base);

    while ((dp = readdir(hdir)) != NULL)
    {
        l2 = strlen(dp->d_name);
        if (l2 == l + 4 &&
            !ncasecmp(base, dp->d_name, l) &&
            dp->d_name[l] == '.' &&
            (allowarc || isdigit(dp->d_name[l+1])) &&
            isdigit(dp->d_name[l+2]) &&
            isdigit(dp->d_name[l+3]))
        {
            if (!add_match(pnl, dp->d_name))
            {
                free_nlist(pnl);
                closedir(hdir);
                return NULL;
            }

        }
    }

    closedir(hdir);

    if (!pnl->n)
    {
        free_nlist(pnl);
        return NULL;
    }

    return pnl;
}

char *findNodelist(s_fidoconfig *config, int i)
{
    char *nl;
    nlist *pnl;
    int j;
    long lastjul= -1, tmp;
    int lastmatch = -1;
    int l;

    pnl = find_nodelistfiles(config->nodelistDir,
                             config->nodelists[i].nodelistName, 0);

    if (pnl == NULL)
        return NULL;

    nl = malloc((l = strlen(config->nodelistDir)) +
                strlen(config->nodelists[i].nodelistName) + 5);

    if (nl == NULL)
    {
        logentry(LOG_ERROR, "Out of memory.");
        free_nlist(pnl);
        return NULL;
    }

    memcpy(nl, config->nodelistDir, l);

    for (j = 0; j < pnl->n; j++)
    {
        strcpy(nl + l, pnl->matches[j]);

        tmp = parse_nodelist_date(nl);

        if (tmp > lastjul)
        {
            lastjul = tmp;
            lastmatch = j;
        }
    }

    if (lastmatch == -1)
    {
        free_nlist(pnl);
        free(nl);
        return NULL;
    }

    strcpy(nl + l, pnl->matches[lastmatch]);

    free_nlist(pnl);
    return nl;
}
