/* 
jluc69@users.sourceforge.net
 */
#define MAX_ENV_VAR 1024
#define OK_CVSROOT "QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwertyuiopasdfghjklzxcvbnm-:./@"
#define OK_PATHNAME "QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwertyuiopasdfghjklzxcvbnm_.-/"
#include "mcvs.h"
#include "xmalloc.h"
#include "strlcpy.h"
#include "astrcat.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>
/* Private functions */
static char *env_cvsroot (const char *mycvsroot);

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
valid_program (const char *program, struct stat *prog)
{
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

static char *
fetch_tmpdir ()
{
  char *env_var;
  char *cp,*ok;
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


void die(char* msg) {
    fprintf(stderr, msg);
    exit(100);
}



void add_safe_key(const char* key) {
    if (n_safe < SAFE_KEYS_MAX) {
        safe[n_safe++] = key;
    }
    else {
        die("safe_env: Too many environment variables added to the safe list.\n
\n");
    }
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
            free(buffer);
        }
        else {
            curr_env_var++;
        }
    }
}

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
