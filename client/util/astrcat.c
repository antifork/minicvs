#include <string.h>
#include <stddef.h>
#include "sstrcpy.h"
size_t
sstrcpy (char *dst, const char *src, size_t dest_siz)
{
  if (dest_siz < 1)
    return -1;

  if (dst == NULL)
    return -1;
  if (src == NULL)
    return -1;
   
  memset (dst, 0, dest_siz);
  strncpy (dst, src, dest_siz - 1);
  return strlen (dst);
}
size_t sstrcat(char *dest, const char *src, size_t dest_siz)
{

}