/*** Copyright (c), The Unregents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* execCmd.h
 */

#ifndef EXEC_CMD_H
#define EXEC_CMD_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"

#define CMD_DIR		"cmd"
typedef struct {
    char cmd[LONG_NAME_LEN];
    char cmdArgv[MAX_NAME_LEN];
    char execAddr[LONG_NAME_LEN];	/* if non empty, exec at this addr */
    char hintPath[MAX_NAME_LEN];	/* exec where is file is located */
    int addPathToArgv;			/* whether to add the resolved phy
					 * path to the argv. */
    keyValPair_t condInput;
} execCmd_t;

typedef struct {
    bytesBuf_t stdoutBuf;
    bytesBuf_t stderrBuf;
    int status;	    /* XXXXXX have to move status to back for 64 bit addr */
} execCmdOut_t;

#define ExecCmd_PI "str cmd[LONG_NAME_LEN]; str cmdArgv[MAX_NAME_LEN]; str execAddr[LONG_NAME_LEN]; str hintPath[MAX_NAME_LEN]; int addPathToArgv; struct KeyValPair_PI;"
#define ExecCmdOut_PI "struct BinBytesBuf_PI; struct BinBytesBuf_PI; int status;" 

#if defined(RODS_SERVER)
#define RS_EXEC_CMD rsExecCmd
/* prototype for the server handler */
int
rsExecCmd (rsComm_t *rsComm, execCmd_t *execCmdInp, 
execCmdOut_t **execCmdOut);
int
_rsExecCmd (rsComm_t *rsComm, execCmd_t *execCmdInp, 
execCmdOut_t **execCmdOut);
int
remoteExecCmd (rsComm_t *rsComm, execCmd_t *execCmdInp,
execCmdOut_t **execCmdOut, rodsServerHost_t *rodsServerHost);
int
execCmd (execCmd_t *execCmdInp, int stdOutFd, int stdErrFd);
int
initCmdArg (char *av[], char *cmdArgv, char *cmdPath);
int
readToByteBuf (int fd, bytesBuf_t *bytesBuf);
#else
#define RS_EXEC_CMD NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* prototype for the client call */
int
rcExecCmd (rcComm_t *conn, execCmd_t *execCmdInp, execCmdOut_t **execCmdOut);

/* rcExecCmd - Execute a command on the server.
 * Input -
 *   rcComm_t *conn - The client connection handle.
 *   execCmd_t *execCmdInp - the execCmd input
 *
 * OutPut -
 *   bytesBuf_t *cmdOutBBuf - The stdout and stderr of the command is stored
 *    int this bytesBuf.
 *   int status - status of the operation.
 */

#ifdef  __cplusplus
}
#endif

#endif	/* EXEC_CMD_H */
