/* 
deneb@penguin.it
 */
#include "mcvs.h"
#include "xmalloc.h"
#include "strlcpy.h"
#include "astrcat.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>
int valid_program (const char *program, struct stat *prog);
int s_cmp(struct stat dev1, struct stat dev2);

int valid_program (const char *program, struct stat *prog)
{
/* the stat struct is here in order to avoid TOCTOU attacks 
  the before use the program you should open the file
  and fstat again.
*/
  int myprog = 1;
  if (lstat (program, prog) < 0)
    return -1;
/* The program must be a regular file */
  if (!S_ISREG (prog->st_mode))
    return -2;
  myprog = (S_ISUID | S_ISGID) & prog->st_mode;
/* The program cannot be suid or gid root */
  if ((myprog) && ((prog->st_uid == 0) || (prog->st_gid == 0)))
    return -3;

  myprog = (S_IXUSR | S_IXGRP | S_IXOTH) & prog->st_mode;
/* The program must be executable */
  if (!myprog)
    return -4;
  return 0;
}

int s_cmp(struct stat dev1, struct stat dev2)
/* stream compare: compare a first stat with a second stat */
{
unsigned int cmp = 0;
cmp = (dev1.st_mode == dev2.st_mode) && (dev1.st_ino == dev2.st_ino) && (dev1.st_dev == dev2.st_dev);
return cmp; 
}

