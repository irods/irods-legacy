/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***
 *****************************************************************************/

#ifndef DBO_HIGHLEVEL_ROUTINES_H
#define DBO_HIGHLEVEL_ROUTINES_H

int dboOpen(char *dboName);
int dboReadConfigItems(char *dboList, int maxChars);
int dboGetInfo(int fd, char *outBuf, int maxOutBuf);
int dboExecute(rsComm_t *rsComm, int fd, char *dboName, char *outBuf,
	       int maxOutBuf);
/*
int dboClose();
int dboCommit();
int dboRollback();
int dboIsConnected();
int dboSqlNoResults(char *sql, char *parm[], int nparms);
int dboSqlWithResults(char *sql, char *parm[], int nparms, char **outBuf);
int dboCheckAccess(char *dboName, rsComm_t *rsComm);

int dboDebug(char *debugMode);
*/

#endif /* DBO_HIGHLEVEL_ROUTINES_H */
