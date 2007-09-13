/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* initServer.c - Server initialization routines
 */

#include "initServer.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "genQuery.h"
#include "rsIcatOpr.h"
#include "miscServerFunct.h"
#include "reGlobalsExtern.h"
#include "reDefines.h"

int
resolveHost (rodsHostAddr_t *addr, rodsServerHost_t **rodsServerHost)
{
    int status;
    rodsServerHost_t *tmpRodsServerHost;
    char *myHostAddr;

    /* check if host exist */

    myHostAddr = addr->hostAddr;

    tmpRodsServerHost = ServerHostHead;
    while (tmpRodsServerHost != NULL) {
	hostName_t *tmpName;
	tmpName = tmpRodsServerHost->hostName;
	while (tmpName != NULL) {
	    if (strcasecmp (tmpName->name, myHostAddr) == 0) {
		/* a match */
		*rodsServerHost = tmpRodsServerHost;
		return (tmpRodsServerHost->localFlag);
	    }
	    tmpName = tmpName->next;
	}
	tmpRodsServerHost = tmpRodsServerHost->next;
    }

    /* no match */ 
 
    tmpRodsServerHost = mkServerHost (myHostAddr, addr->portNum);

    if (tmpRodsServerHost == NULL) {
	rodsLog (LOG_ERROR,
          "resolveHost: mkServerHost error");
        return (SYS_INVALID_SERVER_HOST);
    }

    /* assume it is remote */
    tmpRodsServerHost->localFlag = REMOTE_HOST;

    status = queRodsServerHost (&ServerHostHead, tmpRodsServerHost);
    *rodsServerHost = tmpRodsServerHost;

    return (tmpRodsServerHost->localFlag);
}

rodsServerHost_t *
mkServerHost (char *myHostAddr, int portNum)
{
    rodsServerHost_t *tmpRodsServerHost;
    int status;

    tmpRodsServerHost = malloc (sizeof (rodsServerHost_t));
    memset (tmpRodsServerHost, 0, sizeof (rodsServerHost_t));

    if (portNum > 0) {
        tmpRodsServerHost->portNum = portNum;
    } else {
        tmpRodsServerHost->portNum = ServerHostHead->portNum;
    }
    /* XXXXX need to lookup the zone table when availiable */
    status = queHostName (tmpRodsServerHost, myHostAddr, 0);
    if (status < 0) {
        return (NULL);
    }

    tmpRodsServerHost->localFlag = UNKNOWN_HOST_LOC;

    status = queAddr (tmpRodsServerHost, myHostAddr);

    status = matchHostConfig (tmpRodsServerHost);

    return (tmpRodsServerHost);
}

int
initServerInfo (rsComm_t *rsComm)
{
    int status;

    status = initHostConfigByFile (rsComm);

    status = initLocalServerHost (rsComm);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "initServerInfo: initLocalServerHost error, status = %d",
          status);
        return (status);
    }
    status = initRcatServerHostByFile (rsComm);
    if (status < 0) {
        rodsLog (LOG_SYS_FATAL,
          "initServerInfo: initRcatServerHostByFile error, status = %d",
          status);
        return (status);
    }

#ifdef RODS_CAT
    status = connectRcat (rsComm);
    if (status < 0) {
        return (status);
    }
#endif
    status = initZone (rsComm);
    if (status < 0) {
        rodsLog (LOG_SYS_FATAL,
          "initServerInfo: initZone error, status = %d",
          status);
        return (status);
    }

    status = initResc (rsComm);
    if (status < 0) {
	if (status == CAT_NO_ROWS_FOUND) {
	    rodsLog (LOG_NOTICE, 
	     "initServerInfo: No resource is configured in ICAT");
	    status = 0;
	} else { 
            rodsLog (LOG_SYS_FATAL,
              "initServerInfo: initResc error, status = %d",
              status);
            return (status);
	}
    }

    return (status);
}

int
initLocalServerHost (rsComm_t *rsComm)
{
    int status;
    char myHostName[MAX_NAME_LEN];
    rodsEnv *myEnv = &rsComm->myEnv;

    LocalServerHost = ServerHostHead = malloc (sizeof (rodsServerHost_t));
    memset (ServerHostHead, 0, sizeof (rodsServerHost_t)); 

    LocalServerHost->localFlag = LOCAL_HOST;

    status = matchHostConfig (LocalServerHost);

    queHostName (ServerHostHead, "localhost", 0); 
    status = gethostname (myHostName, MAX_NAME_LEN);
    if (status < 0) {
	status = SYS_GET_HOSTNAME_ERR - errno;
        rodsLog (LOG_NOTICE, 
	  "initLocalServerHost: gethostname error, status = %d", 
	  status);
	return (status);
    }
    status = queHostName (ServerHostHead, myHostName, 0);
    if (status < 0) {
	return (status);
    }

    status = queAddr (ServerHostHead, myHostName);
    if (status < 0) {
        return (status);
    }

    if (myEnv != NULL) {
        ServerHostHead->portNum = myEnv->rodsPort;
    }

    if (ProcessType == SERVER_PT) {
	printServerHost (LocalServerHost);
    }

    return (status);
}

int
printServerHost (rodsServerHost_t *myServerHost)
{
    hostName_t *tmpHostName;

    if (myServerHost->localFlag == LOCAL_HOST) {
        fprintf (stderr, "    LocalHostName: ");
    } else {
        fprintf (stderr, "    RemoteHostName: ");
    }

    tmpHostName = myServerHost->hostName;

    while (tmpHostName != NULL) {
        fprintf (stderr, " %s,", tmpHostName->name);
        tmpHostName = tmpHostName->next;
    }

    fprintf (stderr, " Port Num: %d.\n\n", myServerHost->portNum);

    return (0);
}

int
printZoneInfo ()
{
    zoneInfo_t *tmpZoneInfo;
    rodsServerHost_t *tmpRodsServerHost;

    tmpZoneInfo = ZoneInfoHead;
    fprintf (stderr, "Zone Info:\n");
    while (tmpZoneInfo != NULL) {
        tmpRodsServerHost = (rodsServerHost_t *) tmpZoneInfo->rodsServerHost;
	fprintf (stderr, "    ZoneName: %s   ", tmpZoneInfo->zoneName);
	if (tmpRodsServerHost->rcatEnabled == LOCAL_ICAT) {
	    fprintf (stderr, "Type: LOCAL_ICAT   "); 
	} else if (tmpRodsServerHost->rcatEnabled == LOCAL_SLAVE_ICAT) {
	    fprintf (stderr, "Type: LOCAL_SLAVE_ICAT   "); 
	} else {
	    fprintf (stderr, "Type: REMOTE_ICAT   "); 
	}
        fprintf (stderr, " HostAddr: %s   PortNum: %d\n\n", 
	  tmpRodsServerHost->hostName->name, tmpRodsServerHost->portNum);
	tmpZoneInfo = tmpZoneInfo->next;
    }
}

int
printLocalResc ()
{
    rescInfo_t *myRescInfo;
    rescGrpInfo_t *tmpRescGrpInfo;
    rodsServerHost_t *tmpRodsServerHost;
    int localRescCnt = 0;

    fprintf (stderr, "Local Resource configuration: \n");

    /* search the global RescGrpInfo */

    tmpRescGrpInfo = RescGrpInfo;

    while (tmpRescGrpInfo != NULL) {
        myRescInfo = tmpRescGrpInfo->rescInfo;
	tmpRodsServerHost = myRescInfo->rodsServerHost;
        if (tmpRodsServerHost->localFlag == LOCAL_HOST) {
	    fprintf (stderr, "   RescName: %s, VaultPath: %s\n",
	      myRescInfo->rescName, myRescInfo->rescVaultPath); 
	    localRescCnt ++;
        }
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }

    fprintf (stderr, "\n");

    if (localRescCnt == 0) {
        fprintf (stderr, "   No Local Resource Configured\n");
    }

    return (0);
}

int
initRcatServerHostByFile (rsComm_t *rsComm)
{
    FILE *fptr;
    char *rcatCongFile;
    char inbuf[MAX_NAME_LEN];
    rodsHostAddr_t addr;
    rodsServerHost_t *tmpRodsServerHost;
    int lineLen, bytesCopied, remoteFlag;
    char keyWdName[MAX_NAME_LEN];
    int gptRcatFlag = 0;
    rodsEnv *myEnv = &rsComm->myEnv;

    rcatCongFile =  (char *) malloc((strlen (getConfigDir()) +
        strlen(RCAT_HOST_FILE) + 24));

    sprintf (rcatCongFile, "%-s/%-s", getConfigDir(), RCAT_HOST_FILE);

    fptr = fopen (rcatCongFile, "r");

    if (fptr == NULL) {
        rodsLog (LOG_SYS_FATAL, 
	  "Cannot open RCAT_HOST_FILE  file %s. ernro = %d\n",
          rcatCongFile, errno);
        free (rcatCongFile);
	return (SYS_CONFIG_FILE_ERR);
    }

    free (rcatCongFile);

    memset (&addr, 0, sizeof (addr));
    while ((lineLen = getLine (fptr, inbuf, MAX_NAME_LEN)) > 0) {
	char *inPtr = inbuf;
	if ((bytesCopied = getStrInBuf (&inPtr, keyWdName, 
	  &lineLen, LONG_NAME_LEN)) > 0) {
	    /* advance inPtr */
	    if (strcmp (keyWdName, RE_RULESET_KW) == 0) { 
		if ((bytesCopied = getStrInBuf (&inPtr, reRuleStr,
          	 &lineLen, LONG_NAME_LEN)) < 0) {
                    rodsLog (LOG_SYS_FATAL,
                      "initRcatServerHostByFile: parsing error for keywd %s",
                       keyWdName);
		    return (SYS_CONFIG_FILE_ERR);
		}
	    } else if (strcmp (keyWdName, RE_FUNCMAPSET_KW) == 0) { 
		if ((bytesCopied = getStrInBuf (&inPtr, reFuncMapStr,
          	 &lineLen, LONG_NAME_LEN)) < 0) {
                    rodsLog (LOG_SYS_FATAL,
                      "initRcatServerHostByFile: parsing error for keywd %s",
                       keyWdName);
		    return (SYS_CONFIG_FILE_ERR);
		}
	    } else if (strcmp (keyWdName, RE_VARIABLEMAPSET_KW) == 0) { 
		if ((bytesCopied = getStrInBuf (&inPtr, reVariableMapStr,
          	 &lineLen, LONG_NAME_LEN)) < 0) {
                    rodsLog (LOG_SYS_FATAL,
                      "initRcatServerHostByFile: parsing error for keywd %s",
                       keyWdName);
		    return (SYS_CONFIG_FILE_ERR);
		}
	    } else if (strcmp (keyWdName, ICAT_HOST_KW) == 0) { 
		if ((bytesCopied = getStrInBuf (&inPtr, addr.hostAddr,
          	 &lineLen, LONG_NAME_LEN)) > 0) {
    		    remoteFlag = resolveHost (&addr, &tmpRodsServerHost);
		    if (remoteFlag < 0) {
		        rodsLog (LOG_SYS_FATAL,
		          "initRcatServerHostByFile: resolveHost error for %s, status = %d",
		          addr.hostAddr, remoteFlag);
		        return (remoteFlag);
		    }
		    tmpRodsServerHost->rcatEnabled = LOCAL_ICAT;
		    gptRcatFlag = 1;
		} else {
                    rodsLog (LOG_SYS_FATAL,
                      "initRcatServerHostByFile: parsing error for keywd %s",
                       keyWdName);
		    return (SYS_CONFIG_FILE_ERR);
		}
	    } else if (strcmp (keyWdName, SLAVE_ICAT_HOST_KW) == 0) {
                if ((bytesCopied = getStrInBuf (&inPtr, addr.hostAddr,
                 &lineLen, LONG_NAME_LEN)) > 0) {
                    remoteFlag = resolveHost (&addr, &tmpRodsServerHost);
                    if (remoteFlag < 0) {
                        rodsLog (LOG_SYS_FATAL,
                          "initRcatServerHostByFile: resolveHost error for %s, status = %d",
                          addr.hostAddr, remoteFlag);
                        return (remoteFlag);
                    }
		    tmpRodsServerHost->rcatEnabled = LOCAL_SLAVE_ICAT;
                } else {
                    rodsLog (LOG_SYS_FATAL,
                      "initRcatServerHostByFile: parsing error for keywd %s",
                       keyWdName);
                    return (SYS_CONFIG_FILE_ERR);
		}
	    }
	}
    } 
    fclose (fptr);

    if (gptRcatFlag <= 0) {
       rodsLog (LOG_SYS_FATAL,
          "initRcatServerHostByFile: icatHost entry missing in %s.\n",
          RCAT_HOST_FILE);
        return (SYS_CONFIG_FILE_ERR);
    }

    return (0);
}

int
queAddr (rodsServerHost_t *rodsServerHost, char *myHostName)
{
    struct hostent *hostEnt;
    time_t beforeTime, afterTime;
    int status;

    if (rodsServerHost == NULL || myHostName == NULL) {
	return (0);
    }

    /* gethostbyname could hang for some address */

    beforeTime = time (0);
    if ((hostEnt = gethostbyname (myHostName)) == NULL) {
	status = SYS_GET_HOSTNAME_ERR - errno;
        if (ProcessType == SERVER_PT) {
	    rodsLog (LOG_NOTICE,
              "queAddr: gethostbyname error for %s ,errno = %d\n",
              myHostName, errno);
	}
        return (status);
    }
    afterTime = time (0);
    if (afterTime - beforeTime >= 2) {
        rodsLog (LOG_NOTICE,
         "WARNING WARNING: gethostbyname of %s is taking %d sec. This could severely affect interactivity of your Rods system",
         myHostName, afterTime - beforeTime);
	/* XXXXXX may want to mark resource down later */
    }

    if (strcasecmp (myHostName, hostEnt->h_name) != 0) {
        queHostName (rodsServerHost, hostEnt->h_name, 0);
    }
    return (0);
}

int 
queHostName (rodsServerHost_t *rodsServerHost, char *myName, int topFlag)
{
    hostName_t *myHostName, *lastHostName;
    hostName_t *tmpHostName;

    myHostName = lastHostName = rodsServerHost->hostName;
    
    while (myHostName != NULL) {
	if (strcmp (myName, myHostName->name) == 0) {
	    return (0);
	}
	lastHostName = myHostName;
	myHostName = myHostName->next;
    }
    
    tmpHostName = malloc (sizeof (hostName_t));
    tmpHostName->name = strdup (myName);
 
    if (topFlag > 0) {
	tmpHostName->next = rodsServerHost->hostName;
	rodsServerHost->hostName = tmpHostName;
    } else {
        if (lastHostName == NULL) {
	    rodsServerHost->hostName = tmpHostName;
        } else {
	    lastHostName->next = tmpHostName;
	}
    }
    tmpHostName->next = NULL;

    return (0);
}

int
queRodsServerHost (rodsServerHost_t **rodsServerHostHead, 
rodsServerHost_t *myRodsServerHost)
{
    rodsServerHost_t *lastRodsServerHost, *tmpRodsServerHost;

    lastRodsServerHost = tmpRodsServerHost = *rodsServerHostHead;
    while (tmpRodsServerHost != NULL) {
        lastRodsServerHost = tmpRodsServerHost;
        tmpRodsServerHost = tmpRodsServerHost->next;
    }
  
    if (lastRodsServerHost == NULL) {
	*rodsServerHostHead = myRodsServerHost;
    } else {
        lastRodsServerHost->next = myRodsServerHost;
    }
    myRodsServerHost->next = NULL;

    return (0);
}

char *
getConfigDir()
{
    char *myDir;

    if ((myDir = (char *) getenv("irodsConfigDir")) != (char *) NULL) {
        return (myDir);
    }
    return (DEF_CONFIG_DIR);
}

char *
getLogDir()
{
    char *myDir;

    if ((myDir = (char *) getenv("irodsLogDir")) != (char *) NULL) {
        return (myDir);
    }
    return (DEF_LOG_DIR);
}

/* getAndConnRcatHost - get the rcat enabled host (result given in
 * rodsServerHost) based on the rcatZoneHint.  
 * rcatZoneHint is the hint for which zone to go it. It can be :
 *	a full path - e.g., /A/B/C. In this case, "A" will be taken as the zone
 *	a zone name - a string wth the first character that is no '/' is taken
 *	   as a zone name.
 *	NULL string - default to local zone
 * If the rcat host is remote, it will automatically connect to the rcat host.
 */

int 
getAndConnRcatHost (rsComm_t *rsComm, int rcatType, char *rcatZoneHint,
rodsServerHost_t **rodsServerHost)
{
    int status;

    status = getRcatHost (rcatType, rcatZoneHint, rodsServerHost);

    if (status < 0) {
	return (status);
    }

    if ((*rodsServerHost)->localFlag == LOCAL_HOST) {
	return (LOCAL_HOST);
    }
 
    status = svrToSvrConnect (rsComm, *rodsServerHost); 

    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "getAndConnRcatHost: svrToSvrConnect to %s failed",
	  (*rodsServerHost)->hostName->name);
    }
    if (status >= 0) {
	return (REMOTE_HOST);
    } else {
        return (status);
    }
}

int
getAndConnRcatHostNoLogin (rsComm_t *rsComm, int rcatType, char *rcatZoneHint,
rodsServerHost_t **rodsServerHost)
{
    int status;

    status = getRcatHost (rcatType, rcatZoneHint, rodsServerHost);

    if (status < 0) {
        return (status);
    }

    if ((*rodsServerHost)->localFlag == LOCAL_HOST) {
        return (LOCAL_HOST);
    }

    status = svrToSvrConnectNoLogin (rsComm, *rodsServerHost);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "getAndConnRcatHost: svrToSvrConnectNoLogin to %s failed",
          (*rodsServerHost)->hostName->name);
    }
    return (status);
}

/* getRcatHost - get the rodsServerHost of the rcat enable host based
 * on the rcatZoneHint for identifying the zone.
 *   rcatZoneHint == NULL ==> local rcat zone
 *   rcatZoneHint in the form of a full path,e.g.,/x/y/z ==> x is the zoneName.
 *   rcatZoneHint not in the form of a full path ==> rcatZoneHint is the zone.
 */

int
getRcatHost (int rcatType, char *rcatZoneHint,  
rodsServerHost_t **rodsServerHost)
{
    int status;
    rodsServerHost_t *tmpRodsServerHost;
    zoneInfo_t *tmpZoneInfo;
    int localRcatFlag;
    char zoneName[NAME_LEN];
 
    if (rcatZoneHint == NULL || strlen (rcatZoneHint) == 0) {
	localRcatFlag = 1;
    } else {
	localRcatFlag = 0;
	getZoneNameFromHint (rcatZoneHint, zoneName, NAME_LEN);
    }

    tmpZoneInfo = ZoneInfoHead;
    while (tmpZoneInfo != NULL) {
	tmpRodsServerHost = (rodsServerHost_t *) tmpZoneInfo->rodsServerHost;
	if (localRcatFlag) {
	    if (rcatType == MASTER_RCAT) {
	        if (tmpRodsServerHost->rcatEnabled == LOCAL_ICAT) {
		    *rodsServerHost = tmpRodsServerHost;
		    return (tmpRodsServerHost->localFlag);
		}
	    } else {
	        if (tmpRodsServerHost->rcatEnabled == LOCAL_SLAVE_ICAT) {
                    *rodsServerHost = tmpRodsServerHost;
                    return (tmpRodsServerHost->localFlag);
		}
	    }
	} else {	/* remote zone */
	    if (strcmp (zoneName, tmpZoneInfo->zoneName) == 0) {
                *rodsServerHost = tmpRodsServerHost;
                return (tmpRodsServerHost->localFlag);
            }
	}
	tmpZoneInfo = tmpZoneInfo->next;
    }

    if (localRcatFlag) {
        rodsLog (LOG_ERROR,
          "getRcatHost: No local Rcat"); 
	return (SYS_INVALID_ZONE_NAME);
    } else {
        rodsLog (LOG_ERROR,
          "getRcatHost: Invalid zone name from hint %s", rcatZoneHint);
	/* XXXXX take me out. use the local rcat for now */
	status = getRcatHost (rcatType, NULL, &tmpRodsServerHost);
	if (status < 0) {
            return (SYS_INVALID_ZONE_NAME);
	} else {
	    *rodsServerHost = tmpRodsServerHost;
	    return (tmpRodsServerHost->localFlag);
	}
    }
}

/* Check if there is a connected ICAT host, and if there is, disconnect */
int 
getAndDisconnRcatHost (rsComm_t *rsComm, int rcatType, char *rcatZoneHint,
rodsServerHost_t **rodsServerHost)
{
    int status;

    status = getRcatHost (rcatType, rcatZoneHint, rodsServerHost);

    if (status < 0) return(status);

    if ((*rodsServerHost)->conn != NULL) { /* a connection exists */
       status = rcDisconnect((*rodsServerHost)->conn);
       return(status);
    }
    return(0);
}

int
disconnRcatHost (rsComm_t *rsComm, int rcatType, char *rcatZoneHint)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;

    status = getRcatHost (rcatType, rcatZoneHint, &rodsServerHost);

    if (status < 0) {
        return (status);
    }

    if ((rodsServerHost)->localFlag == LOCAL_HOST) {
        return (LOCAL_HOST);
    }

    if (rodsServerHost->conn != NULL) { /* a connection exists */
        status = rcDisconnect(rodsServerHost->conn);
	rodsServerHost->conn = NULL;
    }
    if (status >= 0) {
	return (REMOTE_HOST);
    } else {
        return (status);
    }
}

int
initZone (rsComm_t *rsComm)
{
    rodsEnv *myEnv = &rsComm->myEnv;

    /* XXXXX this subroutine needs to be redone when we have the Rcat */
    /* configure the local zone */

    rodsServerHost_t *tmpRodsServerHost;

    tmpRodsServerHost = ServerHostHead;
    while (tmpRodsServerHost != NULL) {
	if (tmpRodsServerHost->rcatEnabled > 0) {
	    /* XXXXX assume zone in myEnv */
	    queZone (myEnv->rodsZone, tmpRodsServerHost);
	}
	tmpRodsServerHost = tmpRodsServerHost->next;
    }
    return (0); 
}

int
queZone (char *zoneName, rodsServerHost_t *tmpRodsServerHost) 
{
    zoneInfo_t *tmpZoneInfo, *lastZoneInfo;
    zoneInfo_t *myZoneInfo;

    myZoneInfo = (zoneInfo_t *) malloc (sizeof (zoneInfo_t));

    memset (myZoneInfo, 0, sizeof (zoneInfo_t));

    rstrcpy (myZoneInfo->zoneName, zoneName, NAME_LEN);
    myZoneInfo->rodsServerHost = tmpRodsServerHost;

    /* queue it */

    lastZoneInfo = tmpZoneInfo = ZoneInfoHead;
    while (tmpZoneInfo != NULL) {
        lastZoneInfo = tmpZoneInfo;
        tmpZoneInfo = tmpZoneInfo->next;
    }

    if (lastZoneInfo == NULL) {
        ZoneInfoHead = myZoneInfo;
    } else {
        lastZoneInfo->next = myZoneInfo;
    }
    myZoneInfo->next = NULL;

    return (0);
}

int
initResc (rsComm_t *rsComm)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int status;

    memset (&genQueryInp, 0, sizeof (genQueryInp));

    addInxIval (&genQueryInp.selectInp, COL_R_RESC_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_RESC_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_ZONE_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_TYPE_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_CLASS_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_LOC, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_VAULT_PATH, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_FREE_SPACE, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_RESC_INFO, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_RESC_COMMENT, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_CREATE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_MODIFY_TIME, 1);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
   
    clearGenQueryInp (&genQueryInp);

    if (status < 0) {
	if (status !=CAT_NO_ROWS_FOUND) {
	    rodsLog (LOG_NOTICE,
	      "initResc: rsGenQuery error, status = %d",
	      status);
	}
        return (status);
    }

    status = procAndQueRescResult (genQueryOut);

    freeGenQueryOut (&genQueryOut);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "initResc: rsGenQuery error, status = %d", status);
    }
    return (status);
}

int 
procAndQueRescResult (genQueryOut_t *genQueryOut)
{
    sqlResult_t *rescId, *rescName, *zoneName, *rescType, *rescClass;
    sqlResult_t *rescLoc, *rescVaultPath, *freeSpace, *rescInfo;
    sqlResult_t *rescComments, *rescCreate, *rescModify;
    char *tmpRescId, *tmpRescName, *tmpZoneName, *tmpRescType, *tmpRescClass;
    char *tmpRescLoc, *tmpRescVaultPath, *tmpFreeSpace, *tmpRescInfo;
    char *tmpRescComments, *tmpRescCreate, *tmpRescModify;
    int i, status;
    rodsHostAddr_t addr;
    rodsServerHost_t *tmpRodsServerHost;
    rescInfo_t *myRescInfo;

    if (genQueryOut == NULL) {
        rodsLog (LOG_NOTICE,
          "procAndQueResResult: NULL genQueryOut");
        return (0);
    }

    if ((rescId = getSqlResultByInx (genQueryOut, COL_R_RESC_ID)) == NULL) {
	rodsLog (LOG_NOTICE,
	  "procAndQueResResult: getSqlResultByInx for COL_R_RESC_ID failed");
	return (UNMATCHED_KEY_OR_INDEX);
    } 

    if ((rescName = getSqlResultByInx(genQueryOut, COL_R_RESC_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "procAndQueResResult: getSqlResultByInx for COL_R_RESC_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((zoneName = getSqlResultByInx(genQueryOut, COL_R_ZONE_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "procAndQueResResult: getSqlResultByInx for COL_R_ZONE_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((rescType = getSqlResultByInx(genQueryOut, COL_R_TYPE_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "procAndQueResResult: getSqlResultByInx for COL_R_TYPE_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((rescClass = getSqlResultByInx(genQueryOut,COL_R_CLASS_NAME))==NULL) {
        rodsLog (LOG_NOTICE,
         "procAndQueResResult: getSqlResultByInx for COL_R_CLASS_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((rescLoc = getSqlResultByInx (genQueryOut, COL_R_LOC)) == NULL) {
        rodsLog (LOG_NOTICE,
          "procAndQueResResult: getSqlResultByInx for COL_R_LOC failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((rescVaultPath = getSqlResultByInx (genQueryOut, COL_R_VAULT_PATH)) 
      == NULL) {
        rodsLog (LOG_NOTICE,
         "procAndQueResResult: getSqlResultByInx for COL_R_VAULT_PATH failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((freeSpace = getSqlResultByInx (genQueryOut, COL_R_FREE_SPACE)) == 
      NULL) {
        rodsLog (LOG_NOTICE,
         "procAndQueResResult: getSqlResultByInx for COL_R_FREE_SPACE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((rescInfo = getSqlResultByInx (genQueryOut, COL_R_RESC_INFO)) == 
      NULL) {
        rodsLog (LOG_NOTICE,
          "procAndQueResResult: getSqlResultByInx for COL_R_RESC_INFO failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((rescComments = getSqlResultByInx (genQueryOut, COL_R_RESC_COMMENT)) 
      == NULL) {
        rodsLog (LOG_NOTICE,
        "procAndQueResResult:getSqlResultByInx for COL_R_RESC_COMMENT failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((rescCreate = getSqlResultByInx (genQueryOut, COL_R_CREATE_TIME)) 
      == NULL) {
        rodsLog (LOG_NOTICE,
         "procAndQueResResult:getSqlResultByInx for COL_R_CREATE_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((rescModify = getSqlResultByInx (genQueryOut, COL_R_MODIFY_TIME)) 
      == NULL) {
        rodsLog (LOG_NOTICE,
         "procAndQueResResult:getSqlResultByInx for COL_R_MODIFY_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    for (i = 0;i < genQueryOut->rowCnt; i++) {

	tmpRescId = &rescId->value[rescId->len * i];
	tmpRescName = &rescName->value[rescName->len * i];
	tmpZoneName = &zoneName->value[zoneName->len * i];
	tmpRescType = &rescType->value[rescType->len * i];
	tmpRescClass = &rescClass->value[rescClass->len * i];
	tmpRescLoc = &rescLoc->value[rescLoc->len * i];
	tmpRescVaultPath = &rescVaultPath->value[rescVaultPath->len * i];
	tmpFreeSpace = &freeSpace->value[freeSpace->len * i];
	tmpRescInfo = &rescInfo->value[rescInfo->len * i];
	tmpRescComments = &rescComments->value[rescComments->len * i];
	tmpRescCreate = &rescCreate->value[rescCreate->len * i];
	tmpRescModify = &rescModify->value[rescModify->len * i];

	/* queue the host. XXXXX resolveHost does not deal with zone yet.
	 * need to do so */
	memset (&addr, 0, sizeof (addr));
	rstrcpy (addr.hostAddr, tmpRescLoc, LONG_NAME_LEN);
	rstrcpy (addr.rodsZone, tmpZoneName, NAME_LEN);
	status = resolveHost (&addr, &tmpRodsServerHost);
	if (status < 0) {
	    rodsLog (LOG_NOTICE,
	      "procAndQueRescResult: resolveHost error for %s",
	      addr.hostAddr);
	}
	/* queue the resource */
	myRescInfo = malloc (sizeof (rescInfo_t));
	memset (myRescInfo, 0, sizeof (rescInfo_t));
	myRescInfo->rodsServerHost = tmpRodsServerHost;
	myRescInfo->rescId = strtoll (tmpRescId, 0, 0);
	rstrcpy (myRescInfo->zoneName, tmpZoneName, NAME_LEN);
	rstrcpy (myRescInfo->rescName, tmpRescName, NAME_LEN);
	rstrcpy (myRescInfo->rescLoc, tmpRescLoc, NAME_LEN);
	rstrcpy (myRescInfo->rescType, tmpRescType, NAME_LEN);
	/* convert rescType to rescTypeInx */
	myRescInfo->rescTypeInx = getRescTypeInx (tmpRescType);
	if (myRescInfo->rescTypeInx < 0) {
	    return (myRescInfo->rescTypeInx);
	}
	rstrcpy (myRescInfo->rescClass, tmpRescClass, NAME_LEN);
	myRescInfo->rescClassInx = getRescClassInx (tmpRescClass);
        if (myRescInfo->rescClassInx < 0) {
            return (myRescInfo->rescClassInx);
        }
	rstrcpy (myRescInfo->rescVaultPath, tmpRescVaultPath, MAX_NAME_LEN);
	rstrcpy (myRescInfo->rescInfo, tmpRescInfo, LONG_NAME_LEN);
	rstrcpy (myRescInfo->rescComments, tmpRescComments, LONG_NAME_LEN);
	myRescInfo->freeSpace = strtoll (tmpFreeSpace, 0, 0);
	rstrcpy (myRescInfo->rescCreate, tmpRescCreate, TIME_LEN);
	rstrcpy (myRescInfo->rescModify, tmpRescModify, TIME_LEN);

	queResc (myRescInfo, NULL, &RescGrpInfo, 0);
    }
    return (0);
}

int 
setExecArg (char *commandArgv, char *av[])
{
    char *inpPtr, *outPtr, envBuf[MAX_NAME_LEN];
    int inx = 1;
    int c;
    int len = 0;
    int openQuote = 0;

    if (commandArgv != NULL) {
      inpPtr = strdup (commandArgv);
      outPtr = inpPtr;
      while ((c = *inpPtr) != '\0') {
        if ((c == ' ' && openQuote == 0) || openQuote == 2) {
            openQuote = 0;
            if (len > 0) {      /* end of a argv */
                *inpPtr = '\0'; /* mark the end */
                av[inx] = outPtr;
                /* reset inx and pointer */
                inpPtr++;
                outPtr = inpPtr;
                inx ++;
                len = 0;
            } else {    /* skip over blanks */
                inpPtr++;
                outPtr = inpPtr;
            }
        } else if (c == '\'' || c == '\"') {
            openQuote ++;
            if (openQuote == 1) {
                /* skip the quote */
                inpPtr++;
                outPtr = inpPtr;
            }
        } else {
            len ++;
            inpPtr++;
        }
      }
      if (len > 0) {      /* handle the last argv */
        av[inx] = outPtr;
        inx++;
      }
    }

    av[inx] = NULL;     /* terminate with a NULL */

    return (0);
}

int
initAgent (rsComm_t *rsComm)
{
    int status;
    rsComm_t myComm;
    ruleExecInfo_t rei;

    status = initServerInfo (rsComm);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "initAgent: initServerInfo error, status = %d",
          status);
        return (status);
    }

    initL1desc ();
    initSpecCollDesc ();
    status = initFileDesc ();
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "initAgent: initFileDesc error, status = %d",
          status);
        return (status);
    }

    status = initRuleEngine(reRuleStr, reFuncMapStr, reVariableMapStr);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "initAgent: initRuleEngine error, status = %d", status);
        return(status);
    }

    memset (&rei, 0, sizeof (rei));
    rei.rsComm = rsComm;

    if (ProcessType == AGENT_PT) {
        status = applyRule ("acChkHostAccessControl", NULL, &rei, 
	  NO_SAVE_REI);

        if (status < 0) {
            rodsLog (LOG_ERROR,
              "initAgent: acChkHostAccessControl error, status = %d",
              status);
            return (status);
	}
    }

    srandom((unsigned int) time(0) % getpid());

    if (rsComm->reconnFlag > 0 && getenv ("svrPortReconnect") != NULL) { 
	rsComm->reconnSock = sockOpenForInConn (rsComm, &rsComm->reconnPort,
	  &rsComm->reconnAddr);
	if (rsComm->reconnSock < 0) {
	    rsComm->reconnPort = 0;
	    rsComm->reconnAddr = NULL;
        } else {
	    rsComm->cookie = random ();
	}
    }

    InitialState = INITIAL_DONE;
    ThisComm = rsComm;

    /* use a tmp myComm is needed to get the permission right for the call */
    myComm = *rsComm;
    myComm.clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
    rei.rsComm = &myComm;

    status = applyRule ("acSetPublicUserPolicy", NULL, &rei, NO_SAVE_REI);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "initAgent: acSetPublicUserPolicy error, status = %d",
          status);
        return (status);
    }

    return (status);
}

void
cleanupAndExit (int status)
{
    rodsLog (LOG_NOTICE,
      "Agent exiting with status = %d", status);

#ifdef RODS_CAT
    disconnectRcat (ThisComm);
#endif

    if (InitialState == INITIAL_DONE) {
	/* close all opened descriptors */
	closeAllL1desc (ThisComm);
        /* close any opened server to server connection */
	disconnectAllSvrToSvrConn ();
    }


    if (status >= 0) {
        exit (0);
    } else {
        exit (1);
    }
}

void
signalExit ()
{
    rodsLog (LOG_NOTICE,
     "caught a signal and exiting\n");
    cleanupAndExit (SYS_CAUGHT_SIGNAL);
}

void
rsPipSigalHandler ()
{
    if (ThisComm == NULL || ThisComm->reconnSock <= 0) {
	rodsLog (LOG_NOTICE,
         "caught a broken pipe signal and exiting");
        cleanupAndExit (SYS_CAUGHT_SIGNAL);
    } else {
	int status;

	rodsLog (LOG_NOTICE,
         "caught a broken pipe signal. Attempt to reconnect");
#ifndef _WIN32
	status = svrReconnect (ThisComm);
	if (status >= 0) {
            rodsLog (LOG_NOTICE,
             "rsPipSigalHandler: reconnected");
	} else {
            rodsLog (LOG_ERROR,
             "rsPipSigalHandler: reconnect failed, existing");
            cleanupAndExit (SYS_CAUGHT_SIGNAL);
	}

        signal(SIGPIPE, (void (*)(int)) rsPipSigalHandler);
#endif

    }
}

int
initHostConfigByFile (rsComm_t *rsComm)
{
    FILE *fptr;
    char *hostCongFile;
    char inbuf[MAX_NAME_LEN];
    char hostBuf[LONG_NAME_LEN];
    rodsServerHost_t *tmpRodsServerHost;
    int lineLen, bytesCopied, remoteFlag;
    int status;

    hostCongFile =  (char *) malloc((strlen (getConfigDir()) +
        strlen(HOST_CONFIG_FILE) + 24));

    sprintf (hostCongFile, "%-s/%-s", getConfigDir(), HOST_CONFIG_FILE);

    fptr = fopen (hostCongFile, "r");

    if (fptr == NULL) {
        rodsLog (LOG_NOTICE,
          "Cannot open HOST_CONFIG_FILE  file %s. ernro = %d\n",
          hostCongFile, errno);
        free (hostCongFile);
        return (SYS_CONFIG_FILE_ERR);
    }

    free (hostCongFile);

    while ((lineLen = getLine (fptr, inbuf, MAX_NAME_LEN)) > 0) {
        char *inPtr = inbuf;
	int cnt = 0;
        while ((bytesCopied = getStrInBuf (&inPtr, hostBuf,
          &lineLen, LONG_NAME_LEN)) > 0) {
	    if (cnt == 0) {
		/* first host */
		tmpRodsServerHost = malloc (sizeof (rodsServerHost_t));
                memset (tmpRodsServerHost, 0, sizeof (rodsServerHost_t));
		status = queRodsServerHost (&HostConfigHead, tmpRodsServerHost);

	    }
	    cnt ++;
	    if (strcmp (hostBuf, "localhost") == 0) {
		tmpRodsServerHost->localFlag = LOCAL_HOST;
	    } else {
                status = queHostName (tmpRodsServerHost, hostBuf, 0);
	    }
        }
    }
    fclose (fptr);
    return (0);
}

int
matchHostConfig (rodsServerHost_t *myRodsServerHost)
{
    rodsServerHost_t *tmpRodsServerHost;
    int status;

    if (myRodsServerHost == NULL)
	return (0);

    if (myRodsServerHost->localFlag == LOCAL_HOST) {
        tmpRodsServerHost = HostConfigHead;
	while (tmpRodsServerHost != NULL) {
	    if (tmpRodsServerHost->localFlag == LOCAL_HOST) {
		status = queConfigName (tmpRodsServerHost, myRodsServerHost);
		return (status);
	    }
	    tmpRodsServerHost = tmpRodsServerHost->next;
	}
    } else {
        tmpRodsServerHost = HostConfigHead;
        while (tmpRodsServerHost != NULL) {
	    hostName_t *tmpHostName, *tmpConfigName;

            if (tmpRodsServerHost->localFlag == LOCAL_HOST &&
	      myRodsServerHost->localFlag != UNKNOWN_HOST_LOC) {
		tmpRodsServerHost = tmpRodsServerHost->next;
		continue;
	    }
	
            tmpConfigName = tmpRodsServerHost->hostName;
            while (tmpConfigName != NULL) {
                tmpHostName = myRodsServerHost->hostName;
                while (tmpHostName != NULL) {
		    if (strcmp (tmpHostName->name, tmpConfigName->name) == 0) {
		        status = queConfigName (tmpRodsServerHost,
		          myRodsServerHost);
                        return (0);
		    }
		    tmpHostName = tmpHostName->next;
		}
		tmpConfigName = tmpConfigName->next;
            }
            tmpRodsServerHost = tmpRodsServerHost->next;
        }
    }
	
    return (0);
}

int
queConfigName (rodsServerHost_t *configServerHost, 
rodsServerHost_t *myRodsServerHost)
{ 
    hostName_t *tmpHostName = configServerHost->hostName;
    int cnt = 0;

    while (tmpHostName != NULL) {
        if (cnt == 0) {
            /* queue the first one on top */
            queHostName (myRodsServerHost, tmpHostName->name, 1);
        } else {
            queHostName (myRodsServerHost, tmpHostName->name, 0);
        }
        cnt ++;
        tmpHostName = tmpHostName->next;
    }

    return (0);
}

int
disconnectAllSvrToSvrConn ()
{
    int status;
    rodsServerHost_t *tmpRodsServerHost;

    /* check if host exist */

    tmpRodsServerHost = ServerHostHead;
    while (tmpRodsServerHost != NULL) {
	if (tmpRodsServerHost->conn != NULL) {
	    rcDisconnect (tmpRodsServerHost->conn);
	}
	tmpRodsServerHost = tmpRodsServerHost->next;
    }
    return (0);
}
	 
int
svrReconnect (rsComm_t *rsComm)
{
    fd_set sockMask;
    int numSock;
    int newSock;
    struct timeval reconnTimeout;
    reconnMsg_t *reconnMsg;
    int status;

    if (rsComm == NULL || rsComm->reconnSock <= 0) {
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (rsComm->reconnTime > 0 && time (0) <= rsComm->reconnTime + 10) {
	rodsLog (LOG_NOTICE, 
	  "svrReconnect: reconnection done recently - no action"); 
	return (0);
    }
    irodsCloseSock (rsComm->sock);

    listen (rsComm->reconnSock, MAX_LISTEN_QUE);

    FD_ZERO(&sockMask);
    FD_SET(rsComm->reconnSock, &sockMask);

    rodsLog (LOG_NOTICE, "rodsAgent reconnecting on sock %d port %d",
      rsComm->reconnSock, rsComm->reconnPort);

    memset (&reconnTimeout, 0, sizeof (reconnTimeout));
   reconnTimeout.tv_sec = RECONNECT_TIMEOUT_TIME;

#ifndef _WIN32
    /* ignore SIGPIPE because it probably would get one */
    signal(SIGPIPE, SIG_IGN);
#endif
    if ((numSock = select (rsComm->reconnSock + 1, &sockMask,
      (fd_set *) NULL, (fd_set *) NULL,
      (struct timeval *) &reconnTimeout)) <= 0) {
        irodsCloseSock (rsComm->reconnSock);
	rsComm->reconnSock = 0;
        rodsLog (LOG_NOTICE, "rodsAgent No reconnection");
        return (SYS_SOCK_OPEN_ERR);
    }
#ifndef _WIN32
    /* put back SIGPIPE */
    signal(SIGPIPE, (void (*)(int)) rsPipSigalHandler);
#endif

    rsComm->sock = rsComm->reconnSock;
    newSock = rsAcceptConn (rsComm);

    if (newSock > 0) {
        rsComm->sock = newSock;
#if 0	/* use cookie instead */
        /* have to re-authenticate */
        rsComm->proxyUser.authInfo.authFlag =
        rsComm->clientUser.authInfo.authFlag = 0;
#endif
        if ((status = readReconMsg (newSock, &reconnMsg)) < 0) {
            rodsLog (LOG_ERROR,
              "svrReconnect: readReconMsg error, status = %d", status);
            return (status);
	} else if (reconnMsg->cookie != rsComm->cookie) {
            rodsLog (LOG_ERROR,
	    "svrReconnect: cookie mistach, got = %d vs %d", 
	      reconnMsg->cookie, rsComm->cookie);
	    free (reconnMsg);
	    status = SYS_PORT_COOKIE_ERR;
	}

	if (status < 0) {
	    return status;
	}
	rsComm->reconnOpr = reconnMsg->reconnOpr;
	rsComm->reconnTime = time (0);
    }
    return (newSock);
}

