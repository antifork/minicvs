/* 
Minicvs - imported from GNU CVS 
audit status: clean  - deneb@unixwave.org */
/* valloc -- return memory aligned to the page size.  */


#include "valloc.h"


void *
valloc (size_t byte_siz)
{
  size_t pagesize;
  char *ret;

  pagesize = getpagesize ();
  ret = (char *) xmalloc (byte_siz + pagesize - 1);
  if (ret)
    ret = (char *) ((long) (ret + pagesize - 1) &~ (pagesize - 1));
  return ret;
}
