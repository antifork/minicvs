#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
char *
astrcat (char *dst, const char *src)
{
char *ret;
register size_t len = strlen(dst)+strlen(src)+1;
 
  if (dst == NULL)
    return NULL;
  if (src == NULL)
    return NULL;
  if ((ret = malloc(len))==NULL)
         return NULL;
  memset (ret, 0, len);
  snprintf(ret,len,"%s%s",dst,src); 
  return ret;
}
