#ifndef __NLSTRING_H
#define __NLSTRING_H

#ifdef __EMX__
#define ncasecmp strnicmp
#elif defined(__linux__) || defined(__FreeBSD__)
#define ncasecmp strncasecmp
#else
int ncasecmp(const char *s, const char *t, size_t x);
#endif

#endif
