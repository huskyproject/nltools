/* ul_sort sorts a file in FIDOUSER.LST format alphabetically
   (case-insensitive) by sysop name, so that message readers etc. can
   use a binary search mechanism to look up node numbers by sysop name.

   Note: The file *MUST* be opened in binary mode and SHOULD contain CRLF in
   DOSish style even on UNIX!!!

   Written 1999 by Tobias Ernst and released to the Public Domain.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define CACHESIZE 400

struct ul_info
{
    FILE *f;

    size_t n;       /* number of records */
    size_t reclen;  /* length of a single entry */
    char cache[CACHESIZE][200];
    long cachepos[CACHESIZE];

    int  slots_by_n[CACHESIZE];
    long n_by_n[CACHESIZE];

    int uncommited[CACHESIZE];
    int slot[2], high;
    long store, commit, fetch, hit;
    long nc;
};


static int bynpos(struct ul_info *hulc, long n)
{
    int l, r, pivot=0;

    l = 0; r = CACHESIZE - 1;

    while(r - l > 1)
    {
        pivot = ((r - l) >> 1) + l;
        if (n > hulc->n_by_n[pivot])
        {
            l = pivot;
        }
        else
            r = pivot;
    }

    if (n > hulc->n_by_n[l])
        l++;
    if (n > hulc->n_by_n[l])
        l++;

    return l;
}

static void setrecord(struct ul_info *hul, int cp, long n, int bp2)
{
    int bp;

    bp = bynpos(hul, hul->cachepos[cp]);

    if (bp2 > bp)
    {
        if (bp2 - bp > 1)
        {
            memmove(hul->n_by_n + bp,
                    hul->n_by_n + bp + 1,
                    (bp2 - bp - 1) * sizeof(long));
            memmove(hul->slots_by_n + bp,
                    hul->slots_by_n + bp + 1,
                    (bp2 - bp - 1) * sizeof(int));
        }
        bp2--;
    }
    else if (bp2 < bp)
    {
        memmove(hul->n_by_n + bp2 + 1,
                hul->n_by_n + bp2,
                (bp - bp2) * sizeof(long));
        memmove(hul->slots_by_n + bp2 + 1,
                hul->slots_by_n + bp2,
                (bp - bp2) * sizeof(int));
    }

    hul->n_by_n[bp2] = n;
    hul->slots_by_n[bp2] = cp;
    hul->cachepos[cp] = n;
}

static int ul_commit(struct ul_info *hulc, int i)
{
    if (fseek(hulc->f, hulc->cachepos[i] * hulc->reclen, SEEK_SET))
    {
        return 0;
    }

    if (fwrite(hulc->cache[i], hulc->reclen, 1, hulc->f) != 1)
    {
        return 0;
    }

    hulc->commit++;

    return 1;
}

static int ul_commit_all(struct ul_info* hulc)
{
    int i;

    for (i = 0; i < CACHESIZE; i++)
        if (hulc->uncommited[i])
            if (!ul_commit(hulc, i))
                return 0;

    return 1;
}

static char *ul_fetch(struct ul_info *hulc, long n, int slot)
{
    int  bp;

    hulc->fetch++;
    bp = bynpos(hulc,n);

    if (n == hulc->n_by_n[bp])
    {
        hulc->slot[slot] = hulc->slots_by_n[bp];
        hulc->hit++;
    }
    else
    {

        /* find the oldest cache slot */
        while (hulc->slot[!slot] == hulc->high)
            hulc->high = (hulc->high + 1) % CACHESIZE;

        if (hulc->uncommited[hulc->high])
        {
            if (!ul_commit(hulc, hulc->high))
                return NULL;
            hulc->uncommited[hulc->high] = 0;

        }
        
        hulc->slot[slot] = hulc->high;
        setrecord(hulc, hulc->high, n, bp);
        
        /* read data into this slot */
        if (fseek(hulc->f, n * hulc->reclen, SEEK_SET))
        {
            return NULL;
        }

        if (fread(hulc->cache[hulc->high], hulc->reclen, 1, hulc->f) != 1)
        {
            return NULL;
        }

        /* mark the next slot to be overwritten */
        hulc->high = (hulc->high + 1) % CACHESIZE;
    }

    return hulc->cache[hulc->slot[slot]];
}

static int ul_store(struct ul_info *hulc, long n, char *buf)
{
    int cp, bp;

    hulc->store++;
    bp = bynpos(hulc,n);
    if (n == hulc->n_by_n[bp])
    {
        cp = hulc->slots_by_n[bp];

        if (hulc->slot[0] != cp && hulc->slot[1] != cp)
        {
            if (buf != hulc->cache[cp])
                memcpy(hulc->cache[cp], buf, hulc->reclen);
            hulc->uncommited[cp] = 1;
            return 1;
        }
        else
        {
            /* invalidate this cache entry. we cannot overwrite
               it because the application still has a pointer to it */

            bp = bynpos(hulc, hulc->nc);
            setrecord(hulc, cp, hulc->nc--, bp);
            bp = bynpos(hulc, n);
            hulc->uncommited[cp] = 0;
        }
    }

    /* find a new slot to store the new value in *
            
    /* find the oldest cache slot */
    while (hulc->slot[0] == hulc->high || hulc->slot[1] == hulc->high)
        hulc->high = (hulc->high + 1) % CACHESIZE;

    if (hulc->uncommited[hulc->high])
    {
        if (!ul_commit(hulc, hulc->high))
            return NULL;
    }
    hulc->uncommited[hulc->high] = 1;
    setrecord(hulc, hulc->high, n, bp);
    memmove(hulc->cache[hulc->high], buf, hulc->reclen);
    hulc->high = (hulc->high + 1) % CACHESIZE;
    return 1;
}

#if 0
static int ul_compare(struct ul_info *hul, char *e1, char *e2, int *iflag)
{
    char name[2][200];
    int zone[2], net[2], node[2], point[2];
    char *e[2];
    int i,j,k,l,m,n,res,nl[2];

    e[0] = e1; e[1] = e2;

    for (i = 0; i < 2; i++)
    {
        zone[i] = net[i] = node[i] = point[i] = 0;

        j = hul->reclen - 1;
        while (j>0 && (isspace(e[i][j])))
            j--;
        k = j;      /* k ist last character of node number */
        while (j>0 && (!isspace(e[i][j])))
            j--;
        l = j + 1;  /* l is first character of node number */
        while (j>0 && isspace(e[i][j]))
            j--;
        m = j;  /* m is last character of name */
        if (k <=0 || j <= 0 || l <= 0 || m<0)
        {
            *iflag = 1; return 1-2*i; /* invalid format */
        }

        nl[i]=m;
    }

    if (nl[0] > nl[1])
        m = nl[1];
    else
        m = nl[0];
    
    res = strncasecmp(e[0], e[1], m);
    if (!res)
        res = nl[0] - nl[1];

    return res;
}
#endif
#define ul_compare(a,b,c,d) strncasecmp(b,c,a->reclen)

static int ul_shellsort(struct ul_info *hulc, int insertonly)
{
    long int N = hulc->n, step, h, i;
    int iflag = 0;
    char *temp; char *cmpr;

    if (insertonly)
        h = 1;
    else
        for (h = 1; h <= N / 9; h = 3*h + 1);

    while (h > 0)
    {
        for (step = h; step < N; step++)
        {
            temp = ul_fetch(hulc, step, 0);
            if (temp == NULL)
                return 0;

            for (i = step - h; i >= 0; i-=h)
            {
                cmpr = ul_fetch(hulc, i, 1);
                if (cmpr == NULL)
                    return 0;
                
                if (ul_compare(hulc, temp, cmpr, &iflag) < 0)
                {
                    if ((!ul_store(hulc, i + h, cmpr)) || iflag)
                        return 0;
                }
                else
                    break;
            }
            if ((!ul_store(hulc, i + h, temp)) || iflag)
                return 0;
        }
        h = h / 3;
    }
    return 1;
}


static int ul_xquicksort(struct ul_info *hul, long L, long R)
{
    int iflag = 0;
    
    while (R - L >= 9)
    {
        long i, j, mid;
        char *tm, *tl, *tr, *tmp;

        mid = (L + R) / 2;

        tm = ul_fetch(hul, mid, 0);
        tl = ul_fetch(hul, L, 1);

        if (ul_compare(hul, tl, tm, &iflag) > 0)
        {
            ul_store(hul, L, tm);
            ul_store(hul, mid, tl);
        }

        tl = ul_fetch(hul, L, 1);
        tr = ul_fetch(hul, R, 0);

        if (ul_compare(hul, tl, tr, &iflag) > 0)
        {
            ul_store(hul, L, tr);
            ul_store(hul, R, tl);
        }

        tm = ul_fetch(hul, mid, 1);
        tr = ul_fetch(hul, R, 0);        

        if (ul_compare(hul, tm, tr, &iflag) > 0)
        {
            ul_store(hul, mid, tr);
            ul_store(hul, R, tm);
        }
        
        tm = ul_fetch(hul, mid, 0);
        tr = ul_fetch(hul, R-1, 1);
        ul_store(hul, R-1, tm);
        ul_store(hul, mid, tr);

        i=L;
        j=R-1;

        while(1)
        {
            tr = ul_fetch(hul, R-1, 1);
            do
            {
                tmp = ul_fetch(hul, ++i, 0);
            } while(ul_compare(hul, tmp, tr, &iflag) < 0);
            do
            {
                tmp = ul_fetch(hul, --j, 0);
            } while(ul_compare(hul, tmp, tr, &iflag) > 0);

            if (i >= j)
                break;

            tl = ul_fetch(hul, i, 1);
            ul_store(hul, i, tmp);
            ul_store(hul, j, tl);
        }

        tmp = ul_fetch(hul, i, 0);
        tr  = ul_fetch(hul, R-1, 1);
        ul_store(hul, R-1, tmp);
        ul_store(hul, i, tr);

        if (i - L > R - i)
        {
            ul_xquicksort(hul, i+1, R);
            R = i-1;
        }
        else
        {
            ul_xquicksort(hul, L, i-1);
            L = i+1;
        }
    }
}
        

static int ul_quicksort(struct ul_info *hul)
{
    ul_xquicksort(hul, 0, hul->n - 1);
    ul_shellsort(hul, 1);
}
   

int ul_sort(FILE *f)
{
    struct ul_info hul;
    char buf[200];
    long filelen;
    int i;
    int rv;
    
    hul.f = f;

    if (fseek(f, 0L, SEEK_END))
        return 0;

    if ((filelen = ftell(f)) == -1)
        return 0;
    
    if (fseek(f, 0L, SEEK_SET))
        return 0;

    if (fgets(buf, sizeof buf, f) != NULL)
    {
        hul.reclen = strlen(buf);
    }
    else
    {
        return 0;
    }
    hul.slot[0] = hul.slot[1] = -1;
    hul.high = 0;
    for (i = 0; i < CACHESIZE; i++)
    {
        hul.cachepos[i]=-(CACHESIZE-i);
        hul.uncommited[i] = 0;
        hul.n_by_n[i] = -(CACHESIZE-i);
        hul.slots_by_n[i] = i;
    }
    hul.nc = -i;
    hul.hit = 0; hul.store = 0; hul.fetch = 0; hul. commit = 0;
    
        

    if (fseek(f, 0L, SEEK_SET))
        return 0;

    hul.n = filelen / hul.reclen;

    if ((filelen % hul.reclen) > 1) /* allow for one byte Ctrl+Z garbage */
        return 0;

    rv= ul_quicksort(&hul);

    if (!ul_commit_all(&hul))
        rv = 0;

    printf ("Read: %ld of %ld cached, Write: %ld of %ld cached\n",
            hul.hit, hul.fetch, hul.store-hul.commit, hul.store);

    return rv;
}
