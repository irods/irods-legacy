/**
 * @file reSysDataObjOpr.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* reSysDataObjOpr.c */

#include "reSysDataObjOpr.h"
#include "genQuery.h"


/**
 * \fn msiSetDefaultResc (msParam_t *xdefaultRescList, msParam_t *xoptionStr, ruleExecInfo_t *rei)
 *
 * \brief  This microservice sets the default resource and query resource metadata for
 *    the subsequent use based on an input array and condition given
 *    in the dataObject Input Structure.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  Mike Wan
 * \date    2006-11
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-15
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note This function is mandatory even no defaultResc is specified (null) and should be executed right after the screening function msiSetNoDirectRescInp.
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acSetRescSchemeForCreate||msiSetNoDirectRescInp(xyz%demoResc8%abc)##msiSetDefaultResc(demoResc8,noForce)##msiSetRescSortScheme(default)|nop##nop##nop
 *
 * \param[in] xdefaultRescList - Required - a msParam of type STR_MS_T which is a list
 *    of %-delimited resourceNames. It is a resource to use if no resource is input.
 *    A "null" means there is no defaultResc.
 * \param[in] xoptionStr - a msParam of type STR_MS_T which is an option (preferred, forced, random)
 *    with random as default. A "forced" input means the defaultResc will be used regardless
 *    of the user input. The forced action only apply to to users with normal privilege.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->doinp->condInput, 
 *                    rei->rsComm->proxyUser.authInfo.authFlag
 * \DolVarModified rei->rgi gets set to a group (possibly singleton) list of resources in the preferred order.
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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
    } else if (strcmp (optionStr, "forced") == 0) {
        rei->status = getRescInfo (rei->rsComm, defaultResc, NULL,
          &myRescGrpInfo);
    } else {
        rei->status = getRescInfo (rei->rsComm, defaultResc, condInput, 
          &myRescGrpInfo);
    }

    if (rei->status == CAT_NO_ROWS_FOUND) 
      rei->status = SYS_RESC_DOES_NOT_EXIST;

    if (value != NULL)
        free (value);

    if (rei->status >= 0) {
        rei->rgi = myRescGrpInfo;
    } else {
	rei->rgi = NULL;
    }

    return (rei->status);
}

/**
 * \fn msiSetRescSortScheme (msParam_t *xsortScheme, ruleExecInfo_t *rei)
 *
 * \brief  This microservice sets the scheme for selecting the best resource to use when creating a data object.  
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date
 * 
 * \remark Ketan Palshikar - msi documentation 2009-06-15
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note 
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acSetRescSchemeForCreate||msiSetDefaultResc(demoResc,null)##msiSetRescSortScheme(random)##msiSetRescSortScheme(byRescType)|nop##nop##nop
 *
 * \param[in] xsortScheme - The sorting scheme. Valid schemes are "default", "random" and
 *    "byRescType". The "byRescType" scheme will put the cache class of resource on the top
 *    of the list. The scheme "random" and "byRescType" can be applied in sequence.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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


/**
 * \fn msiSetNoDirectRescInp (msParam_t *xrescList, ruleExecInfo_t *rei)
 *
 * \brief  This microservice sets a list of resources that cannot be used by a normal
 *  user directly.  It checks a given list of taboo-resources against the
 *  user provided resource name and disallows if the resource is in the list of taboo-resources.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  Mike Wan
 * \date    2006-11
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-16
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note This microservice is optional, but if used, should be the first function to execute because it screens the resource input.
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acSetRescSchemeForCreate||msiSetNoDirectRescInp(xyz%demoResc8%abc)##msiSetDefaultResc(demoResc8,noForce)##msiSetRescSortScheme(default)|nop##nop##nop
 *
 * \param[in] xrescList - InpParam is a xrescList of type STR_MS_T which is a list of %-delimited resourceNames e.g., resc1%resc2%resc3.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->doinp->condInput - user set  resource list
 *                   rei->rsComm->proxyUser.authInfo.authFlag
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 if user set resource is allowed or user is privileged.
 * \retval USER_DIRECT_RESC_INPUT_ERR  if resource is taboo.
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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
      (rescName = getValByKey (condInput, DEF_RESC_NAME_KW)) == NULL &&
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

/**
 * \fn msiSetDataObjPreferredResc (msParam_t *xpreferredRescList, ruleExecInfo_t *rei)
 *
 * \brief  If the data has multiple copies, this microservice specifies the preferred copy to use.
 * It sets the preferred resources of the opened object.
 *
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-16
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note The copy stored in this preferred resource will be picked if it exists. More than
 * one resource can be input using the character "%" as separator.
 * e.g., resc1%resc2%resc3. The most preferred resource should be at the top of the list.
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acPreprocForDataObjOpen||msiSetDataObjPreferredResc(demoResc7%demoResc8)|nop
 * 
 *
 * \param[in] xpreferredRescList - a msParam of type STR_MS_T, comma-delimited list of resources
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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

/**
 * \fn msiSetDataObjAvoidResc (msParam_t *xavoidResc, ruleExecInfo_t *rei)
 *
 * \brief  This microservice specifies the copy to avoid.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date
 * 
 * \remark Terrell Russell, msi documentation 2009-06-30
 * 
 * \note 
 *  
 * \usage None
 *
 * \param[in] xavoidResc - a msParam of type STR_MS_T - the name of the resource to avoid
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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

/**
 * \fn msiSortDataObj (msParam_t *xsortScheme, ruleExecInfo_t *rei)
 *
 * \brief  This microservice sorts the copies of the data object using this scheme. Currently, "random" sorting scheme is supported.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-16
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note 
 *  
 * \usage
 *
 *  As seen in server/config/reConfigs/core.irb.orig
 *
 * acPreprocForDataObjOpen||msiSortDataObj(random)##msiSetDataObjPreferredResc(xyz%demoResc8%abc)##msiStageDataObj(demoResc8)|nop##nop##nop
 *
 * \param[in] xsortScheme - input sorting scheme.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
int
msiSortDataObj (msParam_t *xsortScheme, ruleExecInfo_t *rei)
{
  char *sortScheme;

    sortScheme = (char *) xsortScheme->inOutStruct;
    RE_TEST_MACRO ("    Calling msiSortDataObj")

    if (sortScheme != NULL) {
	if (strcmp (sortScheme, "random") == 0) {
            sortDataObjInfoRandom (&rei->doi);
        } else if (strcmp (sortScheme, "byRescClass") == 0) {
	    sortObjInfoForOpen (&rei->doi, NULL, 1);
	}
    }
    rei->status = 0;
    return (0);
}


/**
 * \fn msiSysChksumDataObj (ruleExecInfo_t *rei)
 *
 * \brief  This microservice performs a checksum on the uploaded or copied data object.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-16
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acPostProcForPut||msiSysChksumDataObj|nop 
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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
 * \fn msiSetDataTypeFromExt (ruleExecInfo_t *rei)
 *
 * \brief This microservice checks if the filename has an extension
 *    (string following a period (.)) and if so, checks if the iCAT has a matching
 *    entry for it, and if so sets the dataObj data_type.
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author Wayne Schroeder
 * \date   2007-02-09
 *
 * \remark Terrell Russell - msi documentation, 2009-06-22
 *
 * \note  Always returns success since it is only doing an attempt;
 *   that is, failure is common and not really a failure.
 *
 * \usage As seen in the core.irb
 *
 * #acPostProcForPut||msiSetDataTypeFromExt|nop
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa 
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

    /* see if there's an entry in the catalog for this extension */
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

/**
 * \fn msiStageDataObj (msParam_t *xcacheResc, ruleExecInfo_t *rei)
 *
 * \brief  This microservice stages the data object to the specified resource
 *    before operation. It stages a copy of the data object in the cacheResc before
 *    opening the data object.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-16
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note 
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acPreprocForDataObjOpen||msiSortDataObj(random)##msiSetDataObjPreferredResc(xyz%demoResc8%abc)##msiStageDataObj(demoResc8)|nop##nop##nop
 * 
 * \param[in] xcacheResc - The resource name in which to cache the object
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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

/**
 * \fn msiSysReplDataObj (msParam_t *xcacheResc, msParam_t *xflag, ruleExecInfo_t *rei)
 *
 * \brief  This microservice replicates a data object. It can be used to replicate
 *  a copy of the file just uploaded or copied data object to the specified
 *  replResc. 
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-16
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note The allFlag is only meaningful if the replResc is a resource group.
 *  In this case, setting allFlag to "all" means a copy will be made in all the
 *  resources in the resource group. A "null" input means a single copy will be made in
 *  one of the resources in the resource group.
 *  
 * \usage None
 * 
 * \param[in] xcacheResc - 
 * \param[in] xflag - 
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
int
msiSysReplDataObj (msParam_t *xcacheResc, msParam_t *xflag,
ruleExecInfo_t *rei)
{
    int writeFlag;
    dataObjInfo_t *dataObjInfoHead;
    char *cacheResc;
    char *flag = NULL;

    cacheResc = (char *) xcacheResc->inOutStruct;
    if (xflag != NULL && xflag->inOutStruct != NULL) {
        flag = (char *) xflag->inOutStruct;
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

    rei->status = rsReplAndRequeDataObjInfo (rei->rsComm, &dataObjInfoHead, 
     cacheResc, flag);
    if (rei->status >= 0) {
	 rei->doi = dataObjInfoHead;
    }
    return (rei->status);
}

/**
 * \fn msiSetNumThreads (msParam_t *xsizePerThrInMbStr, msParam_t *xmaxNumThrStr, msParam_t *xwindowSizeStr, ruleExecInfo_t *rei)
 *
 * \brief  This microservice specifies the parameters for determining the number of
 *    threads to use for data transfer. It sets the number of threads and the TCP window size. 
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-16
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note The msiSetNumThreads function must be present or no thread will be used for all transfer.
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 *  acSetNumThreads||msiSetNumThreads(16,4,default)|nop  
 * 
 * \param[in] xsizePerThrInMbStr - The number of threads is computed
 *    using: numThreads = fileSizeInMb / sizePerThrInMb + 1 where sizePerThrInMb
 *    is an integer value in MBytes. It also accepts the word "default" which sets
 *    sizePerThrInMb to a default value of 32.
 * \param[in] xmaxNumThrStr - The maximum number of threads to use. It accepts integer
 *    value up to 16. It also accepts the word "default" which sets maxNumThr to a default value of 4.
 * \param[in] xwindowSizeStr - The TCP window size in Bytes for the parallel transfer. A value of 0 or "dafault" means a default size of 1,048,576 Bytes.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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

/**
 * \fn msiDeleteDisallowed (ruleExecInfo_t *rei)
 *
 * \brief  This microservice sets the policy for determining that certain data cannot be deleted.   
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author
 * \date 
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-17
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note 
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acDataDeletePolicy|$objPath like /foo/bar\*|msiDeleteDisallowed|nop 
 * (the \ should be / but was changed to avoid a compiler warning about
 * a slash* in a comment.)
 *
 * This rule prevents the deletion of any data objects or collections beneath the collection /foo/bar/
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
int
msiDeleteDisallowed (ruleExecInfo_t *rei)
{
    RE_TEST_MACRO ("    Calling msiDeleteDisallowed")

    rei->status = SYS_DELETE_DISALLOWED;

    return (rei->status);
}

/**
 * \fn msiSetMultiReplPerResc (ruleExecInfo_t *rei)
 *
 * \brief  By default, the system allows one copy per resource. This microservice sets the number of copies per resource to unlimited.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author
 * \date 
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-17
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note
 *  
 * \usage None
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
int
msiSetMultiReplPerResc (ruleExecInfo_t *rei)
{
    rstrcpy (rei->statusStr, MULTI_COPIES_PER_RESC, MAX_NAME_LEN);
    return (0);
}

/**
 * \fn msiNoChkFilePathPerm (ruleExecInfo_t *rei)
 *
 * \brief  This microservice does not check file path permissions when registering a file.  
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author
 * \date 
 * 
 * \remark Ketan Palshikar - created msi documentation, 2009-06-17
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \warning WARNING - This function can create a security problem if used incorrectly.
 *  
 * \note 
 *  
 * \usage None
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval NO_CHK_PATH_PERM
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
int
msiNoChkFilePathPerm (ruleExecInfo_t *rei)
{
    rei->status = NO_CHK_PATH_PERM;
    return (NO_CHK_PATH_PERM);
}

/**
 * \fn msiNoTrashCan (ruleExecInfo_t *rei)
 *
 * \brief  This microservice sets the policy to no trash can.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author
 * \date 
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-17
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acTrashPolicy||msiNoTrashCan|nop
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval NO_TRASH_CAN
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
int
msiNoTrashCan (ruleExecInfo_t *rei)
{
    rei->status = NO_TRASH_CAN;
    return (NO_TRASH_CAN);
}

/**
 * \fn msiSetPublicUserOpr (msParam_t *xoprList, ruleExecInfo_t *rei)
 *
 * \brief  This microservice sets a list of operations that can be performed by the user "public".
 *  
 * \module core
 *  
 * \since pre-2.1
 *  
 * \author
 * \date 
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-17
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acSetPublicUserPolicy||msiSetPublicUserOpr(read%query)|nop
 *
 * \param[in] xoprList - Only 2 operations are allowed - "read" - read files; "query" - browse some system level metadata. More than one operation can be
 *                       input using the character "%" as seperator. e.g., read%query.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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

/**
 * \fn msiSetGraftPathScheme (msParam_t *xaddUserName, msParam_t *xtrimDirCnt, ruleExecInfo_t *rei)
 *
 * \brief  This microservice sets the VaultPath scheme to GRAFT_PATH.
 *    It grafts (adds) the logical path to the vault path of the resource
 *    when generating the physical path for a data object.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author
 * \date 
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-17
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acSetVaultPathPolicy||msiSetGraftPathScheme(no,2)|nop
 *
 * \param[in] xaddUserName - This msParam specifies whether the userName should
 *      be added to the physical path. e.g. $vaultPath/$userName/$logicalPath.
 *      "xaddUserName" can have two values - yes or no.
 * \param[in] xtrimDirCnt - This msParam specifies the number of leading directory
 *      elements of the logical path to trim. Sometimes it may not be desirable to
 *      graft the entire logical path. e.g.,for a logicalPath /myZone/home/me/foo/bar,
 *      it may be desirable to graft just the part "foo/bar" to the vaultPath.
 *      "xtrimDirCnt" should be set to 3 in this case.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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

/**
 * \fn msiSetRandomScheme (ruleExecInfo_t *rei)
 *
 * \brief  This microservice sets the scheme for composing the physical path in the vault to RANDOM.  A randomly generated path is appended to the 
 *         vaultPath when generating the physical path. e.g., $vaultPath/$userName/$randomPath. The advantage with the RANDOM scheme is renaming 
 *         operations (imv, irm) are much faster because there is no need to rename the corresponding physical path. 
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author
 * \date 
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-17
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note
 *  
 * \usage
 *
 * As seen in server/config/reConfigs/core.irb.orig
 *
 * acSetVaultPathPolicy||msiSetRandomScheme|nop
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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


/**
 * \fn msiSetReServerNumProc (msParam_t *xnumProc, ruleExecInfo_t *rei)
 *
 * \brief  Sets number of processes for the rule engine server
 * 
 * \module core
 * 
 * \since 2.1
 * 
 * \author
 * \date 
 * 
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note
 *  
 * \usage None
 *
 *
 * \param[in] xnumProc - a STR_MS_T representing number of processes
 *     - this value can be "default" or an integer
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
int
msiSetReServerNumProc (msParam_t *xnumProc, ruleExecInfo_t *rei)
{
    char *numProcStr;
    int numProc;

    numProcStr = xnumProc->inOutStruct;

    if (strcmp (numProcStr, "default") == 0) {
	numProc = DEF_NUM_RE_PROCS;
    } else {
	numProc = atoi (numProcStr);
	if (numProc > MAX_RE_PROCS) {
	    numProc = MAX_RE_PROCS;
	} else if (numProc <= 0) {
	    numProc = DEF_NUM_RE_PROCS;
	}
    }
    rei->status = numProc;

    return (numProc);
}

