// srcEvBiWe2Ta7.c

int msiGetDataObjChksumsTimeStampsFromAVUVol7 (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * outParam1, ruleExecInfo_t * rei);

int
intChkRechkRecompChkSum4DatObjVol7 (rsComm_t * rsComm, char *strFullDataPath,
				time_t tTime, ruleExecInfo_t * rei);
int
intGetDataObjAVUsVol7 (collInp_t * ptrInpColl,
				       UserDefinedMetadata_t * aAVUarray,
				       int *iTotalAVUs, char *strOut,
				       ruleExecInfo_t * rei);
int intAddChkSumDateAvuMetadataVol7 (rsComm_t * rsComm, char *objPath, time_t t1, int *iStatus);

/* *********************************************************************************************************** */
/*
 * \fn msiChkRechkRecompChkSum4DatObjVol7
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

/* *********************************************************************************************************** */
/*
  msiRecurzzzColl (msParam_t *coll, msParam_t *destRescName, msParam_t *options,  msParam_t *outParam, ruleExecInfo_t *rei)
 msiChkRechkRecompChkSum4DatObj222 (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * outParam1, ruleExecInfo_t * rei)
*/

int
msiChkRechkRecompChkSum4DatObjVol7 (msParam_t *coll, msParam_t * inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    collInp_t collInp, *myCollInp;
    int iErr=0, i, continueInx, status;
    transStat_t *transStat = NULL;
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    dataObjInp_t dataObjInp; 
    time_t t1;
    char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strAtributeName;
    //long lTmp = 0;

    UserDefinedMetadata_t aAVUarray[1024];
    //collInp_t *ptrInpColl;
    int iCountUserDefinedMetadata = 0;
  
    RE_TEST_MACRO ("    Calling msiChkRechkRecompChkSum4DatObjVol7")

    if (rei == NULL || rei->rsComm == NULL) {
	    rodsLog (LOG_ERROR,
	    "msiChkRechkRecompChkSum4DatObjVol7: input rei or rsComm is NULL");
	    return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    //  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);
    /* parse inpParam1: coll */
    rei->status = parseMspForCollInp (coll, &collInp,  &myCollInp, 0);
    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiChkRechkRecompChkSum4DatObjVol7: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    /* parse inpParam2 */
    if ((strAtributeName = parseMspForStr (inpParam2)) != NULL)
      {
      }
    else
      {
	sprintf (strOut,
		 "ERROR:  msiChkRechkRecompChkSum4DatObjVol7(), input inpParam2 error\n");
	rodsLog (LOG_ERROR,
	       "msiChkRechkRecompChkSum4DatObjVol7(),  input inpParam2 error.");
	i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
	return (-1);
      }
    
#ifdef no1
    rodsObjStat_t *rodsObjStatOut;

    int status;
    
    
    /* check for valid connection */
    if (rsComm == NULL) {
        rodsLog (LOG_ERROR, "getDataObjACL: input rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    
    
    memset (&genQueryInp, 0, sizeof (genQueryInp_t));
    
    status = rsObjStat(rsComm, myDataObjInp, &rodsObjStatOut);

#endif

    printf("GJK-P P.0004000.0.0. in msiChkRechkRecompChk(), dataObjInp.objPath=(%s), strAtributeName=(%s)\n", dataObjInp.objPath, strAtributeName);

    /* iterate through all files */
    memset (&genQueryInp, 0, sizeof (genQueryInp));
    status = rsQueryDataObjInCollReCur (rsComm, myCollInp->collName, 
      &genQueryInp, &genQueryOut, NULL, 1);
    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
    	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
    	  "msiChkRechkRecompChkSum4DatObjVol7: msiChkRechkRecompChkSum4DatObjVol7 error for %s, stat=%d",
    	  myCollInp->collName, status);
    	rei->status=status;
      return (rei->status);
    }
    while (rei->status >= 0) {
      sqlResult_t *subColl, *dataObj;
      /* get sub coll paths in the batch */
      subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME);
      dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
      if (subColl == NULL) {
	if (dataObj == NULL && status < 0) {
	  rodsLog (LOG_ERROR,
		   "msiChkRechkRecompChkSum4DatObjVol7: msiChkRechkRecompChkSum4DatObjVol7 failed, (%s) is not an iRods data object, istatus=%d, rei->status=%d", myCollInp->collName, status, rei->status);
	  rei->status=UNMATCHED_KEY_OR_INDEX;
	  return (rei->status);
	}
	else {
	  /* delej single object */ 
	  iErr =
	    intGetDataObjAVUsVol7 (myCollInp, aAVUarray,
				   &iCountUserDefinedMetadata, strOut,
				   rei);
	  
	  // iErr = intChkRechkRecompChkSum4DatObjVol7 (rsComm, dataObjInp.objPath, t1, rei);	/* test blbost sobota */
	  printf("GJK-P P.4001.0.2. in msiChkRechkRecompChkSum4DatObjVol7(), dataObjInp.objPath=(%s), i=%d\n", dataObjInp.objPath, i);
	  /* GJK return(0); */
	}
	rodsLog (LOG_ERROR,
		 "msiChkRechkRecompChkSum4DatObjVol7: msiChkRechkRecompChkSum4DatObjVol7 failed, (%s) is not an iRods collection, rei->status=%d", myCollInp->collName, rei->status);
	rei->status=UNMATCHED_KEY_OR_INDEX;  
	return (rei->status);
      }
      /* get data names in the batch */
      if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR, 
              "msiChkRechkRecompChkSum4DatObjVol7: msiChkRechkRecompChkSum4DatObjVol7 for COL_DATA_NAME failed");
            rei->status=UNMATCHED_KEY_OR_INDEX;   
            return (rei->status);
      }
      
      for (i = 0; i < genQueryOut->rowCnt; i++) {
        char *tmpSubColl, *tmpDataName;

        tmpSubColl = &subColl->value[subColl->len * i];
        tmpDataName = &dataObj->value[dataObj->len * i];
        snprintf (dataObjInp.objPath, MAX_NAME_LEN, "%s/%s",
              tmpSubColl, tmpDataName);
	/* GJK  rei->status = rsDataObjRepl (rsComm, &dataObjInp, &transStat); */
        rei->status = 0; 
        if (rei->status<0)
        {
          rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
            "msiChkRechkRecompChkSum4DatObjVol7: rsDataObjRepl failed %s, status = %d",
			    (&dataObjInp)->objPath,
          rei->status);
        }
	else {
	  iErr = intChkRechkRecompChkSum4DatObjVol7 (rsComm, dataObjInp.objPath, t1, rei);	/*  test blbost sobota */
	  printf("GJK-P P.4001.0.1. in msiChkRechkRecompChkSum4DatObjVol7(), dataObjInp.objPath=(%s), i=%d\n", dataObjInp.objPath, i);
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
          "dataObjInp: msiChkRechkRecompChkSum4DatObjVol7 failed (should have catched earlier) %s, status = %d",
			    (&dataObjInp)->objPath,
          rei->status);
    }
    return (rei->status);
} /*  msiChkRechkRecompChkSum4DatObjVol7 */

/* *********************************************************************************************************** */
/*
 
Input : iRods absulute path of an object or collection,
Unix time in seconds for the time , if the file wa not checket after that input time, than check it and recompute the sum
and updata the AVY time stamp
*/

int
msiChkRechkRecompChkSum4DatObjVol7222 (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * outParam1, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInpCache, *ptrInpColl;
  int iErr = 0, i = 0;
  int iCountUserDefinedMetadata = 0;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strAtributeName;
  long lTmp = 0;
  time_t t1;

  /* For testing mode when used with irule --test */
  RE_TEST_MACRO ("RE_TEST_MACRO, begin of msiChkRechkRecompChkSum4DatObjVol7");

  printf ("GJK-P P.2222.0.1. in msiChkRechkRecompChkSum4DatObjVol7()\n");

  rsComm = rei->rsComm;

  /* parse inpParam1 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiChkRechkRecompChkSum4DatObjVol7(),  input inpParam1 error. status = %d",
	       rei->status);
      return (rei->status);
    }

  /* parse inpParam2 */
  if ((strAtributeName = parseMspForStr (inpParam2)) != NULL)
    {
      lTmp = strtol (strAtributeName, (char **) NULL, 10);
      t1 = (time_t) lTmp;
    }
  else
    {
      sprintf (strOut,
	       "ERROR:  msiChkRechkRecompChkSum4DatObjVol7(), input inpParam2 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkRechkRecompChkSum4DatObjVol7(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
      return (-1);
    }

  printf
    ("GJK-P P.2222.0.2. in msiChkRechkRecompChkSum4DatObjVol7(), ptrInpColl->collName=(%s), t1=%ld\n",
     ptrInpColl->collName, t1);

  iErr = intChkRechkRecompChkSum4DatObjVol7 (rsComm, ptrInpColl->collName, t1, rei);	/* test blbost sobota */
  /*  (void) intChkRechkRecompChkSum4DatObjVol7 (rsComm, strFullDataPath, t1, rei); */

  sprintf (strOut,
	   "OK msiChkRechkRecompChkSum4DatObjVol7(), iCountUserDefinedMetadata=%d, t1=(%ld), iErr=%d\n",
	   iCountUserDefinedMetadata, t1, iErr);
  i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
  /* fillBuffInParam */

  printf
    ("GJK-P P.2222.0.9. in msiChkRechkRecompChkSum4DatObjVol7(), iCountUserDefinedMetadata=%d, iErr=%d\n",
     iCountUserDefinedMetadata, iErr);

  return (iErr);
}

/* *********************************************************************************************************** */
int
intChkRechkRecompChkSum4DatObjVol7 (rsComm_t * rsComm, char *strFullDataPath,
				time_t tTime, ruleExecInfo_t * rei)
{
  collInp_t ptrInpColl;
  int iCountUserDefinedMetadata = 0, iTotalAVUs = 0;
  long lMax = 0, lTmp;
  UserDefinedMetadata_t aAVUarray[1024];
  genQueryInp_t genQueryInp;
  genQueryOut_t *genQueryOut;
  sqlResult_t *chksumStr, *modTimVal, *creaTimVal;
  char *objPath;
  int status = 0, i = 0, iErr = 0;
  char *tmpChksumStr, *strModTime, *strCreaTime;
  char collQCond[MAX_NAME_LEN];
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN];
  time_t t1;

  printf
    ("GJK-P P.994.12.1. in intChkRechkRecompChkSum4DatObjVol7(), strFullDataPath=(%s)\n",
     strFullDataPath);

  if ((long) tTime < 0)
    {
      rodsLog (LOG_ERROR,
	       "ERROR in intChkRechkRecompChkSum4DatObjVol7, tTime=(%ld) < 0",
	       tTime);
      return (-1);
    }

  if (strFullDataPath == NULL || strlen (strFullDataPath) < 1)
    {
      rodsLog (LOG_ERROR,
	       "ERROR in intChkRechkRecompChkSum4DatObjVol7, strFullDataPath=(%s) is strange ",
	       strFullDataPath);
      return (-2);
    }

  strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN);
  iErr =
    intGetDataObjAVUsVol7 (&ptrInpColl, aAVUarray,
					   &iCountUserDefinedMetadata, strOut,
					   rei);

  printf ("GJK-P P.994.7.0.a strOut=(%s)", strOut);
  iTotalAVUs = iCountUserDefinedMetadata;
  for (i = 0; i < iTotalAVUs; i++)
    {
      lTmp = strtol (aAVUarray[i].value, (char **) NULL, 10);
      if (lMax < lTmp)
	lMax = lTmp;
    }
  /* tTime time of requested last checking */
  if (iTotalAVUs > 0 && ((tTime - lMax) <= 0))
    {      /*  mam uz AVU a je novejsi ZZZ1  */
      printf
	("GJK-P P.994.7.1. in intChkRechkRecompChkSum4DatObjVol7(), mam uz AVU a je novejsi, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	 iTotalAVUs, lMax, tTime, (tTime - lMax));
      rodsLog (LOG_NOTICE,
	       "GJK-P P.994.7.1. in intChkRechkRecompChkSum4DatObjVol7(), mam uz AVU a je novejsi, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	       iTotalAVUs, lMax, tTime, (tTime - lMax));
      return (0);
    }
  else
    {
      if (iTotalAVUs > 0)
	{	  /* mam uz AVU a je starsi 
		     prepocti chksumu, porovnej a register novy cas 
		     VERIFY_CHKSUM_KW */

	  dataObjInp_t dataObjInp;
	  char *dataObjChksumStr = NULL;
	  dataObjInfo_t *dataObjInfoHead = NULL;

	  /* zero the struct and fill in the iRods object/file name */
	  memset (&dataObjInp, 0, sizeof (dataObjInp));
	  /* fix '...foo.pl/' pozdeji   */
	  rstrcpy (dataObjInp.objPath, strFullDataPath, MAX_NAME_LEN);

	  /* move the cond */
	  memset (&dataObjInp.condInput, 0, sizeof (keyValPair_t));
	  addKeyVal (&dataObjInp.condInput, VERIFY_CHKSUM_KW, "");

	  rei->status =
	    _rsDataObjChksum (rsComm, &dataObjInp, &dataObjChksumStr,
			      &dataObjInfoHead);
	  printf
	    ("GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObjVol7(), mam uz AVU a je starsi, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	     iTotalAVUs, lMax, tTime, (tTime - lMax));
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObjVol7(), mam uz AVU a je starsi, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
		   iTotalAVUs, lMax, tTime, (tTime - lMax));

	  (void) time (&t1);
	  if (rei->status != 0)
	    {
	      rodsLog (LOG_ERROR,
		       "GJK-P P.994.27.1b. ERROR in intChkRechkRecompChkSum4DatObjVol7() in _rsDataObjChksum(), iRods object (%s), returned check sum (%s)\n",
		       dataObjInp.objPath, dataObjChksumStr);
	      return (-2);
	    }
	  /* CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME     */
	  iErr =
	    intAddChkSumDateAvuMetadataVol7 (rei->rsComm, strFullDataPath, t1,
					 &status);
	  if (iErr != 0)
	    {
	      rodsLog (LOG_ERROR,
		       "GJK-P P.994.27.1c. ERROR in intChkRechkRecompChkSum4DatObjVol7() in intAddChkSumDateAvuMetadataVol7(),  iRods object (%s), returned check status %d\n",
		       strFullDataPath, status);
	      return (-3);
	    }
	  printf
	    ("GJK-P P.994.17.1. in intChkRechkRecompChkSum4DatObjVol7(),mam uz AVU a je starsi, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, tTime=%ld, iErr=%d, t1=%ld, rei->status=%d, dataObjInp.objPath=(%s), *dataObjChksumStr=(%s)\n",
	     iTotalAVUs, lMax, (long) tTime, iErr, (long) t1, rei->status,
	     dataObjInp.objPath, dataObjChksumStr);
	  return (iErr);
	}
      else
	{	/*      if (iTotalAVUs > 0)  , nemam zadne AVUs */
	  printf
	    ("GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObjVol7(), nemam uz AVU, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	     iTotalAVUs, lMax, tTime, (tTime - lMax));
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObjVol7(), nemam uz AVU, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
		   iTotalAVUs, lMax, tTime, (tTime - lMax));
	  iTotalAVUs = iTotalAVUs;
	}
    }

  /* Get all collections (recursively) under our input collection */
  /* Prepare query */
  memset (&genQueryInp, 0, sizeof (genQueryInp_t));
  genAllInCollQCond (strFullDataPath, collQCond);

  addInxIval (&genQueryInp.selectInp, COL_D_DATA_CHECKSUM, 1);
  addInxIval (&genQueryInp.selectInp, COL_D_MODIFY_TIME, 1);
  addInxIval (&genQueryInp.selectInp, COL_D_CREATE_TIME, 1);

  genQueryInp.maxRows = MAX_SQL_ROWS;

  /* ICAT query for subcollections */
  rei->status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

  printf
    ("GJK-P P.994.0.2. in intChkRechkRecompChkSum4DatObjVol7(), rei->status=(%d), strFullDataPath=(%s), iCountUserDefinedMetadata=(%d)\n",
     rei->status, strFullDataPath, iCountUserDefinedMetadata);

  if (rei->status != CAT_NO_ROWS_FOUND)
    {
      printf
	("GJK-P P.994.3.3. in intChkRechkRecompChkSum4DatObjVol7(), rei->status=(%d), genQueryOut->rowCnt=(%d), strFullDataPath=(%s)\n",
	 rei->status, genQueryOut->rowCnt, strFullDataPath);

      if (1 != genQueryOut->rowCnt)
	{
	  printf
	    ("GJK-P P.994.4.4. in intChkRechkRecompChkSum4DatObjVol7(), rei->status=(%d), genQueryOut->rowCnt=(%d), strFullDataPath=(%s)\n",
	     rei->status, genQueryOut->rowCnt, strFullDataPath);
	  /* return(-1);  not enough lines); */
	}

      if ((chksumStr =	   getSqlResultByInx (genQueryOut, COL_D_DATA_CHECKSUM)) == NULL)
	{
	  rodsLog (LOG_ERROR,
		   "printLsLong: getSqlResultByInx for COL_D_DATA_CHECKSUM failed GJK-(%s)",
		   objPath);
	  /* return (UNMATCHED_KEY_OR_INDEX); */
	}
      else
	{
	  tmpChksumStr = &chksumStr->value[chksumStr->len * i];
	  printf	    ("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObjVol7(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), tmpChksumStr=(%s)\n",	     objPath, tmpChksumStr);
	}

      if ((modTimVal =	   getSqlResultByInx (genQueryOut, COL_D_MODIFY_TIME)) == NULL)
	{
	  rodsLog (LOG_ERROR,		   "printLsLong: getSqlResultByInx for COL_D_MODIFY_TIME failed GJK-(%s)",		   objPath);
	  /* return (UNMATCHED_KEY_OR_INDEX); */
	}
      else
	{
	  strModTime = &modTimVal->value[modTimVal->len * i];
	  printf	    ("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObjVol7(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), tmpChksumStr=(%s)\n",
	     objPath, tmpChksumStr);
	}

      if ((creaTimVal =	   getSqlResultByInx (genQueryOut, COL_D_CREATE_TIME)) == NULL)
	{
	  rodsLog (LOG_ERROR,
		   "printLsLong: getSqlResultByInx for COL_D_CREATE_TIME failed GJK-(%s)",
		   objPath);
	  /* return (UNMATCHED_KEY_OR_INDEX); */
	}
      else
	{
	  strCreaTime = &creaTimVal->value[creaTimVal->len * i];
	  printf
	    ("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObjVol7(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), tmpChksumStr=(%s)\n",
	     objPath, tmpChksumStr);
	}

      printf	("GJK-P P.994.6.6. in intChkRechkRecompChkSum4DatObjVol7(), rei->status=(%d), genQueryOut->rowCnt=(%d), strFullDataPath=(%s)\n",
	 rei->status, genQueryOut->rowCnt, strFullDataPath);
      /* tady je moje maso AAA ZZZ */

      if (iCountUserDefinedMetadata > 0)
	{	/* mam check sum cas
		   nedelej nic
		   kdy rozdil casu neni moc velky
		*/
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.0.1. in intChkRechkRecompChkSum4DatObjVol7(), mam check sum cas a tedy nedelej nic, after GJKgetDataObjPSmeta((%s) rsComm\n",
		   objPath);
	  printf
	    ("GJK-P P.994.0.1. in intChkRechkRecompChkSum4DatObjVol7(), mam check sum cas a tedy nedelej nic, after GJKgetDataObjPSmeta((%s) rsComm\n",
	     objPath);
	}
      else
	{			/* nemam check sum cas */
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.0.2. in intChkRechkRecompChkSum4DatObjVol7(), nemam check sum cas, after GJKgetDataObjPSmeta(%s), rsComm\n",
		   objPath);
	  printf
	    ("GJK-P P.994.0.2. in intChkRechkRecompChkSum4DatObjVol7(), nemam check sum cas, after GJKgetDataObjPSmeta(%s), rsComm\n",
	     objPath);
	  if (strlen (tmpChksumStr) ==
	      strlen ("6d75827809277a1d50c0ed742764a82c") && 1 == 1)
	    {
	      /* mam check sum hodnotu
		 nemam check sum cas
		 insert check sum cas in Unix number
		 Call the function to insert metadata here. */
	      rodsLog (LOG_NOTICE,
		       "GJK-P P.994.99.2. in intChkRechkRecompChkSum4DatObjVol7(), nemam check sum cas, mam check sum hodnotu, after GJKgetDataObjPSmeta(%s), rsComm\n",
		       objPath);
	    }
	  else
	    {
	      /* nemam check sum hodnotu
		 vypocti check sum hodnotu
		 instert check sum hodnotu do iCat
		 insert check sum cas == ted */
	      
	      rodsLog (LOG_NOTICE,
		       "GJK-P P.994.99.2. in intChkRechkRecompChkSum4DatObjVol7(), nemam check sum cas, mam check sum hodnotu, after GJKgetDataObjPSmeta(%s), rsComm\n",
		       objPath);

	      printf		("GJK-P P.994.0.4. in intChkRechkRecompChkSum4DatObjVol7(), after GJKgetDataObjPSmeta(%s), rsComm\n",
		 objPath);
	    }
	  printf	    ("GJK-P P.994.0.5. in intChkRechkRecompChkSum4DatObjVol7(), after GJKgetDataObjPSmeta((%s), rsComm\n",
	     objPath);
	}

      printf	("GJK-P P.994.0.6. in intChkRechkRecompChkSum4DatObjVol7(), after GJKgetDataObjPSmeta((%s), rsComm\n",
	 objPath);
    }
  printf ("GJK-P P.994.0.8. in intChkRechkRecompChkSum4DatObjVol7()\n");
  return(0);
}

/* *********************************************************************************************************** */
int intAddChkSumDateAvuMetadataVol7 (rsComm_t * rsComm, char *objPath, time_t t1,			     int *iStatus)
{
  modAVUMetadataInp_t modAVUMetadataInp;
  char mytime[256], *chrPtr1;

  chrPtr1 = strrchr (objPath, '/');
  printf
    ("GJK-P P.1.0.1. in intGetDataObjAVUsVol7(), chrPtr1=(%s), objPath=(%s)\n",
     chrPtr1, objPath);
  if (chrPtr1 != NULL && *chrPtr1 == '/'
      && chrPtr1[strlen (chrPtr1) - 1] == '/')
    *chrPtr1 = 0;		/* replace '/' in /myzone/foo/' */
  printf
    ("GJK-P P.1.0.2. in intGetDataObjAVUsVol7(), chrPtr1=(%s), objPath=(%s)\n",
     chrPtr1, objPath);

  memset (&modAVUMetadataInp, 0, sizeof (modAVUMetadataInp));

  modAVUMetadataInp.arg0 = "add";
  modAVUMetadataInp.arg1 = "-d";	/* data */
  modAVUMetadataInp.arg2 = objPath;
  modAVUMetadataInp.arg3 = "MD5checkSumDataStamp";

  (void)
    printf
    ("GJK-P P.123.0.1. in intAddChkSumDateAvuMetadataVol7(), after rsModAVUMetadata((%s), rsComm\n",
     objPath);
#if defined(osx_platform)
  if ((long) t1 < 190509697L || (long) t1 > LONG_MAX)
#else
  if ((long) t1 < 190509697L || (long) t1 > MAXLONG)
#endif
    {
      (void) rodsLog (LOG_ERROR,
		      "The Unix time (%d) is out of reasonable bounds for intAddChkSumDateAvuMetadataVol7() for iRods data object (%s) ",
		      (int) t1, objPath);
      return (-1);
    }
  (void) snprintf (mytime, 255, "%d", (int) t1);
  modAVUMetadataInp.arg4 = mytime;
  modAVUMetadataInp.arg5 = "UnixTimeInSeconds";
  *iStatus = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
  (void)
    printf
    ("GJK-P P.123.0.2. in intAddChkSumDateAvuMetadataVol7(), after rsModAVUMetadata((%s), rsComm\n",
     objPath);
  if (0 != *iStatus)
    (void) rodsLog (LOG_ERROR,
		    "intAddChkSumDateAvuMetadataVol7() rsModAVUMetadata failed objPath=(%s)",
		    objPath);
  else
    {
      (void)
	printf
	("GJK-P P.123.0.3. in intAddChkSumDateAvuMetadataVol7(), after rsModAVUMetadata((%s), sComm, mytime=%ld, *iStatus=%d\n",
	 objPath, (long)mytime, *iStatus);
    }
  (void)
    printf
    ("GJK-P P.123.0.4. OK in intAddChkSumDateAvuMetadataVol7(), after rsModAVUMetadata((%s), rsComm, *iStatus=%d\n",
     objPath, *iStatus);

  return (*iStatus);
}

/* *********************************************************************************************************** */
int
intFindChkSumDateAvuMetadataVol7 (int status, genQueryOut_t * genQueryOut,
			    char *fullName, UserDefinedMetadata_t UAVArray[],
			    int *iCountUserDefinedMetadata)
{
  int i = 0, j = 0, iResult = 0;
  size_t size;

  *iCountUserDefinedMetadata = 0;

  printf
    ("GJK 0001300.0.0. intFindChkSumDateAvuMetadataVol7, fullName=(%s), i=%d, status=%d\n",
     fullName, i, status);

  /* return (0); */

  if (status != 0)
    {
      rodsLog (LOG_ERROR, "rsGenQuery");
    }
  else
    {
      if (status != CAT_NO_ROWS_FOUND)
	{
	  for (i = 0; i < (genQueryOut->rowCnt - 0); i++)
	    {

	      /* appendStrToBBuf(mybuf, strlen(fullName)+1, fullName);
		 gjk1 printf("GJK 0001300.0.1. intFindChkSumDateAvuMetadataVol7, fullName=(%s), i=%d, j=%d, genQueryOut->rowCnt=%d, genQueryOut->attriCnt=%d, iCountUserDefinedMetadata=%d\n", fullName, i, j, genQueryOut->rowCnt, genQueryOut->attriCnt, *iCountUserDefinedMetadata);
		 return (0);
	      */
	      for (j = 0; j < (genQueryOut->attriCnt - 0); j++)
		{
		  char *tResult;

		  /* gjk1 printf ("GJK 0001300.0.2. intFindChkSumDateAvuMetadataVol7, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n", fullName, i, j, genQueryOut->attriCnt);
		     return (0);
		  */
		  tResult = genQueryOut->sqlResult[j].value;
		  /* gjk1 printf ("GJK 0001300.0.3. intFindChkSumDateAvuMetadataVol7, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n", fullName, i, j, genQueryOut->attriCnt);
		  return (0);
		  */
		  tResult += i * genQueryOut->sqlResult[j].len;
		  /* gjk1 printf ("GJK 0001300.0.4. intFindChkSumDateAvuMetadataVol7, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n", fullName, i, j, genQueryOut->attriCnt);
		  return (0);
		  */

		  /* skip final | if no units were defined */
		  if (j < 2 || strlen (tResult))
		    {
		      size = genQueryOut->sqlResult[j].len + 2;
		      /* appendStrToBBuf(mybuf, size, "%s",tResult);
		      gjk1 printf ("GJK 0001300.1.2. intFindChkSumDateAvuMetadataVol7, tResult=(%s), i=%d, j=%d\n", tResult, i, j);
		      */

		    }
		  
		  printf ("GJK 0001300.0.5. intFindChkSumDateAvuMetadataVol7, fullName=(%s), i=%d, j=%d, iCountUserDefinedMetadata=%d\n", fullName, i, j, *iCountUserDefinedMetadata);
		  
		  /*
		    return 0;
		  */

		  switch (j)
		    {
		    case 0:
		      strncpy (UAVArray[*iCountUserDefinedMetadata].attribute,
			       tResult, 256);
		      break;
		    case 1:
		      strncpy (UAVArray[*iCountUserDefinedMetadata].value,
			       tResult, 256);
		      break;
		    case 2:
		      strncpy (UAVArray[*iCountUserDefinedMetadata].units,
			       tResult, 256);
		      break;
		    default:
		      break;
		    }

		  /*             printf ("GJK 0001300.0.6. intFindChkSumDateAvuMetadataVol7, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d, UAVArray[%d].value=(%s)\n", fullName, i, j, genQueryOut->attriCnt, i, UAVArray[i].value);
		  return 0;
		  */
		}		/*  j=0 */
	      /* appendStrToBBuf(mybuf, 2, "\n"); */
	      *iCountUserDefinedMetadata = *iCountUserDefinedMetadata + 1;
	    }			/* i=0 */
	}
    }
  iResult = *iCountUserDefinedMetadata;
  printf
    ("GJK 0001333.3.3. intFindChkSumDateAvuMetadataVol7, i=%d, j=%d, iCountUserDefinedMetadata=%d, iResult=%d\n",
     i, j, *iCountUserDefinedMetadata, iResult);
  return (iResult);
}

/* *********************************************************************************************************** */
int
msiAddDataObjChksumsTimeStampsToAVUVol7 (msParam_t * inpParam1,
				     msParam_t * outParam1,
				     ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  time_t t1;
  collInp_t collInpCache, *ptrInpColl;
  int iErr = 0, i = 0, iStatus;
  int iCountUserDefinedMetadata = 0;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN];

  /* For testing mode when used with irule --test */
  RE_TEST_MACRO
    ("RE_TEST_MACRO, begin of msiAddDataObjChksumsTimeStampsToAVUVol7");

  printf
    ("GJK-P P.0001.0.0. in msiAddDataObjChksumsTimeStampsToAVUVol7(), GJK msiAddDataObjChksumsTimeStampsToAVUVol7: GJK Calling msiGetDataObjChksumsTimeStampsFromAVUVol7\n");

  rsComm = rei->rsComm;

  /*  (void) intChkRechkRecompChkSum4DatObjVol7 (rsComm, "/tempZone/home/rods/loopTest/submit.pl", (time_t) i, rei);         test blbost sobota */

  printf
    ("GJK-P P.991.0.0. in msiAddDataObjChksumsTimeStampsToAVUVol7(), GJK msiAddDataObjChksumsTimeStampsToAVUVol7: GJK Calling msiGetDataObjChksumsTimeStampsFromAVUVol7\n");

  /* parse inpParam11 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiGetDataObjChksumsTimeStampsFromAVUVol7(),  input inpParam1 error. status = %d",
	       rei->status);
      return (rei->status);
    }

  (void) time (&t1);
  iErr =
    intAddChkSumDateAvuMetadataVol7 (rei->rsComm, ptrInpColl->collName, t1,
				 &iStatus);
  (void) snprintf (strOut, 255,
		   "|MD5checkSumDataStamp|%d|UnixTimeInSeconds|\n", (int) t1);
  i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */

  printf
    ("GJK-P P.111.0.7. in msiGetDataObjChksumsTimeStampsFromAVUVol7(), GJK msiGetDataObjChksumsTimeStampsFromAVUVol7: GJK Calling msiGetDataObjChksumsTimeStampsFromAVUVol7, iErr=%d, iCountUserDefinedMetadata=%d\n",
     iErr, iCountUserDefinedMetadata);

  return (iErr);
}

#define no77a
#ifdef no77a
/* *********************************************************************************************************** */
/*
 * Get all Dates of Performed Checksum Operations from metadata AVUs for a given iRods data object.
 * 
 */

int
msiGetDataObjChksumsTimeStampsFromAVUVol7 (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * outParam1, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInpCache, *ptrInpColl;
  int iErr = 0, iI = 0, i = 0;
  UserDefinedMetadata_t aAVUarray[1024];
  int iCountUserDefinedMetadata = 0;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strAtributeName;
  int iEqual = 0, iLess = 0, iMore = 0;    

  /* For testing mode when used with irule --test */
  RE_TEST_MACRO
    ("RE_TEST_MACRO, begin of msiGetDataObjChksumsTimeStampsFromAVUVol7");

  printf
    ("GJK-P P.0001.0.0a. in msiGetDataObjChksumsTimeStampsFromAVUVol7(), GJK msiGetDataObjChksumsTimeStampsFromAVUVol7: GJK Calling msiGetDataObjChksumsTimeStampsFromAVUVol7\n");

  rsComm = rei->rsComm;

  /* parse inpParam11 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiGetDataObjChksumsTimeStampsFromAVUVol7(),  input inpParam1 error. status = %d",
	       rei->status);
      return (rei->status);
    }

  /* parse inpParam2 */
  strAtributeName = parseMspForStr (inpParam2);
  if (strAtributeName != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "ERROR:  msiChkRechkRecompChkSum4DatObjVol7(), input inpParam2 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkRechkRecompChkSum4DatObjVol7(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
      return (-1);
    }
  
  iErr =
    intGetDataObjAVUsVol7 (ptrInpColl, aAVUarray,
					   &iCountUserDefinedMetadata, strOut,
					   rei);

  printf
    ("GJK-P P.0000111.0.7. in msiGetDataObjChksumsTimeStampsFromAVUVol7(), GJK msiGetDataObjChksumsTimeStampsFromAVUVol7: GJK Calling msiGetDataObjChksumsTimeStampsFromAVUVol7, strAtributeName=(%s), iErr=%d, iCountUserDefinedMetadata=%d\n",
     strAtributeName, iErr, iCountUserDefinedMetadata);

  for (iI = 0; iI < iCountUserDefinedMetadata; iI++)
    { int iTmp = 0;
    /*
      snprintf (strTmp, MAX_NAME_LEN, "|%s|%s|%s|\n",
      aAVUarray[iI].attribute, aAVUarray[iI].value,
      aAVUarray[iI].units);
      strncat (strOut, strTmp, MAX_NAME_LEN);
    */

  printf
    ("GJK-P P.0007002.0.0. in msiGetDataObjChksumsTimeStampsFromAVUVol7(), strAtributeName=(%s), iI=%d, iCountUserDefinedMetadata=%d, iLess=(%d), iEqual=(%d), iMore=(%d), aAVUarray[iI].attribute=(%s)\n",
     strAtributeName, iI, iCountUserDefinedMetadata, iEqual, iLess, iMore, aAVUarray[iI].attribute);

  iTmp = strncmp(strAtributeName, aAVUarray[iI].attribute, MAX_NAME_LEN);
  if (iTmp > 0) {
    iMore++;
  }
  else {
    if (iTmp < 0)
      iLess++;
    else
      iEqual++;
  }
  printf
    ("GJK-P P.0007002.0.1. in msiGetDataObjChksumsTimeStampsFromAVUVol7(), aAVUarray[iI].attribute=(%s), strAtributeName=(%s), iErr=%d, iCountUserDefinedMetadata=%d, iLess=(%d), iEqual=(%d), iMore=(%d), iTmp=(%d)\n",
     aAVUarray[iI].attribute, strAtributeName, iErr, iCountUserDefinedMetadata, iLess, iEqual, iMore, iTmp);

    }


  /*   sprintf(strOut, "#1\n#2\n\n#3 lines gjk\n"); */
  i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
  /* fillBuffInParam */


  printf
    ("GJK-P P.111.0.9. in msiGetDataObjChksumsTimeStampsFromAVUVol7(), GJK msiGetDataObjChksumsTimeStampsFromAVUVol7: GJK Calling msiGetDataObjChksumsTimeStampsFromAVUVol7, iCountUserDefinedMetadata=%d\n",
     iCountUserDefinedMetadata);
  //vytiskni array
  return (iErr);
}
#endif

#define gjk001
#ifdef gjk001
/* ****************************************************************************************** */
int
intGetDataObjAVUsVol7 (collInp_t * ptrInpColl,
				       UserDefinedMetadata_t * aAVUarray,
				       int *iTotalAVUs, char *strOut,
				       ruleExecInfo_t * rei)
{
  char *chrPtr1 =
    NULL, strAbsPath[MAX_NAME_LEN], v1[1024], v2[1024], v3[1024],
    strDirName[MAX_NAME_LEN], strFileName[MAX_NAME_LEN], *condVal[10],
    attrName[256] = "MD5checkSumDataStamp";;
  genQueryInp_t genQueryInp;
  int i1a[10], i1b[10], i2a[10], iI = 0, iErr = 0, printCount = 0;
  genQueryOut_t *genQueryOut;
  int iEqual = 0, iLess = 0, iMore = 0;    

  chrPtr1 = strrchr (ptrInpColl->collName, '/');

  printf
    ("GJK-P P.21.0.1. in intGetDataObjAVUsVol7(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n",
     chrPtr1, ptrInpColl->collName);

  if (chrPtr1 != NULL && *chrPtr1 == '/'
      && (ptrInpColl->collName[strlen (ptrInpColl->collName) - 1] == '/'))
    *chrPtr1 = 0;		/* replace '/' in /myzone/foo/' */
/*
  else printf
  ("GJK-P P.21.1.1. in intGetDataObjAVUsVol7(), chrPtr1=(%s), ptrInpColl->collName=(%s), Pmath=%d, strlen=%d\n",
  chrPtr1, ptrInpColl->collName, (int)(chrPtr1 - ptrInpColl->collName), (strlen (ptrInpColl->collName) - 1));
  */

  /* spatne !!!if (chrPtr1 != NULL && *chrPtr1 == '/' &&)    *chrPtr1 = 0;               replace '/' in /myzone/foo/' */
  printf
    ("GJK-P P.21.0.2. in intGetDataObjAVUsVol7(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n",
     chrPtr1, ptrInpColl->collName);

  if ((iI = isData (rei->rsComm, ptrInpColl->collName, NULL)) >= 0)
    {
      rodsLog (LOG_NOTICE,
	       "GJK intGetDataObjAVUsVol7: input (%s) is an iRodsdata.",
	       ptrInpColl->collName);
    }
  else
    {
      printf
	("GJK-P P.00021.0.3a. in intGetDataObjAVUsVol7(), chrPtr1=(%s), ptrInpColl->collName=(%s), iI=%d\n",
	 chrPtr1, ptrInpColl->collName, iI);
      if ((iI = isColl (rei->rsComm, ptrInpColl->collName, NULL)) < 0)
	{
	  rodsLog (LOG_ERROR,
		   "iGetDataObjChksumsTimeStampsFromAVU: input object=(%s) is not an iRods data or collection. Exiting!",
		   ptrInpColl->collName);
	  return (rei->status);
	}
      else
	{
	  printf
	    ("GJK-P P.00021.0.3c. in intGetDataObjAVUsVol7(), chrPtr1=(%s), ptrInpColl->collName=(%s), iI=%d\n",
	     chrPtr1, ptrInpColl->collName, iI);
	  rodsLog (LOG_ERROR,
		   "GJK intGetDataObjAVUsVol7: input (%s) is an iRods collection.",
		   ptrInpColl->collName);
	  return (rei->status);
	}
    }

  printf ("GJK-P P.21.0.4. intGetDataObjAVUsVol7 : input (%s)", ptrInpColl->collName);

  if (rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR, "GJKgetDataObjPSmeta: input rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

  memset (&genQueryInp, 0, sizeof (genQueryInp_t));

  i1a[0] = COL_META_DATA_ATTR_NAME;
  i1b[0] = 0;			/* currently unused */
  i1a[1] = COL_META_DATA_ATTR_VALUE;
  i1b[1] = 0;			/* currently unused */
  i1a[2] = COL_META_DATA_ATTR_UNITS;
  i1b[2] = 0;			/* currently unused */
  genQueryInp.selectInp.inx = i1a;
  genQueryInp.selectInp.value = i1b;
  genQueryInp.selectInp.len = 3;

  strncpy (strAbsPath, ptrInpColl->collName, MAX_NAME_LEN);
  printf
    ("GJK-P P.14.0.11. in intGetDataObjAVUsVol7(), strAbsPath=(%s), ptrInpColl->collName=(%s)\n",
     strAbsPath, ptrInpColl->collName);

  iErr = splitPathByKey (strAbsPath, strDirName, strFileName, '/');

  printf
    ("GJK-P P.14.0.12. in intGetDataObjAVUsVol7(), strAbsPath=(%s), ptrInpColl->collName=(%s), strDirName=(%s), strFileName=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, strDirName, strFileName, iErr);

  i2a[0] = COL_COLL_NAME;
  sprintf (v1, "='%s'", strDirName);
  condVal[0] = v1;

  i2a[1] = COL_DATA_NAME;
  sprintf (v2, "='%s'", strFileName);
  condVal[1] = v2;

  genQueryInp.sqlCondInp.inx = i2a;
  genQueryInp.sqlCondInp.value = condVal;
  genQueryInp.sqlCondInp.len = 2;

  if (attrName != NULL && *attrName != '\0')
    {
      i2a[2] = COL_META_DATA_ATTR_NAME;
      sprintf (v3, "= '%s'", attrName);
      condVal[2] = v3;
      genQueryInp.sqlCondInp.len++;
    }

  genQueryInp.maxRows = 100;
  genQueryInp.continueInx = 0;
  genQueryInp.condInput.len = 0;

  printf
    ("GJK-P P.14.0.13. in intGetDataObjAVUsVol7(), strAbsPath=(%s), ptrInpColl->collName=(%s), v3=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, v3, iErr);

  /* Actual query happens here */
  iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);

  printf
    ("GJK-P P.14.0.14. in intGetDataObjAVUsVol7(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, iErr);

  if (iErr != 0) {
    printf
      ("GJK-P P.00014.0.14z. in intGetDataObjAVUsVol7(), rsGenQuery() failed, strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d\n",
       strAbsPath, ptrInpColl->collName, iErr);
    rodsLog (LOG_ERROR,
	     "rsGenQuery() in intGetDataObjAVUsVol7() failed for DataObject (%s) iErr = %d",
	     strAbsPath, iErr);
    return(iErr);
  }

  if (1 == 2)
    {
      printf ("GJK fake1000.1a STOP\n");
      return (0);
    }
  else
    printf ("GJK fake1000.1a OK\n");
  
  if (iErr == CAT_NO_ROWS_FOUND)
    {
      if (1 == 1)
	{
	  printf ("GJK fake1000.4a STOP\n");
	  return (0);
	}
      else
	printf ("GJK fake1000.4a OK\n");
      
      i1a[0] = COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);
      if (iErr == 0)
	{
	  printf ("GJK GJKgetDataObjPSmeta(),  iErr=%d, None\n", iErr);
	  return (0);
	}
      if (iErr == CAT_NO_ROWS_FOUND)
	{

	  rodsLog (LOG_NOTICE,
		   "GJKgetDataObjPSmeta: DataObject %s not found. iErr = %d",
		   strAbsPath, iErr);
	  *iTotalAVUs = 0;
	  return (0);
	}
      printCount += intFindChkSumDateAvuMetadataVol7 (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	/* proc?? */
    }
  else
    {
      int iI;

      *iTotalAVUs = genQueryOut->rowCnt;

      if (1 == 2)
	{
	  printf ("GJK fake1000.5a STOP, iTotalAVUs=(%d)\n", *iTotalAVUs);
	  return (0);
	}
      else
	printf ("GJK fake1000.5a OK, iTotalAVUs=(%d)\n", *iTotalAVUs);
      
      /* intGetDataObjAVUsVol7 */
      for (iI = 0; iI < *iTotalAVUs; iI++)
	{ 
	  int iTmp = 0;
	  char strAtributeName[MAX_NAME_LEN]="MD5checkSumDataStamp";
	  
	  /*
	    snprintf (strTmp, MAX_NAME_LEN, "|%s|%s|%s|\n",
	    aAVUarray[iI].attribute, aAVUarray[iI].value,
	    aAVUarray[iI].units);
	    strncat (strOut, strTmp, MAX_NAME_LEN);
	  */

	  if (1 == 2)
	    {
	      printf ("GJK fake1000.7a STOP\n");
	      return (0);
	    }
	  else
	    printf ("GJK fake1000.7a OK\n");
	  
	  if (aAVUarray[iI].attribute == NULL) {
	    strncpy (aAVUarray[iI].attribute, "", MAX_NAME_LEN);
            //aAVUarray[iI].attribute = &chTmp;
	    printf
	      ("GJK-P P.0008002.0.0. in intGetDataObjAVUsVol7(), strAtributeName==NULL, iI=%d, *iTotalAVUs=%d, iLess=(%d), iEqual=(%d), iMore=(%d), aAVUarray[iI].attribute=(%s), strAbsPath=(%s)\n",
	       iI, *iTotalAVUs, iEqual, iLess, iMore, aAVUarray[iI].attribute,strAbsPath);
	  }
	  else
	    printf
	      ("GJK-P P.0008002.0.0. in intGetDataObjAVUsVol7(), strAtributeName=(%s) ?= aAVUarray[%d].attribute=(%s), iI=%d, *iTotalAVUs=%d, iLess=(%d), iEqual=(%d), iMore=(%d), aAVUarray[iI].attribute=(%s), strAbsPath=(%s)\n",
	       strAtributeName, iI, aAVUarray[iI].attribute, iI, *iTotalAVUs, iEqual, iLess, iMore, aAVUarray[iI].attribute, strAbsPath);
	  
	  if (1 == 2)
	    {
	      printf ("GJK fake1000.6a STOP, strAtributeName=(%s), aAVUarray[iI].attribute=(%s)\n", strAtributeName, aAVUarray[iI].attribute);
	      return (0);
	    }
	  else
	    printf ("GJK fake1000.6a OK\n");
	  
	  iTmp = strncmp(strAtributeName, aAVUarray[iI].attribute, MAX_NAME_LEN);
	  if(iTmp == 0){
	    iEqual++;
	  }
	  else {
	    if (iTmp < 0)
	      iLess++;
	    else
	      iMore++;
	  }
	  
	  if (1 == 2)
	    {
	      printf ("GJK fake1001.9a STOP, strAtributeName=(%s)\n", strAtributeName);
	      return (0);
	    }
	  else
	    printf ("GJK fake1001.9a OK\n");

	  printf("GJK-P P.0008002.0.1.Aok in intGetDataObjAVUsVol7(), strAtributeName=(%s) ?= aAVUarray[iI].attribute=(%s), iI=(%d), iErr=%d, *iTotalAVUs=%d, iLess=(%d), iEqual=(%d), iMore=(%d), iI=(%d), iTmp=(%d)\n", 
		 strAtributeName, aAVUarray[iI].attribute, iI, iErr, *iTotalAVUs, iEqual, iLess, iMore, iI, iTmp);
	  
	} /* for (  */
      
      if (1 == 2)
	{
	  printf ("GJK fake1000.9a STOP, iTotalAVUs=(%d)\n", *iTotalAVUs);
	  return (0);
	}
      else
	printf ("GJK fake1000.9a OK, iTotalAVUs=(%d)\n", *iTotalAVUs);
      
	/* printCount += intFindChkSumDateAvuMetadataVol7 (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs); */
    }

  if (1 == 3)
    {
      printf ("GJK fake1000.2a STOP\n");
      return (0);
    }
  else
    printf ("GJK fake1000.12 OK\n");
  
  while (iErr == 0 && genQueryOut->continueInx > 0)
    {
      genQueryInp.continueInx = genQueryOut->continueInx;
      iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);
      printCount += intFindChkSumDateAvuMetadataVol7 (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	/* why ?? */
    }

  printf
    ("GJK-P P.14.1.15. in intGetDataObjAVUsVol7(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d, *iTotalAVUs=%d\n",
     strAbsPath, ptrInpColl->collName, iErr, *iTotalAVUs);

  if (1 == 2)
    {
      printf ("GJK fake1000.3a STOP\n");
      return (0);
    }
  else
    printf ("GJK fake1000.3a OK\n");

  return (iEqual); /* AVU metadata atribute == myCheckAtribute */
}

#else

/* ****************************************************************************************** */
int
intGetDataObjAVUsVol71 (collInp_t * ptrInpColl,
				       UserDefinedMetadata_t * aAVUarray,
				       int *iTotalAVUs, char *strOut,
				       ruleExecInfo_t * rei)
{}
#endif
