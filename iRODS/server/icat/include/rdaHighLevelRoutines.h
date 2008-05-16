/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***
 *****************************************************************************/

#ifndef RDA_HIGHLEVEL_ROUTINES_H
#define RDA_HIGHLEVEL_ROUTINES_H

//#include "objInfo.h"
//#include "ruleExecSubmit.h"
//#include "rcConnect.h"
//#include "rodsGeneralUpdate.h"

//int rdaOpen(char *DBUser, char *DBpasswd);
int rdaOpen(char *rdaName);
int rdaClose();
int rdaIsConnected();
int rdaSql(char *sql, char *parm[], int nparms);
int rdaSqlWithResults(char *sql, char *parm[], int nparms, char **outBuf);
int rdaCheckAccess(char *rdaName, rsComm_t *rsComm);

int rdaDebug(char *debugMode);

#endif /* RDA_HIGHLEVEL_ROUTINES_H */
