/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* auditMS.h - header file for auditMS.c
 */

#ifndef AUDITMS_H
#define AUDITMS_H

#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "miscUtil.h"

#define AUDIT_TRAIL_FILENAME	"auditLog.xml"


int logClientInfoForAuditTrail(msParam_t *inpParam, ruleExecInfo_t *rei);


#endif	/* AUDITMS_H */


