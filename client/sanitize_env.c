#define MAX_ENV_VAR 1024
#define OK_CVSROOT "QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwertyuiopasdfghjklzxcvbnm:./@";
#include "xmalloc.h"
#include "sstrcpy.h"

char *
env_cvsroot (const char *mycvsroot)
{
  char *sanitized;
  size_t env_sz = strlen (mycvsroot) + 1;
  size_t pattern_sz = strlen (pattern);
  if (mycvsroot == NULL)
    return NULL;
  if (strlen (mycvsroot) > MAX_ENV_VAR)
    return NULL;

  sanitized = xmalloc (env_sz);
  if (sstrcpy (sanitized, mycvsroot, env_sz) < 0)
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
