/*
 *    Copyright (c) 1992, Brian Berliner and Jeff Polk
 *    Copyright (c) 1989-1992, Brian Berliner
 *
 *    You may distribute under the terms of the GNU General Public License
 *    as specified in the README file that comes with the CVS source distribution.
 *
 * This is the main C driver for the CVS system.
 *
 * Credit to Dick Grune, Vrije Universiteit, Amsterdam, for writing
 * the shell-script CVS system that this is based on.
 *
 *
 * Still a bit mess - cygnus@unixwave.org
 * 25-04-2002 * Just started to review
 */

#include <assert.h>
#include "mcvs.h"
#include "xmalloc.h"
#include "env_vars.h"
#include "strip_path.h"
#include <time.h>

char *program_name;
char *program_path;
char *command_name;

/* I'd dynamically allocate this, but it seems like gethostname
   requires a fixed size array.  If I'm remembering the RFCs right,
   256 should be enough.  */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  256
#endif

char hostname[MAXHOSTNAMELEN];

int use_editor = 1;
int use_cvsrc = 1;
int cvswrite = !CVSREAD_DFLT;
int really_quiet = 0;
int quiet = 0;
int trace = 0;
int noexec = 0;
int logoff = 0;

/* Set if we should be writing CVSADM directories at top level.  At
   least for now we'll make the default be off (the CVS 1.9, not CVS
   1.9.2, behavior). */
int top_level_admin = 0;

mode_t cvsumask = UMASK_DFLT;

char *CurDir;

/*
 * 
 */
char *Tempdir = NULL;
char *Editor = NULL;
char *Cvsroot = NULL;

/* When our working directory contains subdirectories with different
   values in CVS/Root files, we maintain a list of them.  */
List *root_directories = NULL;

/* We step through the above values.  This variable is set to reflect
   the currently active value. */
char *current_root = NULL;


static int set_root_directory (Node *p, void *ignored);




int
main (int argc, char **argv)
{
    
    
    int env_return;
    
    tzset ();

    /*
     * Just save the last component of the path for error messages
     */
    program_path = xstrdup (argv[0]);
    program_name = strip_path(argv[0]);

        
    if ( ( env_return = lookup_env(&Editor,&Cvsroot,&Tempdir) ) <0 )
	{
	purge_vars(&Editor,&Cvsroot,&Tempdir);
	error_env(env_return);
	}

xfree(program_path);
xfree(program_name);
exit(EXIT_SUCCESS);
    /* Look up the command name. */

}


static int
set_root_directory (Node *p, void *ignored)
{
    if (current_root == NULL && p->data == NULL)
    {
	current_root = p->key;
	return 1;
    }
    return 0;
}
