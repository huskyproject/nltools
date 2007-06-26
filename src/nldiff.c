/* Tool to apply a Nodediff upon a nodelist.
   Written 1999 by Tobias Ernst and released to the Public Domain.
   References: FTS-0005
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include <huskylib/huskylib.h>
#include "crc16.h"
#include "version.h"

/* It is no problem if a line grows larger than BUFSZ. This code is
   intelligent. :-)
 */
#define BUFSZ 128

unsigned short actualcrc;

char *versionStr;

/* A state machine to analyse the very first line of a nodelist for day
   number and an optional CRC value. */

enum {SCANDASH, SCANFDIGIT, DAYVAL, SCANCOLON, SCANNDIGIT, CRCVAL, AFTERCRCVAL, FINISH};
enum {COPY, ADD, DEL, END, ILL};

int  analyze_first_line(FILE *f, unsigned short *crcnum, int *has_crc,
                        unsigned short *daynum)
{
    char c=0;
    int state = SCANDASH;
    unsigned short crc, day, hcrc, result = 1;
    crc = day = hcrc = 0;

    while (state != FINISH)
    {
        if (fread(&c, 1, 1, f) != 1)
        {
            result = 0;
            state = FINISH;
            continue;
        }

        if ( c == '\n' )
        {
            hcrc = 1;
            if (state < CRCVAL)
            {
                hcrc = 0;
            }
            if (state < DAYVAL)
            {
                result = 0;     /* analysis failed */
            }
            state = FINISH;
        }
        else if (isdigit(c))
        {
            switch (state)
            {
            case SCANFDIGIT:
                day = c - '0';
                state = DAYVAL;
                break;
            case SCANNDIGIT:
                crc = c - '0';
                state = CRCVAL;
                break;
            case DAYVAL:
                day = day * 10U + (c - '0');
                break;
            case CRCVAL:
                crc = crc * 10U + (c - '0');
                break;
            default:;
            }
        }
        else if (c == '-')
        {
            switch (state)
            {
            case SCANDASH:
                state = SCANFDIGIT;
                break;
            default:;
            }
        }
        else if (c == ':')
        {
            switch (state)
            {
            case SCANCOLON:
                state = SCANNDIGIT;
                break;
            default:;
            }
        }
        else
        {
            switch(state)
            {
            case CRCVAL:
                state = AFTERCRCVAL;
                break;
            case DAYVAL:
                state = SCANCOLON;
                break;
            default:;
            }
        }
    }

    *crcnum = crc; *daynum = day; *has_crc = hcrc;
    return result;
}


/* Copy a line from in to out, no matter how long it is, and register it
   with the CRC checker. */

int passline (FILE *from, FILE *to)
{
    char buf1[BUFSZ];
    size_t l;

    do
    {
        if (fgets(buf1, BUFSZ, from) == NULL)
            return -1;
        if (fputs(buf1, to) == EOF)
            return -1;
        l = strlen(buf1);
        crc16_process(&actualcrc, (unsigned char *)buf1, l);
        if (!l)
            return -1;
    } while (buf1[l - 1] != '\n');
    return 0;
}

/* Skip a line. */

int skipline (FILE *from)
{
    char buf1[BUFSZ];
    size_t l;

    do
    {
        if (fgets(buf1, BUFSZ, from) == NULL)
            return -1;
        l = strlen(buf1);
        if (!l)
            return -1;
    } while (buf1[l - 1] != '\n');
    return 0;
}

/* Compare two lines. */

int compareline (FILE *from1, FILE *from2)
{
    char buf1[BUFSZ], buf2[BUFSZ];
    size_t l;

    do
    {
        if (fgets(buf1, BUFSZ, from1) == NULL)
            return -1;
        if (fgets(buf2, BUFSZ, from2) == NULL)
            return -1;
        l = strlen(buf1);
        if (!l)
            return -1;
        if (memcmp(buf1, buf2, l))
            return -2;
    } while (buf1[l - 1] != '\n');
    return 0;
}

/* Analyse a Nodediff command */


int readcommand(FILE *f, int *argument)
{
    char buffer[64];
    int cmd, arg;

    if (fgets(buffer, 64, f) == NULL)
        return END;

    if (*buffer == '\0' || *buffer == 0x1A)
        return END;

    switch (*buffer)
    {
    case 'A':
        cmd = ADD;
        break;
    case 'C':
        cmd = COPY;
        break;
    case 'D':
        cmd = DEL;
        break;
    default:
        return ILL;
    }

    arg = atoi(buffer+1);
    if (arg < 1)
        return ILL;

    *argument = arg;
    return cmd;
}

/* Construct the name of the new nodelist from the old name plus the
   extension of the nodediff file */

char *construct_new_filename(char *listname, char *diffname)
{
    int l, m;
    char *tempname;

    /* Sanity checks on the given filenames */
    if ((l = strlen(listname)) < 5)
    {
        fprintf (stderr, "%s is not a valid nodelist filename.\n", listname);
        return NULL;
    }
    if (listname[l-4] != '.' || !isdigit(listname[l-3]) ||
        !isdigit(listname[l-2]) || !isdigit(listname[l-1]))
    {
        fprintf (stderr, "%s is not a valid nodelist filename.\n", listname);
        return NULL;
    }
    if ((m = strlen(diffname)) < 5)
    {
        fprintf (stderr, "%s is not a valid nodediff filename.\n", listname);
        return NULL;
    }
    if (diffname[m-4] != '.' || !isdigit(diffname[m-3]) ||
        !isdigit(diffname[m-2]) || !isdigit(diffname[m-1]))
    {
        fprintf (stderr, "%s is not a valid nodediff filename.\n", listname);
        return NULL;
    }

    /* test for misuse like "nldiff -n nodelist.365 nodediff.365 */
    if (!strcmp(diffname+m-4, listname+l-4))
    {
        fprintf (stderr, "%s does not seem to apply to %s.\n",
                 diffname, listname);
        return NULL;
    }

    /* Construct filename of new nodelist */
    if ((tempname = malloc(strlen(listname) + 1)) == NULL)
    {
        fprintf (stderr, "Out of memory.\n");
        return NULL;
    }
    strcpy(tempname, listname);
    strcpy(tempname+l-4, diffname+m-4);

    return tempname;
}

/* Parse command line arguments */

enum { REMOVE_NODELIST = 1, REMOVE_NODEDIFF = 2 };

int parse_args (int argc, char **argv, char **listname, char **diffname,
                int *pflags)
{
    char *args[2];
    int i,j = 0;
    int flags;

    flags = 0;

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'n':
                flags = flags | REMOVE_NODELIST;
                break;
            case 'd':
                flags = flags | REMOVE_NODEDIFF;
                break;
            default:
                return 0;
            }
            if (argv[i][2])
            {
                return 0;
            }
            continue;
        }

        if (j<2)
        {
            args[j++] = argv[i];
        }
    }

    if (j == 2)
    {
        *listname = args[0];
        *diffname = args[1];
        *pflags = flags;
        return 1;
    }
    return 0;
}

/* Usage help */

void usage(void)
{
    versionStr = GenVersionStr( "nldiff", VER_MAJOR, VER_MINOR, VER_PATCH,
                               VER_BRANCH, cvs_date );
    fprintf (stderr, "%s\n", versionStr);
    fprintf (stderr, "Usage:\n"\
"   nldiff [-n] [-d] LISTNAME.DNR DIFFNAME.DNR\n\n"\
"   The -n option causes the original Nodelist to be deleted if the new\n"\
"   nodelist could be successfully generated.\n"\
"   The -d option causes the Nodediff file to be deleted if the new\n"\
"   nodelist could be successfully generated.\n\n"\
"Example:\n"\
"   nldiff -n NODELIST.253 NODEDIFF.260\n\n"\
"Remarks:\n"\
"   If you want a tool that automatically updates your nodelist\n"\
"   without you having to manually specify the day number file\n"\
"   extensions, use \"nlupdate\". It will call nldiff internally.\n");
}

/* Main program */

int main(int argc, char **argv)
{
    FILE *fn = NULL, *fd = NULL, *fo = NULL;
    char *listname = NULL, *diffname = NULL, *tempname = NULL;
    int cmd, arg, i, hascrc, rv, expnewday;
    unsigned short crc, newday;
    int crci = 0;
    int flags = 0;

    /* parse the command line */
    if (!parse_args(argc, argv, &listname, &diffname, &flags))
    {
        usage();
        return 8;
    }

    /* construct the filename of the new nodelist */
    if ((tempname = construct_new_filename(listname, diffname)) == NULL)
        return 8;
    expnewday=atoi(tempname + strlen(tempname) - 3);

    /* open the files */
    if ((fn = fopen(listname, "rb")) == NULL)
    {
        fprintf (stderr, "Cannot open %s.\n", listname); goto abnormal;
    }
    if ((fd = fopen(diffname, "rb")) == NULL)
    {
        fprintf (stderr, "Cannot open %s.\n", diffname); goto abnormal;
    }
    if ((fo = fopen(tempname, "w+b")) == NULL)
    {
        fprintf (stderr, "Cannot create %s.\n", tempname); goto abnormal;
    }

    /* Test if the diff's first line matches the list's first line */
    switch (compareline(fn, fd))
    {
    case -2:
        fprintf (stderr, "%s does not seem to apply to %s.\n",
                 diffname, listname);
        goto abnormal;
    case -1:
        fprintf (stderr, "I/O error.\n");
        goto abnormal;
    default:;                   /* match! */
    }
    if (fseek(fn, 0L, SEEK_SET))
    {
        fprintf (stderr, "File seek error.\n");
        goto abnormal;
    }

    /* Interpret the commands in the Diff file */
    do
    {
        cmd = readcommand(fd, &arg);
        switch(cmd)
        {
        case COPY:
            for (i = 0; i < arg; i++)
            {
                if (passline(fn, fo))
                {
                    fprintf (stderr, "Copy failed (%d / %d)\n", i, arg);
                    goto abnormal;
                }
                if (!crci)
                {
                    crc16_init(&actualcrc);
                    crci = 1;
                }
            }

            break;
        case ADD:
            for (i = 0; i < arg; i++)
            {
                if (passline(fd, fo))
                {
                    fprintf (stderr, "Add failed (%d / %d)\n", i, arg);
                    goto abnormal;
                }
                if (!crci)
                {
                    crc16_init(&actualcrc);
                    crci = 1;
                }
            }
            break;
        case DEL:
            for (i = 0; i < arg; i++)
                if (skipline(fn))
                {
                    fprintf (stderr, "Delete failed (%d / %d)\n", i, arg);
                    goto abnormal;
                }
            break;
        case END:
            break;
        default:
            fprintf (stderr, "Illegal command encountered.\n");
            goto abnormal;
        }
    } while (cmd != END);
    fputc(0x1A, fo);

    /* Now determine the actual day number and the expected CRC of the newly
       written nodelist file */
    if ((fseek(fo, 0L, SEEK_SET)) ||
        (!analyze_first_line(fo, &crc, &hascrc, &newday)))
    {
        fprintf (stderr, "New file is not a valid nodelist.\n");
        goto abnormal;
    }

    /* Compare the CRC values */
    crc16_finalize(&actualcrc);
    if (hascrc && actualcrc != crc)
    {
        fprintf (stderr, "New file does not pass CRC test.\n");
        goto abnormal;
    }
    if (newday != expnewday)
    {
        fprintf (stderr, "New day number and diff file name do not match.\n");
        goto abnormal;
    }

    fclose(fn); fclose(fd); fclose(fo); rv=0;

    /* Delete files that are not needed any more, if the user requested it. */
    if ((flags & REMOVE_NODELIST) && remove(listname))
    {
        fprintf (stderr, "Cannot remove old nodelist file %s.\n", listname);
        rv = 8;
    }
    if ((flags & REMOVE_NODEDIFF) && remove(diffname))
    {
        fprintf (stderr, "Cannot remove old nodediff file %s.\n", diffname);
        rv = 8;
    }

    return rv;

abnormal:
    fprintf (stderr, "Processing aborted.\n");
    if (fn != NULL)
        fclose(fn);
    if (fd != NULL)
        fclose(fd);
    if (fo != NULL)
        fclose(fo);
    if (tempname != NULL)
    {
        if (fo != NULL) remove(tempname);
        free(tempname);
    }
    return 8;
}



