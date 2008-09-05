/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* icCollUtil.c
 */

#include "icCollUtil.h"

/*
#include "objMetaOpr.h"
#include "icatHighLevelRoutines.h"
#include "openCollection.h"
#include "readCollection.h"
#include "closeCollection.h"
#include "dataObjUnlink.h"
#include "rsApiHandler.h"
*/

/* All derived from Mike's rsRmColl.[ch] */
int icListColl (rsComm_t *rsComm, collInp_t *listCollInp, collOprStat_t **collOprStat) {

	int status;

	if (collOprStat != NULL) *collOprStat = NULL;

	if (getValByKey (&listCollInp->condInput, RECURSIVE_OPR__KW) == NULL) {
		status = _icListColl (rsComm, listCollInp, collOprStat);
	} else {
		if (isTrashPath (listCollInp->collName) == False && 
			getValByKey (&listCollInp->condInput, FORCE_FLAG_KW) != NULL) {
		rodsLog (LOG_NOTICE, "icListColl: Recursively listing %s.", listCollInp->collName);
		}

		status = _icListCollRecur (rsComm, listCollInp, collOprStat);
	}

	return status;

}

int _icListColl (rsComm_t *rsComm, collInp_t *listCollInp, collOprStat_t **collOprStat) {

	int status;
	dataObjInp_t dataObjInp;
	dataObjInfo_t *dataObjInfo = NULL;

	if (getValByKey (&listCollInp->condInput, UNREG_COLL_KW) != NULL) {
		//status = svrUnregColl (rsComm, listCollInp);
	} else {
		/* need to check if it is spec coll */
		memset (&dataObjInp, 0, sizeof (dataObjInp));
		rstrcpy (dataObjInp.objPath, listCollInp->collName, MAX_NAME_LEN);
		status = resolveSpecColl (rsComm, &dataObjInp, &dataObjInfo, 1);

		if (status < 0 && status != CAT_NO_ROWS_FOUND) {
			return status;
		} else if (status == COLL_OBJ_T && dataObjInfo->specColl != NULL) {
			//status = l3Rmdir (rsComm, dataObjInfo);
			fprintf (stderr, "collName: %s\n", listCollInp->collName);
			freeDataObjInfo (dataObjInfo);
		} else {
			//status = svrUnregColl (rsComm, listCollInp);
		}
	}

	if (status >= 0 && collOprStat != NULL) {
		*collOprStat = malloc (sizeof (collOprStat_t));
		memset (*collOprStat, 0, sizeof (collOprStat_t));
		(*collOprStat)->filesCnt = 1; 
		(*collOprStat)->totalFileCnt = 1; 
		rstrcpy ((*collOprStat)->lastObjPath, listCollInp->collName, MAX_NAME_LEN);
	}

	return (status);
}

int _icListCollRecur (rsComm_t *rsComm, collInp_t *listCollInp, collOprStat_t **collOprStat) {

	int status;
	dataObjInp_t dataObjInp;
	ruleExecInfo_t rei;
	int trashPolicy;
	dataObjInfo_t *dataObjInfo = NULL;

	memset (&dataObjInp, 0, sizeof (dataObjInp));
	rstrcpy (dataObjInp.objPath, listCollInp->collName, MAX_NAME_LEN);

	/* check for specColl and permission */
	status = resolveSpecColl (rsComm, &dataObjInp, &dataObjInfo, 1);

	if (status < 0 && status != CAT_NO_ROWS_FOUND) {
		return status;
	} else if (status != COLL_OBJ_T || dataObjInfo->specColl == NULL) {

		/* a normal collection */
		if (getValByKey (&listCollInp->condInput, FORCE_FLAG_KW) == NULL) {
			initReiWithDataObjInp (&rei, rsComm, NULL);
			status = applyRule ("acTrashPolicy", NULL, &rei, NO_SAVE_REI);
			trashPolicy = rei.status;
			if (trashPolicy != NO_TRASH_CAN) {
				status = rsMvCollToTrash (rsComm, listCollInp);
				if (status >= 0 && collOprStat != NULL) {
					if (*collOprStat == NULL) {
						*collOprStat = malloc (sizeof (collOprStat_t));
						memset (*collOprStat, 0, sizeof (collOprStat_t));
					}
					(*collOprStat)->filesCnt = 1;
					(*collOprStat)->totalFileCnt = 1;
					rstrcpy ((*collOprStat)->lastObjPath, listCollInp->collName, MAX_NAME_LEN);
				}
				return status;
			}
		}
	}
	/* got here. will recursively list the collection */
	status = _icPhyListColl (rsComm, listCollInp, dataObjInfo, collOprStat);

	if (dataObjInfo != NULL) freeDataObjInfo (dataObjInfo);
	return (status);
}

int _icPhyListColl (rsComm_t *rsComm, collInp_t *listCollInp, dataObjInfo_t *dataObjInfo, collOprStat_t **collOprStat) {

	int status;
	openCollInp_t openCollInp;
	collEnt_t *collEnt;
	int handleInx;
	dataObjInp_t dataObjInp;
	collInp_t tmpCollInp;
	int rmtrashFlag;
	int savedStatus = 0;
	int fileCntPerStatOut = FILE_CNT_PER_STAT_OUT;

	memset (&openCollInp, 0, sizeof (openCollInp));
	rstrcpy (openCollInp.collName, listCollInp->collName, MAX_NAME_LEN);

	/* cannot query recur because collection is sorted in wrong order */
	openCollInp.flags = 0;
	handleInx = rsOpenCollection (rsComm, &openCollInp);
	if (handleInx < 0) {
		rodsLog (LOG_ERROR, "_rsPhyRmColl: rsOpenCollection of %s error. status = %d",
			openCollInp.collName, handleInx);
		return (handleInx);
	}

	if (collOprStat != NULL && *collOprStat == NULL) {
		*collOprStat = malloc (sizeof (collOprStat_t));
		memset (*collOprStat, 0, sizeof (collOprStat_t));
	}

	memset (&dataObjInp, 0, sizeof (dataObjInp));
	memset (&tmpCollInp, 0, sizeof (tmpCollInp));
	addKeyVal (&dataObjInp.condInput, FORCE_FLAG_KW, "");
	addKeyVal (&tmpCollInp.condInput, FORCE_FLAG_KW, "");

	if (getValByKey (&listCollInp->condInput, IRODS_ADMIN_RMTRASH_KW) != NULL) {
		if (isTrashPath (listCollInp->collName) == False) {
			return (SYS_INVALID_FILE_PATH);
		}

		if (rsComm->clientUser.authInfo.authFlag != LOCAL_PRIV_USER_AUTH) {
			return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
		}

		addKeyVal (&tmpCollInp.condInput, IRODS_ADMIN_RMTRASH_KW, "");
		addKeyVal (&dataObjInp.condInput, IRODS_ADMIN_RMTRASH_KW, "");
		rmtrashFlag = 2;

	} else if (getValByKey (&listCollInp->condInput, IRODS_RMTRASH_KW) != NULL) {
		if (isTrashPath (listCollInp->collName) == False) {
			return (SYS_INVALID_FILE_PATH);
		}
		addKeyVal (&tmpCollInp.condInput, IRODS_RMTRASH_KW, "");
		addKeyVal (&dataObjInp.condInput, IRODS_RMTRASH_KW, "");
		rmtrashFlag = 1;
	}

	while ((status = rsReadCollection (rsComm, &handleInx, &collEnt)) >= 0) {
		if (collEnt->objType == DATA_OBJ_T) {
			snprintf (dataObjInp.objPath, MAX_NAME_LEN, "%s/%s",
			collEnt->collName, collEnt->dataName);

			status = rsDataObjUnlink (rsComm, &dataObjInp);
			if (status < 0) {
			rodsLog (LOG_ERROR, "_rsPhyRmColl:rsDataObjUnlink failed for %s. stat = %d",
				dataObjInp.objPath, status);

			/* need to set global error here */
			savedStatus = status;
			} else if (collOprStat != NULL) {
				(*collOprStat)->filesCnt ++;
				if ((*collOprStat)->filesCnt >= fileCntPerStatOut) {
					rstrcpy ((*collOprStat)->lastObjPath, dataObjInp.objPath, MAX_NAME_LEN);
					status = svrSendCollOprStat (rsComm, *collOprStat);
					if (status < 0) {
						rodsLogError (LOG_ERROR, status, "_rsPhyRmColl: svrSendCollOprStat failed for %s. status = %d",
							listCollInp->collName, status);
						*collOprStat = NULL;
						savedStatus = status;
						break;
					}
					*collOprStat = malloc (sizeof (collOprStat_t));
					memset (*collOprStat, 0, sizeof (collOprStat_t));
				}
			}

		} else if (collEnt->objType == COLL_OBJ_T) {
			if (strcmp (collEnt->collName, listCollInp->collName) == 0)
				continue;	/* duplicate */
			rstrcpy (tmpCollInp.collName, collEnt->collName, MAX_NAME_LEN);
			if (collEnt->specColl.collClass != NO_SPEC_COLL) {
				if (strcmp (collEnt->collName, collEnt->specColl.collection) == 0) 
					continue;	/* no mount point */
			}
			status = _icListCollRecur (rsComm, &tmpCollInp, collOprStat);
		}

		if (status < 0) savedStatus = status;
		free (collEnt);     /* just free collEnt but not content */
	}
	rsCloseCollection (rsComm, &handleInx);

	if (rmtrashFlag > 0 && isTrashHome (listCollInp->collName) > 0) {
		/* don't rm user's home trash coll */
		status = 0;
	} else {
		if (dataObjInfo != NULL && dataObjInfo->specColl != NULL) {
			//status = l3Rmdir (rsComm, dataObjInfo);
		} else {
			//status = svrUnregColl (rsComm, listCollInp);
		}
	}

	return (savedStatus);
}

int	msiListColl (msParam_t* mPin1, msParam_t* mPout1, ruleExecInfo_t* rei) {

	rsComm_t* rsComm;
	char* collname;
	collInp_t* listCollInp;
	//collOprStat_t* collOprStat;
	int status;

	RE_TEST_MACRO ("    Calling msiListColl")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiListFields: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* need to turn parameter 1 into a collInp_t struct */
	collname = strdup (mPin1->inOutStruct);
	rstrcpy (listCollInp->collName, collname, MAX_NAME_LEN);
	addKeyVal (&listCollInp->condInput, RECURSIVE_OPR__KW, "");  

	status = icListColl (rsComm, listCollInp, NULL);

	return (status);
	

}

