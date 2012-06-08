/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See pamAuthRequest.h for a description of this API call.*/

#include <sys/wait.h>

#include "pamAuthRequest.h"
#include "genQuery.h"
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"
#include "pamAuth.h"


int
rsPamAuthRequest (rsComm_t *rsComm, pamAuthRequestInp_t *pamAuthRequestInp,
		  pamAuthRequestOut_t **pamAuthRequestOut)
{
    rodsServerHost_t *rodsServerHost;
    int status;

    status = getAndConnRcatHost(rsComm, MASTER_RCAT, NULL, &rodsServerHost);
    if (status < 0) {
       return(status);
    }
    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsPamAuthRequest(rsComm,  pamAuthRequestInp,
				  pamAuthRequestOut);

#else
       status = SYS_NO_RCAT_SERVER_ERR;
#endif
    }
    else {
       status = rcPamAuthRequest(rodsServerHost->conn, pamAuthRequestInp,
				 pamAuthRequestOut);
    }
    return (status);
}


#ifdef RODS_CAT
/*
 Fork and exec the PamAuthCheck program, which is setuid root, to do
 do the PAM authentication.  The username is on the command line but
 the password is sent on a pipe to be more secure.
 */
#ifndef PAM_AUTH_CHECK_PROG
#define PAM_AUTH_CHECK_PROG  "../auth/PamAuthCheck"
#endif
int
runPamAuthCheck(char *username, char *password) 
{
   int p2cp[2]; /* parent to child pipe */
   int pid, i;
   int status;

   if (pipe(p2cp) < 0) {
     return(SYS_PIPE_ERROR);
   }
   pid = fork();
   if (pid == -1) {
     return(SYS_FORK_ERROR);
   }

   if (pid)  {
/* 
  This is still the parent.  Write the message to the child and
  then wait for the exit and status.
*/
      write(p2cp[1], password, strlen(password));
      close(p2cp[1]);
      waitpid(pid,&status,0);
      return(status);
   }
   else {
/* This is the child */
      close(0);          /* close current stdin */
      dup (p2cp[0]);     /* Make stdin come from read end of the pipe */
      close (p2cp[1]);
      i = execl(PAM_AUTH_CHECK_PROG, PAM_AUTH_CHECK_PROG, username,
		(char *)NULL);
      perror("execl");
      printf("execl failed %d\n",i);
   }
   return(SYS_FORK_ERROR); /* avoid compiler warning */
}

int
_rsPamAuthRequest (rsComm_t *rsComm, pamAuthRequestInp_t *pamAuthRequestInp,
		   pamAuthRequestOut_t **pamAuthRequestOut) {
    int status = 0;
    pamAuthRequestOut_t *result;

    *pamAuthRequestOut = (pamAuthRequestOut_t *)
       malloc(sizeof(pamAuthRequestOut_t));
    memset((char *)*pamAuthRequestOut, 0, sizeof(pamAuthRequestOut_t));

    result = *pamAuthRequestOut;

#if defined(PAM_AUTH)

#ifdef RUN_SERVER_AS_ROOT
    status = pamAuthenticate(pamAuthRequestInp->pamUser,
                             pamAuthRequestInp->pamPassword);
#else
    /* Normal mode, fork/exec setuid program to do the Pam check */
    status = runPamAuthCheck(pamAuthRequestInp->pamUser,
                             pamAuthRequestInp->pamPassword);
    if (status == 256) {
      status = PAM_AUTH_PASSWORD_FAILED;
    }
    else {
      /* the exec failed or something (PamAuthCheck not built perhaps) */
      if (status != 0) status = PAM_AUTH_NOT_BUILT_INTO_SERVER;
    }
#endif
    if (status) {
      return(status);
    }
    result->irodsPamPassword = (char*)malloc(100);
    if (result->irodsPamPassword == 0) return (SYS_MALLOC_ERR);
    status = chlUpdateIrodsPamPassword(rsComm, 
				       pamAuthRequestInp->pamUser, 
				       &result->irodsPamPassword);
    return(status);
#else
    status = PAM_AUTH_NOT_BUILT_INTO_SERVER;
    return (status);
#endif
} 
#endif /* RODS_CAT */
