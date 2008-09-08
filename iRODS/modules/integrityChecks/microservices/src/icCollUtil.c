/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* icCollUtil.c
 */

#include "icCollUtil.h"

int	msiListColl (msParam_t* collectionname, msParam_t* buf, ruleExecInfo_t* rei) {

	rsComm_t* rsComm;
	sqlResult_t *collectionName;
	sqlResult_t *collectionID;
	bytesBuf_t* mybuf=NULL;
	genQueryInp_t gqin;
	genQueryOut_t* gqout=NULL;
	int i,status;
	char condStr[MAX_NAME_LEN];
	char* collname;

	RE_TEST_MACRO ("    Calling msiListColl")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiListColl: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* need to turn parameter 1 into a collInp_t struct */
	collname = strdup (collectionname->inOutStruct);
	
	/* init stuff */
	memset (&gqin, 0, sizeof(genQueryInp_t));
	gqin.maxRows = MAX_SQL_ROWS;
	gqout = (genQueryOut_t*) malloc (sizeof (genQueryOut_t));
	memset (gqout, 0, sizeof (genQueryOut_t));
	mybuf = (bytesBuf_t *) malloc (sizeof (bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));

	/* Generate a query - we only want subcollection data objects */
    addInxIval (&gqin.selectInp, COL_COLL_NAME, 1);
    addInxIval (&gqin.selectInp, COL_COLL_ID, 1);
	genAllInCollQCond (collname, condStr);
    addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	/* This is effectively a recursive query */
    status = rsGenQuery (rsComm, &gqin, &gqout);

	if (status < 0) {
		return (status);
	} else if (status != CAT_NO_ROWS_FOUND)  {

		collectionName = getSqlResultByInx (gqout, COL_COLL_NAME);
		collectionID = getSqlResultByInx (gqout, COL_COLL_ID);
		for (i=1; i<gqout->rowCnt; i++) {

			char* subcoll;
			char tmpstr[MAX_NAME_LEN];

			subcoll = &collectionName->value[collectionName->len*i];
			sprintf (tmpstr, "%s\n", subcoll );
			appendToByteBuf (mybuf, tmpstr);
		}

	} else {
		return (status); /* something bad happened */
	}

    fillBufLenInMsParam (buf, mybuf->len, mybuf);	
	return (rei->status);

}
	


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
	fprintf (stderr, "objPath:%s\n", dataObjInp.objPath);

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
				//status = rsMvCollToTrash (rsComm, listCollInp);
				status = 0;
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
	fprintf (stderr, "_icListCollRecur: got here: collname:%s\n", listCollInp->collName);
	status = _icPhyListColl (rsComm, listCollInp, dataObjInfo, collOprStat);
	fprintf (stderr, "_icListCollRecur: got here 1\n");

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
		rodsLog (LOG_ERROR, "_icPhyListColl: rsOpenCollection of %s error. status = %d",
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
			snprintf (dataObjInp.objPath, MAX_NAME_LEN, "%s/%s", collEnt->collName, collEnt->dataName);

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
						rodsLogError (LOG_ERROR, status, "_icPhyListColl: svrSendCollOprStat failed for %s. status = %d",
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
			fprintf (stderr, "_icPhyListColl: tmpCollInp->collname:%s\n", tmpCollInp.collName);
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


