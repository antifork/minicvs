#include <string.h>

char *strip_path(const char *path)
{
    char   *last;

if ((last = strrchr(path, '/')) == 0)
    last = (char *) path;
else
last +=1;
    return (last);
}

