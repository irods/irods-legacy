/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See generalAdmin.h for a description of this API call.*/

#include "generalAdmin.h"
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

int
rsGeneralAdmin (rsComm_t *rsComm, generalAdminInp_t *generalAdminInp )
{
    rodsServerHost_t *rodsServerHost;
    int status;

    rodsLog(LOG_DEBUG, "generalAdmin");

    status = getAndConnRcatHost(rsComm, MASTER_RCAT, NULL, &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsGeneralAdmin (rsComm, generalAdminInp);
#else
       status = SYS_NO_RCAT_SERVER_ERR;
#endif
    }
    else {
       status = rcGeneralAdmin(rodsServerHost->conn,
			       generalAdminInp);
    }

    if (status < 0) { 
       rodsLog (LOG_NOTICE,
		"rsGeneralAdmin: rcGeneralAdmin failed");
    }
    return (status);
}

#ifdef RODS_CAT
int
_rsGeneralAdmin(rsComm_t *rsComm, generalAdminInp_t *generalAdminInp )
{
    int status;
    userInfo_t userInfo;
    collInfo_t collInfo;
    rescInfo_t rescInfo;
    ruleExecInfo_t rei;

    rodsLog (LOG_DEBUG,
	     "_rsGeneralAdmin arg0=%s", 
	     generalAdminInp->arg0);

    if (strcmp(generalAdminInp->arg0,"pvacuum")==0) {
       char *args[2];
       char argStr[100];    /* argument string */
       memset((char*)&rei,0,sizeof(rei));
       rei.rsComm = rsComm;
       rei.uoic = &rsComm->clientUser;
       rei.uoip = &rsComm->proxyUser;
       rstrcpy(argStr,"",100);
       if (atoi(generalAdminInp->arg1) > 0) {
	  snprintf(argStr,100,"<ET>%s</ET>",generalAdminInp->arg1);
       }
       if (atoi(generalAdminInp->arg2) > 0) {
	  strncat(argStr,"<EF>",100);
	  strncat(argStr,generalAdminInp->arg2,100);
	  strncat(argStr,"</EF>",100);
       }
       args[0]=argStr;
       status = applyRuleArg("acVacuum", args, 1, &rei, SAVE_REI);
       return(status);
    }

    if (strcmp(generalAdminInp->arg0,"add")==0) {
       if (strcmp(generalAdminInp->arg1,"user")==0) { 
	  /* run the acCreateUser rule */
	  char *args[2];
	  memset((char*)&rei,0,sizeof(rei));
	  rei.rsComm = rsComm;
	  strncpy(userInfo.userName, generalAdminInp->arg2, NAME_LEN);
	  strncpy(userInfo.userType, generalAdminInp->arg3, NAME_LEN);
	  strncpy(userInfo.rodsZone, generalAdminInp->arg4, NAME_LEN);

	  strncpy(userInfo.authInfo.authStr, generalAdminInp->arg5, NAME_LEN);
	  rei.uoio = &userInfo;
	  rei.uoic = &rsComm->clientUser;
	  rei.uoip = &rsComm->proxyUser;
	  status = applyRuleArg("acCreateUser", args, 0, &rei, SAVE_REI);
	  if (status != 0) chlRollback(rsComm);
          return(status);
       }
       if (strcmp(generalAdminInp->arg1,"dir")==0) {
	  memset((char*)&collInfo,0,sizeof(collInfo));
	  strncpy(collInfo.collName, generalAdminInp->arg2, MAX_NAME_LEN);
	  if (strlen(generalAdminInp->arg3) > 0) {
	     strncpy(collInfo.collOwnerName, generalAdminInp->arg3,
		     MAX_NAME_LEN);
	     status = chlRegCollByAdmin(rsComm, &collInfo);
	     if (status == 0) {
		int status2;
		status2 = chlCommit(rsComm);
	     }
	  }
	  else {
	     status = chlRegColl(rsComm, &collInfo);
	  }
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"zone")==0) {
	  status = chlRegZone(rsComm, generalAdminInp->arg2,
			      generalAdminInp->arg3, 
			      generalAdminInp->arg4,
			      generalAdminInp->arg5);
	  if (status == 0) {
	     if (strcmp(generalAdminInp->arg3,"remote")==0) {
		memset((char*)&collInfo,0,sizeof(collInfo));
		strncpy(collInfo.collName, "/", MAX_NAME_LEN);
		strncat(collInfo.collName, generalAdminInp->arg2,
			MAX_NAME_LEN);
		strncpy(collInfo.collOwnerName, rsComm->proxyUser.userName,
			MAX_NAME_LEN);
		status = chlRegCollByAdmin(rsComm, &collInfo);
		if (status == 0) {
		   chlCommit(rsComm);
		}
	     }
	  }
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"resource")==0) {
	  strncpy(rescInfo.rescName,  generalAdminInp->arg2, NAME_LEN);
	  strncpy(rescInfo.rescType,  generalAdminInp->arg3, NAME_LEN);
	  strncpy(rescInfo.rescClass, generalAdminInp->arg4, NAME_LEN);
	  strncpy(rescInfo.rescLoc,   generalAdminInp->arg5, NAME_LEN);
	  strncpy(rescInfo.rescVaultPath, generalAdminInp->arg6, NAME_LEN);
	  strncpy(rescInfo.zoneName,  generalAdminInp->arg7, NAME_LEN);
	  status = chlRegResc(rsComm, &rescInfo);
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"token")==0) {
	  status = chlRegToken(rsComm, generalAdminInp->arg2,
			       generalAdminInp->arg3, 
			       generalAdminInp->arg4,
			       generalAdminInp->arg5,
			       generalAdminInp->arg6,
			       generalAdminInp->arg7);
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
    }
    if (strcmp(generalAdminInp->arg0,"modify")==0) {
       if (strcmp(generalAdminInp->arg1,"user")==0) {
	  status = chlModUser(rsComm, generalAdminInp->arg2, 
			      generalAdminInp->arg3, generalAdminInp->arg4);
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"group")==0) {
	  status = chlModGroup(rsComm, generalAdminInp->arg2, 
			       generalAdminInp->arg3, generalAdminInp->arg4,
			       generalAdminInp->arg5);
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"zone")==0) {
	  status = chlModZone(rsComm, generalAdminInp->arg2, 
			      generalAdminInp->arg3, generalAdminInp->arg4);
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"localzonename")==0) {
	  /* run the acRenameLocalZone rule */
	  char *args[2];
	  memset((char*)&rei,0,sizeof(rei));
	  rei.rsComm = rsComm;
	  memset((char*)&rei,0,sizeof(rei));
	  rei.rsComm = rsComm;
	  rei.uoic = &rsComm->clientUser;
	  rei.uoip = &rsComm->proxyUser;
	  args[0]=generalAdminInp->arg2;
	  args[1]=generalAdminInp->arg3;
	  status = applyRuleArg("acRenameLocalZone", args, 2, &rei, 
				NO_SAVE_REI);
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"resource")==0) {
	  status = chlModResc(rsComm, generalAdminInp->arg2, 
			      generalAdminInp->arg3, generalAdminInp->arg4);
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"resourcegroup")==0) {
	  status = chlModRescGroup(rsComm, generalAdminInp->arg2, 
			      generalAdminInp->arg3, generalAdminInp->arg4);
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
    }
    if (strcmp(generalAdminInp->arg0,"rm")==0) {
       if (strcmp(generalAdminInp->arg1,"user")==0) { 
	  /* run the acDeleteUser rule */
	  char *args[2];
	  memset((char*)&rei,0,sizeof(rei));
	  rei.rsComm = rsComm;
	  strncpy(userInfo.userName, generalAdminInp->arg2, NAME_LEN);
	  strncpy(userInfo.rodsZone, generalAdminInp->arg3, NAME_LEN);
	  rei.uoio = &userInfo;
	  rei.uoic = &rsComm->clientUser;
	  rei.uoip = &rsComm->proxyUser;
	  status = applyRuleArg("acDeleteUser", args, 0, &rei, SAVE_REI);
	  if (status != 0) chlRollback(rsComm);
          return(status);
       }
       if (strcmp(generalAdminInp->arg1,"dir")==0) {
	  memset((char*)&collInfo,0,sizeof(collInfo));
	  strncpy(collInfo.collName, generalAdminInp->arg2, MAX_NAME_LEN);
	  status = chlDelColl(rsComm, &collInfo);
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"resource")==0) {
	  strncpy(rescInfo.rescName,  generalAdminInp->arg2, NAME_LEN);
	  status = chlDelResc(rsComm, &rescInfo);
	  if (status != 0) chlRollback(rsComm);
	  return(status); 
       }
       if (strcmp(generalAdminInp->arg1,"zone")==0) {
	  status = chlDelZone(rsComm, generalAdminInp->arg2);
	  if (status == 0) {
	     memset((char*)&collInfo,0,sizeof(collInfo));
	     strncpy(collInfo.collName, "/", MAX_NAME_LEN);
	     strncat(collInfo.collName, generalAdminInp->arg2,
		     MAX_NAME_LEN);
	     status = chlDelCollByAdmin(rsComm, &collInfo);
	  }
	  if (status == 0) {
	     status = chlCommit(rsComm);
	  }
	  return(status);
       }
       if (strcmp(generalAdminInp->arg1,"token")==0) {
	  status = chlDelToken(rsComm, generalAdminInp->arg2,
			       generalAdminInp->arg3);
	  if (status != 0) chlRollback(rsComm);
	  return(status);
       }
    }

    return(CAT_INVALID_ARGUMENT);
} 
#endif
