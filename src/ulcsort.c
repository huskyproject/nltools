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
#include <stdlib.h>

#include <huskylib/log.h>
#include "nlstring.h"
#include "ulc.h"

#define CACHESIZE 400

int rl;

static int ulcomp(const void *p1, const void *p2)
{
    return ncasecmp((const char *)p1, (const char *)p2, rl);
}

int ul_sort(FILE *f)
{
    long filelen;
    long reclen;
    long n;
    void *buffer;
    char buf[200];


    if (fseek(f, 0L, SEEK_END))
        return 0;

    if ((filelen = ftell(f)) == -1)
        return 0;

    if (fseek(f, 0L, SEEK_SET))
        return 0;

    if (fgets(buf, sizeof buf, f) != NULL)
    {
        reclen = strlen(buf);
    }
    else
    {
        return 0;
    }

    n = filelen / reclen;

    if ((filelen % reclen) > 1) /* allow for one byte Ctrl+Z garbage */
        return 0;

    buffer = malloc(n * reclen);

    if (buffer == NULL)
    {
        w_log(LL_ERROR, "Out of memory (request for %ld bytes failed).",
                 n * reclen);
        return 0;
    }

    if (fseek(f, 0L, SEEK_SET))
        return 0;

    if (fread(buffer, reclen, n, f)!=n)
        return 0;

    rl = reclen;

    qsort(buffer, n, reclen, ulcomp);

    if (fseek(f, 0L, SEEK_SET))
        return 0;

    if (fwrite(buffer, reclen, n, f)!=n)
        return 0;

    free(buffer);

    return 1;
}


