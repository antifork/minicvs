/* 
Minicvs - imported from GNU CVS 
audit status: clean  - deneb@unixwave.org 

valloc -- return memory aligned to the page size.  */
#include <stdio.h>
#include "xmalloc.h"
#include <unistd.h>
void *valloc (size_t byte_siz);