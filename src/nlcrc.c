/* Tool to check the CRC value in a Fidonet Nodelist
   Written 1999 by Tobias Ernst and released do the Public Domain.
   References: FTS-0005
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "crc16.h"
#include "version.h"

enum {SCANCOLON, SCANDIGIT, CRCVAL, AFTERCRCVAL, FINISH};

unsigned short analyze_first_line(FILE *f)
{
    char c=0;
    int state = SCANCOLON;
    unsigned short crc = 0;
    
    while (state != FINISH)
    {
        fread(&c, 1, 1, f);
        if ( c == '\n' )
        {
            if (state < CRCVAL)
            {
                fprintf (stderr, "Warning: File does not contain checksum.\n");
                exit(4);
            }
            else
            {
                state = FINISH;
            }
        }
        else if (isdigit(c))
        {
            switch (state)
            {
            case SCANDIGIT:
                crc = c - '0';
                state = CRCVAL;
                break;
            case CRCVAL:
                crc = crc * 10U + (c - '0');
                break;
            default:;
            }
        }
        else if (c == ':')
        {
            switch (state)
            {
            case SCANCOLON:
                state = SCANDIGIT;
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
            default:;
            }
        }
    }
    return crc;
}

#define BUFSZ 32768

unsigned short analyze_rest(FILE *f)
{
    static unsigned char buffer[BUFSZ+1];
    size_t l;
    unsigned short crc;
    unsigned char *p = NULL;

    crc16_init(&crc);

    errno = 0;         
    while (((l = fread(buffer, 1, BUFSZ, f)) > 0) && p == NULL)
    {
        buffer[l] = '\0';
        p = (unsigned char *) strchr((char *) buffer, 0x1A);

        if (p != NULL)
        {
            l = p - buffer; 
        }

        crc16_process(&crc, buffer, l);
    }

    crc16_finalize(&crc);

    return crc;
}   


int main(int argc, char **argv)
{
    FILE *f;
    unsigned short should, is;
    char *versionStr = NULL;

    versionStr = GenVersionStr( "nlcrc", VER_MAJOR, VER_MINOR, VER_PATCH,
                               VER_BRANCH, cvs_date );

    fprintf (stderr, "%s\n\n", versionStr);

    if (argc != 2)
    {
        fprintf (stderr, "Usage: nlcrc <nodelist>\n");
        return 8;
    }

    if ((f = fopen(argv[1], "rb")) == NULL)
    {
        fprintf (stderr, "Cannot open %s\n", argv[1]);
        return 8;
    }

    should = analyze_first_line(f);
    is     = analyze_rest(f);

    fclose(f);

    if (should != is)
    {
        fprintf (stderr, "Nodelist %s fails CRC check (%u != %u)\n",
                 argv[1], (unsigned)should, (unsigned)is);
        return 16;
    }

    fprintf (stderr, "Nodelist %s the CRC was OK\n", argv[1]);
    return 0;
}
