/* 
jluc69@users.sourceforge.net
 */
#define MAX_ENV_VAR 1024
#define OK_CVSROOT "QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwertyuiopasdfghjklzxcvbnm-:./@"
#define OK_PATHNAME "QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwertyuiopasdfghjklzxcvbnm_.-/"
#include "mcvs.h"
#include "xmalloc.h"
#include "strlcpy.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>
/* Private functions */
static int checks (char *to_check, const char *check);
static char *env_pathfile (const char *path);
static int valid_program (const char *program);
static char *env_cvsroot (const char *mycvsroot);
static char *fetch_tmpdir (void);

void
purge_vars (char **editor, char **cvsroot, char **tmpdir)
{
  if (*editor != NULL)
    free (*editor);
  if (*cvsroot != NULL)
    free (*cvsroot);
  if (*tmpdir != NULL)
    free (*tmpdir);
}

int
lookup_env (char **editor, char **cvsroot, char **tempdir)
{
  char *cp;
  char *env_var;
  char *ok;
  *tempdir = fetch_tmpdir();
  if (*tempdir == NULL)
         return -1;
/*check editor */
  if ((cp = getenv (EDITOR_ENV)) != NULL)
    env_var = cp;
  else
    env_var = EDITOR_DFLT;

  if ((ok = env_pathfile (env_var)) == NULL)
    {
      xfree (ok);
      return -2;
    }

  if (valid_program (ok) < 0)
    {
      xfree (ok);
      return -2;
    }
  *editor = ok;

  if ((cp = getenv (CVSROOT_ENV)) != NULL)
    env_var = cp;
  else
    env_var = CVSROOT_DFLT;
  if ((ok = env_cvsroot (env_var)) == NULL)
    {
      xfree (ok);
      return -1;
    }
  *cvsroot = ok;
  return 0;
}

void
error_env (int outcode)
{
  switch (outcode)
    {
    case -1:
      {
	fprintf (stderr, "Invalid cvsroot in enviroment variable\n");
	exit (EXIT_FAILURE);
      }
      break;
    case -2:
      {
	fprintf (stderr, "Invalid editor in enviroment variable\n");
	exit (EXIT_FAILURE);
      }
      break;
    case -3:
      {
	fprintf (stderr, "Invalid tempdir in enviroment variable\n");
	exit (EXIT_FAILURE);
      }
      break;
    }
}


static char *
env_pathfile (const char *path)
{
  char *sanitized;
  size_t env_sz = strlen (path) + 1;

  if (path == NULL)
    return NULL;
  if (strlen (path) > PATH_MAX)
    return NULL;

  sanitized = xmalloc (env_sz);
  if (strlcpy (sanitized, path, env_sz) < 0)
    {
      xfree (sanitized);
      return NULL;
    }
/* here the check */
  if (checks (sanitized, OK_PATHNAME) < 0)
    {
      xfree (sanitized);
      return NULL;
    }
  return sanitized;

}
static char *
env_cvsroot (const char *mycvsroot)
{
  char *sanitized;
  size_t env_sz = strlen (mycvsroot) + 1;

  if (mycvsroot == NULL)
    return NULL;
  if (strlen (mycvsroot) > MAX_ENV_VAR)
    return NULL;

  sanitized = xmalloc (env_sz);
  if (strlcpy (sanitized, mycvsroot, env_sz) < 0)
    {
      xfree (sanitized);
      return NULL;
    }
/* here the check */
  if (checks (sanitized, OK_CVSROOT) < 0)
    {
      xfree (sanitized);
      return NULL;
    }
  return sanitized;
}

static int
checks (char *to_check, const char *check)
{
  char *cp;
  int bad = 0;
  for (cp = to_check; *(cp += strspn (cp, check));)
    bad = -1;
  return bad;
}

static int
valid_program (const char *program)
{
  struct stat prog;
  int myprog = 1;
  if (lstat (program, &prog) < 0)
    return -1;
/* The program must be a regular file */
  if (!S_ISREG (prog.st_mode))
    return -2;
  myprog = (S_ISUID | S_ISGID) & prog.st_mode;
/* The program cannot be suid or gid root */
  if ((myprog) && ((prog.st_uid == 0) || (prog.st_gid == 0)))
    return -3;

  myprog = (S_IXUSR | S_IXGRP | S_IXOTH) & prog.st_mode;
/* The program must be executable */
  if (!myprog)
    return -4;
  return 0;
}

static char *
fetch_tmpdir (void)
{
  char *env_var;
  char *cp;
  struct stat buf;
/* check tempdir order:
1) $HOME/tmp
2) TMPDIR_ENV
3) TMPDIR_DFLT
*/
  if ((cp = getenv ("HOME")) != NULL)

    env_var = cp;
  if ((ok = env_pathfile (env_var)) == NULL)
    {
      xfree (ok);
      return NULL;
    }
  if ((env_var = astrcat (ok, "/tmp")) == NULL)	/* $HOME/tmp */
    {
      xfree (ok);
      return NULL;
    }
  if (stat (env_var, &buf) < 0)	/*I'll recheck again before use */
    {
      xfree (ok);
      xfree (env_var);
    }
  else if (S_ISDIR (buf.st_mode))
    {
      xfree (ok);
      return env_var;
    }
 else
    {
      xfree (ok);
      xfree (env_var);
    }

  if ((cp = getenv (TMPDIR_ENV)) != NULL)
    env_var = cp;
  else
    env_var = TMPDIR_DFLT;
  if ((ok = env_pathfile (env_var)) == NULL)
    {
      xfree (ok);
      return NULL;
    }
  return ok;
}
