/*** Copyright (c), CCIN2P3            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* The module is written by Jean-Yves Nief of CCIN2P3 */

#include "reIn2p3SysRule.h"
#include "genQuery.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int checkIPaddress(char *IP, unsigned char IPcomp[IPV4]) {

  const char *delimIP = ".";
  char *eltstrIP, *IPclone;
  int i, nelt = 0;
  
  strcpy(IPclone, IP);
  i = atoi(strtok(IPclone, delimIP));
  if ( i < 0 || i > 255 ) return -1;
  IPcomp[0] = i;
  while ( (eltstrIP = strtok(NULL, delimIP)) ) {
    nelt++;
    i = atoi(eltstrIP);
    if ( i < 0 || i > 255 ) return -1;
    IPcomp[nelt] = i;
  }
  if ( (nelt + 1) != IPV4 ) return -1;
  return (0);
	
}

int checkHostAccessControl (char *username, char *hostclient, 
			    char *groupsname)
{
  
  char *configDir, hostControlAccessFile[LONG_NAME_LEN];
  char grouplist[MAX_SQL_ROWS][MAXSTR];
  const char *delim = " \t\n";
  int groupok, i, indxc, iok, nelt;
  char line[MAXLEN], *eltstr, tempArr[NFIELDS][MAXLEN];
  unsigned char result, IPEntry[IPV4], subnetEntry[IPV4], visitorIP[IPV4];
  FILE *fp;
  
  /* try to open the HostControlAccess if it exists. */
  configDir = getConfigDir ();
  snprintf (hostControlAccessFile, LONG_NAME_LEN, "%s/%s", configDir, 
	    HOST_ACCESS_CONTROL_FILE); 
  fp = fopen(hostControlAccessFile, "r");
  if (fp == NULL) {
    rodsLog (LOG_NOTICE,
	     "hostAuthCheck: can't open HostControlAccess file %s", hostControlAccessFile);
    return (UNIX_FILE_OPEN_ERR - errno);
  }
  /* parse the list of groups for the user from the groupsname char */
  nelt = 0;
  strncpy(grouplist[0], strtok(groupsname, delim), MAXSTR);
  while ( (eltstr = strtok(NULL, delim)) ) {
    nelt++;
    strncpy(grouplist[nelt], eltstr, MAXSTR);
  }
  /* parse HostControlAccess and check if <user,IP,group> is allowed to access this server. */
  while ( !feof(fp) ) {
    indxc = 0;
    if( fgets(line, MAXLEN, fp) ) {
      if ( line[0] != '#' && line[0] != '\n' ) {  /* Comment or empty line, ignore */
	eltstr = strtok(line, delim);
	strncpy(tempArr[indxc], eltstr, MAXSTR);
	while ( (eltstr = strtok(NULL, delim)) ) {
	  indxc++;
	  strncpy(tempArr[indxc], eltstr, MAXSTR);
	}
	if ( (indxc+1) == NFIELDS && checkIPaddress(tempArr[2], IPEntry) == 0 &&
	     checkIPaddress(tempArr[3], subnetEntry) == 0 && 
	     checkIPaddress(hostclient, visitorIP) == 0 ) {
	  /* check through if one of the group does correspond to the one allowed */
	  groupok = 1;
	  for ( i = 0; i <= nelt; i++ ) {
	    if ( strcmp(tempArr[1], grouplist[i]) == 0 ) {
	      groupok = 0;
	      break;
	    }
	  }
	  if ( strcmp(tempArr[1], "all") == 0 || groupok == 0 ) {
	    if ( strcmp(tempArr[0], "all") == 0 || strcmp(tempArr[0], username) == 0 ) {
	      iok = 1;
	      /* check if <client, group, clientIP> match this entry of the control access file
		 (iok=1). Get out immediatly from this function: client is allowed to proceed. */
	      for ( i = 0; i < IPV4; i++ ) {
		result = ~( visitorIP[i]  ^ IPEntry[i] ) | subnetEntry[i];
		if ( result != 255 ) {
		  iok = 0; 
		}
	      }
	      if ( iok == 1 ) { return (0); } 
	    } 
	  }
        }
      }
    }
  }
  return (-1);
}  

int msiCheckHostAccessControl (ruleExecInfo_t *rei) {

  char group[MAX_NAME_LEN], *hostclient, *result, *username;
  char condstr[MAX_NAME_LEN]; /* group[MAX_SQL_ROWS][MAXSTR]; */
  int i, rc, status;
  genQueryInp_t genQueryInp;
  genQueryOut_t *genQueryOut = NULL;
  rsComm_t *rsComm;
  
  RE_TEST_MACRO ("    Calling checkHostAccessControl")
  /* the above line is needed for loop back testing using irule -i option */
 
  group[0] = '\0';
  /* retrieve user name */
  username = rei->rsComm->clientUser.userName;
  /* retrieve client IP address */
  hostclient = inet_ntoa(rei->rsComm->remoteAddr.sin_addr);
  /* retrieve groups to which the user belong */
  memset(&genQueryInp, 0, sizeof (genQueryInp));
  snprintf(condstr, MAX_NAME_LEN, "= '%s'", username);
  addInxVal(&genQueryInp.sqlCondInp, COL_USER_NAME, condstr);
  addInxIval(&genQueryInp.selectInp, COL_USER_GROUP_NAME, 1);
  genQueryInp.maxRows = MAX_SQL_ROWS;
  rsComm = rei->rsComm;
  status =  rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
  if ( status >= 0 ) {
    for (i=0; i<genQueryOut->rowCnt; i++) {
      result = genQueryOut->sqlResult[0].value;
      result += i*genQueryOut->sqlResult[0].len;
      strcat(group, result);
      strcat(group, " ");
    }
  } else {
    rstrcpy(group, "all", MAX_NAME_LEN);
  } 
  clearGenQueryInp(&genQueryInp);
  freeGenQueryOut(&genQueryOut);
  
  rc = checkHostAccessControl(username, hostclient, group);
  if ( rc < 0 ) {
    rodsLog (LOG_NOTICE, "Access to user %s from host %s has been refused.\n", username, hostclient);
  }
  return (rc);
  
}
