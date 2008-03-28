/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* in modules/persistentArchives/microservices/src/reDataRel.c, working, DRAFT, versio, It WILL be cleaned! */

#include <stdarg.h>

#if !defined(osx_platform)
#include <values.h>
#endif

#include "apiHeaderAll.h"
#include "objStat.h"
#include "miscUtil.h"
#include "reDataObjOpr.h"

#include "genQuery.h"
#include "dataObjChksum.h"

#include "reGlobalsExtern.h"
#include "reDataRel.h"

/* #include "ChkDataObjAttr2.c" */
#include "srcEvBiWe2Ta1.c"
#include "srcEvBiWe2Ta2.c"
#include "srcEvBiWe2Ta3.c"

/*
 * \fn msiRecurzzzColl
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

/*
  msiRecurzzzColl (msParam_t *coll, msParam_t *destRescName, msParam_t *options,  msParam_t *outParam, ruleExecInfo_t *rei)
 msiChkRechkRecompChkSum4DatObj222 (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * outParam1, ruleExecInfo_t * rei)
*/

int
msiChkRechkRecompChkSum4DatObj (msParam_t *coll, msParam_t * inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    collInp_t collInp, *myCollInp;
    int iErr=0, i, continueInx, status;
    transStat_t *transStat = NULL;
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    dataObjInp_t dataObjInp; 
    time_t t1;
    char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
    long lTmp = 0;

    RE_TEST_MACRO ("    Calling msiRecurzzzColl")

    if (rei == NULL || rei->rsComm == NULL) {
	    rodsLog (LOG_ERROR,
	    "msiRecurzzzColl: input rei or rsComm is NULL");
	    return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    /* parse inpParam1: coll */
    rei->status = parseMspForCollInp (coll, &collInp, 
      &myCollInp, 0);
    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiRecurzzzColl: input inpParam1 error. status = %d", rei->status);
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
		 "ERROR:  msiChkRechkRecompChkSum4DatObj(), input inpParam2 error\n");
	rodsLog (LOG_ERROR,
	       "msiChkRechkRecompChkSum4DatObj(),  input inpParam2 error.");
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

/* iterate through all files */
    memset (&genQueryInp, 0, sizeof (genQueryInp));
    status = rsQueryDataObjInCollReCur (rsComm, myCollInp->collName, 
      &genQueryInp, &genQueryOut, NULL, 1);
    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
    	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
    	  "msiRecurzzzColl: msiRecurzzzColl error for %s, stat=%d",
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
		   "msiRecurzzzColl: msiRecurzzzColl failed, (%s) is not an iRods data object, istatus=%d, rei->status=%d", myCollInp->collName, status, rei->status);
	  rei->status=UNMATCHED_KEY_OR_INDEX;
	  return (rei->status);
	}
	else {
	  /* delej single object */
	  iErr = intChkRechkRecompChkSum4DatObj (rsComm, dataObjInp.objPath, t1, rei);	/* test blbost sobota */
	  printf("GJK-P P.4001.0.2. in msiRecurzzzColl(), dataObjInp.objPath=(%s), i=%d\n", dataObjInp.objPath, i);
	  /* GJK return(0); */
	}
	rodsLog (LOG_ERROR,
		 "msiRecurzzzColl: msiRecurzzzColl failed, (%s) is not an iRods collection, rei->status=%d", myCollInp->collName, rei->status);
	rei->status=UNMATCHED_KEY_OR_INDEX;  
	return (rei->status);
      }
      /* get data names in the batch */
      if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR, 
              "msiRecurzzzColl: msiRecurzzzColl for COL_DATA_NAME failed");
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
            "msiRecurzzzColl: rsDataObjRepl failed %s, status = %d",
			    (&dataObjInp)->objPath,
          rei->status);
        }
	else {
	  iErr = intChkRechkRecompChkSum4DatObj (rsComm, dataObjInp.objPath, t1, rei);	/*  test blbost sobota */
	  printf("GJK-P P.4001.0.1. in msiRecurzzzColl(), dataObjInp.objPath=(%s), i=%d\n", dataObjInp.objPath, i);
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
          "dataObjInp: msiRecurzzzColl failed (should have catched earlier) %s, status = %d",
			    (&dataObjInp)->objPath,
          rei->status);
    }
    return (rei->status);
} /*  msiRecurzzzColl */

/*
 
Input : iRods absulute path of an object or collection,
Unix time in seconds for the time , if the file wa not checket after that input time, than check it and recompute the sum
and updata the AVY time stamp
*/

int
msiChkRechkRecompChkSum4DatObj222 (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * outParam1, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInpCache, *ptrInpColl;
  int iErr = 0, i = 0;
  int iCountUserDefinedMetadata = 0;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  long lTmp = 0;
  time_t t1;

  /* For testing mode when used with irule --test */
  RE_TEST_MACRO ("RE_TEST_MACRO, begin of msiChkRechkRecompChkSum4DatObj");

  printf ("GJK-P P.2222.0.1. in msiChkRechkRecompChkSum4DatObj()\n");

  rsComm = rei->rsComm;

  /* parse inpParam1 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiChkRechkRecompChkSum4DatObj(),  input inpParam1 error. status = %d",
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
	       "ERROR:  msiChkRechkRecompChkSum4DatObj(), input inpParam2 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkRechkRecompChkSum4DatObj(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
      return (-1);
    }

  printf
    ("GJK-P P.2222.0.2. in msiChkRechkRecompChkSum4DatObj(), ptrInpColl->collName=(%s), t1=%ld\n",
     ptrInpColl->collName, t1);

  iErr = intChkRechkRecompChkSum4DatObj (rsComm, ptrInpColl->collName, t1, rei);	/* test blbost sobota */
  /*  (void) intChkRechkRecompChkSum4DatObj (rsComm, strFullDataPath, t1, rei); */

  sprintf (strOut,
	   "OK msiChkRechkRecompChkSum4DatObj(), iCountUserDefinedMetadata=%d, t1=(%ld), iErr=%d\n",
	   iCountUserDefinedMetadata, t1, iErr);
  i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
  /* fillBuffInParam */

  printf
    ("GJK-P P.2222.0.9. in msiChkRechkRecompChkSum4DatObj(), iCountUserDefinedMetadata=%d, iErr=%d\n",
     iCountUserDefinedMetadata, iErr);

  return (iErr);
}

int
intChkRechkRecompChkSum4DatObj (rsComm_t * rsComm, char *strFullDataPath,
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
    ("GJK-P P.994.12.1. in intChkRechkRecompChkSum4DatObj(), strFullDataPath=(%s)\n",
     strFullDataPath);

  if ((long) tTime < 0)
    {
      rodsLog (LOG_ERROR,
	       "ERROR in intChkRechkRecompChkSum4DatObj, tTime=(%ld) < 0",
	       tTime);
      return (-1);
    }

  if (strFullDataPath == NULL || strlen (strFullDataPath) < 1)
    {
      rodsLog (LOG_ERROR,
	       "ERROR in intChkRechkRecompChkSum4DatObj, strFullDataPath=(%s) is strange ",
	       strFullDataPath);
      return (-2);
    }

  strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN);
  iErr =
    intGetDataObjChksumsTimeStampsFromAVU (&ptrInpColl, aAVUarray,
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
	("GJK-P P.994.7.1. in intChkRechkRecompChkSum4DatObj(), mam uz AVU a je novejsi, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	 iTotalAVUs, lMax, tTime, (tTime - lMax));
      rodsLog (LOG_NOTICE,
	       "GJK-P P.994.7.1. in intChkRechkRecompChkSum4DatObj(), mam uz AVU a je novejsi, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
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
	    ("GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObj(), mam uz AVU a je starsi, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	     iTotalAVUs, lMax, tTime, (tTime - lMax));
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObj(), mam uz AVU a je starsi, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
		   iTotalAVUs, lMax, tTime, (tTime - lMax));

	  (void) time (&t1);
	  if (rei->status != 0)
	    {
	      rodsLog (LOG_ERROR,
		       "GJK-P P.994.27.1b. ERROR in intChkRechkRecompChkSum4DatObj() in _rsDataObjChksum(), iRods object (%s), returned check sum (%s)\n",
		       dataObjInp.objPath, dataObjChksumStr);
	      return (-2);
	    }
	  /* CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME     */
	  iErr =
	    intAddChkSumDateAvuMetadata (rei->rsComm, strFullDataPath, t1,
					 &status);
	  if (iErr != 0)
	    {
	      rodsLog (LOG_ERROR,
		       "GJK-P P.994.27.1c. ERROR in intChkRechkRecompChkSum4DatObj() in intAddChkSumDateAvuMetadata(),  iRods object (%s), returned check status %d\n",
		       strFullDataPath, status);
	      return (-3);
	    }
	  printf
	    ("GJK-P P.994.17.1. in intChkRechkRecompChkSum4DatObj(),mam uz AVU a je starsi, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, tTime=%ld, iErr=%d, t1=%ld, rei->status=%d, dataObjInp.objPath=(%s), *dataObjChksumStr=(%s)\n",
	     iTotalAVUs, lMax, (long) tTime, iErr, (long) t1, rei->status,
	     dataObjInp.objPath, dataObjChksumStr);
	  return (iErr);
	}
      else
	{	/*      if (iTotalAVUs > 0)  , nemam zadne AVUs */
	  printf
	    ("GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObj(), nemam uz AVU, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
	     iTotalAVUs, lMax, tTime, (tTime - lMax));
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.27.1. in intChkRechkRecompChkSum4DatObj(), nemam uz AVU, prepocti chksumu, porovnej a register novy cas, iTotalAVUs=%d, lMax=%ld, Time=%ld, timeDiff=%ld\n",
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
    ("GJK-P P.994.0.2. in intChkRechkRecompChkSum4DatObj(), rei->status=(%d), strFullDataPath=(%s), iCountUserDefinedMetadata=(%d)\n",
     rei->status, strFullDataPath, iCountUserDefinedMetadata);

  if (rei->status != CAT_NO_ROWS_FOUND)
    {
      printf
	("GJK-P P.994.3.3. in intChkRechkRecompChkSum4DatObj(), rei->status=(%d), genQueryOut->rowCnt=(%d), strFullDataPath=(%s)\n",
	 rei->status, genQueryOut->rowCnt, strFullDataPath);

      if (1 != genQueryOut->rowCnt)
	{
	  printf
	    ("GJK-P P.994.4.4. in intChkRechkRecompChkSum4DatObj(), rei->status=(%d), genQueryOut->rowCnt=(%d), strFullDataPath=(%s)\n",
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
	  printf	    ("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObj(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), tmpChksumStr=(%s)\n",	     objPath, tmpChksumStr);
	}

      if ((modTimVal =	   getSqlResultByInx (genQueryOut, COL_D_MODIFY_TIME)) == NULL)
	{
	  rodsLog (LOG_ERROR,		   "printLsLong: getSqlResultByInx for COL_D_MODIFY_TIME failed GJK-(%s)",		   objPath);
	  /* return (UNMATCHED_KEY_OR_INDEX); */
	}
      else
	{
	  strModTime = &modTimVal->value[modTimVal->len * i];
	  printf	    ("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObj(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), tmpChksumStr=(%s)\n",
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
	    ("GJK-P P.994.5.5. in intChkRechkRecompChkSum4DatObj(),  msiGJKExportRecursiveCollMeta(), objPath=(%s), tmpChksumStr=(%s)\n",
	     objPath, tmpChksumStr);
	}

      printf	("GJK-P P.994.6.6. in intChkRechkRecompChkSum4DatObj(), rei->status=(%d), genQueryOut->rowCnt=(%d), strFullDataPath=(%s)\n",
	 rei->status, genQueryOut->rowCnt, strFullDataPath);
      /* tady je moje maso AAA ZZZ */

      if (iCountUserDefinedMetadata > 0)
	{	/* mam check sum cas
		   nedelej nic
		   kdy rozdil casu neni moc velky
		*/
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.0.1. in intChkRechkRecompChkSum4DatObj(), mam check sum cas a tedy nedelej nic, after GJKgetDataObjPSmeta((%s) rsComm\n",
		   objPath);
	  printf
	    ("GJK-P P.994.0.1. in intChkRechkRecompChkSum4DatObj(), mam check sum cas a tedy nedelej nic, after GJKgetDataObjPSmeta((%s) rsComm\n",
	     objPath);
	}
      else
	{			/* nemam check sum cas */
	  rodsLog (LOG_NOTICE,
		   "GJK-P P.994.0.2. in intChkRechkRecompChkSum4DatObj(), nemam check sum cas, after GJKgetDataObjPSmeta(%s), rsComm\n",
		   objPath);
	  printf
	    ("GJK-P P.994.0.2. in intChkRechkRecompChkSum4DatObj(), nemam check sum cas, after GJKgetDataObjPSmeta(%s), rsComm\n",
	     objPath);
	  if (strlen (tmpChksumStr) ==
	      strlen ("6d75827809277a1d50c0ed742764a82c") && 1 == 1)
	    {
	      /* mam check sum hodnotu
		 nemam check sum cas
		 insert check sum cas in Unix number
		 Call the function to insert metadata here. */
	      rodsLog (LOG_NOTICE,
		       "GJK-P P.994.99.2. in intChkRechkRecompChkSum4DatObj(), nemam check sum cas, mam check sum hodnotu, after GJKgetDataObjPSmeta(%s), rsComm\n",
		       objPath);
	    }
	  else
	    {
	      /* nemam check sum hodnotu
		 vypocti check sum hodnotu
		 instert check sum hodnotu do iCat
		 insert check sum cas == ted */
	      
	      rodsLog (LOG_NOTICE,
		       "GJK-P P.994.99.2. in intChkRechkRecompChkSum4DatObj(), nemam check sum cas, mam check sum hodnotu, after GJKgetDataObjPSmeta(%s), rsComm\n",
		       objPath);

	      printf		("GJK-P P.994.0.4. in intChkRechkRecompChkSum4DatObj(), after GJKgetDataObjPSmeta(%s), rsComm\n",
		 objPath);
	    }
	  printf	    ("GJK-P P.994.0.5. in intChkRechkRecompChkSum4DatObj(), after GJKgetDataObjPSmeta((%s), rsComm\n",
	     objPath);
	}

      printf	("GJK-P P.994.0.6. in intChkRechkRecompChkSum4DatObj(), after GJKgetDataObjPSmeta((%s), rsComm\n",
	 objPath);
    }
  printf ("GJK-P P.994.0.8. in intChkRechkRecompChkSum4DatObj()\n");
  return(0);
}

int intAddChkSumDateAvuMetadata (rsComm_t * rsComm, char *objPath, time_t t1,			     int *iStatus)
{
  modAVUMetadataInp_t modAVUMetadataInp;
  char mytime[256], *chrPtr1;

  chrPtr1 = strrchr (objPath, '/');
  printf
    ("GJK-P P.1.0.1. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), objPath=(%s)\n",
     chrPtr1, objPath);
  if (chrPtr1 != NULL && *chrPtr1 == '/'
      && chrPtr1[strlen (chrPtr1) - 1] == '/')
    *chrPtr1 = 0;		/* replace '/' in /myzone/foo/' */
  printf
    ("GJK-P P.1.0.2. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), objPath=(%s)\n",
     chrPtr1, objPath);

  memset (&modAVUMetadataInp, 0, sizeof (modAVUMetadataInp));

  modAVUMetadataInp.arg0 = "add";
  modAVUMetadataInp.arg1 = "-d";	/* data */
  modAVUMetadataInp.arg2 = objPath;
  modAVUMetadataInp.arg3 = "MD5checkSumDataStamp";

  (void)
    printf
    ("GJK-P P.123.0.1. in intAddChkSumDateAvuMetadata(), after rsModAVUMetadata((%s), rsComm\n",
     objPath);
#if defined(osx_platform)
  if ((long) t1 < 190509697L || (long) t1 > LONG_MAX)
#else
  if ((long) t1 < 190509697L || (long) t1 > MAXLONG)
#endif
    {
      (void) rodsLog (LOG_ERROR,
		      "The Unix time (%d) is out of reasonable bounds for intAddChkSumDateAvuMetadata() for iRods data object (%s) ",
		      (int) t1, objPath);
      return (-1);
    }
  (void) snprintf (mytime, 255, "%d", (int) t1);
  modAVUMetadataInp.arg4 = mytime;
  modAVUMetadataInp.arg5 = "UnixTimeInSeconds";
  *iStatus = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
  (void)
    printf
    ("GJK-P P.123.0.2. in intAddChkSumDateAvuMetadata(), after rsModAVUMetadata((%s), rsComm\n",
     objPath);
  if (0 != *iStatus)
    (void) rodsLog (LOG_ERROR,
		    "intAddChkSumDateAvuMetadata() rsModAVUMetadata failed objPath=(%s)",
		    objPath);
  else
    {
      (void)
	printf
	("GJK-P P.123.0.3. in intAddChkSumDateAvuMetadata(), after rsModAVUMetadata((%s), sComm, mytime=%ld, *iStatus=%d\n",
	 objPath, (long)mytime, *iStatus);
    }
  (void)
    printf
    ("GJK-P P.123.0.4. OK in intAddChkSumDateAvuMetadata(), after rsModAVUMetadata((%s), rsComm, *iStatus=%d\n",
     objPath, *iStatus);

  return (*iStatus);
}

int
intFindChkSumDateAvuMetadata (int status, genQueryOut_t * genQueryOut,
			    char *fullName, UserDefinedMetadata_t UAVArray[],
			    int *iCountUserDefinedMetadata)
{
  int i = 0, j = 0, iResult = 0;
  size_t size;

  *iCountUserDefinedMetadata = 0;

  printf
    ("GJK 300.0.0. intFindChkSumDateAvuMetadata, fullName=(%s), i=%d, status=%d\n",
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
		 gjk1 printf("GJK 300.0.1. intFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->rowCnt=%d, genQueryOut->attriCnt=%d, iCountUserDefinedMetadata=%d\n", fullName, i, j, genQueryOut->rowCnt, genQueryOut->attriCnt, *iCountUserDefinedMetadata);
		 return (0);
	      */
	      for (j = 0; j < (genQueryOut->attriCnt - 0); j++)
		{
		  char *tResult;

		  /* gjk1 printf ("GJK 300.0.2. intFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n", fullName, i, j, genQueryOut->attriCnt);
		     return (0);
		  */
		  tResult = genQueryOut->sqlResult[j].value;
		  /* gjk1 printf ("GJK 300.0.3. intFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n", fullName, i, j, genQueryOut->attriCnt);
		  return (0);
		  */
		  tResult += i * genQueryOut->sqlResult[j].len;
		  /* gjk1 printf ("GJK 300.0.4. intFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n", fullName, i, j, genQueryOut->attriCnt);
		  return (0);
		  */

		  /* skip final | if no units were defined */
		  if (j < 2 || strlen (tResult))
		    {
		      size = genQueryOut->sqlResult[j].len + 2;
		      /* appendStrToBBuf(mybuf, size, "%s",tResult);
		      gjk1 printf ("GJK 300.1.2. intFindChkSumDateAvuMetadata, tResult=(%s), i=%d, j=%d\n", tResult, i, j);
		      */

		    }

		  /* gjk1 printf ("GJK 300.0.5. intFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, iCountUserDefinedMetadata=%d\n", fullName, i, j, *iCountUserDefinedMetadata);

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

		  /*             printf ("GJK 300.0.6. intFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d, UAVArray[%d].value=(%s)\n", fullName, i, j, genQueryOut->attriCnt, i, UAVArray[i].value);
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
    ("GJK 333.3.3. intFindChkSumDateAvuMetadata, i=%d, j=%d, iCountUserDefinedMetadata=%d, iResult=%d\n",
     i, j, *iCountUserDefinedMetadata, iResult);
  return (iResult);
}

int
msiAddDataObjChksumsTimeStampsToAVU (msParam_t * inpParam1,
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
    ("RE_TEST_MACRO, begin of msiAddDataObjChksumsTimeStampsToAVU");

  printf
    ("GJK-P P.1.0.0. in msiAddDataObjChksumsTimeStampsToAVU(), GJK msiAddDataObjChksumsTimeStampsToAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU\n");

  rsComm = rei->rsComm;

  /*  (void) intChkRechkRecompChkSum4DatObj (rsComm, "/tempZone/home/rods/loopTest/submit.pl", (time_t) i, rei);         test blbost sobota */

  printf
    ("GJK-P P.991.0.0. in msiAddDataObjChksumsTimeStampsToAVU(), GJK msiAddDataObjChksumsTimeStampsToAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU\n");

  /* parse inpParam11 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiGetDataObjChksumsTimeStampsFromAVU(),  input inpParam1 error. status = %d",
	       rei->status);
      return (rei->status);
    }

  (void) time (&t1);
  iErr =
    intAddChkSumDateAvuMetadata (rei->rsComm, ptrInpColl->collName, t1,
				 &iStatus);
  (void) snprintf (strOut, 255,
		   "|MD5checkSumDataStamp|%d|UnixTimeInSeconds|\n", (int) t1);
  i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */

  printf
    ("GJK-P P.111.0.7. in msiGetDataObjChksumsTimeStampsFromAVU(), GJK msiGetDataObjChksumsTimeStampsFromAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU, iErr=%d, iCountUserDefinedMetadata=%d\n",
     iErr, iCountUserDefinedMetadata);

  return (iErr);
}

/*
 * Get all Dates of Performed Checksum Operations from metadata AVUs for a given iRods data object.
 * 
 */

int
msiGetDataObjChksumsTimeStampsFromAVU (msParam_t * inpParam1,
				       msParam_t * outParam1,
				       ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInpCache, *ptrInpColl;
  int iErr = 0, iI = 0, i = 0;
  UserDefinedMetadata_t aAVUarray[1024];
  int iCountUserDefinedMetadata = 0;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], strTmp[1024];

  /* For testing mode when used with irule --test */
  RE_TEST_MACRO
    ("RE_TEST_MACRO, begin of msiGetDataObjChksumsTimeStampsFromAVU");

  printf
    ("GJK-P P.1.0.0. in msiGetDataObjChksumsTimeStampsFromAVU(), GJK msiGetDataObjChksumsTimeStampsFromAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU\n");

  rsComm = rei->rsComm;

  /* parse inpParam11 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiGetDataObjChksumsTimeStampsFromAVU(),  input inpParam1 error. status = %d",
	       rei->status);
      return (rei->status);
    }

  iErr =
    intGetDataObjChksumsTimeStampsFromAVU (ptrInpColl, aAVUarray,
					   &iCountUserDefinedMetadata, strOut,
					   rei);

  printf
    ("GJK-P P.111.0.7. in msiGetDataObjChksumsTimeStampsFromAVU(), GJK msiGetDataObjChksumsTimeStampsFromAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU, iErr=%d, iCountUserDefinedMetadata=%d\n",
     iErr, iCountUserDefinedMetadata);

  for (iI = 0; iI < iCountUserDefinedMetadata; iI++)
    {

      snprintf (strTmp, MAX_NAME_LEN, "|%s|%s|%s|\n",
		aAVUarray[iI].attribute, aAVUarray[iI].value,
		aAVUarray[iI].units);
      strncat (strOut, strTmp, MAX_NAME_LEN);
    }

  /*   sprintf(strOut, "#1\n#2\n\n#3 lines gjk\n"); */
  i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c */
  /* fillBuffInParam */


  printf
    ("GJK-P P.111.0.9. in msiGetDataObjChksumsTimeStampsFromAVU(), GJK msiGetDataObjChksumsTimeStampsFromAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU, iCountUserDefinedMetadata=%d\n",
     iCountUserDefinedMetadata);

  return (iErr);
}

#define gjk001
#ifdef gjk001
/* ****************************************************************************************** */
int
intGetDataObjChksumsTimeStampsFromAVU (collInp_t * ptrInpColl,
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

  printf ("GJK-P P.21.0.4. intGetDataObjChksumsTimeStampsFromAVU : input (%s)", ptrInpColl->collName);

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
    ("GJK-P P.14.0.13. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), v3=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, v3, iErr);

  /* Actual query happens here */
  iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);

  printf
    ("GJK-P P.14.0.14. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d\n",
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
intGetDataObjChksumsTimeStampsFromAVU1 (collInp_t * ptrInpColl,
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
  if (chrPtr1 != NULL && *chrPtr1 == '/'
      && (ptrInpColl->collName[strlen (ptrInpColl->collName) - 1] == '/'))
    *chrPtr1 = 0;		/* must replace '/' in /myzone/foo/' */
/*
  else printf
  ("GJK-P P.21.1.1. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s), Pmath=%d, strlen=%d\n",
  chrPtr1, ptrInpColl->collName, (int)(chrPtr1 - ptrInpColl->collName), (strlen (ptrInpColl->collName) - 1));
  */

  /* spatne !!!if (chrPtr1 != NULL && *chrPtr1 == '/' &&)    *chrPtr1 = 0;  */            /* must replace '/' in /myzone/foo/' */
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

  printf ("GJK-P P.21.0.4. intGetDataObjChksumsTimeStampsFromAVU : input (%s)", ptrInpColl->collName);

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
    ("GJK-P P.14.0.13. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), v3=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, v3, iErr);

  /* Actual query happens here */
  iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);

  printf
    ("GJK-P P.14.0.14. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d\n",
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
#endif
