#include "reDataObjOpr.h"
#include "apiHeaderAll.h"
#include "rsApiHandler.h"

/* msiDataObjCreate - msi for DataObjCreate.
 * inpParam1 - It can be a DataObjInp_MS_T or 
 *    a STR_MS_T which would be taken as dataObj path.
 * inpParam2 - Optional - a STR_MS_T which specifies the resource.
 * outParam - a INT_MS_T containing the descriptor of the create.
 *
 */ 

int
msiDataObjCreate (msParam_t *inpParam1, msParam_t *inpParam2, 
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    dataObjInp_t dataObjInp, *myDataObjInp;

    RE_TEST_MACRO ("    Calling msiDataObjCreate")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjCreate: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp (inpParam1, &dataObjInp, 
      &myDataObjInp, 0);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjCreate: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = parseMspForCondInp (inpParam2, &myDataObjInp->condInput, 
     DEST_RESC_NAME_KW);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjCreate: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsDataObjCreate (rsComm, myDataObjInp);

    if (myDataObjInp == &dataObjInp) {
        clearKeyVal (&myDataObjInp->condInput);
    }

    if (rei->status >= 0) {
	fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjCreate: rsDataObjCreate failed for %s, status = %d",
			    myDataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/* msiDataObjOpen - msi for DataObjOpen.
 * inpParam - It can be a DataObjInp_MS_T or a STR_MS_T which would be
 *    taken as dataObj path.
 * outParam - a INT_MS_T containing the descriptor of the open.
 *
 */

int
msiDataObjOpen (msParam_t *inpParam, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    dataObjInp_t dataObjInp, *myDataObjInp;

    RE_TEST_MACRO ("    Calling msiDataObjOpen")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjOpen: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp (inpParam, &dataObjInp, 
      &myDataObjInp, 0);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjOpen: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsDataObjOpen (rsComm, myDataObjInp);
    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjOpen: rsDataObjOpen failed for %s, status = %d",
			    dataObjInp.objPath,
          rei->status);
    }

    return (rei->status);
}

/* msiDataObjClose - msi for DataObjClose.
 * inpParam - It can be a DataObjCloseInp_MS_T or
 * INT_MS_T or a STR_MS_T which would be taken as descriptor from a create 
 *    or open.
 * outParam - a INT_MS_T containing the status.
 *
 */

int
msiDataObjClose (msParam_t *inpParam, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    dataObjCloseInp_t dataObjCloseInp, *myDataObjCloseInp;

    RE_TEST_MACRO ("    Calling msiDataObjClose")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjClose: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    if (inpParam == NULL) {
	rei->status = SYS_INTERNAL_NULL_INPUT_ERR;
	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjClose: input inpParam is NULL");
        return (rei->status);
    }

    if (strcmp (inpParam->type, DataObjCloseInp_MS_T) == 0) {
        myDataObjCloseInp = inpParam->inOutStruct;
    } else {
	int myInt;

	myInt = parseMspForPosInt (inpParam);
	if (myInt >= 0) {
            memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
            dataObjCloseInp.l1descInx = myInt;
            myDataObjCloseInp = &dataObjCloseInp;
	} else {
	    rei->status = myInt;
            rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
              "msiDataObjClose: parseMspForPosInt error for param2.");
            return (rei->status);
	}
     }

    rei->status = rsDataObjClose (rsComm, myDataObjCloseInp);
    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjClose: rsDataObjClose failed, status = %d",
          rei->status);
    }

    return (rei->status);
}

/* msiDataObjLseek - msi for DataObjLseek.
 * inpParam1 - It can be a DataObjLseekInp_MS_T or 
 *    INT_MS_T or a STR_MS_T which would be the descriptor.
 * inpParam2 - Optional - It can be a DOUBLE_MS_T or a STR_MS_T which would be 
 *    the offset.
 * inpParam3 - Optional - It can be a INT_MS_T or a STR_MS_T which would be 
 *    the whence.
 * outParam - a DataObjLseekOut_MS_T.
 *
 */

int
msiDataObjLseek (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *inpParam3, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    fileLseekInp_t dataObjLseekInp, *myDataObjLseekInp;
    fileLseekOut_t *dataObjLseekOut = NULL;

    RE_TEST_MACRO ("    Calling msiDataObjLseek")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjLseek: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    if (inpParam1 == NULL) {
	rei->status = SYS_INTERNAL_NULL_INPUT_ERR;
	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjLseek: input inpParam1 is NULL");
        return (rei->status);
    }

    if (strcmp (inpParam1->type, STR_MS_T) == 0) {
	/* str input */ 
	memset (&dataObjLseekInp, 0, sizeof (dataObjLseekInp));
	dataObjLseekInp.fileInx = atoi (inpParam1->inOutStruct);
	myDataObjLseekInp = &dataObjLseekInp;
    } else if (strcmp (inpParam1->type, INT_MS_T) == 0) {
        memset (&dataObjLseekInp, 0, sizeof (dataObjLseekInp));
        dataObjLseekInp.fileInx = *(int *)inpParam1->inOutStruct;
        myDataObjLseekInp = &dataObjLseekInp;
    } else if (strcmp (inpParam1->type, DataObjLseekInp_MS_T) == 0) {
	myDataObjLseekInp = inpParam1->inOutStruct;
    } else {
        rei->status = USER_PARAM_TYPE_ERR;
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjLseek: Unsupported input Param type %s", 
	  inpParam1->type);
        return (rei->status);
    }

    if (inpParam2 != NULL) {
        if (strcmp (inpParam2->type, STR_MS_T) == 0) {
            /* str input */
            if (strcmp ((char *) inpParam2->inOutStruct, "null") != 0) {
                myDataObjLseekInp->offset = strtoll (inpParam2->inOutStruct, 
		0, 0);
	    }
        } else if (strcmp (inpParam2->type, DOUBLE_MS_T) == 0) {
            myDataObjLseekInp->offset = *(rodsLong_t *)inpParam2->inOutStruct;
        } else {
            rei->status = USER_PARAM_TYPE_ERR;
            rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
              "msiDataObjLseek: Unsupported input Param type %s",
              inpParam2->type);
            return (rei->status);
	}
    }

    if (inpParam3 != NULL) {
        /* str input */
        if (strcmp (inpParam3->type, STR_MS_T) == 0) {
	    if (strcmp ((char *) inpParam3->inOutStruct, "SEEK_SET") == 0) {
		myDataObjLseekInp->whence = SEEK_SET;
	    } else if (strcmp ((char *) inpParam3->inOutStruct, 
	      "SEEK_CUR") == 0) {
                myDataObjLseekInp->whence = SEEK_CUR;
            } else if (strcmp ((char *) inpParam3->inOutStruct, 
              "SEEK_END") == 0) {
                myDataObjLseekInp->whence = SEEK_END;
            } else if (strcmp ((char *) inpParam3->inOutStruct, "null") != 0) {
                myDataObjLseekInp->whence = atoi (inpParam3->inOutStruct);
	    }
        } else if (strcmp (inpParam3->type, INT_MS_T) == 0) {
            myDataObjLseekInp->whence = *(int *)inpParam3->inOutStruct;
        } else {
            rei->status = USER_PARAM_TYPE_ERR;
            rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
              "msiDataObjLseek: Unsupported input Param type %s",
              inpParam3->type);
            return (rei->status);
	}
    }

    if (myDataObjLseekInp->whence != SEEK_SET &&
      myDataObjLseekInp->whence != SEEK_CUR &&
      myDataObjLseekInp->whence != SEEK_END) {
        rei->status = USER_PARAM_TYPE_ERR;
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjLseek: Unsupported input whence value %d",
          myDataObjLseekInp->whence);
        return (rei->status);
    }
	
    rei->status = rsDataObjLseek (rsComm, myDataObjLseekInp, &dataObjLseekOut);
    if (rei->status >= 0) {
	if (outParam != NULL) {
	    fillMsParam (outParam, NULL, DataObjLseekOut_MS_T, 
	      dataObjLseekOut, NULL);
	} else {
	    free (dataObjLseekOut);
	}
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjLseek: rsDataObjLseek failed, status = %d",
          rei->status);
    }

    return (rei->status);
}

/* msiDataObjRead - msi for DataObjRead.
 * inpParam1 - It can be a DataObjReadInp_MS_T or
 *    INT_MS_T or a STR_MS_T which would be the descriptor.
 * inpParam2 - Optional - It can be a INT_MS_T or a STR_MS_T which would be
 *    the length.
 * outParam - a BUF_LEN_MS_T.
 *
 */

int
msiDataObjRead (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    dataObjReadInp_t dataObjReadInp, *myDataObjReadInp;
    bytesBuf_t *dataObjReadOutBBuf = NULL;
    int myInt;

    RE_TEST_MACRO ("    Calling msiDataObjRead")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjRead: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    if (inpParam1 == NULL) {
	rei->status = SYS_INTERNAL_NULL_INPUT_ERR;
	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRead: input inpParam1 is NULL");
        return (rei->status);
    }

    if (strcmp (inpParam1->type, DataObjReadInp_MS_T) == 0) {
        myDataObjReadInp = inpParam1->inOutStruct;
    } else {
        myInt = parseMspForPosInt (inpParam1);
        if (myInt >= 0) {
            memset (&dataObjReadInp, 0, sizeof (dataObjReadInp));
            dataObjReadInp.l1descInx = myInt;
            myDataObjReadInp = &dataObjReadInp;
        } else {
            rei->status = myInt;
            rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
              "msiDataObjRead: parseMspForPosInt error for param1.");
            return (rei->status);
        }
    }

    if (inpParam2 != NULL) {
	myInt = parseMspForPosInt (inpParam2);

	if (myInt < 0) {
	    if (myInt != SYS_NULL_INPUT) {
                rei->status = myInt;
                rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
                  "msiDataObjRead: parseMspForPosInt error for param2.");
                return (rei->status);
	    }
        } else {
	    myDataObjReadInp->len = myInt;
	}
    }

    dataObjReadOutBBuf = (bytesBuf_t *) malloc (sizeof (bytesBuf_t));
    memset (dataObjReadOutBBuf, 0, sizeof (bytesBuf_t));

    rei->status = rsDataObjRead (rsComm, myDataObjReadInp, dataObjReadOutBBuf);

    if (rei->status >= 0) {
        fillBufLenInMsParam (outParam, rei->status, dataObjReadOutBBuf);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRead: rsDataObjRead failed, status = %d",
          rei->status);
    }

    return (rei->status);
}

/* msiDataObjjWrite - msi for DataObjjWrite.
 * inpParam1 - It can be a DataObjWriteInp_MS_T or
 *    INT_MS_T or a STR_MS_T which would be the descriptor.
 * inpParam2 - Optional - It can be a BUF_LEN_MS_T or a STR_MS_T which would be
 *    the length of the buffer and the buffer in the BBuf.
 * outParam - a INT_MS_T for the length written.
 *
 */

int
msiDataObjWrite (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    dataObjWriteInp_t dataObjWriteInp, *myDataObjWriteInp;
    int myInt;

    RE_TEST_MACRO ("    Calling msiDataObjWrite")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjWrite: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    if (inpParam1 == NULL || inpParam2->inpOutBuf == NULL) {
	rei->status = SYS_INTERNAL_NULL_INPUT_ERR;
	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjWrite: input inpParam1 or inpOutBuf is NULL");
        return (rei->status);
    }

    if (strcmp (inpParam1->type, DataObjWriteInp_MS_T) == 0) {
        myDataObjWriteInp = inpParam1->inOutStruct;
    } else {
       myInt = parseMspForPosInt (inpParam1);
        if (myInt >= 0) {
            memset (&dataObjWriteInp, 0, sizeof (dataObjWriteInp));
            dataObjWriteInp.l1descInx = myInt;
            myDataObjWriteInp = &dataObjWriteInp;
        } else {
            rei->status = myInt;
            rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
              "msiDataObjWrite: parseMspForPosInt error for param1.");
            return (rei->status);
        }
    }

    if (inpParam2 != NULL) {
	myInt = parseMspForPosInt (inpParam2);
        if (myInt < 0) {
            if (myInt != SYS_NULL_INPUT) {
                rei->status = myInt;
                rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
                  "msiDataObjRead: parseMspForPosInt error for param2.");
                return (rei->status);
            }
        } else {
            myDataObjWriteInp->len = myInt;
        }
    }

    rei->status = rsDataObjWrite (rsComm, myDataObjWriteInp, 
      inpParam2->inpOutBuf);

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjWrite: rsDataObjWrite failed, status = %d",
          rei->status);
    }

    return (rei->status);
}

/* msiDataObjjUnlink - msi for DataObjjUnlink.
 * inpParam - It can be a DataObjInp_MS_T or
 *    STR_MS_T which would be the obj Path.
 * outParam - a INT_MS_T for the status.
 *
 */

int
msiDataObjUnlink (msParam_t *inpParam, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    dataObjInp_t dataObjInp, *myDataObjInp;

    RE_TEST_MACRO ("    Calling msiDataObjUnlink")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjUnlink: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam */
    rei->status = parseMspForDataObjInp (inpParam, &dataObjInp, 
      &myDataObjInp, 0);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjUnlink: input inpParam error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsDataObjUnlink (rsComm, myDataObjInp);
    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjUnlink: rsDataObjUnlink failed for %s, status = %d",
			    myDataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/* msiDataObjjRepl - msi for DataObjjRepl.
 * inpParam1 - It can be a DataObjInp_MS_T or
 *    STR_MS_T which would be the obj Path.
 * inpParam2 - Optional - a STR_MS_T which specifies the resource.
 * outParam - a INT_MS_T for the status.
 *
 */

int
msiDataObjRepl (msParam_t *inpParam1, msParam_t *inpParam2, 
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    dataObjInp_t dataObjInp, *myDataObjInp;
    transStat_t *transStat = NULL;

    RE_TEST_MACRO ("    Calling msiDataObjRepl")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjRepl: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp (inpParam1, &dataObjInp, 
      &myDataObjInp, 0);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRepl: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = parseMspForCondInp (inpParam2, &myDataObjInp->condInput,
      DEST_RESC_NAME_KW);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRepl: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsDataObjRepl (rsComm, myDataObjInp, &transStat);

    if (myDataObjInp == &dataObjInp) {
        clearKeyVal (&myDataObjInp->condInput);
    }

    if (transStat != NULL) {
	free (transStat);
    }

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRepl: rsDataObjRepl failed %s, status = %d",
			    myDataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/* msiDataObjjCopy - msi for DataObjjCopy.
 * inpParam1 - It can be a DataObjCopyInp_MS_T or
 *    DataObjInp_MS_T which is the src DataObjInp or
 *    STR_MS_T which would be the src obj Path.
 * inpParam2 - Optional - It can be a DataObjInp_MS_T which is the dest
 *    DataObjInp or 
 *    STR_MS_T which would be the dest obj Path.
 * inpParam3 - Optional - a STR_MS_T which specifies the resource.
 * outParam - a INT_MS_T for the status.
 *
 */

int
msiDataObjCopy (msParam_t *inpParam1, msParam_t *inpParam2, 
msParam_t *inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    dataObjCopyInp_t dataObjCopyInp, *myDataObjCopyInp;
    dataObjInp_t *myDataObjInp;
    transStat_t *transStat = NULL;

    RE_TEST_MACRO ("    Calling msiDataObjCopy")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjCopy: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjCopyInp (inpParam1, &dataObjCopyInp, 
      &myDataObjCopyInp);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjCopy: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    /* parse inpParam2 */
    rei->status = parseMspForDataObjInp (inpParam2, 
      &myDataObjCopyInp->destDataObjInp, &myDataObjInp, 1);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjCopy: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = parseMspForCondInp (inpParam3, 
      &myDataObjCopyInp->destDataObjInp.condInput, DEST_RESC_NAME_KW);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjCopy: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsDataObjCopy (rsComm, myDataObjCopyInp, &transStat);
    if (transStat != NULL) {
	free (transStat);
    }

    if (myDataObjCopyInp == &dataObjCopyInp) {
        clearKeyVal (&myDataObjCopyInp->destDataObjInp.condInput);
    }

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjCopy: rsDataObjCopy failed for %s, status = %d",
			    myDataObjCopyInp->srcDataObjInp.objPath,
          rei->status);
    }

    return (rei->status);
}

/**
 * \fn msiDataObjPut
 * \author  Michael Wan
 * \date   2007-02-12
 * \brief This microservice requests the client to call a rcDataObjPut API
 *   as part of a workflow execution.  
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called 
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection. Also, it should never
 *  be called through delayExec since it requires client interaction.   
 * \param[in] 
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 *    inpParam2 - Optional - a STR_MS_T which specifies the resource.
 *    inpParam3 - Optional - a STR_MS_T which specifies the client's local 
 *      file path.
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/

int
msiDataObjPut (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    dataObjInp_t *dataObjInp, *myDataObjInp;
    msParamArray_t *myMsParamArray;

    RE_TEST_MACRO ("    Calling msiDataObjPut")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiDataObjPut: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    dataObjInp = malloc (sizeof (dataObjInp_t));
    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp (inpParam1, dataObjInp,
      &myDataObjInp, 1);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPut: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = parseMspForCondInp (inpParam2, &dataObjInp->condInput,
      DEST_RESC_NAME_KW);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPut: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = parseMspForCondInp (inpParam3, &dataObjInp->condInput,
      LOCAL_PATH_KW);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPut: input inpParam3 error. status = %d", rei->status);
        return (rei->status);
    }

    myMsParamArray = malloc (sizeof (msParamArray_t));
    memset (myMsParamArray, 0, sizeof (msParamArray_t));

    rei->status = addMsParam (myMsParamArray, CL_PUT_ACTION, DataObjInp_MS_T, 
      (void *) dataObjInp, NULL);
    
    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPut: addMsParam error. status = %d", rei->status);
        return (rei->status);
    }

    /* tell the client to do the put */
    rei->status = sendAndRecvBranchMsg (rsComm, rsComm->apiInx, 
     SYS_SVR_TO_CLI_MSI_REQUEST, (void *) myMsParamArray, NULL);

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPut: rsDataObjPut failed for %s, status = %d",
			    dataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/*
 * \fn msiDataObjGet
 * \author  Michael Wan
 * \date   2007-02-12
 * \brief This microservice requests the client to call a rcDataObjget API
 *   as part of a workflow execution.
 * \note This call should only be used through the rcExecMyRule (irule) call  
 *  i.e., rule execution initiated by clients and should not be called  
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection. Also, it should never
 *  be called through delayExec since it requires client interaction.
 * \param[in] 
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 *    inpParam2 - Optional - a STR_MS_T which specifies the client's local 
 *      file path.
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/

int
msiDataObjGet (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    dataObjInp_t *dataObjInp, *myDataObjInp;
    msParamArray_t *myMsParamArray;

    RE_TEST_MACRO ("    Calling msiDataObjGet")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiDataObjGet: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    dataObjInp = malloc (sizeof (dataObjInp_t));
    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp (inpParam1, dataObjInp,
      &myDataObjInp, 1);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjGet: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = parseMspForCondInp (inpParam2, &dataObjInp->condInput,
      LOCAL_PATH_KW);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjGet: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    myMsParamArray = malloc (sizeof (msParamArray_t));
    memset (myMsParamArray, 0, sizeof (msParamArray_t));

    rei->status = addMsParam (myMsParamArray, CL_GET_ACTION, DataObjInp_MS_T,
      (void *) dataObjInp, NULL);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjGet: addMsParam error. status = %d", rei->status);
        return (rei->status);
    }

    /* tell the client to do the put */
    rei->status = sendAndRecvBranchMsg (rsComm, rsComm->apiInx,
     SYS_SVR_TO_CLI_MSI_REQUEST, (void *) myMsParamArray, NULL);

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjGet: rsDataObjGet failed for %s, status = %d",
			    dataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/*
 * \fn msiDataObjChksum
 * \author  Michael Wan
 * \date   2007-04-02
 * \brief This microservice calls rsDataObjChksum to chksum the iput dat object
 *  as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call  
 *  i.e., rule execution initiated by clients and should not be called  
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection. 
 * \param[in] 
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 *    inpParam2 - optional - a STR_MS_T which specifies the "ChksumAll"
 *      (CHKSUM_ALL_KW). "verifyChksum"(VERIFY_CHKSUM_KW) or 
 *      "forceChksum"(FORCE_CHKSUM_KW).
 * \param[out] a STR_MS_T containing the chksum value.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/

int
msiDataObjChksum (msParam_t *inpParam1, msParam_t *inpParam2, 
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    dataObjInp_t dataObjInp, *myDataObjInp;
    char *chksum = NULL;

    RE_TEST_MACRO ("    Calling msiDataObjChksum")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiDataObjChksum: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp (inpParam1, &dataObjInp,
      &myDataObjInp, 1);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjChksum: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

   if ((rei->status = parseMspForCondKw (inpParam2, &myDataObjInp->condInput))
      < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjChksum: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsDataObjChksum (rsComm, myDataObjInp, &chksum);

    if (myDataObjInp == &dataObjInp) {
        clearKeyVal (&myDataObjInp->condInput);
    }

    if (rei->status >= 0) {
        fillStrInMsParam (outParam, chksum);
	free (chksum);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjChksum: rsDataObjChksum failed for %s, status = %d",
			  myDataObjInp->objPath ,
          rei->status);
    }

    return (rei->status);
}

/*
 * \fn msiDataObjPhymv
 * \author  Michael Wan
 * \date   2007-04-02
 * \brief This microservice calls rsDataObjPhymv to physically move the iput 
 * dat object to another resource.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection. 
 * \param[in]
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 *    irpParam2 - optional - a STR_MS_T which specifies the dest resourceName.
 *    irpParam3 - optional - a STR_MS_T which specifies the src resourceName.
 *    inpParam4 - optional - a STR_MS_T which specifies the replNum.
 *    inpParam5 - optional - a STR_MS_T which specifies the IRODS_ADMIN_KW.
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
 */

int
msiDataObjPhymv (msParam_t *inpParam1, msParam_t *inpParam2, 
msParam_t *inpParam3, msParam_t *inpParam4, msParam_t *inpParam5,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    dataObjInp_t dataObjInp, *myDataObjInp;
    transStat_t *transStat = NULL;

    RE_TEST_MACRO ("    Calling msiDataObjPhymv")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiDataObjPhymv: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */

    if ((rei->status = parseMspForDataObjInp (inpParam1, &dataObjInp, 
      &myDataObjInp, 0)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPhymv: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam2, &myDataObjInp->condInput,
      DEST_RESC_NAME_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPhymv: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam3, &myDataObjInp->condInput,
      RESC_NAME_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPhymv: input inpParam3 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam4, &myDataObjInp->condInput,
      REPL_NUM_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPhymv: input inpParam4 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam5, &myDataObjInp->condInput,
      IRODS_ADMIN_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPhymv: input inpParam5 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsDataObjPhymv (rsComm, myDataObjInp, &transStat);

    if (transStat != NULL) {
	free (transStat);
    }

    if (myDataObjInp == &dataObjInp) {
        clearKeyVal (&myDataObjInp->condInput);
    }

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPhymv: rsDataObjPhymv failed for %s, status = %d",
			    myDataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/*
 * \fn msiDataObjRename
 * \author  Michael Wan
 * \date   2007-04-02
 * \brief This microservice calls rsDataObjRename to rename the iput
 * data object or collection to another path.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection.
 * \param[in]
 *    inpParam1 - It can be a taObjCopyInp_MS_T or
 *      a STR_MS_T which would be taken as the src dataObj path.
 *    irpParam2 - optional - It can be a DataObjInp_MS_T which is the dest
 *       DataObjInp or STR_MS_T which would be the dest obj Path.
 *    irpParam3 - optional - a INT_MS_T or STR_MS_T which specifies the 
 I       the object type. A 0 means data obj and > 0 mean collection. 
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
 */

int
msiDataObjRename (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    dataObjCopyInp_t dataObjRenameInp, *myDataObjRenameInp;
    dataObjInp_t *myDataObjInp;

    RE_TEST_MACRO ("    Calling msiDataObjRename")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiDataObjRename: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjCopyInp (inpParam1, &dataObjRenameInp,
      &myDataObjRenameInp);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRename: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    /* parse inpParam2 */
    rei->status = parseMspForDataObjInp (inpParam2,
      &myDataObjRenameInp->destDataObjInp, &myDataObjInp, 1);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRename: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    if (inpParam3 != NULL) {
	int myInt;
	myInt = parseMspForPosInt (inpParam3);
	if (myInt > 0) {
	    myDataObjRenameInp->srcDataObjInp.oprType = RENAME_COLL;
	} else {
	    myDataObjRenameInp->srcDataObjInp.oprType = RENAME_DATA_OBJ;
	}
    }

    rei->status = rsDataObjRename (rsComm, myDataObjRenameInp);

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRename: rsDataObjRename failed for %s, status = %d",
			    myDataObjRenameInp->srcDataObjInp.objPath,
          rei->status);
    }

    return (rei->status);
}

/*
 * \fn msiDataObjTrim
 * \author  Michael Wan
 * \date   2007-04-02
 * \brief This microservice calls rsDataObjTrim to trim down the number
 * of replica of a data object.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection.
 * \param[in]
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 *    irpParam2 - optional - a STR_MS_T which specifies the resourceName.
 *    inpParam3 - optional - a STR_MS_T which specifies the replNum.
 *    irpParam4 - optional - a STR_MS_T which specifies the minimum number of 
 *      copies to keep.
 *    inpParam5 - optional - a STR_MS_T which specifies the IRODS_ADMIN_KW.
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
 */

int
msiDataObjTrim (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *inpParam3, msParam_t *inpParam4, msParam_t *inpParam5,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    dataObjInp_t dataObjInp, *myDataObjInp;

    RE_TEST_MACRO ("    Calling msiDataObjTrim")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiDataObjTrim: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */

    if ((rei->status = parseMspForDataObjInp (inpParam1, &dataObjInp,
      &myDataObjInp, 0)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjTrim: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam2, &myDataObjInp->condInput,      RESC_NAME_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjTrim: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam3, &myDataObjInp->condInput,
      REPL_NUM_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjTrim: input inpParam3 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam4, &myDataObjInp->condInput,
      COPIES_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjTrim: input inpParam4 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam5, &myDataObjInp->condInput,
      IRODS_ADMIN_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjTrim: input inpParam5 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsDataObjTrim (rsComm, myDataObjInp);

    if (myDataObjInp == &dataObjInp) {
        clearKeyVal (&myDataObjInp->condInput);
    }

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjTrim: rsDataObjTrim failed for %s, status = %d",
			    myDataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/*
 * \fn msiCollCreate
 * \author  Michael Wan
 * \date   2007-04-02
 * \brief This microservice calls rsCollCreate to recursively rm a collection
 *  as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection.
 * \param[in]
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/

int
msiCollCreate (msParam_t *inpParam1, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    collInp_t collCreateInp, *myCollCreateInp;

    RE_TEST_MACRO ("    Calling msiCollCreate")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiCollCreate: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForCollInp (inpParam1, &collCreateInp,
      &myCollCreateInp, 1);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiCollCreate: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsCollCreate (rsComm, myCollCreateInp);

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiCollCreate: rsCollCreate failed %s, status = %d",
			collCreateInp.collName,  
          rei->status);
    }

    if (myCollCreateInp == &collCreateInp) {
	clearKeyVal (&myCollCreateInp->condInput);
    }

    return (rei->status);
}

/*
 * \fn msiRmColl
 * \author  Michael Wan
 * \date   2007-04-02
 * \brief This microservice calls rsRmColl to recursively rm a collection
 *  as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection.
 * \param[in]
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 *    inpParam2 - optional - a STR_MS_T which specifies the "forceFlag"
 *      (FORCE_FLAG_KW). "irodsAdminRmTrash"(IRODS_ADMIN_RMTRASH_KW) or
 *      "irodsRmTrash"(IRODS_RMTRASH_KW).
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/

int
msiRmColl (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    collInp_t rmCollInp, *myRmCollInp;

    RE_TEST_MACRO ("    Calling msiRmColl")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiRmColl: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForCollInp (inpParam1, &rmCollInp,
      &myRmCollInp, 1);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiRmColl: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondKw (inpParam2, &myRmCollInp->condInput))
      < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiRmColl: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    addKeyVal (&myRmCollInp->condInput, RECURSIVE_OPR__KW, "");

    rei->status = rsRmColl (rsComm, myRmCollInp);

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiRmColl: rsRmColl failed for %s, status = %d",
			    myRmCollInp->collName, 
          rei->status);
    }

    if (myRmCollInp == &rmCollInp) {
	clearKeyVal (&myRmCollInp->condInput);
    }

    return (rei->status);
}

/*
 * \fn msiReplColl
 * \author  Sifang Lu
 * \date   2007-10-01
 * \brief This microservice iterate through collection, and calls 
 *  rsDataObjRepl to recursively replication a collection
 *  as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection.
 * \param[in]
 *    coll     : It can be a collInp_t or a STR_MS_T which would be taken 
 *               as destination collection path.
 *    destResc : STR_MS_T destination resource name
 *    options  : STR_MS_T a group of options in a string delimited by '%%'.
 *               If the string is empty ("\0") or null ("NULL") it will not 
 *               be used.  
 *               The options can be the following
 *              - "all"(ALL_KW) 
 *              - "irodsAdmin" (IRODS_ADMIN_KW).
 *              - "backupMode" if specified, it will try to use 'backup mode' 
 *                to the destination resource. Means if a good copy already
 *                exists in destination resource, it will not throw an error
 *
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/
int
msiReplColl (msParam_t *coll, msParam_t *destRescName, msParam_t *options,
  msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    collInp_t collInp, *myCollInp;
    int i, continueInx, status;
    transStat_t *transStat = NULL;
    strArray_t optArray;
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    dataObjInp_t dataObjInp;
    
    RE_TEST_MACRO ("    Calling msiReplColl")

    if (rei == NULL || rei->rsComm == NULL) {
	    rodsLog (LOG_ERROR,
	    "msiReplColl: input rei or rsComm is NULL");
	    return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1: coll */
    rei->status = parseMspForCollInp (coll, &collInp, 
      &myCollInp, 0);
    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiReplColl: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    /* parse inpParam2: destRescName, and assign the destination 
       resource to dataobjinp */
    memset (&dataObjInp, 0, sizeof (dataObjInp_t));
    rei->status = parseMspForCondInp (destRescName, 
      &(&dataObjInp)->condInput, DEST_RESC_NAME_KW);
    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiReplColl: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }
    
    /* parse inpParam3: options, and assign the then to conditional 
       keywords */
    if ( (strlen(options->inOutStruct)>0) && 
         (0!=strcmp(options->inOutStruct,"null")) )
    { 
      memset (&optArray, 0, sizeof (optArray));
      status = parseMultiStr ((char *)options->inOutStruct, &optArray);
      if (status <= 0)
      {
        rodsLog (LOG_ERROR,
          "msiReplColl: Could not parse options string '%s'",
          options->inOutStruct);
      }
      for(i=0; i<optArray.len; i++)
      {
        char *option;
        option= &optArray.value[i*optArray.size];
        if ( strcmp(option,ALL_KW) && 
             strcmp(option,IRODS_ADMIN_KW) &&
             strcmp(option,"backupMode")
           )
        {
          rodsLog (LOG_ERROR,"msiReplColl: invalid option: '%s'",option);
          continue;  
        }
        if (strcmp(option,"backupMode")==0)
          addKeyVal (&(dataObjInp.condInput), BACKUP_RESC_NAME_KW, 
            (char *)destRescName->inOutStruct);
        else
          addKeyVal (&(dataObjInp.condInput), option, "");
      }
    }  
    
    /* iterate through all files */
    memset (&genQueryInp, 0, sizeof (genQueryInp));
    status = rsQueryDataObjInCollReCur (rsComm, myCollInp->collName, 
      &genQueryInp, &genQueryOut, NULL, 1);
    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
    	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
    	  "msiReplColl: msiReplColl error for %s, stat=%d",
    	  myCollInp->collName, status);
    	rei->status=status;
      return (rei->status);
    }
    while (rei->status >= 0) {
      sqlResult_t *subColl, *dataObj;
      /* get sub coll paths in the batch */
      if ((subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
        == NULL) {
          rodsLog (LOG_ERROR,
            "msiReplColl: msiReplColl for COL_COLL_NAME failed");
          rei->status=UNMATCHED_KEY_OR_INDEX;  
          return (rei->status);
      }
      /* get data names in the batch */
      if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR, 
              "msiReplColl: msiReplColl for COL_DATA_NAME failed");
            rei->status=UNMATCHED_KEY_OR_INDEX;   
            return (rei->status);
      }
      
      for (i = 0; i < genQueryOut->rowCnt; i++) {
        char *tmpSubColl, *tmpDataName;

        tmpSubColl = &subColl->value[subColl->len * i];
        tmpDataName = &dataObj->value[dataObj->len * i];
        snprintf (dataObjInp.objPath, MAX_NAME_LEN, "%s/%s",
              tmpSubColl, tmpDataName);
        rei->status = rsDataObjRepl (rsComm, &dataObjInp, &transStat);
        if (rei->status<0)
        {
          rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
            "msiReplColl: rsDataObjRepl failed %s, status = %d",
			    (&dataObjInp)->objPath,
          rei->status);
        }
        if (transStat != NULL) {
    	    free (transStat);
        }
      }
      
      continueInx = genQueryOut->continueInx;
      freeGenQueryOut (&genQueryOut);
      if (continueInx > 0) {
        /* More to come */
        genQueryInp.continueInx = continueInx;
        rei->status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
      } else {
        break;
      }
    }
    
    clearKeyVal (&dataObjInp.condInput);
    
    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "dataObjInp: msiReplColl failed (should have catched earlier) %s, status = %d",
			    (&dataObjInp)->objPath,
          rei->status);
    }
    return (rei->status);
}

/*
 * \fn msiPhyPathReg
 * \author  Michael Wan
 * \date   2007-04-02
 * \brief This microservice calls rsPhyPathReg to register a physical path
 * with the iCat. recursively rm a collection
 *  as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection.
 * \param[in]
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 *    irpParam2 - optional - a STR_MS_T which specifies the dest resourceName.
 *    irpParam3 - optional - a STR_MS_T which specifies the physical path to
 *      be registered.
 *    inpParam4 - optional - a STR_MS_T which specifies the "collection"
 *      (COLLECTION_KW) indicating the path to be registered is a directory. 
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/

int
msiPhyPathReg (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *inpParam3, msParam_t *inpParam4, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    dataObjInp_t dataObjInp, *myDataObjInp;

    RE_TEST_MACRO ("    Calling msiPhyPathReg")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiPhyPathReg: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp (inpParam1, &dataObjInp,
      &myDataObjInp, 1);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiPhyPathReg: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam2, &myDataObjInp->condInput,
      DEST_RESC_NAME_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPhymv: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam3, &myDataObjInp->condInput,
      FILE_PATH_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjPhymv: input inpParam3 error. status = %d", rei->status);
        return (rei->status);
    }

   if ((rei->status = parseMspForCondKw (inpParam4, &myDataObjInp->condInput))
      < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiPhyPathReg: input inpParam4 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsPhyPathReg (rsComm, myDataObjInp);

    if (myDataObjInp == &dataObjInp) {
        clearKeyVal (&myDataObjInp->condInput);
    }

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiPhyPathReg: rsPhyPathReg failed for %s, status = %d",
			    myDataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/*
 * \fn msiObjStat
 * \author  Michael Wan
 * \date   2007-04-02
 * \brief This microservice calls rsObjStat to get the stat of an iRods path 
 *  as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection.
 * \param[in]
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 * \param[out] a INT_MS_T containing an integer with value COLL_OBJ_T or 
 *    DATA_OBJ_T.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/

int
msiObjStat (msParam_t *inpParam1, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    dataObjInp_t dataObjInp, *myDataObjInp;
    rodsObjStat_t *rodsObjStatOut = NULL;

    RE_TEST_MACRO ("    Calling msiObjStat")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiObjStat: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp (inpParam1, &dataObjInp,
      &myDataObjInp, 0);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiObjStat: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    rei->status = rsObjStat (rsComm, myDataObjInp, &rodsObjStatOut);

    if (rei->status >= 0) {
        fillMsParam (outParam, NULL, RodsObjStat_MS_T, rodsObjStatOut, NULL);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiObjStat: rsObjStat failed for %s, status = %d",
			    myDataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/**
 * \fn msiDataObjRsync
 * \author  Michael Wan
 * \date   2007-02-12
 * \brief This microservice requests the client to call a rcDataObjRsync API
 *   as part of a workflow execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection. Also, it should never
 *  be called through delayExec since it requires client interaction.
 * \param[in]
 *    inpParam1 - It can be a DataObjInp_MS_T or
 *      a STR_MS_T which would be taken as dataObj path.
 *    inpParam2 - Optional - a STR_MS_T which specifies the rsync mode 
 *      (RSYNC_MODE_KW). Valid modes are IRODS_TO_LOCAL, LOCAL_TO_IRODS 
 *	and IRODS_TO_IRODS
 *    inpParam3 - Optional - a STR_MS_T which specifies the chksum value
 *      (RSYNC_CHKSUM_KW).
 *    inpParam4 - Optional - a STR_MS_T which specifies the dest path
 *      (RSYNC_DEST_PATH_KW). Valid only for IRODS_TO_IRODS mode.
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/

int
msiDataObjRsync (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *inpParam3, msParam_t *inpParam4, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    dataObjInp_t dataObjInp, *myDataObjInp;

    RE_TEST_MACRO ("    Calling msiDataObjRsync")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiDataObjRsync: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForDataObjInp (inpParam1, &dataObjInp,
      &myDataObjInp, 1);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRsync: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam2, &myDataObjInp->condInput,
      RSYNC_MODE_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRsync: input inpParam2 error. status = %d", rei->status);
        return (rei->status);
    }


    if ((rei->status = parseMspForCondInp (inpParam3, &myDataObjInp->condInput,
      RSYNC_CHKSUM_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRsync: input inpParam3 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((rei->status = parseMspForCondInp (inpParam3, &myDataObjInp->condInput,
      RSYNC_CHKSUM_KW)) < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRsync: input inpParam3 error. status = %d", rei->status);
        return (rei->status);
    }

    /* just call rsDataObjRsync for now. client must supply the chksum of
     * the local file. Could ask the client to do a chksum first */

    rei->status = rsDataObjRsync (rsComm, myDataObjInp, NULL);

    if (myDataObjInp == &dataObjInp) {
        clearKeyVal (&myDataObjInp->condInput);
    }

    if (rei->status >= 0) {
        fillIntInMsParam (outParam, rei->status);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiDataObjRsync: rsDataObjRsync failed for %s, status = %d",
			    myDataObjInp->objPath,
          rei->status);
    }

    return (rei->status);
}

/**
 * \fn msiExecCmd
 * \author  Michael Wan
 * \date   2007-05-08
 * \brief This microservice requests the client to call a rcExecCmd API
 *   to fork and exec a command in the server/ibn/cmd directory.
 * \param[in]
 *    inpParam1 - It can be a ExecCmd_MS_T or
 *      a STR_MS_T which specify the command (cmd) to execute.
 *    inpParam2 - Optional - a STR_MS_T which specifies the argv (cmdArgv) 
 *	of the command
 *    inpParam3 - Optional - a STR_MS_T which specifies the host address
 *      (execAddr) to execute to command.
 *    inpParam4 - Optional - a STR_MS_T which specifies an iRods file path
 *	(hintPath). The command will be executed on the host where this 
 *	file is stored. 
 *    inpParam5 - Optional - A INT_MS_T or a STR_MS_T which specifies 
 *	which the resolved physical path from the hintPath (inpParam4)
 *	will be part of the argv. 
 * \param[out] a ExecCmdOut_MS_T containing the status of the command
 *	execution and the stdout/strerr output.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/

int
msiExecCmd (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *inpParam3, 
msParam_t *inpParam4, msParam_t *inpParam5, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    execCmd_t execCmdInp, *myExecCmdInp;
    execCmdOut_t *execCmdOut = NULL;
    char *tmpPtr;

    RE_TEST_MACRO ("    Calling msiExecCmd")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiExecCmd: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1 */
    rei->status = parseMspForExecCmdInp (inpParam1, &execCmdInp, 
      &myExecCmdInp);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiExecCmd: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    if ((tmpPtr = parseMspForStr (inpParam2)) != NULL) {
	rstrcpy (myExecCmdInp->cmdArgv, tmpPtr, MAX_NAME_LEN); 
    }

    if ((tmpPtr = parseMspForStr (inpParam3)) != NULL) {
        rstrcpy (myExecCmdInp->execAddr, tmpPtr, MAX_NAME_LEN); 
    }

    if ((tmpPtr = parseMspForStr (inpParam4)) != NULL) {
        rstrcpy (myExecCmdInp->hintPath, tmpPtr, MAX_NAME_LEN); 
    }

    if (parseMspForPosInt (inpParam5) > 0) {
	myExecCmdInp->addPathToArgv = 1;
    }  

    rei->status = rsExecCmd (rsComm, myExecCmdInp, &execCmdOut);

    if (myExecCmdInp == &execCmdInp) {
        clearKeyVal (&myExecCmdInp->condInput);
    }

    if (rei->status >= 0) {
        fillMsParam (outParam, NULL, ExecCmdOut_MS_T, execCmdOut, NULL);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiExecCmd: rsExecCmd failed for %s, status = %d",
			    myExecCmdInp->cmd,
          rei->status);
    }

    return (rei->status);
}

