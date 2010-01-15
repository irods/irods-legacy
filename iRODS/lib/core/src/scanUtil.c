/*** Copyright (c), CCIN2P3            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* Written by Jean-Yves Nief of CCIN2P3 */

#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"
#include "scanUtil.h"
#include "miscUtil.h"

int
scanObj (rcComm_t *conn, rodsArguments_t *myRodsArgs, rodsPathInp_t *rodsPathInp, char hostname[LONG_NAME_LEN])
{
	char inpPath[LONG_NAME_LEN] = "";
	char *inpPathO, *lastChar;
	struct stat sbuf;
	int lenInpPath, status;
	
	if ( rodsPathInp->numSrc != 1 ) {
		rodsLog (LOG_ERROR, "scanObj: gave %i input source path, should give one and only one", rodsPathInp->numSrc);
		status = USER_INPUT_PATH_ERR;
	}
	else {
		inpPathO = rodsPathInp->srcPath[0].outPath;
		status = lstat(inpPathO, &sbuf);
		if ( status == 0 ) {
			/* remove any trailing "/" from inpPathO */
			lenInpPath = strlen(inpPathO);
			lastChar = strrchr(inpPathO, '/');
			if ( strlen(lastChar) == 1 ) {
				lenInpPath = lenInpPath - 1;
			}
			strncpy(inpPath, inpPathO, lenInpPath);
			if ( S_ISDIR(sbuf.st_mode) == 1 ) {   /* check if it is not included into a mounted collection */
				status = checkIsMount(conn, inpPath);
				if ( status != 0 ) {  /* if it is part of a mounted collection, abort */
					printf("The directory %s or one of its subdirectories to be scanned is declared as being used for a mounted collection: abort!\n", inpPath);
					return (status);
				}
			}
			status = scanObjDir(conn, myRodsArgs, inpPath, hostname);
		}
		else {
			status = USER_INPUT_PATH_ERR;
			rodsLog (LOG_ERROR, "scanObj: %s does not exist", inpPath);
		}
	}
	
	return (status);
}

int 
scanObjDir (rcComm_t *conn, rodsArguments_t *myRodsArgs, char *inpPath, char *hostname)
{
	DIR *dirPtr;
	struct dirent *myDirent;
	struct stat sbuf;
	int status;
	char fullPath[LONG_NAME_LEN] = "\0";
	
	dirPtr = opendir (inpPath);
	/* check if it is a directory */
	lstat(inpPath, &sbuf);
	if ( S_ISDIR(sbuf.st_mode) == 1 ) {
		if ( dirPtr == NULL ) {
			return (-1);
		}
	}
	else {
		status = chkObjExist(conn, inpPath, hostname);
		return (status);
	}
	
	while ( myDirent = readdir (dirPtr) ) {
        if ( strcmp(myDirent->d_name, ".") == 0 || strcmp(myDirent->d_name, "..") == 0 ) {
			continue;
        }
        strcpy(fullPath, inpPath);
        strcat(fullPath, "/");
        strcat(fullPath, myDirent->d_name);
        lstat(fullPath, &sbuf);
		if ( S_ISDIR(sbuf.st_mode) == 1 ) {
			if ( myRodsArgs->recursive == True ) {
				status = scanObjDir(conn, myRodsArgs, fullPath, hostname);
			}
        }
        else {
			status = chkObjExist(conn, fullPath, hostname);
        }
	}
   
   return (status);
	
}

int
chkObjExist (rcComm_t *conn, char *inpPath, char *hostname)
{
	int status;
	genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN];

	memset (&genQueryInp, 0, sizeof (genQueryInp));
	addInxIval(&genQueryInp.selectInp, COL_D_DATA_ID, 1);
	genQueryInp.maxRows = 0;
	
	snprintf (condStr, MAX_NAME_LEN, "='%s'", inpPath);
    addInxVal (&genQueryInp.sqlCondInp, COL_D_DATA_PATH, condStr);
    snprintf (condStr, MAX_NAME_LEN, "like '%s%s' || ='%s'", hostname, "%", hostname);
	addInxVal (&genQueryInp.sqlCondInp, COL_R_LOC, condStr);
	
	status =  rcGenQuery (conn, &genQueryInp, &genQueryOut);
	if (status == CAT_NO_ROWS_FOUND) {
	    printf ("%s tagged as orphan file\n", inpPath);
	}
	
	clearGenQueryInp(&genQueryInp);
	freeGenQueryOut(&genQueryOut);
	
	return (status);
	
}

int 
checkIsMount (rcComm_t *conn, char *inpPath)
{
	int i, minLen, status, status1;
	genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN], *dirMPath;
	
	memset (&genQueryInp, 0, sizeof (genQueryInp));
	addInxIval(&genQueryInp.selectInp, COL_COLL_INFO1, 1);
	genQueryInp.maxRows = MAX_SQL_ROWS;

	snprintf (condStr, MAX_NAME_LEN, "='%s'", "mountPoint");
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_TYPE, condStr);
	
	status1 = rcGenQuery (conn, &genQueryInp, &genQueryOut);
	if (status1 == CAT_NO_ROWS_FOUND) {       
		status = 0;  /* there is no mounted collection, so no potential problem */
	}
	else {  /* check if inpPath is part of one of the mounted collections */
		status = 0;
		for (i = 0; i < genQueryOut->rowCnt; i++) {
			dirMPath = genQueryOut->sqlResult[0].value;
			dirMPath += i*genQueryOut->sqlResult[0].len;
			if ( strlen(dirMPath) <= strlen(inpPath) ) {
				minLen = strlen(dirMPath);
			}
			else {
				minLen = strlen(inpPath);
			}
			if (strncmp(dirMPath, inpPath, minLen) == 0) { 
				status = -1;
			}
		}
	}
	
	clearGenQueryInp(&genQueryInp);
	freeGenQueryOut(&genQueryOut);
	
	return (status);
	
}
