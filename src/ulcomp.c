/* Fido Userlist Compiler
   Written 1999 by Tobias Ernst and released to the Public Domain */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

enum { F_NODELIST, F_POINTS24 };
enum { T_ZONE, T_REGION, T_HOST, T_HUB, T_NODE };


/* A node or point has been parsed in a node list. Write it in Fidouser.Lst
   format to the output file. The output file is not yet sorted. The
   "username" parameter should contain underscores instead of spaces. */

#define ENTRYLENGTH 63 /* without trailing \r\n! */

static int parsednode(FILE *fout, unsigned zone, unsigned net, unsigned node,
                      unsigned point, char *username)
{
    char rev_username[ENTRYLENGTH+3], *cp, *cp2;
    char nodenr[24];
    
    cp = strrchr(username, '_');
    
    if (cp == NULL)
    {
        strncpy(rev_username, username, ENTRYLENGTH);
        rev_username[ENTRYLENGTH]='\0';
    }
    else
    {
        strncpy(rev_username, cp + 1, ENTRYLENGTH - 2);
        strcat(rev_username, ", ");
        *cp = 0;

        for (cp = username, cp2 = rev_username + strlen(rev_username);
             *cp && cp2 - rev_username < ENTRYLENGTH ; cp++, cp2++)
        {
            if (*cp == '_')
            {
                *cp2 = ' ';
            }
            else
            {
                *cp2 = *cp;
            }
        }
        *cp2 = '\0';
    }

    sprintf(nodenr, "%u:%u/%u.%u", zone%65536, net%65536, node%65536,
            point%65536);

    if (strlen(rev_username) > ENTRYLENGTH - strlen(nodenr) - 1)
    {
        rev_username[ENTRYLENGTH - strlen(nodenr) - 1] = '\0';
    }
    else
    {
        for (cp = rev_username + strlen(rev_username);
             cp - rev_username < ENTRYLENGTH - strlen(nodenr) - 1;
             cp++)
        {
            *cp = ' ';
        }
    }
    
    strcat(rev_username, " ");
    strcat(rev_username, nodenr);
    strcat(rev_username, "\r\n");

    return (fwrite(rev_username, ENTRYLENGTH+2, 1, fout) == 1);

    return 1;
}

/* Process a line from a nodelist file */

static int ulc_line(FILE *fin, FILE *fout, unsigned format,
                    unsigned *zone, unsigned *net)
{
    char linebuf[128], *ptr, *sptr;
    size_t l; int incomplete;
    int k, rv=1;
    char *username;
    unsigned node;
    int type;

    if (fgets(linebuf, sizeof(linebuf), fin) != NULL)
    {
        l = strlen(linebuf);
        incomplete = (l && linebuf[l-1] != '\n');
 
        /* analyze the string */

        switch (*linebuf)
        {
        case ';':
            ptr = NULL;
            break; /* ignore the comment */

        case ',':
            ptr = "";
            sptr = linebuf+1;
            break;

        default:
            ptr = strtok(linebuf, ",\r\n");
            sptr = NULL;
            break;

        }

        for (k = 0; ptr != NULL && k <= 4; k++)
        {

            switch (k)
            {
            case 0:
                if (!strcmp(ptr, "Zone"))
                {
                    type = T_ZONE;
                }
                else if (!strcmp(ptr, "Region"))
                {
                    type = T_REGION;
                }
                else if (!strcmp(ptr, "Host"))
                {
                    type = T_HOST;
                }
                else if (!strcmp(ptr, "Hub"))
                {
                    type = T_HUB;
                }
                else /* Pvt, or empty */
                {
                    type = T_NODE;
                }
                break;

            case 1:
                switch (type)
                {
                case T_ZONE:
                    *zone = *net = atoi(ptr);
                    node = 0;
                    break;
                case T_REGION:
                case T_HOST:
                    *net = atoi(ptr);
                    node = 0;
                    break;
                case T_HUB:
                case T_NODE:
                    node = atoi(ptr);
                    break;
                }
                break;

            case 2: /* System Name */
                break;

            case 3: /* Country */
                break;

            case 4: /* Sysop Name */
                username = ptr;
                rv = parsednode(fout, *zone, *net, node, 0,  username);
                break;
            }


            ptr  = strtok(sptr, ",\r\n");
            sptr = NULL;
        }
            

        /* if the string is longer than 128 characters, ignore the rest */

        while (incomplete &&
               fgets(linebuf, sizeof(linebuf), fin) != NULL)
        {
            l = strlen(linebuf);
            incomplete = (l && linebuf[l-1] != '\n');
        }

        return rv;
    }
    if (feof(fin))
        return 2;  /* EOF is a normal situation */
    
    return 0;      /* everything else is an error */
}

int ul_compile (FILE *fin, FILE *fout, int type, int defzone)
{
    unsigned int zone, net, code;

    zone = net = defzone;

    while ((code = ulc_line(fin, fout, type, &zone, &net)) == 1);

    return code == 2;
}
    
