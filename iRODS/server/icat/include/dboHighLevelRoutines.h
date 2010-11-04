/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***
 *****************************************************************************/

#ifndef DBO_HIGHLEVEL_ROUTINES_H
#define DBO_HIGHLEVEL_ROUTINES_H

int dbrOpen(char *dbrName);
int dboReadConfigItems(char *dboList, int maxChars);
int dboGetInfo(int fd, char *outBuf, int maxOutBuf);
int dboExecute(rsComm_t *rsComm, char *dbrName, char *dboName, char *outBuf,
	       int maxOutBuf);
int dbrClose(char *dbrName);
int dbrCommit(rsComm_t *rsComm, char *dbrName);
int dbrRollback(rsComm_t *rsComm, char *dbrName);

/*
int dboIsConnected();
int dboSqlNoResults(char *sql, char *parm[], int nparms);
int dboSqlWithResults(char *sql, char *parm[], int nparms, char **outBuf);
int dboCheckAccess(char *dboName, rsComm_t *rsComm);

int dboDebug(char *debugMode);
*/

#endif /* DBO_HIGHLEVEL_ROUTINES_H */
