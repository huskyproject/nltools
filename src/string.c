#include <ctype.h>
#include <stddef.h>

#if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__EMX__)

int ncasecmp( const char *s, const char *t, size_t x )
{
  long n;

  n = ( long ) x;

  while( n-- && tolower( *s ) == tolower( *t ) )
  {
    if( *s == '\0' )
    {
      /* equal */
      return 0;
    }
    s++;
    t++;
  }

  if( n < 0 )
  {
    /* maximum hit, equal */
    return 0;
  }

  /* fell through, not equal */
  if( tolower( *s ) > tolower( *t ) )
  {
    return 1;
  }
  else
  {
    return -1;
  }
}


int casecmp( const char *s, const char *t )
{
  while( tolower( *s ) == tolower( *t ) )
  {
    if( *s == '\0' )
    {
      /* equal */
      return 0;
    }
    s++;
    t++;
  }

  /* fell through, not equal */
  if( tolower( *s ) > tolower( *t ) )
  {
    return 1;
  }
  else
  {
    return -1;
  }
}

#endif
