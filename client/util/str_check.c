#include <string.h>
/* Check if  a string has bad characters
  char *to_check <- string to check
  const char *check <- string of all good characters.
  Thanks to of@securityfocus.com.
  */
int
str_check (char *to_check, const char *check)
{
  char *cp;
  int bad = 0;
  for (cp = to_check; *(cp += strspn (cp, check));)
    bad = -1;
  return bad;
}

