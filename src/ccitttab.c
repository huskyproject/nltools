/* This program can be used to build ccitttab.h */

#include <stdio.h>

unsigned short CalculateCCITT (unsigned int);

unsigned short table [256];

void main (void)
{
    unsigned int i;

    for (i = 0; i < 256; i++)
        table [i] = CalculateCCITT(i);

    printf ("/* CCITT Lookup Table */\n");
    printf ("unsigned short ccitt_table[256] =\n{");

    for (i=0; i<256; i++)
    {
        if ((i%8) == 0)
        {
            printf ("\n  /* %3u */  ", i);
        }

        printf ("0x%04X", table[i]);

        if (i != 255)
        {
            printf (", ");
        }
    }
        printf ("\n};\n");
}

unsigned short CalculateCCITT (unsigned int index)
{
    unsigned short a, i;
    a = 0;

    index <<= 8;

    for (i=8; i>0; i--)
    {
        if (( index ^ a ) & 0x8000 )
            a = (a << 1) ^ 0x1021;
        else
            a <<= 1;

        index <<= 1;
    }

    return a;
}
            
            
