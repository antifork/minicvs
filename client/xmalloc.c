/* 

MiniCVS - imported from OpenSSH code.
audit status: clean - deneb@unixwave.org
changes: 
- added fatal function. In the previous code was
  implemented in a different way.
  fatal is an internal function.
*/



/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Versions of malloc and friends that check their results, and never return
 * failure (they call fatal if they encounter an error).
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#include "xmalloc.h"

static void fatal(const char *fmt, ...);


static void
fatal(const char *fmt, ...)
{
  char buf[256];
  char *p;
  va_list ap;
 
  memset (buf, 0, sizeof (buf) - 1);
  va_start (ap, fmt);
  vsnprintf (buf, sizeof (buf), fmt, ap);
  if ((p = strrchr (buf, '\r')) != NULL)
    *p = '\0';
  if ((p = strrchr (buf, '\n')) != NULL)
    *p = '\0';
  fprintf(stderr,"%s\n",buf);
  va_end (ap);
exit(1);
}



void *
xmalloc(size_t size)
{
	void *ptr;

	if (size == 0)
		fatal("xmalloc: zero size");
	ptr = malloc(size);
	if (ptr == NULL)
		fatal("xmalloc: out of memory (allocating %lu bytes)", (u_long) size);
	return ptr;
}

void *
xrealloc(void *ptr, size_t new_size)
{
	void *new_ptr;

	if (new_size == 0)
		fatal("xrealloc: zero size");
	if (ptr == NULL)
		new_ptr = malloc(new_size);
	else
		new_ptr = realloc(ptr, new_size);
	if (new_ptr == NULL)
		fatal("xrealloc: out of memory (new_size %lu bytes)", (u_long) new_size);
	return new_ptr;
}

void
xfree(void *ptr)
{
	if (ptr == NULL)
		fatal("xfree: NULL pointer given as argument");
	free(ptr);
}

char *
xstrdup(const char *str)
{
	size_t len;
	char *cp;

	len = strlen(str) + 1;
	cp = xmalloc(len);
	strlcpy(cp, str, len);
	return cp;
}