/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
 /* The module is written by Jean-Yves Nief of CCIN2P3 */
 
#include "univMSSDriver.h"

/* univMSSSyncToArch - This function is for copying the file from cacheFilename to filename in the MSS. 
 * optionalInfo info is not used.
 */

int univMSSSyncToArch (rsComm_t *rsComm, fileDriverType_t cacheFileType, 
				   int mode, int flags, char *filename,
				   char *cacheFilename,  rodsLong_t dataSize, keyValPair_t *condInput) {
				   
    int lenDir, rc, status;
	execCmd_t execCmdInp;
	char *lastpart;
	char cmdArgv[MAX_NAME_LEN] = "";
	char dirname[MAX_NAME_LEN] = "";
	execCmdOut_t *execCmdOut = NULL;
	
	/* first create the directory on the target MSS */
	lastpart = strrchr(filename, '/');
	lenDir = strlen(filename) - strlen(lastpart);
	strncpy(dirname, filename, lenDir);
	mode = DEFAULT_ACL;   /* to be modified in the near future */
	status = univMSSFileMkdir (rsComm, dirname, mode);
	if ( status == 0 ) {
		rstrcpy(execCmdInp.cmd, UNIV_MSS_INTERF_SCRIPT, LONG_NAME_LEN);
		strcat(cmdArgv, "syncToArch");
		strcat(cmdArgv, " ");
		strcat(cmdArgv, cacheFilename);
		strcat(cmdArgv, " ");
		strcat(cmdArgv, filename);
		rstrcpy(execCmdInp.cmdArgv, cmdArgv, MAX_NAME_LEN);
		rstrcpy(execCmdInp.execAddr, "localhost", LONG_NAME_LEN);
		status = _rsExecCmd(rsComm, &execCmdInp, &execCmdOut);
		if ( status == 0 ) {
			rc = univMSSFileChmod (rsComm, filename, mode);
		}
		else {
			status = UNIV_MSS_SYNCTOARCH_ERR - errno;
			rodsLog (LOG_ERROR, "univMSSSyncToArch: copy of %s to %s failed, status = %d",
					cacheFilename, filename, status);
		}
	}
    return (status);
}

/* univMSSStageToCache - This function is to stage filename (stored in the MSS) to cacheFilename. 
 * optionalInfo info is not used.
 */
int univMSSStageToCache (rsComm_t *rsComm, fileDriverType_t cacheFileType, 
				     int mode, int flags, char *filename,
				     char *cacheFilename,  rodsLong_t dataSize, keyValPair_t *condInput) {
				   
    int status;
	execCmd_t execCmdInp;
	char cmdArgv[MAX_NAME_LEN] = "";
	execCmdOut_t *execCmdOut = NULL;
	
	rstrcpy(execCmdInp.cmd, UNIV_MSS_INTERF_SCRIPT, LONG_NAME_LEN);
	strcat(cmdArgv, "stageToCache");
	strcat(cmdArgv, " ");
	strcat(cmdArgv, filename);
	strcat(cmdArgv, " ");
	strcat(cmdArgv, cacheFilename);
	rstrcpy(execCmdInp.cmdArgv, cmdArgv, MAX_NAME_LEN);
	rstrcpy(execCmdInp.execAddr, "localhost", LONG_NAME_LEN);
	status = _rsExecCmd(rsComm, &execCmdInp, &execCmdOut);
	
	if (status < 0) {
        status = UNIV_MSS_STAGETOCACHE_ERR - errno; 
		rodsLog (LOG_ERROR, "univMSSStageToCache: staging from %s to %s failed, status = %d",
         filename, cacheFilename, status);
    }
	
    return (status);
	
}

/* univMSSFileUnlink - This function is to remove a file stored in the MSS. 
 */
int univMSSFileUnlink (rsComm_t *rsComm, char *filename) {
    
	int status;
	execCmd_t execCmdInp;
	char cmdArgv[MAX_NAME_LEN] = "";
	execCmdOut_t *execCmdOut = NULL;
	
	rstrcpy(execCmdInp.cmd, UNIV_MSS_INTERF_SCRIPT, LONG_NAME_LEN);
	strcat(cmdArgv, "rm");
	strcat(cmdArgv, " ");
	strcat(cmdArgv, filename);
	rstrcpy(execCmdInp.cmdArgv, cmdArgv, MAX_NAME_LEN);
	rstrcpy(execCmdInp.execAddr, "localhost", LONG_NAME_LEN);
	status = _rsExecCmd(rsComm, &execCmdInp, &execCmdOut);

    if (status < 0) {
        status = UNIV_MSS_UNLINK_ERR - errno;
		rodsLog (LOG_ERROR, "univMSSFileUnlink: unlink of %s error, status = %d",
         filename, status);
    }

    return (status);
}

/* univMSSFileMkdir - This function is to create a directory in the MSS. 
 */
int univMSSFileMkdir (rsComm_t *rsComm, char *dirname, int mode) {
	
	int status;
	execCmd_t execCmdInp;
	char cmdArgv[MAX_NAME_LEN] = "";
	execCmdOut_t *execCmdOut = NULL;  

	rstrcpy(execCmdInp.cmd, UNIV_MSS_INTERF_SCRIPT, LONG_NAME_LEN);
	strcat(cmdArgv, "mkdir");
	strcat(cmdArgv, " ");
	strcat(cmdArgv, dirname);
	rstrcpy(execCmdInp.cmdArgv, cmdArgv, MAX_NAME_LEN);
	rstrcpy(execCmdInp.execAddr, "localhost", LONG_NAME_LEN);
	status = _rsExecCmd(rsComm, &execCmdInp, &execCmdOut);
	
	if (status < 0) {
		status = UNIV_MSS_MKDIR_ERR - errno;
		rodsLog (LOG_NOTICE, "univMSSFileMkdir: cannot create directory for %s error, status = %d",
		         dirname, status);
    }
	
	status = univMSSFileChmod(rsComm, dirname, mode);
	
	return (status);
}

/* univMSSFileChmod - This function is to change ACL for a directory a file in the MSS. 
 */
int univMSSFileChmod (rsComm_t *rsComm, char *name, int mode) {
	
	int status;
	execCmd_t execCmdInp;
	char cmdArgv[MAX_NAME_LEN] = "";
	char strmode[4];
	execCmdOut_t *execCmdOut = NULL;  
	
	rstrcpy(execCmdInp.cmd, UNIV_MSS_INTERF_SCRIPT, LONG_NAME_LEN);
	strcat(cmdArgv, "chmod");
	strcat(cmdArgv, " ");
	strcat(cmdArgv, name);
	strcat(cmdArgv, " ");
	sprintf (strmode, "%i", mode);
	strcat(cmdArgv, strmode);
	rstrcpy(execCmdInp.cmdArgv, cmdArgv, MAX_NAME_LEN);
	rstrcpy(execCmdInp.execAddr, "localhost", LONG_NAME_LEN);
	status = _rsExecCmd(rsComm, &execCmdInp, &execCmdOut);
	
	if (status < 0) {
		status = UNIV_MSS_CHMOD_ERR - errno;
		rodsLog (LOG_NOTICE, "univMSSFileChmod: cannot chmod for %s, status = %d",
		         name, status);
    }
	
	return (status);
}
