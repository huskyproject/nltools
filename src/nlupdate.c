#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#ifdef UNIX
#include <sys/stat.h> /* S_I... constants */
#endif

#include <smapi/compiler.h>
#include <fidoconf/fidoconf.h>
#include <fidoconf/adcase.h>
#include <smapi/progprot.h>
#include <fidoconf/common.h>
#if !(defined(_MSC_VER) && (_MSC_VER >= 1200))
#include <fidoconf/dirlayer.h>
#endif
#include "nllog.h"
#include "nlstring.h"
#include "nlfind.h"
#include "nldate.h"
#include "julian.h"
#include "ulc.h" /* for REV */

/* store the nldiff command name */
static char *differ = NULL;

# if 0

/* This is not needed now because I imported fexist.c from smapi, and the
   routines in fexist.c are a little more intelligent than this one! */

static int nl_fexist(char *filename)
{
   FILE *f = fopen(filename, "rb");
   if (f == NULL)
      return 0;
   fclose(f);
   return 1;
}

#else
#define nl_fexist fexist
#endif
   

static char *mk_uncompressdir(char *nldir)
{
    char *upath;
    size_t l;
    FILE *f;

    /* construct a temporary directory name below nodelist dir */
    upath = malloc((l = strlen(nldir) + 12) + 1 + FILENAME_MAX);
    if (upath == NULL)
    {
        logentry(LOG_ERROR, "out of memory");
        return NULL;
    }
    strcpy(upath, nldir);
    strcat(upath, "nlupdate.tmp");
    adaptcase(upath);

    /* try to create it */
    mymkdir(upath);
    strcat(upath, "/");

    /* check if we can write to the directory */
    strcpy(upath + l + 1, "nlupdate.flg");
    f=fopen(upath, "w");
    if (f == NULL)
    {
        fprintf (stderr, "%s\n",upath);
        upath[l] = '\0';
        logentry(LOG_ERROR, "cannot create/access %s", upath);
        free(upath);
        return NULL;
    }
    fclose(f);
    remove(upath);
    upath[l+1] = '\0';

    return upath;
}

int destroy_uncompressdir(char *upath)
{
    size_t l = strlen(upath);
    struct dirent *dp;
    DIR *hdir;

    hdir = opendir(upath);
    if (hdir == NULL)
    {
        logentry(LOG_ERROR, "cannot read temporary directory %s", upath);
        free(upath);
        return 0;
    }

    while ((dp = readdir(hdir)) != NULL)
    {
        if ((strcmp(dp->d_name, ".")) &&
            (strcmp(dp->d_name, "..")))
        {
            strcpy(upath + l, dp->d_name);
            /* there is room for this because of the
               addition of FILENAME_MAX in mk_uncompressdir */

            if (remove(upath))
            {
                logentry(LOG_ERROR, "cannot erase %s", upath);
            }
        }
    }
    closedir(hdir);

    upath[l-1] = '\0';
    if (rmdir(upath))
    {
        logentry(LOG_ERROR, "cannot remove temporary directory %s", 
                 upath);
        free(upath);
        return 0;
    }

    free(upath);
    return 1;
}

static char *unpack_command(const char *template, const char *srcdir,
                     const char *srcfn, const char *dstdir)
{
    const char *start, *tmp, *add1, *add2;
    char *cmd;
    int anr = 0; int pnr = 0;

    cmd = malloc(strlen(srcdir) + strlen(srcfn) + strlen(dstdir) +
                 strlen(template) + 1);

    if (cmd == NULL)
    {
        logentry(LOG_ERROR, "out of memory");
        return NULL;
    }

    *cmd = '\0';  start = NULL;
    for (tmp = template; (start = strchr(tmp, '$')) != NULL; tmp = start + 2)
    {
        switch(*(start + 1))
        {
        case 'a':
            if (!anr)  /* only add 1 copy of $a to avoid buffer overrun */
            {
                add1 = srcdir; add2 = srcfn;
                anr = 1;
                break;
            }
            else
                anr = 2;
        case 'p':
            if (*(start + 1) == 'p')
            {
                if (!pnr)
                {
                    add1 = dstdir; add2 = NULL;
                    pnr = 1;
                    break;
                }
                else
                    pnr = 2;
            }
        default:
            strncat(cmd, tmp, (size_t) (start - tmp + 1));
            start--;
            continue;
        }
        strncat(cmd, tmp, (size_t) (start - tmp));
        strcat(cmd, add1);
        if (add2 != NULL)
            strcat(cmd, add2);
    }
    strcat(cmd, tmp);

    if (anr == 2 || pnr == 2)
    {
        logentry(LOG_ERROR, "error in unpacker definition (only one occurence of $a and only one occurence of $p are supported)");
        free(cmd);
        return NULL;
    }
    return cmd;
}

static int uncompress(s_fidoconfig *config, char *directory, char *filename,
                      char *tempdir)
{
    char *tmp = malloc(strlen(directory) + strlen(filename) + 1);
    char *cmd;
    FILE *f;
    int i, j, found, exitcode;

    /* construct the complete filename */
    if (tmp == NULL)
    {
        logentry(LOG_ERROR, "out of memory");
        return 0;
    }
    strcpy(tmp, directory);
    strcat(tmp, filename);

    /* open the packet so we can see what archiver has been used */
    f = fopen(tmp, "rb");
    if (f == NULL)
    {
        logentry(LOG_ERROR, "cannot access %s", tmp);
        return 0;
    }
        
    /* find what unpacker to use */
    for (i = 0, found = 0; (i < config->unpackCount) && !found; i++)
    {
        fseek(f, config->unpack[i].offset,
              config->unpack[i].offset >= 0 ? SEEK_SET : SEEK_END);
        if (ferror(f))
        { 
            logentry(LOG_ERROR, "seek error on %s", tmp);
            break;
        }
        for (found = 1, j = 0; j < config->unpack[i].codeSize; j++)
        {
            if ((getc(f) & config->unpack[i].mask[j]) !=
                config->unpack[i].matchCode[j])
                found = 0;
        }
        if (found) break;
    }
    
    fclose(f);

    if (!found)
    {
        logentry(LOG_ERROR,"did not find an appropriate unpacker for %s", tmp);
        free(tmp);
        return 0;
    }

    /* create the unpack command */
    cmd = unpack_command(config->unpack[i].call, directory, filename, tempdir);
    if (!cmd)
    {
        free(tmp);
        return 0;
    }

    /* execute the unpacker */
    logentry(LOG_MSG, "executing %s", cmd);
    exitcode=system(cmd);
    if (exitcode != 0)
    {
        logentry(LOG_MSG, "unpacker returned %d", exitcode);
        free(tmp);
        return 0;
    }
    adaptcase_refresh_dir(tempdir);
    
    free(tmp);
    return 1;
}

static char *get_uncompressed_filename(s_fidoconfig *config,
                                char *directory, char *filename,
                                char *tempdir, int expday)
{
    size_t l = strlen(filename);
    char *rv;

    assert (l >= 4); /* a match should never happen w/o ".???" at the end */

    if ( isdigit(filename[l-3]) && expday == atoi(filename + l-3))
    {
        /* the file is not compressed and the day number in the file
           name matches the expected day number */

        if ((rv = malloc(strlen(directory) + l + 1)) == NULL)
        {
            logentry(LOG_ERROR, "out of memory");
            return NULL;
        }
        strcpy(rv, directory);
        strcat(rv, filename);
        return rv;
    }
    else if (expday % 100 == atoi(filename + l-2))
    {
        /* the file is compressed and its name looks like it could match */

        if (!uncompress(config, directory, filename, tempdir))
        {
            return NULL;
        }

        if ((rv = malloc(strlen(tempdir) + l + 1)) == NULL)
        {
            logentry(LOG_ERROR, "out of memory");
            return NULL;
        }
        strcpy(rv, tempdir);
        strcat(rv, filename);
        sprintf(rv + strlen(tempdir) + l - 3, "%03d", expday % 1000);

        adaptcase(rv);
        if (!nl_fexist(rv))
        {
            free(rv);
            return NULL;
        }
        return rv;
    }
    else
    {
        /* no match */
        return NULL;
    }
}

static char *basedir(const char *stemname)
{
    char *rv; 
    size_t l;
    int j;

    l = strlen(stemname);

    if ((rv = malloc(l + 1)) == NULL)
    {
        return NULL;
    }
    strcpy(rv, stemname);

    for (j = l - 1; j >= 0 &&
         rv[j] != '/' && rv[j] != '\\' && rv[j] != ':'; j--)
    {
        rv[j] = '\0';
    }

    return rv;
}

static int call_differ(char *nodelist, char *nodediff)
{
    char *cmd;
    int rv;

    cmd = malloc(strlen(differ) + strlen(nodelist) + strlen(nodediff) + 6);
    if (cmd == NULL)
    {
        logentry(LOG_ERROR, "out of memory");
        return 0;
    }

    sprintf(cmd,"%s -n %s %s", differ, nodelist, nodediff);
    logentry(LOG_MSG, "executing %s", cmd);
    rv = system(cmd);
    if (rv != 0)
    {
        logentry(LOG_ERROR, "diff processor returned %d", rv);
        return 0;
    }
    return 1;
}


/* today is passed as argument, because we only want to call julian_today()
   once. Imagine nlupdate being started at 23:59:59 .... */

static int do_update(s_fidoconfig *config, int nl, char *rawnl, long today,
                     char *tmpdir)
{
    long julian, i;
    int ndnr;
    int checkdiff = 1;
    int j;
    int rv = 1;
    char  *diffbase = NULL, *fullbase = NULL;
    nlist *difflist = NULL, *fulllist = NULL;
    int hit;
    char *ufn;

    if ((julian = parse_nodelist_date(rawnl)) == -1)
        return 0;

    if (today - julian < 7)
    {
        logentry(LOG_MSG, "younger than 7 days, no update necessary");
        return 1;
    }

    /* build the list of candidate nodediff files */
    if (config->nodelists[nl].diffUpdateStem)
    {
        if ((diffbase = basedir(config->nodelists[nl].diffUpdateStem)) != NULL)
        {
            difflist = find_nodelistfiles(diffbase,
                                          config->nodelists[nl].diffUpdateStem 
                                          + strlen(diffbase), 1);
        }
        else
            difflist = NULL;
    }

    /* build the list of candidate full update files */
    if (config->nodelists[nl].fullUpdateStem)
    {
        if ((fullbase = basedir(config->nodelists[nl].fullUpdateStem)) != NULL)
        {
            fulllist =  find_nodelistfiles(fullbase,
                                           config->nodelists[nl].fullUpdateStem 
                                           + strlen(fullbase), 1);
        }
        else
            fullbase = NULL;
    }


    /* search for diffs or full updates */

    for (i = julian + 7; i <= today && rawnl; i += 7)
    {
        decode_julian_date(i, NULL, NULL, NULL, &ndnr);
        hit = 0;

        if (checkdiff && difflist != NULL)  /* search for diffs */
        {
            for (j = 0; j < difflist->n && !hit; j++)
            {
                ufn = get_uncompressed_filename(config, diffbase,
                                                difflist->matches[j],
                                                tmpdir, ndnr);
                if (ufn != NULL)
                {

                    /* nodediffs contain the header of the nodelist to which
                       they apply as the first line, so we need to compare the
                       parsed date with julian, not with i */
                    
                    if (julian == parse_nodelist_date(ufn))
                    {
                        logentry(LOG_MSG, "found diff update: %s", ufn);

                        if (call_differ(rawnl, ufn))
                        {
                            hit = 1;
                        }
                    }
                    else
                    {
                        logentry(LOG_MSG,
                                 "%s is not applicable to %s", rawnl);
                    }
                    free(ufn);
                }
            }
        }

        if (fulllist != NULL && !hit)       /* search for fulls */
        {
            for (j = 0; j < fulllist->n && !hit; j++)
            {
                ufn = get_uncompressed_filename(config, fullbase,
                                                fulllist->matches[j],
                                                tmpdir, ndnr);
                if (ufn != NULL)
                {
                    if (i == parse_nodelist_date(ufn))
                    {
                        char *newfn;

                        logentry(LOG_MSG, "found full update: %s", ufn);

                        newfn = malloc(strlen(config->nodelistDir) +
                                       strlen(config->nodelists[nl].
                                              nodelistName) + 5);
                        if (newfn == NULL)
                        {
                            logentry(LOG_ERROR, "out of memory");
                            rv = 0;
                            i = today; /* exit loop */
                        }
                        else
                        {
                            strcpy(newfn, config->nodelistDir);
                            strcat(newfn, config->nodelists[nl].nodelistName);
                            sprintf(newfn + strlen(newfn), ".%03d", ndnr);
                            
                            if (copy_file(ufn, newfn))
                            {
                                logentry(LOG_ERROR, "error copying %s to %s",
                                         ufn, newfn);
                                rv = 0;
                                i = today; /* exit loop */
                            }
                            else
                            {
                                logentry(LOG_MSG, "replacing %s with %s",
                                         rawnl, newfn);
                                hit = 1;
                                if (remove(rawnl))
                                {
                                    logentry(LOG_ERROR, "cannot remove %s",
                                             rawnl);
                                    rv = 0; /* but still proceed */
                                }
                            }
                            free(newfn);
                        }
                    }
                    else
                    {
                        logentry(LOG_MSG,
                                 "%s does not contain nodelist for day %03d",
                                 ndnr);
                    }
                    free(ufn);
                }
            }
        }

        if (hit)                            /* analyse the new nodelist */
        {
            free(rawnl);
            rawnl = findNodelist(config, nl);
            if (rawnl == NULL)
            {
                logentry(LOG_ERROR,
                         "instance of nodelist %s vanished during update!?",
                         config->nodelists[nl].nodelistName);
            }
            if ((i = parse_nodelist_date(rawnl)) == -1)
            {
                free(rawnl);
                rawnl = NULL;
            }
            if (i <= julian)
            {
                logentry(LOG_ERROR,
                         "nodelist %s still as old as before!?", rawnl);
                rv = 0;
                i = today; /* exit loop :-( */
            }
            else
            {
                julian = i;
            }
            checkdiff = 1;
        }
        else
        {
            checkdiff = 0;
        }
    }

    if (fullbase != NULL) free(fullbase);
    if (diffbase != NULL) free(diffbase);
    
    /* check if nodelist is extraordinarily old, just for information */
    if (today - julian > 14)
    {
        logentry(LOG_WARNING,
                 "%s is more than two weeks old, but still no way to update it",
                 rawnl);
    }

    free(rawnl);
    return rv;
}

static int process(s_fidoconfig *config)
{
    int i, rv=0;
    char *nodelist;
    char *tmpdir;
    long today = julian_today();

    if (config->nodelistDir == NULL)
    {
        logentry(LOG_ERROR,
                 "no nodelist directory configured in fidoconfig");
        return 8;
    }

    if (config->nodelistCount < 1 )
    {
        logentry(LOG_ERROR,
                 "no nodelist configured in fidoconfig");
        return 8;
    }

    if ((tmpdir = mk_uncompressdir(config->nodelistDir)) != NULL)
    {
        for (i = 0; i < config->nodelistCount; i++)
        {
            nodelist = findNodelist(config, i);
            
            if (nodelist == NULL)
            {
                logentry(LOG_WARNING, "no instance of nodelist %s found.\n",
                         config->nodelists[i].nodelistName);
                if (rv < 4) rv = 4;
            }
            else if (config->nodelists[i].diffUpdateStem == NULL &&
                     config->nodelists[i].fullUpdateStem == NULL)
            {
                logentry(LOG_WARNING, "don't know how to update nodelist %s",
                         config->nodelists[i].nodelistName);
                if (rv < 4) rv = 4;
                free(nodelist);
            }
            else
            {
                logentry(LOG_MSG, "trying to update %s", nodelist);
                
                if (!do_update(config, i, nodelist, today, tmpdir))
                {
                    if (rv < 8) rv = 8;
                }
            }
        }

        destroy_uncompressdir(tmpdir);
    }

    logentry(LOG_MSG, "done");
    return rv;
}

int main(int argc, char **argv)
{
    s_fidoconfig *config = readConfig(NULL);
    int rv;
    int l = 0;

    /* construct the name of the nldiff command */
    if (argc)
    {
        l = strlen(argv[0]);
        if (l)
        {
            for (l--; l >= 0 && argv[0][l] != '/' && argv[0][l] != '\\' &&
                 argv[0][l] != ':'; l--);

            l++;
        }
    }
    differ = malloc(l + 7);
    if (l) memcpy(differ, argv[0], l);
    strcpy(differ + l, "nldiff");


    /* run the main program */
    if (config != NULL)
    {
        loginit(config);
        logentry(LOG_MSG, "nlupdate - nodelist updater rev. %s", REV);

        rv=process(config);

        logdeinit();
        disposeConfig(config);
        free(differ);
        return rv;
        
    }
    else
    {
        fprintf (stderr, "Fatal: Cannot open fidoconfig.\n");
        free(differ);
        return 8;
    }
}
