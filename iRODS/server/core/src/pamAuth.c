/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* pamAuth.c - functions for performing PAM authentication with username and password
 */

#include "rodsErrorTable.h"
#include "pamAuth.h"

#ifdef PAM_AUTH
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>

const char pam_service[] = "irods";

struct pam_response *reply;
 
int 
null_conv(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr) 
{
  *resp = reply;
  return PAM_SUCCESS;
}
 
static struct pam_conv conv = { null_conv, NULL };
 
#endif /* PAM_AUTH */

int
pamAuthenticate(char *username, char *password)
{
#ifdef PAM_AUTH
    pam_handle_t *pamh = NULL;
    int rc = 0;
    
    if (username == NULL) {
        return USER__NULL_INPUT_ERR;
    }

    rc = pam_start(pam_service, username, &conv, &pamh); 
    if (rc != PAM_SUCCESS) {
        return PAM_AUTH_PASSWORD_FAILED;
    }
    
    reply = (struct pam_response*)malloc(sizeof(struct pam_response));
    if (reply == NULL) {
        return SYS_MALLOC_ERR;
    }
    
    reply[0].resp = strdup(password);
    reply[0].resp_retcode = 0;
    
    rc = pam_authenticate(pamh, 0);

    pam_end(pamh, rc);
    
    if (rc != PAM_SUCCESS) {
        return PAM_AUTH_PASSWORD_FAILED;
    }
    else {
      return 0;
    }
#else
    return PAM_AUTH_PASSWORD_FAILED;
#endif
}

