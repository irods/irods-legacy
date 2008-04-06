// mozna potrebuju delku souboru?! Musim udelat vstup stejny (mozna vyvolat z verze #0 primo?

#include "objStat.h"

int intChkDataType (rsComm_t * rsComm, char *strFullDataPath, time_t tTime,
		    ruleExecInfo_t * rei);
int intGetDataObjACL (dataObjInp_t * myDataObjInp, bytesBuf_t * mybuf,
		      rsComm_t * rsComm);
int intGetDataObjAVUsVol6 (collInp_t * ptrInpColl,
			   UserDefinedMetadata_t * aAVUarray, int *iTotalAVUs,
			   char *strOut, ruleExecInfo_t * rei);
int
intFindChkSumDateAvuMetadata (int status, genQueryOut_t * genQueryOut,
                            char *fullName, UserDefinedMetadata_t UAVArray[],
                            int *iCountUserDefinedMetadata);
/*
 * \fn msiChkDataAvuAttrOneValueOnly
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

int msiChkDataAvuAttrOneValueOnly (msParam_t *coll, msParam_t * inpParam2, msParam_t * inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
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
 
    RE_TEST_MACRO ("    Calling msiChkDataAvuAttrOneValueOnly")

    if (rei == NULL || rei->rsComm == NULL) {
	    rodsLog (LOG_ERROR,
	    "msiChkDataAvuAttrOneValueOnly: input rei or rsComm is NULL");
	    return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1: coll */
    rei->status = parseMspForCollInp (coll, &collInp, 
      &myCollInp, 0);
    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiChkDataAvuAttrOneValueOnly: input inpParam1 error. status = %d", rei->status);
	sprintf (strOut,
                 "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataAvuAttrOneValueOnly(), input inpParam1 error\n");
        rodsLog (LOG_ERROR,
		 "msiChkDataAvuAttrOneValueOnly(),  input inpParam1 error.");
        i = fillStrInMsParam (outParam, strOut);        
  
        return (rei->status);
    }

  /* parse inpParam2 MinSize*/
    if ((strTimeDiff = parseMspForStr (inpParam2)) != NULL)
      {
	lMin = strtol (strTimeDiff, (char **) NULL, 10);
      }
    else
      {
	sprintf (strOut,
		 "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataAvuAttrOneValueOnly(), input inpParam2 error\n");
	rodsLog (LOG_ERROR,
	       "msiChkDataAvuAttrOneValueOnly(),  input inpParam2 error.");
	i = fillStrInMsParam (outParam, strOut);
	return (-1);
      }
    
  /* parse inpParam3 MaxSize*/
    if ((strTimeDiff = parseMspForStr (inpParam3)) != NULL)
      {
	lMax = strtol (strTimeDiff, (char **) NULL, 10);
      }
    else
      {
	sprintf (strOut,
		 "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataAvuAttrOneValueOnly(), input inpParam3 error\n");
	rodsLog (LOG_ERROR,
	       "msiChkDataAvuAttrOneValueOnly(),  input inpParam3 error.");
	i = fillStrInMsParam (outParam, strOut);
	return (-1);
      }
    

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
			  "msiChkDataAvuAttrOneValueOnly: msiChkDataAvuAttrOneValueOnly error for %s, stat=%d",
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
		   "msiChkDataAvuAttrOneValueOnly: msiChkDataAvuAttrOneValueOnly failed, (%s) is not an iRods data object (dataObj == NULL), istatus=%d, rei->status=%d", myCollInp->collName, status, rei->status);
	  rei->status=UNMATCHED_KEY_OR_INDEX;
	  return (rei->status);
	}
	else {
	  iErr = intChkRechkRecompChkSum4DatObjEvBiWe2Ta2 (rsComm, dataObjInp.objPath, t1, rei);
	}
	rodsLog (LOG_ERROR,
		 "msiChkDataAvuAttrOneValueOnly: msiChkDataAvuAttrOneValueOnly failed, (%s) is not an iRods collection, rei->status=%d", myCollInp->collName, rei->status);
	rei->status=UNMATCHED_KEY_OR_INDEX;  
	return (rei->status);
      }
      /* get data names in the batch */
      if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
	rodsLog (LOG_ERROR, 
		 "msiChkDataAvuAttrOneValueOnly: msiChkDataAvuAttrOneValueOnly for COL_DATA_NAME failed");
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

	printf ("GJK-P P.000300.33.1i genQueryOut->rowCnt=(%d), i=(%d), tmpSubColl=(%s), tmpDataName=(%s)\n", 
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
	    rodsLog (LOG_ERROR, "msiChkDataAvuAttrOneValueOnly(): input rsComm is NULL");
	    return (SYS_INTERNAL_NULL_INPUT_ERR);
	  }
	  
	  memset (&genQueryInp7, 0, sizeof (genQueryInp_t));
	  memset (&myDataObjInp7, 0, sizeof (dataObjInp_t));
	  rstrcpy(myDataObjInp7.objPath, dataObjInp.objPath, MAX_NAME_LEN);  /* rstrcpy(destination, source, max_len) */
	  status7 = rsObjStat(rsComm, &myDataObjInp7, &rodsObjStatOut);
	  if ((long)rodsObjStatOut->objSize <= lMin) iCountMin++;
	  if ((long)rodsObjStatOut->objSize >= lMax) iCountMax++;
	  if ((long)rodsObjStatOut->objSize > lMin && (long)rodsObjStatOut->objSize < lMax) iCountMid++;
	  
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
            "msiChkDataAvuAttrOneValueOnly: rsDataObjRepl failed %s, status = %d",
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
 * \fn msiChkRechkRecompChkSum4DatObjVol2
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
msiChkDataObjAVU6 (msParam_t * coll, msParam_t * inpParam2,
		   msParam_t * inpParam3, msParam_t * outParam,
		   ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp, *myCollInp;
  int iErr = 0, i, continueInx, status;
  transStat_t *transStat = NULL;
  genQueryInp_t genQueryInp;
  genQueryOut_t *genQueryOut = NULL;
  dataObjInp_t dataObjInp;
  time_t t1;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff,
    myGlbPar1[MAX_NAME_LEN] = "", *strDataTypeInput1;
  long lMax = 100;
  int iCountMin = 0, iCountMax = 0, iCountMid = 0;
  bytesBuf_t *mybuf;
  UserDefinedMetadata_t *aAVUarray;
  int iCountUserDefinedMetadata = 0;

  RE_TEST_MACRO ("    Calling msiChkDataObjAVU6")
    if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR, "msiChkDataObjAVU6: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

  rsComm = rei->rsComm;

  /* parse inpParam1: coll */
  rei->status = parseMspForCollInp (coll, &collInp, &myCollInp, 0);
  if (rei->status < 0)
    {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			  "msiChkDataObjAVU6: input inpParam1 error. status = %d",
			  rei->status);
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6(), input inpParam1 error\n");
      rodsLog (LOG_ERROR, "msiChkDataObjAVU6(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */

      return (rei->status);
    }

  /* parse inpParam2 MinSize */
  if ((strDataTypeInput1 = parseMspForStr (inpParam2)) != NULL)
    {
      iCountMin = iCountMin;
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDataObjAVU6(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
      return (-1);
    }

  /* parse inpParam3 MaxSize */
  if ((strTimeDiff = parseMspForStr (inpParam3)) != NULL)
    {
      lMax = strtol (strTimeDiff, (char **) NULL, 10);
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6(), input inpParam3 error\n");
      rodsLog (LOG_ERROR, "msiChkDataObjAVU6(),  input inpParam3 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
      return (-1);
    }

  printf
    ("GJK- begin 00021.1.2 in msiChkDataObjAVU6(), Fri Feb 29 22:31:47 PST 2008, status=(%d), myCollInp->collName=(%s)\n",
     status, myCollInp->collName);
  //return(0); // fake3

  /* (void) intChkDataType (rsComm, "/tempZone/home/rods/CVS/Entries.Log", (time_t) i, rei);        test blbost sobota */

/* iterate through all files */
  memset (&genQueryInp, 0, sizeof (genQueryInp));
  memset (&genQueryOut, 0, sizeof (genQueryOut));

  addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
  addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);

  //addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);

  //addInxIval (&genQueryInp.selectInp, COL_D_DATA_CHECKSUM, 1);
  //addInxIval (&genQueryInp.selectInp, COL_D_MODIFY_TIME, 1);
  //addInxIval (&genQueryInp.selectInp, COL_D_CREATE_TIME, 1);

  //addInxIval (&genQueryInp.selectInp, COL_COLL_TYPE,  1); 
  /* ?? addInxIval (&genQueryInp->selectInp, COL_COLL_TYPE, 1); */

  /* #gjk2 file size in range
     #gjk3 ACL user-uthorization pairs
     #gjk4 ACL contain at least
     #gjk5 does not ACL
     #gjk6 AVU contain exactly
     #gjk7 AVU only one
     #gjk8 AVU does not have duplicates
   */

  printf
    ("GJK- begin 00021.1.3 in msiChkDataObjAVU6(), status=(%d), myCollInp->collName=(%s)\n",
     status, myCollInp->collName);
  //return(0); //fake4

  status =
    rsQueryDataObjInCollReCur (rsComm, myCollInp->collName, &genQueryInp,
			       &genQueryOut, NULL, 1);

  printf
    ("GJK- begin 00021.1.4 in msiChkDataObjAVU6(), status=(%d), myCollInp->collName=(%s)\n",
     status, myCollInp->collName);
  //return(0); //fake5

  /* printf("GJK- begin 0001.1.1 in msiChkDataObjAVU6(), status=(%d), myCollInp->collName=(%s)\n", status, myCollInp->collName); */
  printf ("GJK- begin 00041.0.1 status=(%d), myCollInp->collName=(%s)\n",
	  status, myCollInp->collName);

  printf ("GJKa2a myGlbPar1=(%s), myCollInp->collName=(%s)\n", myGlbPar1,
	  myCollInp->collName);

  rstrcpy (myGlbPar1, myCollInp->collName, MAX_NAME_LEN);
  printf ("GJKa2b myGlbPar1=(%s), myCollInp->collName=(%s)\n", myGlbPar1,
	  myCollInp->collName);

  if (status < 0 && status != CAT_NO_ROWS_FOUND)
    {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			  "msiChkDataObjAVU6: msiChkDataObjAVU6 error for %s, stat=%d",
			  myCollInp->collName, status);
      rei->status = status;
      return (rei->status);
    }
  if (status < 0 && status == CAT_NO_ROWS_FOUND)
    {
      printf
	("GJK- begin 00041.2.3a CAT_NO_ROWS_FOUND==status=(%d), myCollInp->collName=(%s)\n",
	 status, myCollInp->collName);
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			  "msiChkDataObjAVU6: msiChkDataObjAVU6 error for %s, stat=%d",
			  myCollInp->collName, status);
    }

  if (status != 0)
    {
      printf
	("GJK- begin 00041.2.3z CAT_NO_ROWS_FOUND?=status=(%d), myCollInp->collName=(%s)\n",
	 status, myCollInp->collName);
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			  "msiChkDataObjAVU6: msiChkDataObjAVU6 error for %s, stat=%d",
			  myCollInp->collName, status);
      rei->status = status;
      return (rei->status);
    }

  printf ("GJK- begin 00041.2.3b status=(%d), myCollInp->collName=(%s)\n",
	  status, myCollInp->collName);
  //return(0); //fake6

  while (rei->status >= 0)
    {
      sqlResult_t *subColl, *dataObj;

      printf ("GJK- begin 00041.2.5 status=(%d), myCollInp->collName=(%s)\n",
	      status, myCollInp->collName);
      if (1 == 2)
	{
	  printf ("GJK fake7 STOP\n");
	  return (0);
	}
      else
	printf ("GJK fake7 OK\n");

      /* get sub coll paths in the batch */
      subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME);
      dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
      /* COL_DATA_SIZE */
      /* sqlDatSize = getSqlResultByInx (genQueryOut, COL_D_CREATE_TIME); */
      /* sqlDatSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE); */

      if (1 == 2)
	{
	  printf ("GJK fake7b STOP\n");
	  return (0);
	}
      else
	printf ("GJK fake7b OK\n");

#ifdef BAD1
      if (sqlDatSize == NULL)
	{
	  printf
	    ("GJK-P P.003.2.2g ERROR sqlDatSize == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i);
	}
      else
	{
	  printf
	    ("GJK-P P.003.2.2g OK sqlDatSize != NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i);
	}
#endif

      printf ("GJK- begin 00041.2.4 status=(%d), myCollInp->collName=(%s)\n",
	      status, myCollInp->collName);
      if (1 == 2)
	{
	  printf ("GJK fake8 STOP\n");
	  return (0);
	}
      else
	printf ("GJK fake8 OK\n");

      if (subColl == NULL)
	{

	  if (1 == 2)
	    {
	      printf ("GJK fake22 STOP\n");
	      return (0);
	    }
	  else
	    printf ("GJK fake22 OK\n");
	  printf ("GJK-P P.004.2.2 ERROR subColl == NULL\n");

	  /*      if (dataObj == NULL && status < 0) { */
	  if (dataObj == NULL)
	    {

	      if (1 == 2)
		{
		  printf ("GJK fake23 STOP\n");
		  return (0);
		}
	      else
		printf ("GJK fake23 OK\n");
	      printf ("GJK-P P.004.2.3 ERROR dataObj == NULL\n");
	      /* rei->status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut); */

	      rodsLog (LOG_ERROR,
		       "msiChkDataObjAVU6: msiChkDataObjAVU6 failed, (%s) is not an iRods data object (dataObj == NULL), istatus=%d, rei->status=%d",
		       myCollInp->collName, status, rei->status);
	      rei->status = UNMATCHED_KEY_OR_INDEX;
	      if (1 == 2)
		{
		  printf ("GJK fake34 STOP\n");
		  return (0);
		}
	      else
		printf ("GJK fake34 OK\n");
	      return (rei->status);
	    }
	  else
	    {
	      /* delej single object */
	      iErr = intChkDataType (rsComm, dataObjInp.objPath, t1, rei);	/* test blbost sobota */
	      /* printf("GJK-P P.4001.0.2. in msiChkDataObjAVU6(), dataObjInp.objPath=(%s), i=%d\n", dataObjInp.objPath, i); */
	      /* GJK return(0); */
	    }			/* dataObj == NULL) { */

	  rodsLog (LOG_ERROR,
		   "msiChkDataObjAVU6: msiChkDataObjAVU6 failed, (%s) is not an iRods collection, rei->status=%d",
		   myCollInp->collName, rei->status);
	  rei->status = UNMATCHED_KEY_OR_INDEX;
	  return (rei->status);

	}			/* if (subColl == NULL) { */

      if (1 == 2)
	{
	  printf ("GJK fake24 STOP\n");
	  return (0);
	}
      else
	printf ("GJK fake24 OK\n");

      /* get data names in the batch */
      if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME)) == NULL)
	{
	  if (1 == 2)
	    {
	      printf ("GJK fake45 STOP\n");
	      return (0);
	    }
	  else
	    printf ("GJK fake45 OK\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDataObjAVU6: msiChkDataObjAVU6 for COL_DATA_NAME failed");
	  rei->status = UNMATCHED_KEY_OR_INDEX;
	  return (rei->status);
	}

      if (1 == 2)
	{
	  printf ("GJK fake46y STOP, rowCnt=%d\n", genQueryOut->rowCnt);
	  return (0);
	}
      else
	printf ("GJK fake46y OK, rowCnt=%d\n", genQueryOut->rowCnt);
      // OK Fri Feb 29 15:32:24 PST 2008

      for (i = 0; i < genQueryOut->rowCnt; i++)
	{
	  char *tmpSubColl, *tmpDataName, *tmpDataSize;

	  /*      ./lib/api/include/objStat.h:__rsObjStat (rsComm_t *rsComm, dataObjInp_t *dataObjInp, int interFlag, */
	  printf
	    ("GJK-P P.0002000.2.2j dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i);
	  
	  if (1 == 2)
	    {
	      printf ("GJK fake76 STOP\n");
	      return (0);
	    }
	  else
	    printf ("GJK fake76 OK\n");
	  // OK Fri Feb 29 15:33:35 PST 2008

	  // printf ("GJK-P P.002.2.2a  muj hlavni cyklus ! in msiChkDataObjAVU6(), dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n", dataObjInp.objPath, genQueryOut->rowCnt, i);
	  printf
	    ("GJK-P P.002.2.2a dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i);

	  if (1 == 2)
	    {
	      printf ("GJK fake77b STOP, i=(%d)\n", i);
	      return (0);
	    }
	  else
	    printf ("GJK fake77b OK, i=(%d)\n", i);
	  // OK Fri Feb 29 15:35:12 PST 2008

	  tmpSubColl = &subColl->value[subColl->len * i];
	  tmpDataName = &dataObj->value[dataObj->len * i];
	  //tmpDataSize = &sqlDatSize->value[sqlDatSize->len * i];
	  //return(0); // fake9

	  if (1 == 2)
	    {
	      printf ("GJK fake81e STOP\n");
	      return (0);
	    }
	  else
	    printf ("GJK fake81e OK\n");

	  snprintf (dataObjInp.objPath, MAX_NAME_LEN, "%s/%s",
		    tmpSubColl, tmpDataName);

	  if (1 == 2)
	    {
	      printf ("GJK fake82c STOP\n");
	      return (0);
	    }
	  else
	    printf ("GJK fake82c OK\n");

	  {
	    //rodsObjStat_t *rodsObjStatOut;
	    int status7;
	    genQueryInp_t genQueryInp7;
	    dataObjInp_t myDataObjInp7;
	    collInp_t ptrInpColl;

	    dataObjInfo_t *dataObjInfoHead = NULL;
	    char *accessPerm;

	    /* check for valid connection */
	    if (rsComm == NULL)
	      {
		rodsLog (LOG_ERROR,
			 "msiChkDataObjAVU6(): input rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	      }

	    memset (&genQueryInp7, 0, sizeof (genQueryInp_t));
	    memset (&myDataObjInp7, 0, sizeof (dataObjInp_t));
	    rstrcpy (myDataObjInp7.objPath, dataObjInp.objPath, MAX_NAME_LEN);	/* rstrcpy(destination, source, max_len) */
	    /* memset (&rodsObjStatOut, 0, sizeof (rodsObjStat_t));
	       status = rsObjStat(rsComm, myDataObjInp, &rodsObjStatOut);

	       if data_type only

	       status7 = intGetDataObjAVUsVol6 (collInp_t * ptrInpColl,
	       UserDefinedMetadata_t * aAVUarray,
	       int *iTotalAVUs, char *strOut,
	       ruleExecInfo_t * rei);
	     */

	    if (1 == 3)
	      {
		printf ("GJK fake0900a STOP\n");
		return (0);
	      }
	    else
	      printf ("GJK fake0900a OK\n");

	    //strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN);         
	    //status7 = intGetDataObjAVUsVol6 (&ptrInpColl, aAVUarray, &iCountUserDefinedMetadata, strOut,rei
	    strncpy (ptrInpColl.collName, dataObjInp.objPath, MAX_NAME_LEN);
	    status7 =
	      intGetDataObjAVUsVol6 (&ptrInpColl, aAVUarray,
				     &iCountUserDefinedMetadata, strOut, rei);

	    if (1 == 2)
	      {
		printf ("GJK fake0901b STOP\n");
		return (0);
	      }
	    else
	      printf ("GJK fake0901b OK\n");

	    /*     
	       status7 = rsObjStat(rsComm, &myDataObjInp7, &rodsObjStatOut);
	       status7 = rsObjStat(rsComm, &dataObjInp, &rodsObjStatOut);

	       if (status7 == 0  && status7 == 1) {
	       if ((long)rodsObjStatOut->objSize <= lMin) iCountMin++;
	       if ((long)rodsObjStatOut->objSize >= lMax) iCountMax++;
	       if ((long)rodsObjStatOut->objSize > lMin && (long)rodsObjStatOut->objSize < lMax) iCountMid++;

	       }
	       printf ("GJK-P P.007.2.2c dataObjInp.objPath=(%s), size=(%ld), createTime=(%s), status7=(%d)\n", dataObjInp.objPath, (long)rodsObjStatOut->objSize, rodsObjStatOut->createTime, status7);
	     */

	    /* 2008.02.11.
	       typedef struct DataObjInfo {
	       char objPath[MAX_NAME_LEN];
	       char rescName[MAX_NAME_LEN];       / * This could be resource group * /
	       char rescGroupName[MAX_NAME_LEN];       / * This could be resource group * /
	       char dataType[MAX_NAME_LEN];
	       rodsLong_t dataSize;
	       char chksum[MAX_NAME_LEN];
	       char version[MAX_NAME_LEN];
	       char filePath[MAX_NAME_LEN];
	       rescInfo_t *rescInfo;
	       char dataOwnerName[MAX_NAME_LEN];
	       char dataOwnerZone[MAX_NAME_LEN];
	       int  replNum;
	       int  replStatus;     / * isDirty flag * /
	       char statusString[MAX_NAME_LEN];
	       rodsLong_t  dataId;
	       rodsLong_t  collId;
	       int  dataMapId;
	       char dataComments[LONG_MAX_NAME_LEN];
	       char dataExpiry[TIME_LEN];
	       char dataCreate[TIME_LEN];
	       char dataModify[TIME_LEN];
	       char dataAccess[MAX_NAME_LEN];
	       int  dataAccessInx;
	       char destRescName[MAX_NAME_LEN];
	       char backupRescName[MAX_NAME_LEN];
	       char subPath[MAX_NAME_LEN];
	       specColl_t *specColl;
	       struct DataObjInfo *next;
	       } dataObjInfo_t;
	     */

	    if (1 == 1)
	      {
		if (1 == 2)
		  {
		    printf ("GJK fake85j STOP\n");
		    return (0);
		  }
		else
		  printf ("GJK fake85f OK\n");
		status =
		  getDataObjInfo (rsComm, &myDataObjInp7, &dataObjInfoHead,
				  accessPerm, 1);

		if (status < 0)
		  {
		    rodsLog (LOG_ERROR,
			     "in msiChkDataObjAVU6(),getDataObjInfo() for (%s), status=%d",
			     myDataObjInp7.objPath, status);
		    return (status);
		  }
		else
		  {
		    printf
		      ("GJK in msiChkDataObjAVU6(), getDataObjInfo(), OK, for (%s), type=(%s), status=%d\n",
		       myDataObjInp7.objPath, dataObjInfoHead->dataType,
		       status);

		    if (strcmp (strDataTypeInput1, dataObjInfoHead->dataType)
			== 0)
		      {
			iCountMin++;
		      }
		    else
		      {
			iCountMid++;
		      }
		    iCountMax++;

		    freeAllDataObjInfo (dataObjInfoHead);

		  }
	      }

	    if (2 == 3)
	      {
		status = intGetDataObjACL (&myDataObjInp7, mybuf, rsComm);
		printf ("GJK intGetDataObjACL, mybuf=(%s), status=%d\n",
			(char *) mybuf, status);
	      }

	  }

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

	  printf
	    ("GJK-P P.002.2.2b1  muj hlavni cyklus ! in msiChkDataObjAVU6(), dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i);
	  printf
	    ("GJK-P P.002.2.2b2 dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d), tmpDataSize=(%s)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i, tmpDataSize);

	  if (1 != genQueryOut->rowCnt)
	    {
	      printf
		("GJK-P P.000994.4.4. in intChkDataType(), rei->status=(%d), genQueryOut->rowCnt=(%d), (dataObjInp.objPath=(%s)\n",
		 rei->status, genQueryOut->rowCnt, dataObjInp.objPath);
	      return (-1);	/* not enough lines */
	    }

	  if (1 == 2)
	    {
	      printf ("GJK fake86j STOP\n");
	      return (0);
	    }
	  else
	    printf ("GJK fake86f OK\n");

	  /* GJK  rei->status = rsDataObjRepl (rsComm, &dataObjInp, &transStat); */
	  rei->status = 0;	/* GJK */
	  if (rei->status < 0)
	    {
	      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
				  "msiChkDataObjAVU6: rsDataObjRepl failed %s, status = %d",
				  (&dataObjInp)->objPath, rei->status);
	    }
	  else
	    {
	      /* GJK fake 2 iErr = intChkDataType (rsComm, dataObjInp.objPath, t1, rei);        test blbost sobota */
	      /* printf("GJK-P P.004001.0.1. in msiChkDataObjAVU6(), dataObjInp.objPath=(%s), i=%d\n", dataObjInp.objPath, i); */
	    }

	  if (1 == 2)
	    {
	      printf ("GJK fake87j STOP\n");
	      return (0);
	    }
	  else
	    printf ("GJK fake87j OK\n");
	  if (transStat != NULL)
	    {
	      free (transStat);
	    }
	}

      continueInx = genQueryOut->continueInx;
      freeGenQueryOut (&genQueryOut);
      if (continueInx > 0)
	{
	  /* More to come */
	  genQueryInp.continueInx = continueInx;
	  if (1 == 2)
	    {
	      printf ("GJK fake88j STOP\n");
	      return (0);
	    }
	  else
	    printf ("GJK fake88j OK\n");
	  rei->status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
	}
      else
	{
	  break;
	}
    }

  clearKeyVal (&dataObjInp.condInput);

  if (1 == 2)
    {
      printf ("GJK fake89m STOP\n");
      return (0);
    }
  else
    printf ("GJK fake89m OK\n");
  (void) snprintf (strOut, 255,
		   "%d data objects are of data type (%d data objects are not of data type) '%s' from total of %d data objects in the input '%s' iRods collection\n",
		   iCountMin, iCountMax - iCountMin, strDataTypeInput1,
		   iCountMax, myGlbPar1);
  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse  add formated string to bytes WriteBytesBuff printMsParam.c */
  printf ("GJK end strOut=(%s)\n", strOut);

  return (rei->status);
}				/* msiChkDataObjAVU6 */

/*
 
Input : iRods absulute path of an object or collection,
Unix time in seconds for the time , if the file wa not checket after that input time, than check it and recompute the sum
and updata the AVY time stamp
*/

int
msiChkDataObjAVU6222 (msParam_t * inpParam1, msParam_t * inpParam2,
		      msParam_t * outParam1, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInpCache, *ptrInpColl;
  int iErr = 0, i = 0;
  int iCountUserDefinedMetadata = 0;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  long lTmp = 0;
  time_t t1;

  /* For testing mode when used with irule --test */
  RE_TEST_MACRO ("RE_TEST_MACRO, begin of msiChkDataObjAVU6");

  printf ("GJK-P P.2222.0.1. in msiChkDataObjAVU6()\n");

  rsComm = rei->rsComm;

  /* parse inpParam1 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDataObjAVU6(),  input inpParam1 error. status = %d",
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
	       "ERROR:  msiChkDataObjAVU6(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDataObjAVU6(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
      return (-1);
    }

  printf
    ("GJK-P P.2222.0.2. in msiChkDataObjAVU6(), ptrInpColl->collName=(%s), t1=%ld\n",
     ptrInpColl->collName, t1);

  iErr = intChkDataType (rsComm, ptrInpColl->collName, t1, rei);	/* test blbost sobota */
  /*  (void) intChkDataType (rsComm, strFullDataPath, t1, rei); */

  sprintf (strOut,
	   "OK msiChkDataObjAVU6(), iCountUserDefinedMetadata=%d, t1=(%ld), iErr=%d\n",
	   iCountUserDefinedMetadata, t1, iErr);
  i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
  /* fillBuffInParam */

  printf
    ("GJK-P P.2222.0.9. in msiChkDataObjAVU6(), iCountUserDefinedMetadata=%d, iErr=%d\n",
     iCountUserDefinedMetadata, iErr);

  return (iErr);
}


#define OLD1
#ifdef OLD1
/* ****************************************************************************************** */
int
intGetDataObjAVUsVol6
//intGetDataObjChksumsTimeStampsFromAVU 
(collInp_t * ptrInpColl,
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

  chrPtr1 = strrchr (ptrInpColl->collName, '/');
  printf
    ("GJK-P P.21.0.1. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n",
     chrPtr1, ptrInpColl->collName);


            if (1 == 3)
              {
                printf ("GJK fake92b STOP\n");
                return (0);
              }
            else
              printf ("GJK fake92b OK\n");

  if (chrPtr1 != NULL && *chrPtr1 == '/'
      && (ptrInpColl->collName[strlen (ptrInpColl->collName) - 1] == '/'))
    *chrPtr1 = 0;		/* replace '/' in /myzone/foo/' */
/*
  else printf
  ("GJK-P P.21.1.1. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s), Pmath=%d, strlen=%d\n",
  chrPtr1, ptrInpColl->collName, (int)(chrPtr1 - ptrInpColl->collName), (strlen (ptrInpColl->collName) - 1));
  */

  /* spatne !!!if (chrPtr1 != NULL && *chrPtr1 == '/' &&)    *chrPtr1 = 0;               replace '/' in /myzone/foo/' */
  printf
    ("GJK-P P.21.0.2. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n",
     chrPtr1, ptrInpColl->collName);

  if ((iI = isData (rei->rsComm, ptrInpColl->collName, NULL)) >= 0)
    {
      rodsLog (LOG_NOTICE,
	       "GJK intGetDataObjChksumsTimeStampsFromAVU: input (%s) is data.",
	       ptrInpColl->collName);
    }
  else
    {
      printf
	("GJK-P P.21.0.3. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s), iI=%d\n",
	 chrPtr1, ptrInpColl->collName, iI);
      if ((iI = isColl (rei->rsComm, ptrInpColl->collName, NULL)) < 0)
	{
	  rodsLog (LOG_ERROR,
		   "iGetDataObjChksumsTimeStampsFromAVU: input object=(%s) is not data or collection. Exiting!",
		   ptrInpColl->collName);
	  /* return (rei->status); */
	}
      else
	{
	  rodsLog (LOG_ERROR,
		   "GJK intGetDataObjChksumsTimeStampsFromAVU: input (%s) is a collection.",
		   ptrInpColl->collName);
	  /* return (rei->status); */
	}
    }

  printf ("GJK-P P.21.0.4. intGetDataObjChksumsTimeStampsFromAVU : input (%s)\n", ptrInpColl->collName);

  if (rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR, "GJKgetDataObjPSmeta: input rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }


  if (1 == 2)
    {
      printf ("GJK fake91a STOP\n");
                return (0);
    }
  else
    printf ("GJK fake91a OK\n");
  
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
    ("GJK-P P.14.0.11. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s)\n",
     strAbsPath, ptrInpColl->collName);

  iErr = splitPathByKey (strAbsPath, strDirName, strFileName, '/');

  printf
    ("GJK-P P.14.0.12. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), strDirName=(%s), strFileName=(%s), iErr=%d\n",
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
    ("GJK-P P.0914.0.13. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), v3=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, v3, iErr);

  /* Actual query happens here */
  iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);

  printf
    ("GJK-P P.0914.0.14. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, iErr);

  if (iErr == CAT_NO_ROWS_FOUND)
    {
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
      printCount += intFindChkSumDateAvuMetadata (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	/* proc?? */
    }
  else
    {
      printCount += intFindChkSumDateAvuMetadata (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	/* proc?? */
    }

  while (iErr == 0 && genQueryOut->continueInx > 0)
    {
      genQueryInp.continueInx = genQueryOut->continueInx;
      iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);
      printCount += intFindChkSumDateAvuMetadata (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	/* why ?? */
    }

  printf
    ("GJK-P P.14.1.15. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d, *iTotalAVUs=%d\n",
     strAbsPath, ptrInpColl->collName, iErr, *iTotalAVUs);

  return (*iTotalAVUs);
}


#else
/* ****************************************************************************************** */
int
intGetDataObjAVUsVol6 (collInp_t * ptrInpColl,
		       UserDefinedMetadata_t * aAVUarray,
		       int *iTotalAVUs, char *strOut, ruleExecInfo_t * rei)
{
  char *chrPtr1 =
    NULL, strAbsPath[MAX_NAME_LEN], v1[1024], v2[1024], v3[1024],
    strDirName[MAX_NAME_LEN], strFileName[MAX_NAME_LEN], *condVal[10],
    attrName[256] = "MD5checkSumDataStamp";
  genQueryInp_t genQueryInp;
  int i1a[10], i1b[10], i2a[10], iI = 0, iErr = 0, printCount = 0;
  genQueryOut_t *genQueryOut;

  if (1 == 2)
    {
      printf ("GJK fake90a STOP\n");
      return (0);
    }
  else
    printf ("GJK fake90a OK\n");

  chrPtr1 = strrchr (ptrInpColl->collName, '/');
  printf
    ("GJK-P P.000221.0.1. in intGetDataObjAVUsVol6(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n",
     chrPtr1, ptrInpColl->collName);
  if (chrPtr1 != NULL && *chrPtr1 == '/'
      && (ptrInpColl->collName[strlen (ptrInpColl->collName) - 1] == '/'))
    *chrPtr1 = 0;		/* replace '/' in /myzone/foo/' */
  else
    printf
      ("GJK-P P.000221.1.1. in intGetDataObjAVUsVol6(), chrPtr1=(%s), ptrInpColl->collName=(%s), Pmath=%d, strlen=%d\n",
       chrPtr1, ptrInpColl->collName, (int) (chrPtr1 - ptrInpColl->collName),
       (strlen (ptrInpColl->collName) - 1));


  /* spatne !!!if (chrPtr1 != NULL && *chrPtr1 == '/' &&)    *chrPtr1 = 0;           replace '/' in /myzone/foo/' */
  printf
    ("GJK-P P.000221.200.2. in intGetDataObjAVUsVol6(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n",
     chrPtr1, ptrInpColl->collName);

  if ((iI = isData (rei->rsComm, ptrInpColl->collName, NULL)) >= 0)
    {
      printf
	("GJK-P P.000221.202.3. in intGetDataObjAVUsVol6(), chrPtr1=(%s), ptrInpColl->collName=(%s) is data\n",
	 chrPtr1, ptrInpColl->collName);
      rodsLog (LOG_NOTICE, "GJK intGetDataObjAVUsVol6: input (%s) is data.",
	       ptrInpColl->collName);
    }
  else
    {
      printf
	("GJK-P P.000221.0.3. in intGetDataObjAVUsVol6(), chrPtr1=(%s), ptrInpColl->collName=(%s) is NOT data , iI=%d\n",
	 chrPtr1, ptrInpColl->collName, iI);
      if ((iI = isColl (rei->rsComm, ptrInpColl->collName, NULL)) < 0)
	{
	  printf
	    ("GJK-P P.000221.400.2. in intGetDataObjAVUsVol6(), chrPtr1=(%s), ptrInpColl->collName=(%s) is NOT COLLECTION \n",
	     chrPtr1, ptrInpColl->collName);

	  rodsLog (LOG_ERROR,
		   "iGetDataObjChksumsTimeStampsFromAVU: input object=(%s) is not data or collection. Exiting!",
		   ptrInpColl->collName);
	  /* return (rei->status); */
	}
      else
	{
	  printf
	    ("GJK-P P.000221.300.2. in intGetDataObjAVUsVol6(), chrPtr1=(%s), ptrInpColl->collName=(%s) is a collection\n",
	     chrPtr1, ptrInpColl->collName);

	  rodsLog (LOG_ERROR,
		   "GJK intGetDataObjAVUsVol6: input (%s) is a collection.",
		   ptrInpColl->collName);
	  return (rei->status);
	}
    }

  printf
    ("GJK-P P.000221.244.4. in intGetDataObjAVUsVol6(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n",
     chrPtr1, ptrInpColl->collName);

  if (rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR, "GJKgetDataObjPSmeta: input rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

  //strncpy (strAbsPath, ptrInpColl->collName, MAX_NAME_LEN); 
  rstrcpy (strAbsPath, "/tempZone/home/rods/HELP.looptest", MAX_NAME_LEN);	/* rstrcpy(destination, source, max_len) */

  memset (&genQueryInp, 0, sizeof (genQueryInp_t));

  //return (0);			// fake11

  /* what do I want to know 'SELECT' what part of my SQL statement */

  i1a[0] = COL_META_DATA_ATTR_NAME;
  i1b[0] = 0;			/* currently unused */
  i1a[1] = COL_META_DATA_ATTR_VALUE;
  i1b[1] = 0;			/* currently unused */
  i1a[2] = COL_META_DATA_ATTR_UNITS;
  i1b[2] = 0;			/* currently unused */
  genQueryInp.selectInp.inx = i1a;
  genQueryInp.selectInp.value = i1b;
  genQueryInp.selectInp.len = 3;

  printf
    ("GJK-P P.14.0.11. in intGetDataObjAVUsVol6(), strAbsPath=(%s), ptrInpColl->collName=(%s)\n",
     strAbsPath, ptrInpColl->collName);

  iErr = splitPathByKey (strAbsPath, strDirName, strFileName, '/');

  printf
    ("GJK-P P.000214.0.12. in intGetDataObjAVUsVol6(), strAbsPath=(%s), ptrInpColl->collName=(%s), strDirName=(%s), strFileName=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, strDirName, strFileName, iErr);

#define AVU2
#ifdef AVU2
  /* 'WHERE' part of my SQL SELECT statement */ 

  i2a[0] = COL_COLL_NAME;
  sprintf (v1, "='%s'", strDirName);
  condVal[0] = v1;

  i2a[1] = COL_DATA_NAME;
  sprintf (v2, "='%s'", strFileName);
  condVal[1] = v2;

  //i2a[2] = COL_DATA_SIZE;
  //* sprintf (v4, "='%s'", strFileSize); */
  //condVal[3] = v3;

  genQueryInp.sqlCondInp.inx = i2a;
  genQueryInp.sqlCondInp.value = condVal;
  //genQueryInp.sqlCondInp.len = 3;
  genQueryInp.sqlCondInp.len = 2;
#endif

#ifdef ATTR1
  if (attrName != NULL && *attrName != '\0')
    {
      i2a[2] = COL_META_DATA_ATTR_NAME;
      sprintf (v3, "= '%s'", attrName);
      condVal[2] = v3;
      genQueryInp.sqlCondInp.len++;
    }
#endif

  genQueryInp.maxRows = 100;
  genQueryInp.continueInx = 0;
  genQueryInp.condInput.len = 0;

  printf
    ("GJK-P P.000214.0.13. in intGetDataObjAVUsVol6(), strAbsPath=(%s), ptrInpColl->collName=(%s), v3=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, v3, iErr);

  /* Actual query happens here */
  memset (&genQueryOut, 0, sizeof (genQueryOut_t));
  iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);

  printf
    ("GJK-P P.000214.0.14. in intGetDataObjAVUsVol6(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, iErr);

  if (iErr == CAT_NO_ROWS_FOUND)
    {
      i1a[0] = COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);
      if (iErr == 0)
	{
	  printf ("GJK-P P.414.0.15. GJKgetDataObjPSmeta(),  iErr=%d, None\n",
		  iErr);
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
      printCount += intFindChkSumDateAvuMetadataVol2 (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	/* why?? */
    }
  else
    {
      printCount += intFindChkSumDateAvuMetadataVol2 (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	/* why?? */
    }

  while (iErr == 0 && genQueryOut->continueInx > 0)
    {
      genQueryInp.continueInx = genQueryOut->continueInx;
      iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);
      printCount += intFindChkSumDateAvuMetadataVol2 (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	/* why ?? */
    }

  printf
    ("GJK-P P.00214.1.15. in intGetDataObjAVUsVol6(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d, *iTotalAVUs=%d\n",
     strAbsPath, ptrInpColl->collName, iErr, *iTotalAVUs);

  return (*iTotalAVUs);
}

#endif
/* #ifdef OLD1 */
