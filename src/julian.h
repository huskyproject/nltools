#ifndef JULIAN_H
#define JULIAN_H

/* ============================================================================
   Julian Date Routines
   Written 1999 by Tobias Ernst and release to the Public Domain.

   These routines calculate a Julian day number for a given date. Julian day
   numbers are a consecutive sequence of day numbers, with the origin at the
   January 1st, 4713 (B.C.) (Julian Day #1). Julian day numbers can be used to
   calculate date differences in days (just subtract the day numbers), to
   determine the day of week (julian day number modulo 7 yields 0 for Monday, 6
   for Sunday), etc.

   The calculcations in these routines are based on the Gregorian calendar
   being in effect since Oct. 15, 1582 (European convention). In any case,
   result for dates previous to September 14, 1752, will probably not be too
   much in coincidence with the local reality at that time.

   Conventions are: dd (day in month): 1 .. 31
                    mm (month): January = 1, December = 12
                    yy (year):  Positive four digit year number (e.g. 1999).

   The routines do not work year numbers < 1. There is no problem with big year
   numbers, i.E. the code is year 2000 safe (and it of course respects Feburary
   29, 2000).  
   ============================================================================
*/

long get_julian_date(int dd, int mm, int yy);
void decode_julian_date(long julian, int *dd, int *mm, int *yy, int *diny);

long julian_today(void);

#endif
