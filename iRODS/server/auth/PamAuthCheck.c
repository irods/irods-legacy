/*
  This program does a PAM authentication check using the username on
  the command line and reading the password from stdin (to be more
  secure).  We plan to have the irodsAgent spawn this process and
  write the password on stdin, for users using LDAP/PAM iRODS
  authentication.  You can also run this manually, entering the
  password after PamAuthCheck is started; which will be echoed:
  $ ./PamAuthCheck testuser2
  asfkskdlfkd
  Authenticated
  $

  You may need to install PAM libraries, such as libpam0g-dev:
  sudo apt-get install libpam0g-dev

  To build:
  gcc PamAuthCheck.c -L /usr/lib -l pam -l pam_misc -o PamAuthCheck

  It needs to be set UID root:
    sudo chown root PamAuthCheck
    sudo chmod u+s PamAuthCheck

  You may need to add the following (or equivalent) to the /etc/pam.conf file.
  # check authorization
  check   auth       required     pam_unix_auth.so
  check   account    required     pam_unix_acct.so

  Loosely based on PAM example programs: check_user.c and ckpasswd.c.
*/

#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>


/*
**  The PAM conversation function.
**
**  Since we already have all the information and can't ask the user
**  questions, we can't quite follow the real PAM protocol.  Instead, we just
**  return the password in response to every question that PAM asks.  There
**  appears to be no generic way to determine whether the message in question
**  is indeed asking for the password....
**
**  This function allocates an array of struct pam_response to return to the
**  PAM libraries that's never freed.  For this program, this isn't much of an
**  issue, since it will likely only be called once and then the program will
**  exit.  This function uses malloc and strdup instead of xmalloc and xstrdup
**  intentionally so that the PAM conversation will be closed cleanly if we
**  run out of memory rather than simply terminated.
**
**  appdata_ptr contains the password we were given.
*/
static int
pass_conv(int num_msg, const struct pam_message **msgm,
          struct pam_response **response, void *appdata_ptr)
{
    int i;

    *response = malloc(num_msg * sizeof(struct pam_response));
    if (*response == NULL)
        return PAM_CONV_ERR;
    for (i = 0; i < num_msg; i++) {
        (*response)[i].resp = strdup((char *)appdata_ptr);
        (*response)[i].resp_retcode = 0;
    }
    return PAM_SUCCESS;
}

int main(int argc, char *argv[])
{
    pam_handle_t *pamh=NULL;
    int retval;
    int nb;
    int debug=0;

    static char password[500];

    static char *user="nobody";
    struct pam_conv conv;

    if(argc == 2) {
	user = argv[1];
    }
    else {
	fprintf(stderr, "Usage: PamAuthCheck username\n");
	exit(1);
    }

    /* read the pw from stdin */
    nb = read(0, (void*)&password, sizeof(password));
    if (debug>0) printf("nb=%d\n",nb);
    if (password[nb-1]=='\n') password[nb-1]='\0';

    conv.conv = pass_conv;
    conv.appdata_ptr = (void *)&password;
    retval = pam_start("check", user, &conv, &pamh);
    if (debug>0) printf("retval 1=%d\n",retval);
	
    if (retval == PAM_SUCCESS) {
        retval = pam_authenticate(pamh, 0);    /* check username-password */
	if (debug>0) printf("retval 2=%d\n",retval);
    }

    strcpy(password, "                    ");

    if (retval == PAM_SUCCESS) {
	fprintf(stdout, "Authenticated\n");
    } else {
	fprintf(stdout, "Not Authenticated\n");
    }

    if (pam_end(pamh,retval) != PAM_SUCCESS) {   /* close Linux-PAM */
	pamh = NULL;
	fprintf(stderr, "PamAuthCheck: failed to release authenticator\n");
	exit(1);
    }

    return ( retval == PAM_SUCCESS ? 0:1 );   /* indicate success (valid
						 username and password) or
						 not */
}
