/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* reSysDataObjOpr.c */

#include "reSysDataObjOpr.h"
#include "genQuery.h"


/* msiSetDefaultResc - Rule handler for setting the scheme for
 * for setting the default resource.
 * defaultResc - the default resource if a resource is not specified
 * optionStr - Thr string "force" indicate that the defaultResc will
 * be used regardless of the user input.
 */
 
int
msiSetDefaultResc (msParam_t *xdefaultRescList, msParam_t *xoptionStr, ruleExecInfo_t *rei)
{
    rescGrpInfo_t *myRescGrpInfo = NULL;
    rescGrpInfo_t *tmpRescGrpInfo, *prevRescGrpInfo;
    keyValPair_t *condInput;
    char *value = NULL;
    strArray_t strArray;
    int i, status;
    char *defaultResc;
    char *defaultRescList;
    char *optionStr;
    int startInx; 

    defaultRescList = (char *) xdefaultRescList->inOutStruct;

    optionStr = (char *) xoptionStr->inOutStruct;

    RE_TEST_MACRO ("    Calling msiSetDefaultResc")

    rei->status = 0;

    if (defaultRescList != NULL && strcmp (defaultRescList, "null") != 0 && 
      optionStr != NULL &&  strcmp (optionStr, "force") == 0 &&
      rei->rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	condInput = NULL;
    } else {
	condInput = &rei->doinp->condInput;
    }

    memset (&strArray, 0, sizeof (strArray));

    status = parseMultiStr (defaultRescList, &strArray);

    if (status <= 0)
        return (0);

    value = strArray.value;
    if (strArray.len <= 1) {
	startInx = 0;
        defaultResc = value;
    } else {
        /* select one on the list randomly */
        startInx = random() % strArray.len;
        defaultResc = &value[startInx * strArray.size];
    }


    if (strcmp (optionStr, "preferred") == 0) {
        rei->status = getRescInfo (rei->rsComm, NULL, condInput,
          &myRescGrpInfo);
	if (rei->status >= 0) {
	    if (strlen (myRescGrpInfo->rescGroupName) > 0) {
	        for (i = 0; i < strArray.len; i++) {
		    int j;
		    j = startInx + i;
		    if (j >= strArray.len) {
		        /* wrap around */
		        j = strArray.len - j;
		    }
	            tmpRescGrpInfo = myRescGrpInfo;
		    prevRescGrpInfo = NULL;
	            while (tmpRescGrpInfo != NULL) {
		        if (strcmp (&value[j * strArray.size], 
		          tmpRescGrpInfo->rescInfo->rescName) == 0) {
			    /* put it on top */  
			    if (prevRescGrpInfo != NULL) {
			        prevRescGrpInfo->next = tmpRescGrpInfo->next;
			        tmpRescGrpInfo->next = myRescGrpInfo;
			        myRescGrpInfo = tmpRescGrpInfo;
			    }
                            break;
		        }
		        prevRescGrpInfo = tmpRescGrpInfo;
	                tmpRescGrpInfo = tmpRescGrpInfo->next;
		    }
		}
	    }
	} else {
	    /* error may mean there is no input resource. try to use the 
	     * default resource by dropping down */
            rei->status = getRescInfo (rei->rsComm, defaultResc, condInput,
              &myRescGrpInfo);
	}
    } else {
        rei->status = getRescInfo (rei->rsComm, defaultResc, condInput, 
          &myRescGrpInfo);
    }

    if (value != NULL)
        free (value);

    if (rei->status >= 0) {
        rei->rgi = myRescGrpInfo;
    } else {
	rei->rgi = NULL;
    }

    return (rei->status);
}

/* msiSetRescSortScheme - Rule handler for setting the scheme for
 * for selecting the best resource to use when creating a data object.
 * sortScheme - The sorting scheme. Valid scheme are "default" and "random".
 */
 
int
msiSetRescSortScheme (msParam_t *xsortScheme, ruleExecInfo_t *rei)
{
    rescGrpInfo_t *myRescGrpInfo;
    char *sortScheme;

    sortScheme = (char *) xsortScheme->inOutStruct;

    RE_TEST_MACRO ("    Calling msiSetRescSortScheme")

    rei->status = 0;
    myRescGrpInfo = rei->rgi;
    if (myRescGrpInfo == NULL) {
	rei->status = SYS_INVALID_RESC_INPUT;
	return (0);
    }
    sortResc (&myRescGrpInfo, &rei->doinp->condInput, sortScheme);
    rei->rgi = myRescGrpInfo;
    return(0);
}

/* msiSetNoDirectRescInp - Set a list of resources that cannot be
 * input directly by a normal user. The resources are seperated by %
 */

int
msiSetNoDirectRescInp (msParam_t *xrescList, ruleExecInfo_t *rei)
{

    keyValPair_t *condInput;
    char *rescName;
    char *value;
    strArray_t strArray; 
    int status, i;
    char *rescList;

    rescList = (char *) xrescList->inOutStruct;

    RE_TEST_MACRO ("    Calling msiSetNoDirectRescInp")

    rei->status = 0;

    if (rescList == NULL || strcmp (rescList, "null") == 0) {
	return (0);
    }

    if (rei->rsComm->proxyUser.authInfo.authFlag >= LOCAL_PRIV_USER_AUTH) {
	/* have enough privilege */
	return (0);
    }

    condInput = &rei->doinp->condInput;

    if ((rescName = getValByKey (condInput, BACKUP_RESC_NAME_KW)) == NULL &&
      (rescName = getValByKey (condInput, DEST_RESC_NAME_KW)) == NULL &&
      (rescName = getValByKey (condInput, RESC_NAME_KW)) == NULL) { 
	return (0);
    }

    memset (&strArray, 0, sizeof (strArray));
 
    status = parseMultiStr (rescList, &strArray);

    if (status <= 0) 
	return (0);

    value = strArray.value;
    for (i = 0; i < strArray.len; i++) {
        if (strcmp (rescName, &value[i * strArray.size]) == 0) {
            /* a match */
            rei->status = USER_DIRECT_RESC_INPUT_ERR;
	    free (value);
            return (USER_DIRECT_RESC_INPUT_ERR);
        }
    }
    if (value != NULL)
        free (value);
    return (0);
}

int 
msiSetDataObjPreferredResc (msParam_t *xpreferredRescList, ruleExecInfo_t *rei)
{
    int writeFlag;
    char *value;
    strArray_t strArray;
    int status, i;
    char *preferredRescList;

    preferredRescList = (char *) xpreferredRescList->inOutStruct;

    RE_TEST_MACRO ("    Calling msiSetDataObjPreferredResc")

    rei->status = 0;

    if (preferredRescList == NULL || strcmp (preferredRescList, "null") == 0) {
	return (0);
    }

    writeFlag = getWriteFlag (rei->doinp->openFlags);

    memset (&strArray, 0, sizeof (strArray));

    status = parseMultiStr (preferredRescList, &strArray);

    if (status <= 0)
        return (0);

    if (rei->doi == NULL || rei->doi->next == NULL)
	return (0);

    value = strArray.value;
    for (i = 0; i < strArray.len; i++) {
        if (requeDataObjInfoByResc (&rei->doi, &value[i * strArray.size], 
          writeFlag, 1) >= 0) {
	    rei->status = 1;
	    return (rei->status);
        }
    }
    return (rei->status);
}

int
msiSetDataObjAvoidResc (msParam_t *xavoidResc, ruleExecInfo_t *rei)
{
    int writeFlag;
    char *avoidResc;

    avoidResc = (char *) xavoidResc->inOutStruct;

    RE_TEST_MACRO ("    Calling msiSetDataObjAvoidResc")

    rei->status = 0;

    writeFlag = getWriteFlag (rei->doinp->openFlags);

    if (avoidResc != NULL && strcmp (avoidResc, "null") != 0) {
        if (requeDataObjInfoByResc (&rei->doi, avoidResc, writeFlag, 0)
          >= 0) {
            rei->status = 1;
        }
    }
    return (rei->status);
}

int
msiSortDataObj (msParam_t *xsortScheme, ruleExecInfo_t *rei)
{
  char *sortScheme;

    sortScheme = (char *) xsortScheme->inOutStruct;
    RE_TEST_MACRO ("    Calling msiSortDataObj")

    if (sortScheme != NULL && strcmp (sortScheme, "random") == 0) {
        sortDataObjInfoRandom (&rei->doi);
    }
    rei->status = 0;
    return (0);
}


/* msiSysChksumDataObj - this rule handling routine to checksum a dataObj 
 * 
 */
int
msiSysChksumDataObj (ruleExecInfo_t *rei)
{
    dataObjInfo_t *dataObjInfoHead;
    char *chksumStr = NULL;

    RE_TEST_MACRO ("    Calling msiSysChksumDataObj")


    rei->status = 0;

    /* don't cache replicate or copy operation */

    dataObjInfoHead = rei->doi;

    if (dataObjInfoHead == NULL) {
        return (0);
    }

    if (strlen (dataObjInfoHead->chksum) == 0) {
	/* not already checksumed */
	rei->status = dataObjChksumAndReg (rei->rsComm, dataObjInfoHead, 
	  &chksumStr);
	if (chksumStr != NULL) {
	    rstrcpy (dataObjInfoHead->chksum, chksumStr,NAME_LEN);
	    free (chksumStr);
	}
    }

    return (0);
}

/**
 * \fn msiSetDataTypeFromExt
 * \author  Wayne Schroeder
 * \date   2007-02-09
 * \brief This microservice checks if the filename has an extension
 * (string following a .) and if so, checks if the iCAT has a matching
 * entry for it, and if so sets the dataObj data_type.
 * \note  Always returns success since it is only doing an attempt;
 *   that is, failure is common and not really a failure.
 * \param[in] just the rei
 * \param[out] nothing
 * \return integer
 * \retval 0 on success
 * \sa 
 * \post
 * \pre
 * \bug  no known bugs
**/
int
msiSetDataTypeFromExt (ruleExecInfo_t *rei)
{
    dataObjInfo_t *dataObjInfoHead;
    int status;
    char logicalCollName[MAX_NAME_LEN];
    char logicalFileName[MAX_NAME_LEN]="";
    char logicalFileName1[MAX_NAME_LEN]="";
    char logicalFileNameExt[MAX_NAME_LEN]="";
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut=NULL;
    char condStr1[MAX_NAME_LEN];
    char condStr2[MAX_NAME_LEN];
    modDataObjMeta_t modDataObjMetaInp;
    keyValPair_t regParam;

    RE_TEST_MACRO ("    Calling msiSetDataType")

    rei->status = 0;

    dataObjInfoHead = rei->doi;

    if (dataObjInfoHead == NULL) {   /* Weirdness */
       return (0); 
    }

    status = splitPathByKey(dataObjInfoHead->objPath, 
			   logicalCollName, logicalFileName, '/');
    if (strlen(logicalFileName)<=0) return(0);

    status = splitPathByKey(logicalFileName, 
			    logicalFileName1, logicalFileNameExt, '.');
    if (strlen(logicalFileNameExt)<=0) return(0);

    /* see if there's an entry in the catalog for  this extension */
    memset (&genQueryInp, 0, sizeof (genQueryInp));

    addInxIval (&genQueryInp.selectInp, COL_TOKEN_NAME, 1);

    snprintf (condStr1, MAX_NAME_LEN, "= 'data_type'");
    addInxVal (&genQueryInp.sqlCondInp,  COL_TOKEN_NAMESPACE, condStr1);

    snprintf (condStr2, MAX_NAME_LEN, "like '%s|.%s|%s'", "%", 
	      logicalFileNameExt,"%");
    addInxVal (&genQueryInp.sqlCondInp,  COL_TOKEN_VALUE2, condStr2);

    genQueryInp.maxRows=1;

    status =  rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);
    if (status != 0) return(0);

    rodsLog (LOG_NOTICE,
	     "query status %d rowCnt=%d",status, genQueryOut->rowCnt);

    if (genQueryOut->rowCnt != 1) return(0);

    status = svrCloseQueryOut (rei->rsComm, genQueryOut);

    /* register it */ 
    memset (&regParam, 0, sizeof (regParam));
    addKeyVal (&regParam, DATA_TYPE_KW,  genQueryOut->sqlResult[0].value);

    modDataObjMetaInp.dataObjInfo = dataObjInfoHead;
    modDataObjMetaInp.regParam = &regParam;

    status = rsModDataObjMeta (rei->rsComm, &modDataObjMetaInp);

    return (0);
}

/* msiStageDataObj - this rule stage an data obj to a cache resource
 * 
 */
int
msiStageDataObj (msParam_t *xcacheResc, ruleExecInfo_t *rei)
{
    int status;
    char *cacheResc;

    cacheResc = (char *) xcacheResc->inOutStruct;

    RE_TEST_MACRO ("    Calling msiStageDataObj")

    rei->status = 0;

    if (cacheResc == NULL || strcmp (cacheResc, "null") == 0) {
        return (rei->status);
    }

    /* preProcessing */
    if (rei->doinp->oprType == REPLICATE_OPR ||
      rei->doinp->oprType == COPY_DEST ||
      rei->doinp->oprType == COPY_SRC) {
        return (rei->status);
    }

    if (getValByKey (&rei->doinp->condInput, RESC_NAME_KW) != NULL ||
      getValByKey (&rei->doinp->condInput, REPL_NUM_KW) != NULL) {
        /* a specific replNum or resource is specified. Don't cache */
        return (rei->status);
    }

    status = msiSysReplDataObj (xcacheResc, NULL, rei);

    return (status);
}

/* msiSysReplDataObj - this rule handling routine replicate a dataObj to the
 * resource.
 */
int
msiSysReplDataObj (msParam_t *xcacheResc, msParam_t *xallFlag,
ruleExecInfo_t *rei)
{
    int writeFlag;
    dataObjInfo_t *dataObjInfoHead, *myDataObjInfo;
    transStat_t transStat;
    char *cacheResc;
    char *allFlag;
    dataObjInp_t dataObjInp;
    char tmpStr[NAME_LEN];

    memset (&dataObjInp, 0, sizeof (dataObjInp));

    cacheResc = (char *) xcacheResc->inOutStruct;
    if (xallFlag != NULL && xallFlag->inOutStruct != NULL) {
        allFlag = (char *) xallFlag->inOutStruct;
	if (strcmp (allFlag, ALL_KW) == 0) {
    	    addKeyVal (&dataObjInp.condInput, ALL_KW, "");
	}
    }
    
    RE_TEST_MACRO ("    Calling msiSysReplDataObj")

    rei->status = 0;

    if (cacheResc == NULL || strcmp (cacheResc, "null") == 0) {
	return (rei->status);
    }

    dataObjInfoHead = rei->doi;

    if (dataObjInfoHead == NULL) {
	return (rei->status);
    }

    writeFlag = getWriteFlag (rei->doinp->openFlags);

    if (requeDataObjInfoByResc (&dataObjInfoHead, cacheResc, writeFlag, 1) 
      >= 0) {
	/* we have a good copy on cache */
	rei->status = 1;
        return (rei->status);
    }

#if 0
    /* stage the copy by replicating it */

    status = _getRescInfo (rei->rsComm, cacheResc, &myRescGrpInfo);

    if (status < 0) {
	rodsLog (LOG_ERROR,
	  "msiSysReplDataObj: cacheResc %s does not exit", cacheResc);
        return (rei->status);
    }

    myDataObjInfo = malloc (sizeof (dataObjInfo_t));
    memset (myDataObjInfo, 0, sizeof (dataObjInfo_t));

    rei->status = _rsDataObjRepl (rei->rsComm, rei->doinp, dataObjInfoHead,
      myRescGrpInfo, &transStat, myDataObjInfo);

    freeAllRescGrpInfo (myRescGrpInfo);
#endif

    myDataObjInfo = malloc (sizeof (dataObjInfo_t));
    memset (myDataObjInfo, 0, sizeof (dataObjInfo_t));
    memset (&transStat, 0, sizeof (transStat));

    rstrcpy (dataObjInp.objPath, dataObjInfoHead->objPath, MAX_NAME_LEN);
    snprintf (tmpStr, NAME_LEN, "%d", dataObjInfoHead->replNum);
    addKeyVal (&dataObjInp.condInput, REPL_NUM_KW, tmpStr);
    addKeyVal (&dataObjInp.condInput, DEST_RESC_NAME_KW, cacheResc);

    rei->status = rsDataObjReplWithOutDataObj (rei->rsComm, &dataObjInp,
      &transStat, myDataObjInfo);

    if (rei->status >= 0) {
	rei->status = 1;
	/* que the cache copy at the top */
	queDataObjInfo (&dataObjInfoHead, myDataObjInfo, 0, 1);
	rei->doi = dataObjInfoHead;
    } else {
	freeAllDataObjInfo (myDataObjInfo);
    }

    return (rei->status);
}

int
msiSetNumThreads (msParam_t *xsizePerThrInMbStr, msParam_t *xmaxNumThrStr, 
msParam_t *xwindowSizeStr, ruleExecInfo_t *rei)
{
    int sizePerThr;
    int maxNumThr;
    dataObjInp_t *doinp;
    int numThr;
    char *sizePerThrInMbStr;
    char *maxNumThrStr;
    char *windowSizeStr;

    sizePerThrInMbStr = (char *) xsizePerThrInMbStr->inOutStruct;
    maxNumThrStr = (char *) xmaxNumThrStr->inOutStruct;
    windowSizeStr = (char *) xwindowSizeStr->inOutStruct;

    if (rei->rsComm != NULL) {
	if (strcmp (windowSizeStr, "null") == 0 || 
	  strcmp (windowSizeStr, "default") == 0) {
	    rei->rsComm->windowSize = 0;
	} else {
	    rei->rsComm->windowSize = atoi (windowSizeStr);
	}
    }

    if (strcmp (sizePerThrInMbStr, "default") == 0) { 
	sizePerThr = SZ_PER_TRAN_THR;
    } else {
	sizePerThr = atoi (sizePerThrInMbStr) * (1024*1024);
        if (sizePerThr <= 0) {
	    rodsLog (LOG_ERROR,
	     "msiSysReplDataObj: Bad input sizePerThrInMb %s", sizePerThrInMbStr);
	    sizePerThr = SZ_PER_TRAN_THR;
	}
    }

    if (strcmp (maxNumThrStr, "default") == 0) {
        maxNumThr = MAX_NUM_TRAN_THR;
    } else {
        maxNumThr = atoi (maxNumThrStr);
        if (maxNumThr <= 0) {
            rodsLog (LOG_ERROR,
             "msiSysReplDataObj: Bad input maxNumThr %s", maxNumThrStr);
            maxNumThr = MAX_NUM_TRAN_THR;
        } else if (maxNumThr > MAX_NUM_CONFIG_TRAN_THR) {
	    rodsLog (LOG_ERROR,
             "msiSysReplDataObj: input maxNumThr %s too large", maxNumThrStr);
	    maxNumThr = MAX_NUM_CONFIG_TRAN_THR;
	}
    }

    doinp = rei->doinp;

    if (doinp->numThreads > 0) {
        numThr = doinp->dataSize / TRANS_BUF_SZ + 1;
        if (numThr > doinp->numThreads) {
            numThr = doinp->numThreads;
        }
    } else {
        numThr = doinp->dataSize / sizePerThr + 1;
    }

    if (numThr > maxNumThr)
        numThr = maxNumThr;

    rei->status = numThr;
    return (rei->status);

}

/* msiDeleteDisallowed - 
 *
 */
int
msiDeleteDisallowed (ruleExecInfo_t *rei)
{
    RE_TEST_MACRO ("    Calling msiDeleteDisallowed")

    rei->status = SYS_DELETE_DISALLOWED;

    return (rei->status);
}

int
msiSetMultiReplPerResc (ruleExecInfo_t *rei)
{
    rstrcpy (rei->statusStr, MULTI_COPIES_PER_RESC, MAX_NAME_LEN);
    return (0);
}

int
msiNoChkFilePathPerm (ruleExecInfo_t *rei)
{
    rei->status = NO_CHK_PATH_PERM;
    return (NO_CHK_PATH_PERM);
}

int
msiNoTrashCan (ruleExecInfo_t *rei)
{
    rei->status = NO_TRASH_CAN;
    return (NO_TRASH_CAN);
}

int
msiSetPublicUserOpr (msParam_t *xoprList, ruleExecInfo_t *rei)
{

    char *value;
    strArray_t strArray; 
    int status, i;
    char *oprList;

    oprList = (char *) xoprList->inOutStruct;

    RE_TEST_MACRO ("    Calling msiSetPublicUserOpr")

    rei->status = 0;

    if (oprList == NULL || strcmp (oprList, "null") == 0) {
	return (0);
    }

    if (rei->rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	/* not enough privilege */
	return (SYS_NO_API_PRIV);
    }

    memset (&strArray, 0, sizeof (strArray));
 
    status = parseMultiStr (oprList, &strArray);

    if (status <= 0) 
	return (0);

    value = strArray.value;
    for (i = 0; i < strArray.len; i++) {
        if (strcmp ("read", &value[i * strArray.size]) == 0) {
            /* a match */
	    setApiPerm (DATA_OBJ_OPEN_AN, PUBLIC_USER_AUTH, PUBLIC_USER_AUTH);
	    setApiPerm (FILE_OPEN_AN, REMOTE_PRIV_USER_AUTH, 
	      PUBLIC_USER_AUTH);
	    setApiPerm (FILE_READ_AN, REMOTE_PRIV_USER_AUTH, 
	      PUBLIC_USER_AUTH);
	    setApiPerm (DATA_OBJ_LSEEK_AN, PUBLIC_USER_AUTH, 
	      PUBLIC_USER_AUTH);
	    setApiPerm (FILE_LSEEK_AN, REMOTE_PRIV_USER_AUTH, 
	      PUBLIC_USER_AUTH);
            setApiPerm (DATA_OBJ_CLOSE_AN, PUBLIC_USER_AUTH,
              PUBLIC_USER_AUTH);
            setApiPerm (FILE_CLOSE_AN, REMOTE_PRIV_USER_AUTH,
              PUBLIC_USER_AUTH);
            setApiPerm (OBJ_STAT_AN, PUBLIC_USER_AUTH,
              PUBLIC_USER_AUTH);
	    setApiPerm (DATA_OBJ_GET_AN, PUBLIC_USER_AUTH, PUBLIC_USER_AUTH);
	    setApiPerm (DATA_GET_AN, REMOTE_PRIV_USER_AUTH, PUBLIC_USER_AUTH);
	} else if (strcmp ("query", &value[i * strArray.size]) == 0) {
	    setApiPerm (GEN_QUERY_AN, PUBLIC_USER_AUTH, PUBLIC_USER_AUTH);
        } else {
            rodsLog (LOG_ERROR,
	     "msiSetPublicUserOpr: operation %s for user public not allowed",
	      &value[i * strArray.size]);
	}
    }

    if (value != NULL)
        free (value);

    return (0);
}

int
setApiPerm (int apiNumber, int proxyPerm, int clientPerm) 
{
    int apiInx;

    if (proxyPerm < NO_USER_AUTH || proxyPerm > LOCAL_PRIV_USER_AUTH) {
	rodsLog (LOG_ERROR,
        "setApiPerm: input proxyPerm %d out of range", proxyPerm);
	return (SYS_INPUT_PERM_OUT_OF_RANGE);
    }

    if (clientPerm < NO_USER_AUTH || clientPerm > LOCAL_PRIV_USER_AUTH) {
        rodsLog (LOG_ERROR,
        "setApiPerm: input clientPerm %d out of range", clientPerm);
        return (SYS_INPUT_PERM_OUT_OF_RANGE);
    }

    apiInx = apiTableLookup (apiNumber);

    if (apiInx < 0) {
	return (apiInx);
    }

    RsApiTable[apiInx].proxyUserAuth = proxyPerm;
    RsApiTable[apiInx].clientUserAuth = clientPerm;

    return (0);
}

int
msiSetGraftPathScheme (msParam_t *xaddUserName, msParam_t *xtrimDirCnt,
ruleExecInfo_t *rei)
{
    char *addUserNameStr;
    char *trimDirCntStr;
    int addUserName;
    int trimDirCnt;
    msParam_t *msParam;
    vaultPathPolicy_t *vaultPathPolicy;

    RE_TEST_MACRO ("    Calling msiSetGraftPathScheme")

    addUserNameStr = (char *) xaddUserName->inOutStruct;
    trimDirCntStr = (char *) xtrimDirCnt->inOutStruct;

    if (strcmp (addUserNameStr, "no") == 0) {
	addUserName = 0;
    } else if (strcmp (addUserNameStr, "yes") == 0) {
        addUserName = 1;
    } else {
        rodsLog (LOG_ERROR,
        "msiSetGraftPathScheme: invalid input addUserName %s", addUserNameStr);
	rei->status = SYS_INPUT_PERM_OUT_OF_RANGE;
        return (SYS_INPUT_PERM_OUT_OF_RANGE);
    }

    if (!isdigit (trimDirCntStr[0])) { 
        rodsLog (LOG_ERROR,
        "msiSetGraftPathScheme: input trimDirCnt %s", trimDirCntStr);
        rei->status = SYS_INPUT_PERM_OUT_OF_RANGE;
        return (SYS_INPUT_PERM_OUT_OF_RANGE);
    } else {
	trimDirCnt = atoi (trimDirCntStr);
    }

    rei->status = 0;

    if ((msParam = getMsParamByLabel (&rei->inOutMsParamArray, 
      VAULT_PATH_POLICY)) != NULL) {
	vaultPathPolicy = (vaultPathPolicy_t *) msParam->inOutStruct;
	if (vaultPathPolicy == NULL) {
	    vaultPathPolicy = malloc (sizeof (vaultPathPolicy_t));
	    msParam->inOutStruct = (void *) vaultPathPolicy; 
	}
        vaultPathPolicy->scheme = GRAFT_PATH_S;
        vaultPathPolicy->addUserName = addUserName;
        vaultPathPolicy->trimDirCnt = trimDirCnt;
	return (0);
    } else {
        vaultPathPolicy = (vaultPathPolicy_t *) malloc (
	  sizeof (vaultPathPolicy_t));
        vaultPathPolicy->scheme = GRAFT_PATH_S;
        vaultPathPolicy->addUserName = addUserName;
        vaultPathPolicy->trimDirCnt = trimDirCnt;
	addMsParam (&rei->inOutMsParamArray, VAULT_PATH_POLICY, 
	  VaultPathPolicy_MS_T, (void *) vaultPathPolicy, NULL);
    }
    return (0);
}

int
msiSetRandomScheme (ruleExecInfo_t *rei)
{
    msParam_t *msParam;
    vaultPathPolicy_t *vaultPathPolicy;

    RE_TEST_MACRO ("    Calling msiSetRandomScheme")

    rei->status = 0;

    if ((msParam = getMsParamByLabel (&rei->inOutMsParamArray, 
      VAULT_PATH_POLICY)) != NULL) {
        vaultPathPolicy = (vaultPathPolicy_t *) msParam->inOutStruct;
        if (vaultPathPolicy == NULL) {
            vaultPathPolicy = malloc (sizeof (vaultPathPolicy_t));
            msParam->inOutStruct = (void *) vaultPathPolicy;
        }
	memset (vaultPathPolicy, 0, sizeof (vaultPathPolicy_t));
        vaultPathPolicy->scheme = RANDOM_S;
        return (0);
    } else {
        vaultPathPolicy = (vaultPathPolicy_t *) malloc (
          sizeof (vaultPathPolicy_t));
       memset (vaultPathPolicy, 0, sizeof (vaultPathPolicy_t));
        vaultPathPolicy->scheme = RANDOM_S;
        addMsParam (&rei->inOutMsParamArray, VAULT_PATH_POLICY, 
	  VaultPathPolicy_MS_T, (void *) vaultPathPolicy, NULL);
    }
    return (0);
}

