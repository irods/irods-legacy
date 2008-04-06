#include "objStat.h"


int intChkRechkRecompChkSum4DatObjEvBiWe2Ta2  (rsComm_t * rsComm, char *strFullDataPath,	time_t tTime, ruleExecInfo_t * rei); 

/*
 * \fn msiChkRechkRecompChkSum4DatObjEvBiWe2
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

int msiChkDataSize (msParam_t *coll, msParam_t * inpParam2, msParam_t * inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    collInp_t collInp, *myCollInp;
    int iErr=0, i, continueInx, status;
    transStat_t *transStat = NULL;
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    dataObjInp_t dataObjInp; 
    time_t t1;
    char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff, myGlbPar1[MAX_NAME_LEN]="";
    long lMin=55, lMax=100;
    int iCountMin = 0, iCountMax = 0, iCountMid = 0;
 
    RE_TEST_MACRO ("    Calling msiChkDataSize")

    if (rei == NULL || rei->rsComm == NULL) {
	    rodsLog (LOG_ERROR,
	    "msiChkDataSize: input rei or rsComm is NULL");
	    return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1: coll */
    rei->status = parseMspForCollInp (coll, &collInp, 
      &myCollInp, 0);
    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiChkDataSize: input inpParam1 error. status = %d", rei->status);
	sprintf (strOut,
                 "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataSize(), input inpParam1 error\n");
        rodsLog (LOG_ERROR,
		 "msiChkDataSize(),  input inpParam1 error.");
        i = fillStrInMsParam (outParam, strOut);        
  
        return (rei->status);
    }

  /* parse inpParam2 MinSize*/
    if ((strTimeDiff = parseMspForStr (inpParam2)) != NULL)
      {
	lMin = strtol (strTimeDiff, (char **) NULL, 10);
        printf("GJK ########################## p2=(%s)\n", strTimeDiff);
      }
    else
      {
	sprintf (strOut,
		 "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataSize(), input inpParam2 error\n");
	rodsLog (LOG_ERROR,
	       "msiChkDataSize(),  input inpParam2 error.");
	i = fillStrInMsParam (outParam, strOut);
	return (-1);
      }
    
  /* parse inpParam3 MaxSize*/
    if ((strTimeDiff = parseMspForStr (inpParam3)) != NULL)
      {
	lMax = strtol (strTimeDiff, (char **) NULL, 10);
        printf("GJK ########################## p3=(%s)\n", strTimeDiff);
      }
    else
      {
	sprintf (strOut,
		 "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataSize(), input inpParam3 error\n");
	rodsLog (LOG_ERROR,
	       "msiChkDataSize(),  input inpParam3 error.");
	i = fillStrInMsParam (outParam, strOut);
	return (-1);
      }
    
    return(0);

/* iterate through all files */
    memset (&genQueryInp, 0, sizeof (genQueryInp));
    memset (&genQueryOut, 0, sizeof (genQueryOut));

    addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);
    
    addInxIval (&genQueryInp.selectInp, COL_D_DATA_CHECKSUM, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_MODIFY_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_CREATE_TIME, 1);
    
    addInxIval (&genQueryInp.selectInp, COL_COLL_TYPE,  1); 
    addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1); 

    status = rsQueryDataObjInCollReCur (rsComm, myCollInp->collName, 
					&genQueryInp, &genQueryOut, NULL, 1);
    
    printf("GJK- begin 0001.0.1 Sat Mar  1 21:16:44 PST 2008 status=(%d), myCollInp->collName=(%s)\n", status, myCollInp->collName);
    
    printf("GJKa2a myGlbPar1=(%s), myCollInp->collName=(%s)\n", myGlbPar1, myCollInp->collName);
    rstrcpy(myGlbPar1, myCollInp->collName, MAX_NAME_LEN);
    printf("GJKa2b myGlbPar1=(%s), myCollInp->collName=(%s)\n", myGlbPar1, myCollInp->collName);
    
    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			  "msiChkDataSize: msiChkDataSize error for %s, stat=%d",
			  myCollInp->collName, status);
      rei->status=status;
      return (rei->status);
    }
    
    while (rei->status >= 0) {
      sqlResult_t *subColl, *dataObj, *sqlDatSize;
      
      /* get sub coll paths in the batch */
      subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME);
      dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
      sqlDatSize = getSqlResultByInx (genQueryOut, COL_COLL_NAME);
      
      if (sqlDatSize == NULL) {
	printf ("GJK-P P.003.2.2 ERROR sqlDatSize == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n", dataObjInp.objPath, genQueryOut->rowCnt, i);
      }
      else {
	printf ("GJK-P P.003.2.2 OK sqlDatSize != NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n", dataObjInp.objPath, genQueryOut->rowCnt, i);
      }
      
      if (subColl == NULL) {
	printf ("GJK-P P.004.2.2 ERROR subColl == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n", dataObjInp.objPath, genQueryOut->rowCnt, i);
        if (dataObj == NULL) {
	  printf ("GJK-P P.004.2.3 ERROR dataObj == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n", dataObjInp.objPath, genQueryOut->rowCnt, i);
	  
	  rodsLog (LOG_ERROR,
		   "msiChkDataSize: msiChkDataSize failed, (%s) is not an iRods data object (dataObj == NULL), istatus=%d, rei->status=%d", myCollInp->collName, status, rei->status);
	  rei->status=UNMATCHED_KEY_OR_INDEX;
	  return (rei->status);
	}
	else {
	  iErr = intChkRechkRecompChkSum4DatObjEvBiWe2Ta2 (rsComm, dataObjInp.objPath, t1, rei);
	}
	rodsLog (LOG_ERROR,
		 "msiChkDataSize: msiChkDataSize failed, (%s) is not an iRods collection, rei->status=%d", myCollInp->collName, rei->status);
	rei->status=UNMATCHED_KEY_OR_INDEX;  
	return (rei->status);
      }
      /* get data names in the batch */
      if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
	rodsLog (LOG_ERROR, 
		 "msiChkDataSize: msiChkDataSize for COL_DATA_NAME failed");
	rei->status=UNMATCHED_KEY_OR_INDEX;   
	return (rei->status);
      }
      
      for (i = 0; i < genQueryOut->rowCnt; i++) {
        char *tmpSubColl, *tmpDataName, *tmpDataSize;
	
        tmpSubColl = &subColl->value[subColl->len * i];
        tmpDataName = &dataObj->value[dataObj->len * i];
        tmpDataSize = &sqlDatSize->value[sqlDatSize->len * i];
	
	snprintf (dataObjInp.objPath, MAX_NAME_LEN, "%s/%s",
		  tmpSubColl, tmpDataName);

	printf ("GJK-P P.000300.33.1o genQueryOut->rowCnt=(%d), i=(%d), tmpSubColl=(%s), tmpDataName=(%s)\n", 
		genQueryOut->rowCnt, i, tmpSubColl, tmpDataName);
	
#define no18
#ifdef no18
	{
	  rodsObjStat_t *rodsObjStatOut;
	  int status7;
	  genQueryInp_t genQueryInp7;    
	  dataObjInp_t myDataObjInp7;
	  
	  /* check for valid connection */
	  if (rsComm == NULL) {
	    rodsLog (LOG_ERROR, "msiChkDataSize(): input rsComm is NULL");
	    return (SYS_INTERNAL_NULL_INPUT_ERR);
	  }
	  
	  memset (&genQueryInp7, 0, sizeof (genQueryInp_t));
	  memset (&myDataObjInp7, 0, sizeof (dataObjInp_t));
	  rstrcpy(myDataObjInp7.objPath, dataObjInp.objPath, MAX_NAME_LEN);  /* rstrcpy(destination, source, max_len) */
	  status7 = rsObjStat(rsComm, &myDataObjInp7, &rodsObjStatOut);
	  if ((long)rodsObjStatOut->objSize <= lMin) iCountMin++;
	  if ((long)rodsObjStatOut->objSize >= lMax) iCountMax++;
	  if ((long)rodsObjStatOut->objSize > lMin && (long)rodsObjStatOut->objSize < lMax) iCountMid++;
	  printf ("GJK-P P.000301.34.2p rodsObjStatOut->objSize=(%ld)\n", (long)rodsObjStatOut->objSize);
	  
	}
#endif
	/*
	  typedef struct DataObjInp {
	  char objPath[MAX_NAME_LEN];
	  int createMode;
	  int openFlags;      / * used for specCollInx in rcQuerySpecColl * /
	  rodsLong_t offset;
	  rodsLong_t dataSize;
	  int numThreads;
	  int oprType;
	  specColl_t *specColl;
	  keyValPair_t condInput;   / * include chksum flag and value * /
	  } dataObjInp_t;
	*/
    /*
    typedef struct rodsObjStat {
      rodsLong_t          objSize;        / * file size * /
      objType_t           objType;        / * DATA_OBJ_T or COLL_OBJ_T * /
      int                 numCopies;
      char                dataId[MAX_NAME_LEN];
      char                chksum[MAX_NAME_LEN];
      char                ownerName[MAX_NAME_LEN];
      char                ownerZone[MAX_NAME_LEN];
      char                createTime[TIME_LEN];
      char                modifyTime[TIME_LEN];
      specColl_t          *specColl;
    } rodsObjStat_t;
*/

     if (1 != genQueryOut->rowCnt)
        {
        }

        rei->status = 0; 
        if (rei->status<0)
        {
          rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
            "msiChkDataSize: rsDataObjRepl failed %s, status = %d",
			    (&dataObjInp)->objPath,
          rei->status);
        }
	else {

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

    (void) snprintf (strOut, 255,
		     "%d data objects are within the size range, %d data objects are smaller than %ld bytes and %d data objects are larger than %ld bytes in the input '%s' collection\n", iCountMid, iCountMin, lMin, iCountMax, lMax, myGlbPar1);
    i = fillStrInMsParam (outParam, strOut);     
    printf("GJK end strOut=(%s)\n", strOut);
    return (rei->status);
} 

/*
 
Input : iRods absulute path of an object or collection,
Unix time in seconds for the time , if the file wa not checket after that input time, than check it and recompute the sum
and updata the AVY time stamp
*/

int
msiChkDataSize222 (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * outParam1, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInpCache, *ptrInpColl;
  int iErr = 0, i = 0;
  int iCountUserDefinedMetadata = 0;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  long lTmp = 0;
  time_t t1;

  /* For testing mode when used with irule --test */
  RE_TEST_MACRO ("RE_TEST_MACRO, begin of msiChkDataSize");

  printf ("GJK-P P.2222.0.1. in msiChkDataSize()\n");

  rsComm = rei->rsComm;

  /* parse inpParam1 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDataSize(),  input inpParam1 error. status = %d",
	       rei->status);
      return (rei->status);
    }

  /* parse inpParam2 */
  if ((strTimeDiff = parseMspForStr (inpParam2)) != NULL)
    {
      lTmp = strtol (strTimeDiff, (char **) NULL, 10);
      t1 = (time_t) lTmp;
    }
  else
    {
      sprintf (strOut,
	       "ERROR:  msiChkDataSize(), input inpParam2 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkDataSize(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam1, strOut);
      return (-1);
    }

  printf
    ("GJK-P P.2222.0.2. in msiChkDataSize(), ptrInpColl->collName=(%s), t1=%ld\n",
     ptrInpColl->collName, t1);

  iErr = intChkRechkRecompChkSum4DatObjEvBiWe2Ta2 (rsComm, ptrInpColl->collName, t1, rei);

  sprintf (strOut,
	   "OK msiChkDataSize(), iCountUserDefinedMetadata=%d, t1=(%ld), iErr=%d\n",
	   iCountUserDefinedMetadata, t1, iErr);
  i = fillStrInMsParam (outParam1, strOut);	

  printf
    ("GJK-P P.2222.0.9. in msiChkDataSize(), iCountUserDefinedMetadata=%d, iErr=%d\n",
     iCountUserDefinedMetadata, iErr);

  return (iErr);
}

int
intChkRechkRecompChkSum4DatObjEvBiWe2Ta2  (rsComm_t * rsComm, char *strFullDataPath,
				time_t tTime, ruleExecInfo_t * rei)
{ 
  int iCountUserDefinedMetadata = 0;
  genQueryInp_t genQueryInp;
  genQueryOut_t *genQueryOut;
  sqlResult_t *chksumStr, *modTimVal, *creaTimVal, *sqltColDataSize;
  char *objPath;
  int iIterSqlQuery = 0;
  char *tmpChksumStr, *strModTime, *strCreaTime;
  char collQCond[MAX_NAME_LEN];

  char *strColDataSize ;

  printf
    ("GJK-P P.994.12.1. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), strFullDataPath=(%s)\n",
     strFullDataPath);

#ifdef gjk004

  if ((long) tTime < 0)
    {
      rodsLog (LOG_ERROR,
	       "ERROR in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2, tTime=(%ld) < 0",
	       tTime);
      return (-1);
    }

  if (strFullDataPath == NULL || strlen (strFullDataPath) < 1)
    {
      rodsLog (LOG_ERROR,
	       "ERROR in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2, strFullDataPath=(%s) is strange ",
	       strFullDataPath);
      return (-2);
    }

  strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN);
  iErr =
    intGetDataObjChksumsTimeStampsFromAVUEvBiWe2 (&ptrInpColl, aAVUarray,
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
    {    
      printf
	("GJK-P P.994.7.1. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), mam uz AVU a je novejsi, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	 iTotalAVUs, lMax, tTime, (tTime - lMax));
      rodsLog (LOG_NOTICE,
	       "GJK-P P.994.7.1. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), mam uz AVU a je novejsi, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	       iTotalAVUs, lMax, tTime, (tTime - lMax));
      return (0);
    }
  else
    {
      if (iTotalAVUs > 0)
	{
	  dataObjInp_t dataObjInp;
	  char *dataObjChksumStr = NULL;
	  dataObjInfo_t *dataObjInfoHead = NULL;

	  /* zero the struct and fill in the iRods object/file name */
	  memset (&dataObjInp, 0, sizeof (dataObjInp));
	  rstrcpy (dataObjInp.objPath, strFullDataPath, MAX_NAME_LEN);

	  /* move the cond */
	  memset (&dataObjInp.condInput, 0, sizeof (keyValPair_t));
	  addKeyVal (&dataObjInp.condInput, VERIFY_CHKSUM_KW, "");

	  rei->status =
	    _rsDataObjChksum (rsComm, &dataObjInp, &dataObjChksumStr,
			      &dataObjInfoHead);
	  printf
	    ("GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), mam uz AVU a je starsi, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	     iTotalAVUs, lMax, tTime, (tTime - lMax));
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), mam uz AVU a je starsi, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
		   iTotalAVUs, lMax, tTime, (tTime - lMax));

	  (void) time (&t1);
	  if (rei->status != 0)
	    {
	      rodsLog (LOG_ERROR,
		       "GJK-P P.994.27.1b. ERROR in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2() in _rsDataObjChksum(), iRods object (%s), returned check sum (%s)\n",
		       dataObjInp.objPath, dataObjChksumStr);
	      return (-2);
	    }
	  iErr =
	    intAddChkSumDateAvuMetadataEvBiWe2 (rei->rsComm, strFullDataPath, t1,
					 &status);
	  if (iErr != 0)
	    {
	      rodsLog (LOG_ERROR,
		       "GJK-P P.994.27.1c. ERROR in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2() in intAddChkSumDateAvuMetadataEvBiWe2(),  iRods object (%s), returned check status %d\n",
		    `   strFullDataPath, status);
	      return (-3);
	    }
	  printf
	    ("GJK-P P.994.17.1. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(),mam uz AVU a je starsi, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, tTime=%ld, iErr=%d, t1=%ld, rei->status=%d, dataObjInp.objPath=(%s), *dataObjChksumStr=(%s)\n",
	     iTotalAVUs, lMax, (long) tTime, iErr, (long) t1, rei->status,
	     dataObjInp.objPath, dataObjChksumStr);
	  return (iErr);
	}
      else
	{
	  printf
	    ("GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), nemam uz AVU, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	     iTotalAVUs, lMax, tTime, (tTime - lMax));
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), nemam uz AVU, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
		   iTotalAVUs, lMax, tTime, (tTime - lMax));
	  iTotalAVUs = iTotalAVUs;
	}
    }

#endif

  /* Get all collections (recursively) under our input collection */
  /* Prepare query */
  memset (&genQueryInp, 0, sizeof (genQueryInp_t));
  memset (&genQueryOut, 0, sizeof (genQueryOut_t));
  genAllInCollQCond (strFullDataPath, collQCond);

  addInxIval (&genQueryInp.selectInp, COL_D_DATA_CHECKSUM, 1);
  addInxIval (&genQueryInp.selectInp, COL_D_MODIFY_TIME, 1);
  addInxIval (&genQueryInp.selectInp, COL_D_CREATE_TIME, 1);

  addInxIval (&genQueryInp.selectInp, COL_COLL_TYPE,  1);
  addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);

  genQueryInp.maxRows = MAX_SQL_ROWS;

  /* ICAT query for subcollections */
  rei->status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

  printf
    ("GJK-P P.994.0.2. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), rei->status=(%d), strFullDataPath=(%s), iCountUserDefinedMetadata=(%d), genQueryOut->rowCnt=(%d)\n",
     rei->status, strFullDataPath, iCountUserDefinedMetadata, genQueryOut->rowCnt);

  /* copy all subcollections */

  if (rei->status != CAT_NO_ROWS_FOUND)
    { int i; 
dataObjInp_t  destDataObjInp;     /* for rsDataObjCopy() */ 

      printf
	("GJK-P P.994.3.3. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), rei->status=(%d), genQueryOut->rowCnt=(%d), strFullDataPath=(%s)\n",
	 rei->status, genQueryOut->rowCnt, strFullDataPath);

        /* copy all data objects */
        for (i=0; i < genQueryOut->rowCnt; i++) {

#ifdef gjkl005
                /* prepare parameters for rsDataObjCopy() */
                memset (&srcDataObjInp, 0, sizeof (dataObjInp_t));
                memset (&destDataObjInp, 0, sizeof (dataObjInp_t));

                /* first attribute here is the parent collection name */
                subCollName = genQueryOut->sqlResult[0].value;
                subCollName += i*genQueryOut->sqlResult[0].len;

                /* second attribute is the filename */
                fileName = genQueryOut->sqlResult[1].value;
                fileName += i*genQueryOut->sqlResult[1].len;

                /* get full path of source file */
                snprintf(srcDataObjInp.objPath, MAX_NAME_LEN, "%s/%s", subCollName, fileName);

                /* get full path of new file to be created */
                snprintf(destDataObjInp.objPath, MAX_NAME_LEN, "%s%s/%s", destColl, subCollName+strlen(srcColl), fileName);
#endif

printf ("GJK-P P.009.1.2. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), i=(%d), destDataObjInp.objPath=(%s)\n", i, destDataObjInp.objPath);

}

      if (1 != genQueryOut->rowCnt)
	{
	  printf
	    ("GJK-P P.994.4.4. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), rei->status=(%d), genQueryOut->rowCnt=(%d), strFullDataPath=(%s)\n",
	     rei->status, genQueryOut->rowCnt, strFullDataPath);
	}

      tmpChksumStr = NULL;

      if ((chksumStr = getSqlResultByInx (genQueryOut, COL_D_DATA_CHECKSUM)) == NULL)
	{
	  rodsLog (LOG_ERROR,
		   "printLsLong: getSqlResultByInx for COL_D_DATA_CHECKSUM failed GJK-(%s)",
		   objPath);
	}
      else
	{
	  tmpChksumStr = &chksumStr->value[chksumStr->len * iIterSqlQuery];
	  printf("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), tmpChksumStr=(%s)\n",	     objPath, tmpChksumStr);
	}

	tmpChksumStr = NULL;
	strModTime = NULL;

      if ((modTimVal = getSqlResultByInx (genQueryOut, COL_D_MODIFY_TIME)) == NULL)
	{
	  rodsLog (LOG_ERROR,		   "printLsLong: getSqlResultByInx for COL_D_MODIFY_TIME failed GJK-(%s)",		   objPath);
	}
      else
	{
	  strModTime = &modTimVal->value[modTimVal->len * iIterSqlQuery];
	  printf	    ("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), strModTime=(%s)\n",
	     objPath, strModTime);
	}

        tmpChksumStr = NULL;
strCreaTime = NULL;

      if ((creaTimVal = getSqlResultByInx (genQueryOut, COL_D_CREATE_TIME)) == NULL)
	{
	  rodsLog (LOG_ERROR,
		   "printLsLong: getSqlResultByInx for COL_D_CREATE_TIME failed GJK-(%s)",
		   objPath);
	}
      else
	{
	  strCreaTime = &creaTimVal->value[creaTimVal->len * iIterSqlQuery];
	  printf
	    ("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), strCreaTime=(%s)\n",
	     objPath, strCreaTime);
	}

      tmpChksumStr = NULL;

      if ((chksumStr =	   getSqlResultByInx (genQueryOut, COL_D_DATA_CHECKSUM)) == NULL)
	{
	  rodsLog (LOG_ERROR,
		   "printLsLong: getSqlResultByInx for COL_D_DATA_CHECKSUM failed GJK-(%s)",
		   objPath);
	}
      else
	{
	  tmpChksumStr = &chksumStr->value[chksumStr->len * iIterSqlQuery];
	  printf	    ("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), COL_D_DATA_CHECKSUM=tmpChksumStr=(%s)\n",	     objPath, tmpChksumStr);
	}

strColDataSize = NULL;
tmpChksumStr = NULL;

      sqltColDataSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE);
      if (sqltColDataSize == NULL )
        {
          rodsLog (LOG_ERROR,
                   "printLsLong: getSqlResultByInx for COL_DATA_SIZE failed GJK-(%s)",
                   objPath);
        }
      else
        {
	  strColDataSize = &sqltColDataSize->value[sqltColDataSize->len * iIterSqlQuery];
          printf("GJK-P P.007.5.5. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(),  getSqlResultByInx for COL_DATA_SIZE OK, msiGJKExportRecursiveCollMeta(), objPath=(%s), strColDataSize=(%s)\n", objPath, strColDataSize);
        }

      printf	("GJK-P P.994.6.6. (tady je moje maso AAA ZZZ) in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), rei->status=(%d), genQueryOut->rowCnt=(%d), strFullDataPath=(%s), strColDataSize=(%s), tmpChksumStr=(%s), iIterSqlQuery=(%d)\n",
		 rei->status, genQueryOut->rowCnt, strFullDataPath, strColDataSize, tmpChksumStr, iIterSqlQuery);

      printf	("GJK-P P.994.0.6. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2(), after GJKgetDataObjPSmeta((%s), rsComm\n",
	 objPath);
    } 
  printf ("GJK-P P.994.0.8. in intChkRechkRecompChkSum4DatObjEvBiWe2Ta2()\n");
  return(0);
}

int intAddChkSumDateAvuMetadataEvBiWe2 (rsComm_t * rsComm, char *objPath, time_t t1,			     int *iStatus)
{
  modAVUMetadataInp_t modAVUMetadataInp;
  char mytime[256], *chrPtr1;

  chrPtr1 = strrchr (objPath, '/');
  printf
    ("GJK-P P.1.0.1. in intGetDataObjChksumsTimeStampsFromAVUEvBiWe2(), chrPtr1=(%s), objPath=(%s)\n",
     chrPtr1, objPath);
  if (chrPtr1 != NULL && *chrPtr1 == '/'
      && chrPtr1[strlen (chrPtr1) - 1] == '/')
    *chrPtr1 = 0;		
  printf
    ("GJK-P P.1.0.2. in intGetDataObjChksumsTimeStampsFromAVUEvBiWe2(), chrPtr1=(%s), objPath=(%s)\n",
     chrPtr1, objPath);

  memset (&modAVUMetadataInp, 0, sizeof (modAVUMetadataInp));

  modAVUMetadataInp.arg0 = "add";
  modAVUMetadataInp.arg1 = "-d";	
  modAVUMetadataInp.arg2 = objPath;
  modAVUMetadataInp.arg3 = "MD5checkSumDataStamp";

  (void)
    printf
    ("GJK-P P.123.0.1. in intAddChkSumDateAvuMetadataEvBiWe2(), after rsModAVUMetadata((%s), rsComm\n",
     objPath);
#if defined(osx_platform)
  if ((long) t1 < 190509697L || (long) t1 > LONG_MAX)
#else
  if ((long) t1 < 190509697L || (long) t1 > MAXLONG)
#endif
    {
      (void) rodsLog (LOG_ERROR,
		      "The Unix time (%d) is out of reasonable bounds for intAddChkSumDateAvuMetadataEvBiWe2() for iRods data object (%s) ",
		      (int) t1, objPath);
      return (-1);
    }
  (void) snprintf (mytime, 255, "%d", (int) t1);
  modAVUMetadataInp.arg4 = mytime;
  modAVUMetadataInp.arg5 = "UnixTimeInSeconds";
  *iStatus = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
  (void)
    printf
    ("GJK-P P.123.0.2. in intAddChkSumDateAvuMetadataEvBiWe2(), after rsModAVUMetadata((%s), rsComm\n",
     objPath);
  if (0 != *iStatus)
    (void) rodsLog (LOG_ERROR,
		    "intAddChkSumDateAvuMetadataEvBiWe2() rsModAVUMetadata failed objPath=(%s)",
		    objPath);
  else
    {
      (void)
	printf
	("GJK-P P.123.0.3. in intAddChkSumDateAvuMetadataEvBiWe2(), after rsModAVUMetadata((%s), sComm, mytime=%ld, *iStatus=%d\n",
	 objPath, (long)mytime, *iStatus);
    }
  (void)
    printf
    ("GJK-P P.123.0.4. OK in intAddChkSumDateAvuMetadataEvBiWe2(), after rsModAVUMetadata((%s), rsComm, *iStatus=%d\n",
     objPath, *iStatus);

  return (*iStatus);
}

int
intFindChkSumDateAvuMetadataEvBiWe2 (int status, genQueryOut_t * genQueryOut,
			    char *fullName, UserDefinedMetadata_t UAVArray[],
			    int *iCountUserDefinedMetadata)
{
  int i = 0, j = 0, iResult = 0;
  size_t size;

  *iCountUserDefinedMetadata = 0;

  printf
    ("GJK 300.0.0. intFindChkSumDateAvuMetadataEvBiWe2, fullName=(%s), i=%d, status=%d\n",
     fullName, i, status);

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
	      for (j = 0; j < (genQueryOut->attriCnt - 0); j++)
		{
		  char *tResult;

		  tResult = genQueryOut->sqlResult[j].value;

		  tResult += i * genQueryOut->sqlResult[j].len;

		  /* skip final | if no units were defined */
		  if (j < 2 || strlen (tResult))
		    {
		      size = genQueryOut->sqlResult[j].len + 2;
		    }

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

		}		
	      *iCountUserDefinedMetadata = *iCountUserDefinedMetadata + 1;
	    }		
	}
    }
  iResult = *iCountUserDefinedMetadata;
  printf
    ("GJK 333.3.3. intFindChkSumDateAvuMetadataEvBiWe2, i=%d, j=%d, iCountUserDefinedMetadata=%d, iResult=%d\n",
     i, j, *iCountUserDefinedMetadata, iResult);
  return (iResult);
}

int
msiAddDataObjChksumsTimeStampsToAVUEvBiWe2 (msParam_t * inpParam1,
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
    ("RE_TEST_MACRO, begin of msiAddDataObjChksumsTimeStampsToAVUEvBiWe2");

  printf
    ("GJK-P P.1.0.0. in msiAddDataObjChksumsTimeStampsToAVUEvBiWe2(), GJK msiAddDataObjChksumsTimeStampsToAVUEvBiWe2: GJK Calling msiGetDataObjChksumsTimeStampsFromAVUEvBiWe2\n");

  rsComm = rei->rsComm;


  printf
    ("GJK-P P.991.0.0. in msiAddDataObjChksumsTimeStampsToAVUEvBiWe2(), GJK msiAddDataObjChksumsTimeStampsToAVUEvBiWe2: GJK Calling msiGetDataObjChksumsTimeStampsFromAVUEvBiWe2\n");

  /* parse inpParam11 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiGetDataObjChksumsTimeStampsFromAVUEvBiWe2(),  input inpParam1 error. status = %d",
	       rei->status);
      return (rei->status);
    }

  (void) time (&t1);
  iErr =
    intAddChkSumDateAvuMetadataEvBiWe2 (rei->rsComm, ptrInpColl->collName, t1,
				 &iStatus);
  (void) snprintf (strOut, 255,
		   "|MD5checkSumDataStamp|%d|UnixTimeInSeconds|\n", (int) t1);
  i = fillStrInMsParam (outParam1, strOut);
  printf
    ("GJK-P P.111.0.7. in msiGetDataObjChksumsTimeStampsFromAVUEvBiWe2(), GJK msiGetDataObjChksumsTimeStampsFromAVUEvBiWe2: GJK Calling msiGetDataObjChksumsTimeStampsFromAVUEvBiWe2, iErr=%d, iCountUserDefinedMetadata=%d\n",
     iErr, iCountUserDefinedMetadata);

  return (iErr);
}
