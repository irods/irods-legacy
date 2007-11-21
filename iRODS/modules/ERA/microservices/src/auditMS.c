/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "auditMS.h"




/*
 * records client user info
 *
 */
int
logClientInfoForAuditTrail(msParam_t *inpParam, ruleExecInfo_t *rei)
{
	char *at_file = NULL, *label;
	char tStr0[TIME_LEN], tStr[TIME_LEN];
	FILE *auditOut;
	
	
	/* routine checks */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "logClientInfoForAuditTrail: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	

	/* Get action type from input param */
	label = parseMspForStr(inpParam);
	
	
	/* Open audit trail file */
	getLogfileName (&at_file, NULL, AUDIT_TRAIL_FILENAME);

	auditOut = fopen (at_file, "a");
	if (auditOut == NULL) {
		rodsLog (LOG_ERROR, "logClientInfoForAuditTrail: Could not open %s. errno = %d", at_file, errno);
		return (FILE_OPEN_ERR);
	}
	
	
	/* Get local system time */
	getNowStr(tStr0);
	getLocalTimeFromRodsTime(tStr0, tStr);
	
	
	/* Write info to audit trail file */
	fprintf(auditOut, "<action>\n");
	
	fprintf(auditOut, "<date>%s</date>\n", tStr);
	fprintf(auditOut, "<cuser>%s</cuser>\n", rei->uoic->userName);
	fprintf(auditOut, "<puser>%s</puser>\n", rei->uoip->userName);
	fprintf(auditOut, "<operation>%s</operation>\n", label);
	fprintf(auditOut, "<target>%s</target>\n", rei->doi->objPath);
	
	fprintf(auditOut, "</action>\n");
	
	fclose(auditOut);
	
	
	/* done */
	return (rei->status);

}

