/* 
deneb@penguin.it
 */
#define MAX_ENV_VAR 1024
#define OK_PATHNAME "QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwertyuiopasdfghjklzxcvbnm_.-/"
#include "mcvs.h"
#include "xmalloc.h"
#include "strlcpy.h"
#include "astrcat.h"
#include "str_check.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>

/* ripped from a post to security audit ml 
by "Sean Hunter" <sean@uncarved.com>
*/
#define NELEM(x) (sizeof(x) / sizeof(x[0]))
#define SAFE_KEYS_MAX 1024

const char* SET_PATH = "/bin:/sbin:/usr/bin:/usr/sbin";

static const char* whitelist[] = {
    "HOME",
    "DISPLAY",
    "TMPDIR",
    "USER",
    "ROOT",
    "SHELL"
};


static const char* blacklist[] = {
    "IFS",
    "CDPATH",
    "ENV",
    "BASH_ENV"
};


const char* safe[SAFE_KEYS_MAX + 1];
size_t n_safe = 0;

char *
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
  if (str_check (sanitized, OK_PATHNAME) < 0)
    {
      xfree (sanitized);
      return NULL;
    }
  return sanitized;

}

char *
fetch_tmpdir (struct stat *tmp)
{
  char *env_var;
  char *cp,*ok;
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
  if (stat (env_var, tmp) < 0)	/*I'll recheck again before use */
    {
      xfree (ok);
      xfree (env_var);
    }
  else if (S_ISDIR (tmp->st_mode))
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


int add_safe_key(const char* key) {
    if (n_safe < SAFE_KEYS_MAX) {
        safe[n_safe++] = key;
	return 0;
    }
return -1;
}


void add_questionable_key(const char* key) {
    int found, i, len, n_unsafe;

    found = 0;

    n_unsafe = NELEM(blacklist);
    for(i = 0; !found && i < n_unsafe; i++) {
        len = strlen(blacklist[i]);
        if (memcmp(blacklist[i], key, len) == 0 &&
            key[len] == '\0') {
            found = 1;
        }
    }
    if (found) {
        fprintf(stderr,
                "safe_env: Environment variable %s is known to be unsafe.\n"
                "Ignoring request to add.\n"
                "If you have to have this env var, use '-S' to add it\n",
                key);
    }
    else {
        add_safe_key(key);
    }
}


void init_safe_keys() {
    int i;

    for (i = 0; i < SAFE_KEYS_MAX; i++) {
        safe[i] = NULL;
    }
}


void add_builtin_keys() {
    int i;
    size_t n_builtins = NELEM(whitelist);

    for (i = 0; i < n_builtins; i++) {
        add_safe_key(whitelist[i]);
    }
}


int is_safe(const char* env_string) {
    int found, i, safe_len;

    found = 0;

    for(i = 0; !found && i < n_safe; i++) {
        safe_len = strlen(safe[i]);
        if (memcmp(safe[i], env_string, safe_len) == 0 &&
            env_string[safe_len] == '=') {
            found = 1;
        }
    }

    return(found);
}

/*
void zap_env(char** environ) {
    size_t i, len, curr_env_var;
    int found;
    char* buffer;
        
    curr_env_var = 0;
        
    for(i = 0; environ[curr_env_var] != NULL; i++) {
        found = is_safe(environ[curr_env_var]);
        if (!found) {
            for(len = 0;
                environ[curr_env_var][len] != '=' && environ[curr_env_var][len]
 != '\0';
                len++) {
                ;
            }
            buffer = (char *)malloc(len+1);
            if (!buffer) {
                die("safe_env: Unable to allocate memory for copy\n\n");
            }
            memcpy(buffer, environ[curr_env_var], len);
            buffer[len] = '\0';
            unsetenv(buffer);
            xfree(buffer);
        }
        else {
            curr_env_var++;
        }
    }
}
*/
/*
int main(int argc, char** argv, char** environ) {
    int replacepath = 0;
    int use_builtins = 1;
    int opt;

    init_safe_keys();

    while ((opt = getopt(argc,argv,"+s:S:pPbB")) != -1) {
        switch(opt) {
        case 'p': replacepath = 0; add_safe_key("PATH"); break;
        case 'P': replacepath = 1; add_safe_key("PATH"); break;
        case 'b': use_builtins = 1; break;
        case 'B': use_builtins = 0; break;
        case 's': add_questionable_key(optarg); break;
        case 'S': add_safe_key(optarg); break;
        default: usage();
        }
    }

    if (use_builtins) {
        add_builtin_keys();
    }

    if (replacepath) {
        setenv("PATH", SET_PATH, 1);
    }

    zap_env(environ);


    execve(argv[optind], argv + optind, environ);

    fprintf(stderr, "safe_env: Unable to exec '%s'.\n\n"
                    "Remember that PATH may not be set as usual.  Try an absolu
te path\n", argv[optind]);
    return 100;
}
*/
