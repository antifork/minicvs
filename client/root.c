/*
 * Copyright (c) 1992, Mark D. Baushke
 *
 * You may distribute under the terms of the GNU General Public License as
 * specified in the README file that comes with the CVS source distribution.
 * 
 * Name of Root
 * 
 * Determine the path to the CVSROOT and set "Root" accordingly.
 *
 * Modified by cygnus@unixwave.org
 */


/* Printable names for things in the CVSroot_method enum variable.
   Watch out if the enum is changed in cvs.h! */
#include "mcvs.h"

char *method_names[] = {
    "local", "server (rsh)", "pserver","tlsserver", "ext" 
};

#ifndef DEBUG

char *Name_Root (char *dir, char *update_dir)
{
    FILE *fpin;
    char *ret, *xupdate_dir;
    char *root = NULL;
    size_t root_allocated = 0;
    char *tmp;
    char *cvsadm;
    size_t cvsadm_sz;
    size_t tmp_sz;
    char *cp;

    if (update_dir && *update_dir)
	xupdate_dir = update_dir;
    else
	xupdate_dir = ".";

    if (dir != NULL)
    {	cvsadm_sz = strlen (dir) + sizeof (CVSADM) + 10;
	cvsadm = xmalloc (cvsadm_sz);
	(void) snprintf (cvsadm, cvsadm_sz,"%s/%s", dir, CVSADM);
	tmp = xmalloc (strlen (dir) + sizeof (CVSADM_ROOT) + 10);
	(void) snprintf (tmp, tmp_sz, "%s/%s", dir, CVSADM_ROOT);
    }
    else
    {
	cvsadm = xstrdup (CVSADM);
	tmp = xstrdup (CVSADM_ROOT);
    }

    /*
     * Do not bother looking for a readable file if there is no cvsadm
     * directory present.
     *
     * It is possible that not all repositories will have a CVS/Root
     * file. This is ok, but the user will need to specify -d
     * /path/name or have the environment variable CVSROOT set in
     * order to continue.  */
    if ((!isdir (cvsadm)) || (!isreadable (tmp)))
    {
	ret = NULL;
	goto out;
    }

    /*
     * The assumption here is that the CVS Root is always contained in the
     * first line of the "Root" file.
     */
    if ((fpin = fopen (tmp, "r"))==NULL)
	{
	xfree(cvsadm);
	xfree(tmp);
	return NULL;
	}

    if (getline (&root, &root_allocated, fpin) < 0)
    {
	/* FIXME: should be checking for end of file separately; errno
	   is not set in that case.  
	error (0, 0, "in directory %s:", xupdate_dir);
	error (0, errno, "cannot read %s", CVSADM_ROOT);
	error (0, 0, "please correct this problem");
	*/
	ret = NULL;
	goto out;
    }
    (void) fclose (fpin);
    if ((cp = strrchr (root, '\n')) != NULL)
	*cp = '\0';			/* strip the newline */

    /*
     * root now contains a candidate for CVSroot. It must be an
     * absolute pathname or specify a remote server.
     */

    if ((strchr (root, ':') == NULL) && !isabsolute (root))
    {
/*	error (0, 0, "in directory %s:", xupdate_dir);
	error (0, 0,
	       "ignoring %s because it does not contain an absolute pathname.",
	       CVSADM_ROOT);
	      */
	ret = NULL;
	goto out;
    }

    if ((strchr (root, ':') == NULL) && !isdir (root))
    {
    /*
	error (0, 0, "in directory %s:", xupdate_dir);
	error (0, 0,
	       "ignoring %s because it specifies a non-existent repository %s",
	       CVSADM_ROOT, root);
	       */
	ret = NULL;
	goto out;
    }

    /* allocate space to return and fill it in */
    strip_trailing_slashes (root);
    ret = xstrdup (root);
 out:
    xfree (cvsadm);
    xfree (tmp);
    if (root != NULL)
	xfree (root);
    return (ret);
}

/*
 * Write the CVS/Root file so that the environment variable CVSROOT
 * and/or the -d option to cvs will be validated or not necessary for
 * future work.
 */
int
Create_Root (char *dir, char *rootdir)
{
    FILE *fout;
    char *tmp;
    size_t tmp_sz;
    if (noexec)
	return -1;

    /* record the current cvs root */

    if (rootdir != NULL)
    {
        if (dir != NULL)
	{
	    tmp_sz = strlen (dir) + sizeof (CVSADM_ROOT) + 10;
	    tmp = xmalloc (tmp_sz);
	    snprintf (tmp, tmp_sz,"%s/%s", dir, CVSADM_ROOT);
	}
        else
	    tmp = xstrdup (CVSADM_ROOT);

        if ((fout = fopen (tmp, "w+"))==NULL)
	    {
	    xfree(tmp);
	    return -1;
	    }
        if (fprintf (fout, "%s\n", rootdir) < 0)
	    error (1, errno, "write to %s failed", tmp);
        if (fclose (fout) == EOF)
	    error (1, errno, "cannot close %s", tmp);
	xfree (tmp);
    }
}

#endif /* ! DEBUG */


/* The root_allow_* stuff maintains a list of legal CVSROOT
   directories.  Then we can check against them when a remote user
   hands us a CVSROOT directory.  */

static int root_allow_count;
static char **root_allow_vector;
static size_t root_allow_size;

void
root_allow_add (char *arg)
{
    char *p;
    size_t p_sz;
    
    if (root_allow_size <= root_allow_count)
    {
	if (root_allow_size == 0)
	{
	    root_allow_size = 1;
	    root_allow_vector =
		(char **) xmalloc (root_allow_size * sizeof (char *));
	}
	else
	{
	    root_allow_size *= 2;
	    root_allow_vector =
		(char **) xrealloc (root_allow_vector,
				   root_allow_size * sizeof (char *));
	}

    }
    p_sz = strlen (arg) + 1;
    p = xmalloc (p_sz);
    sstrcpy (p,arg,p_sz);
    root_allow_vector[root_allow_count++] = p;
}

void
root_allow_free ()
{
    if (root_allow_vector != NULL)
	free_names (&root_allow_count, root_allow_vector);
    root_allow_size = 0;
}

int
root_allow_ok (char *arg)
{
    int i;

    if (root_allow_count == 0)
    {
	/* Probably someone upgraded from CVS before 1.9.10 to 1.9.10
	   or later without reading the documentation about
	   --allow-root.  Printing an error here doesn't disclose any
	   particularly useful information to an attacker because a
	   CVS server configured in this way won't let *anyone* in.  */

	/* Note that we are called from a context where we can spit
	   back "error" rather than waiting for the next request which
	   expects responses.  */
	printf ("\
error 0 Server configuration missing --allow-root in inetd.conf\n");
	error_exit ();
    }

    for (i = 0; i < root_allow_count; ++i)
	if (strcmp (root_allow_vector[i], arg) == 0)
	    return 1;
    return 0;
}

/* This global variable holds the global -d option.  It is NULL if -d
   was not used, which means that we must get the CVSroot information
   from the CVSROOT environment variable or from a CVS/Root file.  */

char *CVSroot_cmdline;

/* Parse a CVSROOT variable into its constituent parts -- method,
 * username, hostname, directory.  The prototypical CVSROOT variable
 * looks like:
 *
 * :method:user@host:path
 *
 * Some methods may omit fields; local, for example, doesn't need user
 * and host.
 *
 * Returns zero on success, non-zero on failure. */

char *CVSroot_original = NULL;	/* the CVSroot that was passed in */
int client_active;		/* nonzero if we are doing remote access */
CVSmethod CVSroot_method;	/* one of the enum values defined in cvs.h */
char *CVSroot_username;		/* the username or NULL if method == local */
char *CVSroot_hostname;		/* the hostname or NULL if method == local */
char *CVSroot_directory;	/* the directory name */

int
parse_cvsroot (char *CVSroot)
{
    static int cvsroot_parsed = 0;
    char *cvsroot_copy, *cvsroot_save, *p;
    int check_hostname;

    /* Don't go through the trouble twice. */
    if (cvsroot_parsed)
    {
/*	error (0, 0, "WARNING (parse_cvsroot): someone called me twice!\n");*/
	return 0;
    }

    if (CVSroot_original != NULL)
	free (CVSroot_original);
    if (CVSroot_directory != NULL)
	free (CVSroot_directory);
    if (CVSroot_username != NULL)
	free (CVSroot_username);
    if (CVSroot_hostname != NULL)
	free (CVSroot_hostname);

    CVSroot_original = xstrdup (CVSroot);
    cvsroot_save = cvsroot_copy = xstrdup (CVSroot);

    if (*cvsroot_copy == ':')
    {
	char *method = ++cvsroot_copy;

	/* Access method specified, as in
	 * "cvs -d :pserver:user@host:/path",
	 * "cvs -d :local:e:\path",
	 * "cvs -d :tlsserver:user@host:/path", or
	 * "cvs -d :ext:user@host:/cvs"
	 * We need to get past that part of CVSroot before parsing the
	 * rest of it.
	 */

	if (! (p = strchr (method, ':')))
	{
	    /*error (0, 0, "bad CVSroot: %s", CVSroot);*/
	    xfree (cvsroot_save);
	    return 1;
	}
	*p = '\0';
	cvsroot_copy = ++p;

	/* Now we have an access method -- see if it's valid. */

	if (strcmp (method, "local") == 0)
	    CVSroot_method = local_method;
	else if (strcmp (method, "pserver") == 0)
	    CVSroot_method = pserver_method;
	else if (strcmp (method, "tlsserver") == 0)
	    CVSroot_method = tlsserver_method;
	else if (strcmp (method, "server") == 0)
	    CVSroot_method = server_method;
	else if (strcmp (method, "ext") == 0)
	    CVSroot_method = ext_method;
	else
	{
	    error (0, 0, "unknown method in CVSroot: %s", CVSroot);
	    xfree (cvsroot_save);
	    return 1;
	}
    }
    else
    {
	/* If the method isn't specified, assume
	   SERVER_METHOD/EXT_METHOD if the string contains a colon or
	   LOCAL_METHOD otherwise.  */

	CVSroot_method = ((strchr (cvsroot_copy, ':'))
#ifdef RSH_NOT_TRANSPARENT
			  ? server_method
#else
			  ? ext_method
#endif
			  : local_method);
    }

    client_active = (CVSroot_method != local_method);

    /* Check for username/hostname if we're not LOCAL_METHOD. */

    CVSroot_username = NULL;
    CVSroot_hostname = NULL;

    if ((CVSroot_method != local_method)
	&& (CVSroot_method != fork_method))
    {
	/* Check to see if there is a username in the string. */

	if ((p = strchr (cvsroot_copy, '@')) != NULL)
	{
	    *p = '\0';
	    CVSroot_username = xstrdup (cvsroot_copy);
	    cvsroot_copy = ++p;
	    if (*CVSroot_username == '\0')
		CVSroot_username = NULL;
	}

	if ((p = strchr (cvsroot_copy, ':')) != NULL)
	{
	    *p = '\0';
	    CVSroot_hostname = xstrdup (cvsroot_copy);
	    cvsroot_copy = ++p;
      
	    if (*CVSroot_hostname == '\0')
		CVSroot_hostname = NULL;
	}
    }

    CVSroot_directory = xstrdup(cvsroot_copy);
    xfree (cvsroot_save);

#if ! defined (DEBUG)
    if (CVSroot_method != local_method)
    {
	error (0, 0, "Your CVSROOT is set for a remote access method");
	error (0, 0, "but your CVS executable doesn't support it");
	error (0, 0, "(%s)", CVSroot);
	return 1;
    }
#endif
  
    /* Do various sanity checks. */

    if (CVSroot_username && ! CVSroot_hostname)
    {
	/*error (0, 0, "missing hostname in CVSROOT: %s", CVSroot);*/
	return 1;
    }

    check_hostname = 0;
    switch (CVSroot_method)
    {
    case local_method:
	if (CVSroot_username || CVSroot_hostname)
	{
	/*
	    error (0, 0, "can't specify hostname and username in CVSROOT");
	    error (0, 0, "when using local access method");
	    error (0, 0, "(%s)", CVSroot);*/
	    return 1;
	}
	/* cvs.texinfo has always told people that CVSROOT must be an
	   absolute pathname.  Furthermore, attempts to use a relative
	   pathname produced various errors (I couldn't get it to work),
	   so there would seem to be little risk in making this a fatal
	   error.  */
	if (!isabsolute (CVSroot_directory))
	    exit(1);
	/*    error (1, 0, "CVSROOT %s must be an absolute pathname",
		   CVSroot_directory);*/
	break;
   
    case server_method:
    case tlsserver_method:
    case ext_method:
    case pserver_method:
	check_hostname = 1;
	break;
    }

    if (check_hostname)
    {
	if (! CVSroot_hostname)
	{
	    error (0, 0, "didn't specify hostname in CVSROOT: %s", CVSroot);
	    return 1;
	}
    }

    if (*CVSroot_directory == '\0')
    {
	error (0, 0, "missing directory in CVSROOT: %s", CVSroot);
	return 1;
    }
    
    /* Hooray!  We finally parsed it! */
    return 0;
}


/* Set up the global CVSroot* variables as if we're using the local
   repository DIR.  */

void
set_local_cvsroot (char *dir)
{
    if (CVSroot_original != NULL)
	free (CVSroot_original);
    CVSroot_original = xstrdup(dir);
    CVSroot_method = local_method;
    if (CVSroot_directory != NULL)
	free (CVSroot_directory);
    CVSroot_directory = xstrdup(dir);
    if (CVSroot_username != NULL)
	free (CVSroot_username);
    CVSroot_username = NULL;
    if (CVSroot_hostname != NULL)
	free (CVSroot_hostname);
    CVSroot_hostname = NULL;
    client_active = 0;
}


/* here debug stuff */
#ifdef DEBUG
/* This is for testing the parsing function.  Use

     gcc -I. -I.. -I../lib -DDEBUG root.c -o root

   to compile.  */

#include <stdio.h>

char *CVSroot;
char *program_name = "testing";
char *command_name = "parse_cvsroot";		/* XXX is this used??? */

/* Toy versions of various functions when debugging under unix.  Yes,
   these make various bad assumptions, but they're pretty easy to
   debug when something goes wrong.  */

void
error_exit(void)
{
    exit (1);
}


int
isabsolute (dir)
    const char *dir;
{
    return (dir && (*dir == '/'));
}

void
main (argc, argv)
    int argc;
    char *argv[];
{
    program_name = argv[0];

    if (argc != 2)
    {
	fprintf (stderr, "Usage: %s <CVSROOT>\n", program_name);
	exit (2);
    }
  
    if (parse_cvsroot (argv[1]))
    {
	fprintf (stderr, "%s: Parsing failed.\n", program_name);
	exit (1);
    }
    printf ("CVSroot: %s\n", argv[1]);
    printf ("CVSroot_method: %s\n", method_names[CVSroot_method]);
    printf ("CVSroot_username: %s\n",
	    CVSroot_username ? CVSroot_username : "NULL");
    printf ("CVSroot_hostname: %s\n",
	    CVSroot_hostname ? CVSroot_hostname : "NULL");
    printf ("CVSroot_directory: %s\n", CVSroot_directory);

   exit (0);
   /* NOTREACHED */
}
#endif
