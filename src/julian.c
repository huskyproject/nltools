#include <stdlib.h>
#include <time.h>

#include "julian.h"

/* See julian.h for license and documentation */

#ifdef TEST
#include <stdio.h>
#endif

static int is_leap_year (int year)
{
    if (year % 4)
    {
        return 0;
    }
    if (year < 1582)
    {
        return 1;
    }
    if ((year % 100) || (!(year % 400)))
    {
        return 1;
    }
    return 0;
}

static long s_days_in_month[2][12] = 
{
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

long get_julian_date( int dd, int mm, int yy)
{
    long julian;

    /* Account all preceding years */
    julian = (yy - 1) * 365L;
    
    /* Account the leap years */
    julian += (yy - 1) / 4L;

    if (yy - 1 > 1582L)
    {

        /* Subtract all xx00 years */
        julian -= (yy - 1) / 100L;
        
        /* But all xx00 years that can be divided by 400 are leap years! */
        julian += (yy - 1) / 400L;

        /* Account for the centuries before 1582, which all were leap years */
        julian += 12L; /* 100, 200, 300, 500, 600, 700, 900,
                          1000, 1100, 1300, */
    }

    if (mm)
    {
        /* mm > 0: assume dd is day number in month */

        /* add the days of the current year */
        while (--mm)
        {
            julian += s_days_in_month[is_leap_year(yy)][mm-1];
        }
        
        /* days in current month */
        julian += dd;
    }
    else
    {
        /* mm == 0: assume dd is day number in year as per fts */
        julian += (dd - 1);
    }

    /* adjust for the cut from oct 4 to oct 15, 1582 */
    if (julian > 577737L)
    {
        julian -= 10L;
    }

    /* By now, we have days since Jan 1, 1 (AC). Now convert to Julian day
       numbers, which by convention start at Jan 1, 4713 (BC) */

    julian += (2299161L - 577738L);

    return julian;
}

void decode_julian_date(long julian, int *pdd, int *pmm, int *pyy,
                        int *pddiny)
{
    int yy, dd, mm; long days;
    int i;

    /* convert to days since B.C. */
    julian -= (2299161L - 577738L);
    
    /* undo the oct. 1582 gap */
    if (julian > 577737L)
    {
        julian += 10L;
    }

    yy = (int)(julian / 365L);
    days = (julian % 365L);

    /* account for the leap years */
    if (yy < 1700)
    {
        days -= (yy / 4);
    }
    else
    {
        days -= (yy / 4);
        days += (yy / 100);
        days -= (yy / 400);
        days -= 12;
    }

    /* if we did a backwards overrun, adjust properly */
    while (days <= 0)
    {
        if (is_leap_year(yy))
        {
            yy -= 1;
            days += 366;
        }
        else
        {
            yy -= 1;
            days += 365;
        }
    }

    dd = (int) days;
    yy++;

    /* store year, and days in year */
    if (pyy != NULL)
    {
        *pyy = yy;
    }
    if (pddiny != NULL)
    {
        *pddiny = dd;
    }

    /* now calculate the month number and decrease the day number
       accordingly */
    for (i = 1; i <= 12; i++)
    {
        mm = i;
        if (dd <= s_days_in_month[is_leap_year(yy)][i-1])
            break;
        else
            dd -= s_days_in_month[is_leap_year(yy)][i-1];
    }

    if (pdd != NULL)
    {
        *pdd = dd;
    }

    if (pmm != NULL)
    {
        *pmm  = mm;
    }
}

long julian_today(void)
{
    time_t t;
    struct tm *tm;

    time(&t);
    tm=localtime(&t);

    return get_julian_date(tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
}

#ifdef TEST
void test(int dd, int mm, int yy)
{
    int rdd, rmm, ryy, rdiny;
    long l;

    l = get_julian_date(dd, mm, yy);
    decode_julian_date(l, &rdd, &rmm, &ryy, &rdiny);
    printf ("%2d.%2d.%4d %8ld %2d.%2d.%4d. (%3d)\n",
            dd,mm,yy,l,rdd,rmm,ryy,rdiny);
}

int main(void)
{
    test (31, 12, 1899);
    test (1, 1, 1900);
    test (28, 2, 1900);
    test (1, 3, 1900);
    test (31, 12, 1999);
    test (1, 1, 2000);
    test (28, 2, 2000);
    test (1, 3, 2000);
    test (31, 12, 2099);
    test (1, 1, 2100);
    test (28, 2, 2100);
    test (1, 3, 2100);
    test (22, 6, 1976); /* tobi's birthday <g> */
    test (11, 10, 1999);
    test (1, 1, 1);
    test (4, 10, 1582);
    test (15, 10, 1582);
    
    return 0;
}
#endif



