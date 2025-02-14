#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

/* Utilities **********************************/
#define _PERRET_F(c,f,r) if ((c)) {\
	perror((f));		  \
	return (r);		  \
 }
#define PERRET(c,r) _PERRET_F(c,__func__,r)
#define _EOFCHK_F(s,f,p) if ((p) >= s) {	\
	printf("%s: unexpected EOF\n", (f));	\
	exit(1);				\
 }
#define EOFCHK(s,p) _EOFCHK_F(s,__func__,p)
// Allocated 1 new element in array, returns new element (assignable)
#define ALLOCNEW(p,t,s) p = realloc(p, (++s)*sizeof(t)); p[s-1]

char *readfile(const char *filename, size_t *sz);

#endif
