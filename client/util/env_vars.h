/* 
deneb@penguin.it
 */
#define MAX_ENV_VAR 1024
#define OK_PATHNAME "QWERTYUIOPASDFGHJKLZXCVBNM1234567890qwertyuiopasdfghjklzxcvbnm_.-/"
char *env_pathfile (const char *path);
int valid_program (const char *program);
char *fetch_tmpdir (struct stat *tmpstat);