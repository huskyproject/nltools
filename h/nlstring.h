#ifndef __NLSTRING_H
#define __NLSTRING_H

#ifdef __EMX__
#define ncasecmp strnicmp
#define casecmp stricmp
#elif defined(__linux__) || defined(__FreeBSD__)
#define ncasecmp strncasecmp
#define casecmp strcasecmp
#else
int ncasecmp(const char *s, const char *t, size_t x);
int casecmp(const char *s, const char *t);
#endif

#endif
