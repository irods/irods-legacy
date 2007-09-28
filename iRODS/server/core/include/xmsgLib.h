/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* xmsgLib.h - header file for xmsgLib
 */



#ifndef XMSG_LIB_H
#define XMSG_LIB_H

#include "rods.h"
#include "rsGlobalExtern.h"   /* server global */
#include "rcGlobalExtern.h"     /* client global */
#include "rsLog.h" 
#include "rodsLog.h"
#include "sockComm.h"
#include "rsMisc.h"
#include "getRodsEnv.h"
#include "rcConnect.h"
#include "initServer.h"
#include "irodsXmsgServer.h"

int
addXmsgToQue (irodsXmsg_t *xmsg, xmsgQue_t *xmsgQue);
int
rmXmsgStructFromQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue);

#endif	/* XMSG_LIB_H */

