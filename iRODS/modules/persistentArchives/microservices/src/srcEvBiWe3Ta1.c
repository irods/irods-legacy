/*
 * pwd /scratch/s1/kremenek/STREDA/iRODS    gdb clients/icommands/bin/irule
 * 
 * gdb> run  -F clients/icommands/test/msiChkDatNameOnce11.ir /tempZone/home/rx /tempZone/home/ro Root
 * /tempZone/home/rods MD5checkSumDataStamp foo3
 * 
 * break 192 #break 15
 * 
 * server ps -ef | grep irodsAgent gdb irodsAgent 20930 gdb> cont gdb> quit
 */

/*
 * Sat Mar 1 23: 26:30 PST 2008 do7 ! !! mozna potrebuju delku souboru ?
 * !Musim udelat vstup stejny(mozna vyvolat z verze
 */

#include "objStat.h"

/* ****************************************************************************************** */
int
intChkDataType (rsComm_t * rsComm, char *strFullDataPath, time_t tTime,
		ruleExecInfo_t * rei);
int intGetDataObjACL (dataObjInp_t * myDataObjInp, bytesBuf_t * mybuf,
		      rsComm_t * rsComm);
int intGetDataObjAVUsVol10 (collInp_t * ptrInpColl,
			    UserDefinedMetadata_t * aAVUarray,
			    int *iTotalAVUs, char *strOut,
			    ruleExecInfo_t * rei);
int intGetDataObjAVUsVol10 (collInp_t * ptrInpColl,
			    UserDefinedMetadata_t * aAVUarray,
			    int *iTotalAVUs, char *strOut,
			    ruleExecInfo_t * rei);
int intFindChkSumDateAvuMetadata (int status, genQueryOut_t * genQueryOut,
				  char *fullName,
				  UserDefinedMetadata_t UAVArray[],
				  int *iCountUserDefinedMetadata);

/* ****************************************************************************************** */
/* descent in to the iRods collection tree and do "ls -r"  */
int
intDescColTree (rsComm_t * rsComm, collInp_t * myCollInp,
		ruleExecInfo_t * rei, lsInfo_t lsArray[], int intLsArraySize,
		int iAction)
{
  int iErr = 0, i, continueInx, status;
  transStat_t *transStat = NULL;
  genQueryInp_t genQueryInp;
  genQueryOut_t *genQueryOut = NULL;
  dataObjInp_t dataObjInp;
  time_t t1;
  char myGlbPar1[MAX_NAME_LEN] = "";
  int iTotalObjInSubtree = 0;
  char tmpStr1[MAX_NAME_LEN + 1] = "";
  char strDirName[MAX_NAME_LEN + 1], strFileName[MAX_NAME_LEN + 1];

  if (1 == 2)
    {
      printf
	("GJK fake00012bD STOP, intDescColTree(), myCollInp->collName=(%s)\n",
	 myCollInp->collName);
      return (0);
    }
  else
    printf
      ("GJK fake00012bD OK, intDescColTree(), myCollInp->collName=(%s)\n",
       myCollInp->collName);

  /* in lsUtil.c  getRodsObjType (conn, &rodsPathInp->srcPath[i]); */

  if (iAction == 1)
    {				/* 1 == 'equal' */
      iErr =
	splitPathByKey (myCollInp->collName, strDirName, strFileName, '/');
      if (iErr != 0)
	printf
	  ("GJK in intDescColTree(), 12bf, splitPathByKey((%s), (%s), (%s), '/') ERROR=%d\n",
	   myCollInp->collName, strDirName, strFileName, iErr);
      snprintf (tmpStr1, MAX_NAME_LEN, " = '%s'", strDirName);

      snprintf (tmpStr1, MAX_NAME_LEN, " = '%s'", myCollInp->collName);
      /* snprintf (tmpStr2, MAX_NAME_LEN, " = '%s'", strFileName); */
    }
  else
    {				/* 0 == 'like 'foo%' */
      snprintf (tmpStr1, MAX_NAME_LEN, " LIKE '%s%%'", myCollInp->collName);
    }

/*  #include "junk4.c"
./irodsctl restart
#pwd /scratch/s1/kremenek/STREDA/iRODS
gdb clients/icommands/bin/irule
break 192
run  -F clients/icommands/test/msiChkDatNameOnce11.ir /tempZone/home/rods MD5checkSumDataStamp foo3 foo4

break 192
#break 15

server
ps -ef | grep irodsAgent
gdb irodsAgent 20930
gdb> cont
gdb> quit

clients/icommands/bin/irule -F clients/icommands/test/msiChkDataAvuAttrOneValueOnlyVol11.ir /tempZone/home/rods a b

*/

  {
    if (1 == 2)
      {
	printf ("GJK fake1234junk4.11a STOP\n");
	return (0);
      }
    else
      {
	printf ("GJK fake1234junk4.11a OK, myCollInp->collName=(%s)\n",
		myCollInp->collName);
      }

    /* iterate through all files ./lib/core/include/rodsGenQuery.h */
    memset (&genQueryInp, 0, sizeof (genQueryInp));
    memset (&genQueryOut, 0, sizeof (genQueryOut));

    /* emacs -nw ./lib/core/include/rodsGenQueryNames.h */
    /* Currently necessary since other namespaces exist in the token table */
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, tmpStr1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_MODIFY_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_CREATE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_TYPE, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);
    addInxIval (&genQueryInp.selectInp, COL_R_RESC_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_EXPIRY, 1);
    addInxIval (&genQueryInp.selectInp, COL_USER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_ACCESS_NAME, 1);
    /* 
       addInxIval(&genQueryInp.selectInp, COL_D_DATA_CHECKSUM, 1); 
       addInxIval(&genQueryInp.selectInp, RESC_ID, 1);
     */
    genQueryInp.maxRows = MAX_SQL_ROWS;

    if (1 == 2)
      {
	printf ("GJK fake1234junk4.12bD STOP\n");
	return (0);
      }
    else
      printf
	("GJK fake1234junk4.12bE OK, tmpStr1=(%s), myCollInp->collName=(%s)\n",
	 tmpStr1, myCollInp->collName);

    status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

    if (status != 0)
      {
	printf
	  ("GJK fake1234junk4.113c, BAD, status=(%d) after rsGenQuery()\n",
	   status);
	return (-1);
      }
    else
      printf
	("GJK fake1234junk4.113c OK, genQueryOut->rowCnt=(%d), status=(%d)\n",
	 genQueryOut->rowCnt, status);

    printf
      ("GJK- begin 0001.0.1k1 Sat Mar  1 21:16:44 PST 2008 status=(%d), myCollInp->collName=(%s)\n",
       status, myCollInp->collName);

    printf ("GJKa2a myGlbPar1=(%s), myCollInp->collName=(%s)\n", myGlbPar1,
	    myCollInp->collName);
    rstrcpy (myGlbPar1, myCollInp->collName, MAX_NAME_LEN);
    printf ("GJKa2b myGlbPar1=(%s), myCollInp->collName=(%s)\n", myGlbPar1,
	    myCollInp->collName);

    if (status < 0 && status != CAT_NO_ROWS_FOUND)
      {
	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			    "msiChkDataSize: msiChkDataSize error for %s, stat=%d",
			    myCollInp->collName, status);
	rei->status = status;
	return (rei->status);
      }
    while (rei->status >= 0)
      {
	sqlResult_t *subColl, *dataObj, *sqlDatSize, *sqlRescName,
	  *sqlModifyTime, *sqlDataExpiry, *sqlUserName, *sqlDataAccessName;

	/* get sub coll paths in the batch */
	subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME);
	dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
	sqlDatSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE);
	sqlRescName = getSqlResultByInx (genQueryOut, COL_R_RESC_NAME);
	sqlModifyTime = getSqlResultByInx (genQueryOut, COL_D_MODIFY_TIME);
	sqlDataExpiry = getSqlResultByInx (genQueryOut, COL_D_EXPIRY);
	sqlUserName = getSqlResultByInx (genQueryOut, COL_USER_NAME);
	sqlDataAccessName = getSqlResultByInx (genQueryOut, COL_DATA_ACCESS_NAME);

	if (sqlDataExpiry == NULL)
	  {
	    printf
	      ("GJK-P P.003.2.2 ERROR sqlDataExpiry == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	       dataObjInp.objPath, genQueryOut->rowCnt, i);
	  }
	else
	  {
	    printf
	      ("GJK-P P.003.2.2 OK sqlDataExpiry != NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	       dataObjInp.objPath, genQueryOut->rowCnt, i);
	  }

	if (sqlModifyTime == NULL)
	  {
	    printf
	      ("GJK-P P.003.2.2 ERROR sqlModifyTime == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	       dataObjInp.objPath, genQueryOut->rowCnt, i);
	  }
	else
	  {
	    printf
	      ("GJK-P P.003.2.2 OK sqlModifyTime != NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	       dataObjInp.objPath, genQueryOut->rowCnt, i);
	  }

	if (sqlDatSize == NULL)
	  {
	    printf
	      ("GJK-P P.003.2.2 ERROR sqlDatSize == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	       dataObjInp.objPath, genQueryOut->rowCnt, i);
	  }
	else
	  {
	    printf
	      ("GJK-P P.003.2.2 OK sqlDatSize != NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	       dataObjInp.objPath, genQueryOut->rowCnt, i);
	  }

	if (sqlRescName == NULL)
	  {
	    printf
	      ("GJK-P P.003.2.2 ERROR sqlRescName == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	       dataObjInp.objPath, genQueryOut->rowCnt, i);
	  }
	else
	  {
	    printf
	      ("GJK-P P.003.2.2 OK sqlRescName != NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	       dataObjInp.objPath, genQueryOut->rowCnt, i);
	  }

	if (subColl == NULL)
	  {
	    printf
	      ("GJK-P P.004.2.2 ERROR subColl == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	       dataObjInp.objPath, genQueryOut->rowCnt, i);
	    if (dataObj == NULL)
	      {
		printf
		  ("GJK-P P.004.2.3 ERROR dataObj == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
		   dataObjInp.objPath, genQueryOut->rowCnt, i);

		rodsLog (LOG_ERROR,
			 "msiChkDataSize: msiChkDataSize failed, (%s) is not an iRods data object (dataObj == NULL), istatus=%d, rei->status=%d",
			 myCollInp->collName, status, rei->status);
		rei->status = UNMATCHED_KEY_OR_INDEX;
		return (rei->status);
	      }
	    else
	      {
		iErr =
		  intChkRechkRecompChkSum4DatObjEvBiWe2Ta2 (rsComm,
							    dataObjInp.
							    objPath, t1, rei);
	      }
	    rodsLog (LOG_ERROR,
		     "msiChkDataSize: msiChkDataSize failed, (%s) is not an iRods collection, rei->status=%d",
		     myCollInp->collName, rei->status);
	    rei->status = UNMATCHED_KEY_OR_INDEX;
	    return (rei->status);
	  }
	/* get data names in the batch */
	if ((dataObj =
	     getSqlResultByInx (genQueryOut, COL_DATA_NAME)) == NULL)
	  {
	    rodsLog (LOG_ERROR,
		     "msiChkDataSize: msiChkDataSize for COL_DATA_NAME failed");
	    rei->status = UNMATCHED_KEY_OR_INDEX;
	    return (rei->status);
	  }
	/* emacs -nw ./server/re/include/reDataRel.h */

	for (i = 0; i < genQueryOut->rowCnt; i++)
	  {
	    char *tmpSubColl, *tmpDataName, *tmpDataSize, *tmpRescName,
	      *tmpModifyTime, *tmpDataExpiry, *tmpUserName, *tmpDataAccessName;

	    tmpSubColl = &subColl->value[subColl->len * i];
	    tmpDataName = &dataObj->value[dataObj->len * i];
	    tmpDataSize = &sqlDatSize->value[sqlDatSize->len * i];
	    tmpRescName = &sqlRescName->value[sqlRescName->len * i];
	    tmpModifyTime = &sqlModifyTime->value[sqlModifyTime->len * i];
	    tmpDataExpiry = &sqlDataExpiry->value[sqlDataExpiry->len * i];
	    tmpUserName =  &sqlUserName->value[sqlUserName->len * i];
	    tmpDataAccessName = &sqlDataAccessName->value[sqlDataAccessName->len * i];

	    snprintf (dataObjInp.objPath, MAX_NAME_LEN, "%s/%s",
		      tmpSubColl, tmpDataName);

	    printf
	      ("GJK-P P.000300.33.1p genQueryOut->rowCnt=(%d), i=(%d), tmpSubColl=(%s), tmpDataName=(%s), tmpDataSize=(%s))\n",
	       genQueryOut->rowCnt, i, tmpSubColl, tmpDataName, tmpDataSize);

	    {
	      rodsObjStat_t *rodsObjStatOut;
	      int status7;
	      genQueryInp_t genQueryInp7;
	      dataObjInp_t myDataObjInp7;
	      rodsLong_t rodsLong_tobjSize;

	      if (1 == 2)
		{
		  /* check for valid connection */
		  if (rsComm == NULL)
		    {
		      rodsLog (LOG_ERROR,
			       "msiChkDataSize(): input rsComm is NULL");
		      return (SYS_INTERNAL_NULL_INPUT_ERR);
		    }
		  memset (&genQueryInp7, 0, sizeof (genQueryInp_t));
		  memset (&myDataObjInp7, 0, sizeof (dataObjInp_t));
		  rstrcpy (myDataObjInp7.objPath, dataObjInp.objPath, MAX_NAME_LEN);	/* rstrcpy(destination,
											 * source, max_len) */
		  status7 =
		    rsObjStat (rsComm, &myDataObjInp7, &rodsObjStatOut);

		  /* work around how to get the size */
		  if (iTotalObjInSubtree > (MAX_NAME_LEN - 2))
		    return (-2);	/* not enough array filed available for the
					 * output lsArray */
		}

	      rstrcpy (lsArray[iTotalObjInSubtree].chUserName, tmpUserName, MAX_NAME_LEN);
	      rstrcpy (lsArray[iTotalObjInSubtree].chDataAccessName, tmpDataAccessName, MAX_NAME_LEN); 
	      rstrcpy (lsArray[iTotalObjInSubtree].chCollection, tmpSubColl, MAX_NAME_LEN);	/* rstrcpy(dst,src,len) */
	      rstrcpy (lsArray[iTotalObjInSubtree].chObjName, tmpDataName,
		       MAX_NAME_LEN);
	      rstrcpy (lsArray[iTotalObjInSubtree].chRescName, tmpRescName,
		       MAX_NAME_LEN);
	      rstrcpy (lsArray[iTotalObjInSubtree].chModifyTime,
		       tmpModifyTime, MAX_NAME_LEN);
	      rstrcpy (lsArray[iTotalObjInSubtree].chDataSize, tmpDataSize,
		       MAX_NAME_LEN);
	      rstrcpy (lsArray[iTotalObjInSubtree].chDataExpiry,
		       tmpDataExpiry, MAX_NAME_LEN);
	      *(lsArray[iTotalObjInSubtree].lDataSize) = atol (tmpDataSize);

	      printf
		("GJK-P P.000404.44.1p genQueryOut->rowCnt=(%d), i=(%d), tmpSubColl=(%s), tmpDataName=(%s), lsArray[].chDataSize=(%s), rodsLong_tobjSize=(%ld), lsArray[].chmodifyTime=(%s), lsArray[].chRescName=(%s)\n",
		 genQueryOut->rowCnt, i, tmpSubColl, tmpDataName,
		 lsArray[iTotalObjInSubtree].chDataSize,
		 (long) rodsLong_tobjSize,
		 lsArray[iTotalObjInSubtree].chModifyTime,
		 lsArray[iTotalObjInSubtree].chRescName);

	      iTotalObjInSubtree++;

	    }

	    /*
	     * typedef struct DataObjInp { char objPath[MAX_NAME_LEN]; int
	     * createMode; int openFlags;      / * used for specCollInx in
	     * rcQuerySpecColl * / rodsLong_t offset; rodsLong_t dataSize; int
	     * numThreads; int oprType; specColl_t *specColl; keyValPair_t
	     * condInput;   / * include chksum flag and value * / }
	     * dataObjInp_t;
	     */
	    /*
	     * typedef struct rodsObjStat { rodsLong_t          objSize; / *
	     * file size * / objType_t           objType;        / * DATA_OBJ_T
	     * or COLL_OBJ_T * / int                 numCopies; char
	     * dataId[MAX_NAME_LEN]; char                chksum[MAX_NAME_LEN]; char
	     * ownerName[MAX_NAME_LEN]; char                ownerZone[MAX_NAME_LEN];
	     * char                createTime[TIME_LEN]; char
	     * modifyTime[TIME_LEN]; specColl_t          *specColl; }
	     * rodsObjStat_t;
	     */

	    if (1 != genQueryOut->rowCnt)
	      {
	      }
	    rei->status = 0;
	    if (rei->status < 0)
	      {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
				    "msiChkDataSize: rsDataObjRepl failed %s, status = %d",
				    (&dataObjInp)->objPath, rei->status);
	      }
	    else
	      {

	      }
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
	    rei->status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
	  }
	else
	  {
	    break;
	  }
      }

    clearKeyVal (&dataObjInp.condInput);

    printf
      ("GJK-P P.1234.junk4.000404.44.1R, end of junk4.c,  status=(%d)\n",
       status);


  }

  if (1 == 2)
    {
      printf ("GJK fake2999z STOP, iTotalObjInSubtree=%d\n",
	      iTotalObjInSubtree);
      return (0);
    }
  else
    printf ("GJK fake2999z OK, iTotalObjInSubtree=%d\n", iTotalObjInSubtree);

  return (iTotalObjInSubtree);
}

/* ****************************************************************************************** */
/*
 * \fn msiChkDataAvuAttrOneValueOnlyVol10 \author  Sifang Lu \date 2007-10-01
 * \brief This microservice iterate through collection, and calls
 * rsDataObjRepl to recursively replication a collection as part of a
 * workflow  execution. \note This call should only be used through the
 * rcExecMyRule (irule) call i.e., rule execution initiated by clients and
 * should not be called internally by the server since it interacts with the
 * client through the normal client/server socket connection. \param[in] coll
 * : It can be a collInp_t or a STR_MS_T which would be taken as destination
 * collection path. destResc : STR_MS_T destination resource name options  :
 * STR_MS_T a group of options in a string delimited by '%%'. If the string
 * is empty ("\0") or null ("NULL") it will not be used.  The options can be
 * the following - "all"(ALL_KW) - "irodsAdmin" (IRODS_ADMIN_KW). -
 * "backupMode" if specified, it will try to use 'backup mode' to the
 * destination resource. Means if a good copy already exists in destination
 * resource, it will not throw an error
 * 
 * \param[out] a INT_MS_T containing the status. \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

int
msiChkDataAvuAttrOneValueOnlyVol10 (msParam_t * coll,
				    msParam_t * inpParam2,
				    msParam_t * inpParam3,
				    msParam_t * outParam,
				    ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp, *myCollInp;
  int i, status;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strAtrName;
  long lMin = 55, lMax = 100;
  int iCountMin = 0, iCountMax = 0, iCountMid = 0;
  int iChkFailed1 = 0;
  int intLsArraySize = (MAX_NAME_LEN);
  lsInfo_t lsArray[MAX_NAME_LEN];

  RE_TEST_MACRO ("    Calling msiChkDataAvuAttrOneValueOnlyVol10")
    printf
    ("GJK- begin 0001.0.0-11p, Fri Mar 14 18:29:18 PDT 2008, in msiChkDataAvuAttrOneValueOnlyVol10(), status=(%d), myCollInp->collName=(%s) BBBBBBBBBBBBBBBBBBBBBB\n",
     status, myCollInp->collName);

  if (1 == 2)
    {
      printf ("GJK fake0001b STOP\n");
      return (0);
    }
  else
    printf ("GJK fake0001b OK\n");

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDataAvuAttrOneValueOnlyVol10: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  /* parse inpParam1: coll */
  rei->status = parseMspForCollInp (coll, &collInp, &myCollInp, 0);
  if (rei->status < 0)
    {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			  "msiChkDataAvuAttrOneValueOnlyVol10: input inpParam1 error. status = %d",
			  rei->status);
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataAvuAttrOneValueOnlyVol10(), input inpParam1 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkDataAvuAttrOneValueOnlyVol10(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);

      return (rei->status);
    }
  /* parse inpParam2 MinSize */
  if ((strAtrName = parseMspForStr (inpParam2)) != NULL)
    {
      lMin = 0;
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataAvuAttrOneValueOnlyVol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkDataAvuAttrOneValueOnlyVol10(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam, strOut);
      return (-1);
    }

#ifdef PAR3
  /* parse inpParam3 MaxSize */
  if ((strAtrName = parseMspForStr (inpParam3)) != NULL)
    {
      lMax = strtol (strAtrName, (char **) NULL, 10);
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataAvuAttrOneValueOnlyVol10(), input inpParam3 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkDataAvuAttrOneValueOnlyVol10(),  input inpParam3 error.");
      i = fillStrInMsParam (outParam, strOut);
      return (-1);
    }
#endif

  if (1 == 2)
    {
      printf ("GJK fake11bC STOP\n");
      return (0);
    }
  else
    printf ("GJK fake11bC OK\n");

  iChkFailed1 =
    intDescColTree (rsComm, myCollInp, rei, lsArray, intLsArraySize, 0);
  for (i = 80; i < iChkFailed1; i++)
    {
      printf
	("GJK fake9998d intLsArraySize[%3d].chCollection=(%s), intLsArraySize[%3d].chObjName=(%s), intLsArraySize[%3d].isData=(%d), lsArray[%3d].chDataSize=(%s), lsArray[i].lDataSize=(%ld), i=(%3d)\n",
	 i, lsArray[i].chCollection, i, lsArray[i].chObjName, i,
	 lsArray[i].isData, i, lsArray[i].chDataSize,
	 (long) lsArray[i].lDataSize, i);
    }

  if (iChkFailed1 < 1)
    (void) snprintf (strOut, 255,
		     "Test failed, iChkFailed1=%d total objects found, %d data objects Vol10, %d data objects are smaller than %ld bytes and %d data objects are larger than %ld bytes in the input '%s' collection\n",
		     iChkFailed1, iCountMid, iCountMin, lMin, iCountMax, lMax,
		     myCollInp->collName);
  else
    (void) snprintf (strOut, 255,
		     "Test OK, %d iChkFailed1=%d total objects found, data objects Vol10, %d data objects are smaller than %ld bytes and %d data objects are larger than %ld bytes in the input '%s' collection\n",
		     iChkFailed1, iCountMid, iCountMin, lMin, iCountMax, lMax,
		     myCollInp->collName);

  i = fillStrInMsParam (outParam, strOut);

  if (1 == 2)
    {
      printf ("GJK fake9999t Tue Mar  4 18:47:03 PST 2008 STOP\n");
      return (0);
    }
  else
    printf ("GJK fake9999t Tue Mar  4 18:47:03 PST 2008 OK\n");

  printf ("GJK end strOut=(%s)\n", strOut);
  return (rei->status);
}

/* ****************************************************************************************** */
/*
 * \fn msiChkDatNameOnce11 \author  Sifang Lu \date   2007-10-01 \brief This
 * microservice iterate through collection, and calls rsDataObjRepl to
 * recursively replication a collection as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 * i.e., rule execution initiated by clients and should not be called
 * internally by the server since it interacts with the client through the
 * normal client/server socket connection. \param[in] coll     : It can be a
 * collInp_t or a STR_MS_T which would be taken as destination collection
 * path. destResc : STR_MS_T destination resource name options  : STR_MS_T a
 * group of options in a string delimited by '%%'. If the string is empty
 * ("\0") or null ("NULL") it will not be used.  The options can be the
 * following - "all"(ALL_KW) - "irodsAdmin" (IRODS_ADMIN_KW). - "backupMode"
 * if specified, it will try to use 'backup mode' to the destination
 * resource. Means if a good copy already exists in destination resource, it
 * will not throw an error
 * 
 * \param[out] a INT_MS_T containing the status. \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* ****************************************************************************************** */
int
msiChkDataSize3 (msParam_t * coll, msParam_t * inpParam2,
		 msParam_t * inpParam3, msParam_t * outParam,
		 ruleExecInfo_t * rei)
{

  collInp_t collInp;
  rsComm_t *rsComm;
  collInp_t *myCollInp;
  int i;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  long lMin = 55, lMax = 100;

  RE_TEST_MACRO ("    Calling msiChkDataSize")
    if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR, "msiChkDataSize: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  /* parse inpParam1: coll */
  rei->status = parseMspForCollInp (coll, &collInp, &myCollInp, 0);
  if (rei->status < 0)
    {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			  "msiChkDataSize: input inpParam1 error. status = %d",
			  rei->status);
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataSize(), input inpParam1 error\n");
      rodsLog (LOG_ERROR, "msiChkDataSize(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);

      return (rei->status);
    }
  /* parse inpParam2 MinSize */
  if ((strTimeDiff = parseMspForStr (inpParam2)) != NULL)
    {
      lMin = strtol (strTimeDiff, (char **) NULL, 10);
      printf ("GJK ########################## p2=(%s)\n", strTimeDiff);
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataSize(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDataSize(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam, strOut);
      return (-1);
    }

  /* parse inpParam3 MaxSize */
  if ((strTimeDiff = parseMspForStr (inpParam3)) != NULL)
    {
      lMax = strtol (strTimeDiff, (char **) NULL, 10);
      printf ("GJK ########################## p3=(%s)\n", strTimeDiff);
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataSize(), input inpParam3 error\n");
      rodsLog (LOG_ERROR, "msiChkDataSize(),  input inpParam3 error.");
      i = fillStrInMsParam (outParam, strOut);
      return (-1);
    }
  return (0);
}

int
msiChkDataSize2 (msParam_t * coll1, msParam_t * inpParam2,
		 msParam_t * inpParam3, msParam_t * outParam,
		 ruleExecInfo_t * rei)
{
  return (0);
};


/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnce11 \author  Sifang Lu \date   2007-10-01 \brief This
 * microservice iterate through collection, and calls rsDataObjRepl to
 * recursively replication a collection as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 * i.e., rule execution initiated by clients and should not be called
 * internally by the server since it interacts with the client through the
 * normal client/server socket connection. \param[in] coll     : It can be a
 * collInp_t or a STR_MS_T which would be taken as destination collection
 * path. destResc : STR_MS_T destination resource name options  : STR_MS_T a
 * group of options in a string delimited by '%%'. If the string is empty
 * ("\0") or null ("NULL") it will not be used.  The options can be the
 * following - "all"(ALL_KW) - "irodsAdmin" (IRODS_ADMIN_KW). - "backupMode"
 * if specified, it will try to use 'backup mode' to the destination
 * resource. Means if a good copy already exists in destination resource, it
 * will not throw an error
 * 
 * \param[out] a INT_MS_T containing the status. \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */
/* **************************************************************************** */
int
msiChkDatNameOnce11 (msParam_t * coll1, msParam_t * coll2, msParam_t * coll3,
		     msParam_t * outParam, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2, *myCollInp1, *myCollInp2;
  int i, j, iSame = (-1);
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  int iChkFailed1 = 0, iChkFailed2 = 0;
  int intLsArraySize1 = (MAX_NAME_LEN), intLsArraySize2 = (MAX_NAME_LEN);
  lsInfo_t lsArray1[MAX_NAME_LEN + 1], lsArray2[MAX_NAME_LEN + 1];

  char *strDataTypeInput1, *strDataTypeInput2, *strDataTypeInput3;

  RE_TEST_MACRO ("    Calling msiChkDatNameOnce11")
    printf
    ("GJK- begin 0001.0.0-11r,Fri Mar 14 18:29:18 PDT 2008, in msiChkDatNameOnce11(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############01x STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01x OK\n");

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR, "msiChkDatNameOnce11: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  /* parse inpParam1: coll */
  if (1 == 2)
    {
      rei->status = parseMspForCollInp (coll1, &collInp1, &myCollInp1, 0);
      if (rei->status < 0)
	{
	  rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			      "msiChkDatNameOnce11: input inpParam1 error. status = %d",
			      rei->status);
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDatNameOnce11(), input inpParam1 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);

	  return (rei->status);
	}
    }

  if (1 == 1)
    {
      /* parse input parameter 1 'coll1' */
      if ((strDataTypeInput1 = parseMspForStr (coll1)) != NULL)
	{
	}
      else
	{
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
							 * addformatedtrsing to
							 * bytes WriteBytesBuff
							 * printMsParam.c */
	  return (-1);
	}
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############1a STOP\n");
      return (0);
    }
  else
    {
      printf
	("GJK ############### fake00 ##############1a OK, myCollInp1->collName=(%s), strDataTypeInput1=(%s)\n",
	 myCollInp1->collName, strDataTypeInput1);
      sleep (1);
    }

  /* parse inpParam2: coll */
  if (1 == 2)
    {
      rei->status = parseMspForCollInp (coll2, &collInp2, &myCollInp2, 0);
      if (rei->status < 0)
	{
	  rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			      "msiChkDatNameOnce11: input inpParam1 error. status = %d",
			      rei->status);
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDatNameOnce11(), input inpParam1 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);

	  return (rei->status);
	}
    }

  if (1 == 1)
    {
      /* parse input parameter 2 'coll2' */
      if ((strDataTypeInput2 = parseMspForStr (coll2)) != NULL)
	{
	}
      else
	{
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
							 * addformatedtrsing to
							 * bytes WriteBytesBuff
							 * printMsParam.c */
	  return (-1);
	}
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############2aK STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############2aK OK,  myCollInp2->collName=(%s), strDataTypeInput2=(%s)\n",
       myCollInp2->collName, strDataTypeInput2);

#define noPAR3
#ifdef PAR3
  /* parse coll3 MaxSize */
  if ((strFileObjName = parseMspForStr (coll3)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataSize(), input coll3 error\n");
      rodsLog (LOG_ERROR, "msiChkDataSize(),  input coll3 error.");
      i = fillStrInMsParam (outParam, strOut);
      return (-1);
    }
#else
  if (1 == 2)
    {
      /* parse input parameter 3 'coll3' */
      if ((strDataTypeInput3 = parseMspForStr (coll3)) != NULL)
	{
	}
      else
	{
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
							 * addformatedtrsing to
							 * bytes WriteBytesBuff
							 * printMsParam.c */
	  return (-1);
	}
    }
#endif

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  rstrcpy (collInp1.collName, strDataTypeInput1, MAX_NAME_LEN);
  if (1 == 2)
    {
      printf ("GJK fake00021xA STOP\n");
      return (0);
    }
  else
    printf
      ("GJK fake00021xA OK, before intDescColTree(), collInp1.collName=(%s)\n",
       collInp1.collName);

  iChkFailed1 =
    intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 0);
  rstrcpy (strOut, "", 2);

  for (i = 80; i < iChkFailed1; i++)
    {
      printf
	("GJK fake9998x intlsArray1Size[%3d].chCollection=(%s), intlsArray1Size[%3d].chObjName=(%s), intlsArray1Size[%3d].isData=(%d), lsArray1[%3d].chDataSize=(%s), lsArray1[i].lDataSize=(%ld), i=(%3d)\n",
	 i, lsArray1[i].chCollection, i, lsArray1[i].chObjName, i,
	 lsArray1[i].isData, i, lsArray1[i].chDataSize,
	 (long) lsArray1[i].lDataSize, i);
    }

  rstrcpy (collInp2.collName, strDataTypeInput2, MAX_NAME_LEN);
  if (1 == 2)
    {
      printf ("GJK fake00022xB STOP\n");
      return (0);
    }
  else
    printf
      ("GJK fake00022xB OK, before intDescColTree(), collInp2.collName=(%s)\n",
       collInp2.collName);

  iChkFailed2 =
    intDescColTree (rsComm, &collInp2, rei, lsArray2, intLsArraySize2, 0);

  for (i = 80; i < iChkFailed2; i++)
    {
      printf
	("GJK fake9998y intlsArray2Size[%3d].chCollection=(%s), intlsArray2Size[%3d].chObjName=(%s), intlsArray2Size[%3d].isData=(%d), lsArray2[%3d].chDataSize=(%s), lsArray2[i].lDataSize=(%ld), i=(%3d)\n",
	 i, lsArray2[i].chCollection, i, lsArray2[i].chObjName, i,
	 lsArray2[i].isData, i, lsArray2[i].chDataSize,
	 (long) lsArray2[i].lDataSize, i);
    }

  iSame = 0;
  for (i = 0; i < iChkFailed1; i++)
    {
      for (j = 0; j < iChkFailed2; j++)
	{
	  if (strcmp (lsArray1[i].chObjName, lsArray2[j].chObjName) == 0)
	    {
	      printf
		("GJK fake9977y  i=(%3d), j=(%3d),  lsArray1[i].chObjName=(%s), lsArray2[j].chObjName=(%s),iSame=(%d)\n",
		 i, j, lsArray1[i].chObjName, lsArray2[j].chObjName, iSame);
	      sprintf (strOut, "%sobject1='%s', object2='%s', count=%d\n",
		       strOut, lsArray1[i].chObjName, lsArray2[j].chObjName,
		       iSame);
	      iSame++;
	    }
	  else
	    {
	      /*
	       * printf ("GJK fake9977z  i=(%3d),
	       * j=(%d),
	       * lsArray1[i].chObjName=(%s),
	       * lsArray2[j].chObjName=(%s)\n", i,
	       * j, lsArray1[i].chObjName ,
	       * lsArray2[j].chObjName);
	       */
	    }
	}
    }

  if (iSame < 1)
    (void) snprintf (strOut, 255,
		     "%sTest OK. No file object duplicates found in '%s' and '%s' collections.\n",
		     strOut, collInp1.collName, collInp2.collName);
  else
    {
      (void) snprintf (strOut, 255,
		       "%sTest failed. %d file object duplicates found in '%s' and '%s' collections.\n",
		       strOut, iSame, collInp1.collName, collInp2.collName);
    }

  i = fillStrInMsParam (outParam, strOut);

  if (1 == 2)
    {
      printf ("GJK fake9999a Tue Mar  4 18:47:03 PST 2008 STOP\n");
      return (0);
    }
  else
    printf ("GJK fake9999a Tue Mar  4 18:47:03 PST 2008 OK\n");

  printf
    ("GJK end of msiChkDatNameOnce11(), Fri Mar 14 18:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11(msParam... */

/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnce11b \author  Sifang Lu \date   2007-10-01 \brief This
 * microservice iterate through collection, and calls rsDataObjRepl to
 * recursively replication a collection as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 * i.e., rule execution initiated by clients and should not be called
 * internally by the server since it interacts with the client through the
 * normal client/server socket connection. \param[in] coll     : It can be a
 * collInp_t or a STR_MS_T which would be taken as destination collection
 * path. destResc : STR_MS_T destination resource name options  : STR_MS_T a
 * group of options in a string delimited by '%%'. If the string is empty
 * ("\0") or null ("NULL") it will not be used.  The options can be the
 * following - "all"(ALL_KW) - "irodsAdmin" (IRODS_ADMIN_KW). - "backupMode"
 * if specified, it will try to use 'backup mode' to the destination
 * resource. Means if a good copy already exists in destination resource, it
 * will not throw an error
 * 
 * \param[out] a INT_MS_T containing the status. \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */
/* **************************************************************************** */
int
msiChkDatNameOnce11b (msParam_t * coll1, msParam_t * coll2, msParam_t * coll3,
		      msParam_t * outParam, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2, *myCollInp1, *myCollInp2;
  int i, j, iSame = (-1);
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  int iChkFailed1 = 0, iChkFailed2 = 0;
  int intLsArraySize1 = (MAX_NAME_LEN), intLsArraySize2 = (MAX_NAME_LEN);
  lsInfo_t lsArray1[MAX_NAME_LEN + 1], lsArray2[MAX_NAME_LEN + 1];

  char *strDataTypeInput1, *strDataTypeInput2, *strDataTypeInput3;

  RE_TEST_MACRO ("    Calling msiChkDatNameOnce11b")
    printf
    ("GJK- begin 0001.0.0-11br,Fri Mar 14 18:29:18 PDT 2008, in msiChkDatNameOnce11b(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############01.11b.x STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.11.b.x OK\n");

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDatNameOnce11b: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  /* parse inpParam1: coll */
  if (1 == 2)
    {
      rei->status = parseMspForCollInp (coll1, &collInp1, &myCollInp1, 0);
      if (rei->status < 0)
	{
	  rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			      "msiChkDatNameOnce11b: input inpParam1 error. status = %d",
			      rei->status);
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDatNameOnce11b(), input inpParam1 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11b(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);

	  return (rei->status);
	}
    }

  if (1 == 1)
    {
      /* parse input parameter 1 'coll1' */
      if ((strDataTypeInput1 = parseMspForStr (coll1)) != NULL)
	{
	}
      else
	{
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11b(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
							 * addformatedtrsing to
							 * bytes WriteBytesBuff
							 * printMsParam.c */
	  return (-1);
	}
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############011b.1a STOP\n");
      return (0);
    }
  else
    {
      printf
	("GJK ############### fake00 ##############011b.1a OK, myCollInp1->collName=(%s), strDataTypeInput1=(%s)\n",
	 myCollInp1->collName, strDataTypeInput1);
      sleep (1);
    }

  /* parse inpParam2: coll */
  if (1 == 2)
    {
      rei->status = parseMspForCollInp (coll2, &collInp2, &myCollInp2, 0);
      if (rei->status < 0)
	{
	  rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			      "msiChkDatNameOnce11b: input inpParam1 error. status = %d",
			      rei->status);
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDatNameOnce11b(), input inpParam1 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11b(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);

	  return (rei->status);
	}
    }

  if (1 == 1)
    {
      /* parse input parameter 2 'coll2' */
      if ((strDataTypeInput2 = parseMspForStr (coll2)) != NULL)
	{
	}
      else
	{
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11b(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
							 * addformatedtrsing to
							 * bytes WriteBytesBuff
							 * printMsParam.c */
	  return (-1);
	}
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############011b.2aK STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############011b.2aK OK,  myCollInp2->collName=(%s), strDataTypeInput2=(%s)\n",
       myCollInp2->collName, strDataTypeInput2);

#define noPAR3
#ifdef PAR3
  /* parse coll3 MaxSize */
  if ((strFileObjName = parseMspForStr (coll3)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataSize(), input coll3 error\n");
      rodsLog (LOG_ERROR, "msiChkDataSize(),  input coll3 error.");
      i = fillStrInMsParam (outParam, strOut);
      return (-1);
    }
#else
  if (1 == 1)
    {
      /* parse input parameter 3 'coll3' */
      if ((strDataTypeInput3 = parseMspForStr (coll3)) != NULL)
	{
	}
      else
	{
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnce11b(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
							 * addformatedtrsing to
							 * bytes WriteBytesBuff
							 * printMsParam.c */
	  return (-1);
	}
    }
#endif

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############011b.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############011b.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  rstrcpy (collInp1.collName, strDataTypeInput1, MAX_NAME_LEN);
  if (1 == 2)
    {
      printf ("GJK fake00021xA STOP\n");
      return (0);
    }
  else
    printf
      ("GJK fake00021xA OK, before intDescColTree(), collInp1.collName=(%s)\n",
       collInp1.collName);

  iChkFailed1 =
    intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 0);
  rstrcpy (strOut, "", 2);

  for (i = 80; i < iChkFailed1; i++)
    {
      printf
	("GJK fake9998x intlsArray1Size[%3d].chCollection=(%s), intlsArray1Size[%3d].chObjName=(%s), intlsArray1Size[%3d].isData=(%d), lsArray1[%3d].chDataSize=(%s), lsArray1[i].lDataSize=(%ld), i=(%3d)\n",
	 i, lsArray1[i].chCollection, i, lsArray1[i].chObjName, i,
	 lsArray1[i].isData, i, lsArray1[i].chDataSize,
	 (long) lsArray1[i].lDataSize, i);
    }

  rstrcpy (collInp2.collName, strDataTypeInput2, MAX_NAME_LEN);
  if (1 == 2)
    {
      printf ("GJK fake00022xB STOP\n");
      return (0);
    }
  else
    printf
      ("GJK fake00022xB OK, before intDescColTree(), collInp2.collName=(%s)\n",
       collInp2.collName);

  iChkFailed2 =
    intDescColTree (rsComm, &collInp2, rei, lsArray2, intLsArraySize2, 0);

  for (i = 80; i < iChkFailed2; i++)
    {
      printf
	("GJK fake9998y intlsArray2Size[%3d].chCollection=(%s), intlsArray2Size[%3d].chObjName=(%s), intlsArray2Size[%3d].isData=(%d), lsArray2[%3d].chDataSize=(%s), lsArray2[i].lDataSize=(%ld), i=(%3d)\n",
	 i, lsArray2[i].chCollection, i, lsArray2[i].chObjName, i,
	 lsArray2[i].isData, i, lsArray2[i].chDataSize,
	 (long) lsArray2[i].lDataSize, i);
    }

  iSame = 0;
  for (i = 0; i < iChkFailed1; i++)
    {
      for (j = 0; j < iChkFailed2; j++)
	{
	  if (strcmp (lsArray1[i].chObjName, lsArray2[j].chObjName) == 0 &&
	      strcmp (lsArray1[i].chObjName, strDataTypeInput3) == 0)
	    {
	      printf
		("GJK fake9977y  i=(%3d), j=(%3d),  lsArray1[i].chCollection=(%s), lsArray2[i].chCollection=(%s), lsArray1[i].chObjName=(%s), lsArray2[j].chObjName=(%s), strDataTypeInput3=(%s), iSame=(%d)\n",
		 i, j, lsArray1[i].chCollection, lsArray2[i].chCollection,
		 lsArray1[i].chObjName, lsArray2[j].chObjName,
		 strDataTypeInput3, iSame);
	      sprintf (strOut, "%sobject1='%s', object2='%s', count=%d\n",
		       strOut, lsArray1[i].chObjName, lsArray2[j].chObjName,
		       iSame);
	      iSame++;
	    }
	  else
	    {
	      /*
	       * printf ("GJK fake9977z  i=(%3d),
	       * j=(%d),
	       * lsArray1[i].chObjName=(%s),
	       * lsArray2[j].chObjName=(%s)\n", i,
	       * j, lsArray1[i].chObjName ,
	       * lsArray2[j].chObjName);
	       */
	    }
	}
    }

  if (iSame < 1)
    (void) snprintf (strOut, 255,
		     "%sTest OK. No file object '%s' duplicates found in '%s' and '%s' collections.\n",
		     strOut, strDataTypeInput3, collInp1.collName,
		     collInp2.collName);
  else
    {
      (void) snprintf (strOut, 255,
		       "%sTest failed. %d file object '%s' duplicates found in '%s' and '%s' collections.\n",
		       strOut, iSame, strDataTypeInput3, collInp1.collName,
		       collInp2.collName);
    }

  i = fillStrInMsParam (outParam, strOut);

  if (1 == 2)
    {
      printf ("GJK fake9999a Tue Mar  4 18:47:03 PST 2008 STOP\n");
      return (0);
    }
  else
    printf ("GJK fake9999a Tue Mar  4 18:47:03 PST 2008 OK\n");

  printf
    ("GJK end of msiChkDatNameOnce11b(), Fri Mar 14 18:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */

/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW3T1 \author  Sifang Lu \date   2007-10-01 \brief This
 * microservice iterate through collection, and calls rsDataObjRepl to
 * recursively replication a collection as part of a workflow  execution.
 * \note This call should only be used through the rcExecMyRule (irule) call
 * i.e., rule execution initiated by clients and should not be called
 * internally by the server since it interacts with the client through the
 * normal client/server socket connection. \param[in] coll     : It can be a
 * collInp_t or a STR_MS_T which would be taken as destination collection
 * path. destResc : STR_MS_T destination resource name options  : STR_MS_T a
 * group of options in a string delimited by '%%'. If the string is empty
 * ("\0") or null ("NULL") it will not be used.  The options can be the
 * following - "all"(ALL_KW) - "irodsAdmin" (IRODS_ADMIN_KW). - "backupMode"
 * if specified, it will try to use 'backup mode' to the destination
 * resource. Means if a good copy already exists in destination resource, it
 * will not throw an error
 * 
 * \param[out] a INT_MS_T containing the status. \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */
/* **************************************************************************** */
int
msiChkDatNameOnceW3T1 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * coll3, msParam_t * outParam,
		       ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2, *myCollInp1, *myCollInp2;
  int i, j, iSame = (-1);
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  int iChkFailed1 = 0, iChkFailed2 = 0;
  int intLsArraySize1 = (MAX_NAME_LEN), intLsArraySize2 = (MAX_NAME_LEN);
  lsInfo_t lsArray1[MAX_NAME_LEN + 1], lsArray2[MAX_NAME_LEN + 1];

  char *strDataTypeInput1, *strDataTypeInput2, *strDataTypeInput3;

  RE_TEST_MACRO ("    Calling msiChkDatNameOnceW3T1")
    printf
    ("GJK- begin 0001.0.0-W3T1r,Fri Mar 14 18:29:18 PDT 2008, in msiChkDatNameOnceW3T1(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############01.W3T1.x STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.11.b.x OK\n");

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDatNameOnceW3T1: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  /* parse inpParam1: coll */
  if (1 == 2)
    {
      rei->status = parseMspForCollInp (coll1, &collInp1, &myCollInp1, 0);
      if (rei->status < 0)
	{
	  rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			      "msiChkDatNameOnceW3T1: input inpParam1 error. status = %d",
			      rei->status);
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDatNameOnceW3T1(), input inpParam1 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnceW3T1(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);

	  return (rei->status);
	}
    }

  if (1 == 1)
    {
      /* parse input parameter 1 'coll1' */
      if ((strDataTypeInput1 = parseMspForStr (coll1)) != NULL)
	{
	}
      else
	{
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnceW3T1(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
							 * addformatedtrsing to
							 * bytes WriteBytesBuff
							 * printMsParam.c */
	  return (-1);
	}
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W3T1.1a STOP\n");
      return (0);
    }
  else
    {
      printf
	("GJK ############### fake00 ##############0W3T1.1a OK, myCollInp1->collName=(%s), strDataTypeInput1=(%s)\n",
	 myCollInp1->collName, strDataTypeInput1);
      sleep (1);
    }

  /* parse inpParam2: coll */
  if (1 == 2)
    {
      rei->status = parseMspForCollInp (coll2, &collInp2, &myCollInp2, 0);
      if (rei->status < 0)
	{
	  rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			      "msiChkDatNameOnceW3T1: input inpParam1 error. status = %d",
			      rei->status);
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDatNameOnceW3T1(), input inpParam1 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnceW3T1(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);

	  return (rei->status);
	}
    }

  if (1 == 1)
    {
      /* parse input parameter 2 'coll2' */
      if ((strDataTypeInput2 = parseMspForStr (coll2)) != NULL)
	{
	}
      else
	{
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnceW3T1(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
							 * addformatedtrsing to
							 * bytes WriteBytesBuff
							 * printMsParam.c */
	  return (-1);
	}
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W3T1.2aK STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W3T1.2aK OK,  myCollInp2->collName=(%s), strDataTypeInput2=(%s)\n",
       myCollInp2->collName, strDataTypeInput2);

#define noPAR3
#ifdef PAR3
  /* parse coll3 MaxSize */
  if ((strFileObjName = parseMspForStr (coll3)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataSize(), input coll3 error\n");
      rodsLog (LOG_ERROR, "msiChkDataSize(),  input coll3 error.");
      i = fillStrInMsParam (outParam, strOut);
      return (-1);
    }
#else
  if (1 == 1)
    {
      /* parse input parameter 3 'coll3' */
      if ((strDataTypeInput3 = parseMspForStr (coll3)) != NULL)
	{
	}
      else
	{
	  sprintf (strOut,
		   "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
	  rodsLog (LOG_ERROR,
		   "msiChkDatNameOnceW3T1(),  input inpParam1 error.");
	  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
							 * addformatedtrsing to
							 * bytes WriteBytesBuff
							 * printMsParam.c */
	  return (-1);
	}
    }
#endif

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W3T1.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W3T1.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  rstrcpy (collInp1.collName, strDataTypeInput1, MAX_NAME_LEN);
  if (1 == 2)
    {
      printf ("GJK fake00021xA STOP\n");
      return (0);
    }
  else
    printf
      ("GJK fake00021xA OK, before intDescColTree(), collInp1.collName=(%s)\n",
       collInp1.collName);

  iChkFailed1 =
    intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 1);
  rstrcpy (strOut, "", 2);

  for (i = 80; i < iChkFailed1; i++)
    {
      printf
	("GJK fake9998x intlsArray1Size[%3d].chCollection=(%s), intlsArray1Size[%3d].chObjName=(%s), intlsArray1Size[%3d].isData=(%d), lsArray1[%3d].chDataSize=(%s), lsArray1[i].lDataSize=(%ld), i=(%3d)\n",
	 i, lsArray1[i].chCollection, i, lsArray1[i].chObjName, i,
	 lsArray1[i].isData, i, lsArray1[i].chDataSize,
	 (long) lsArray1[i].lDataSize, i);
    }

  rstrcpy (collInp2.collName, strDataTypeInput2, MAX_NAME_LEN);
  if (1 == 2)
    {
      printf ("GJK fake00022xB STOP\n");
      return (0);
    }
  else
    printf
      ("GJK fake00022xB OK, before intDescColTree(), collInp2.collName=(%s)\n",
       collInp2.collName);

  iChkFailed2 =
    intDescColTree (rsComm, &collInp2, rei, lsArray2, intLsArraySize2, 1);

  for (i = 80; i < iChkFailed2; i++)
    {
      printf
	("GJK fake9998y intlsArray2Size[%3d].chCollection=(%s), intlsArray2Size[%3d].chObjName=(%s), intlsArray2Size[%3d].isData=(%d), lsArray2[%3d].chDataSize=(%s), lsArray2[i].lDataSize=(%ld), i=(%3d)\n",
	 i, lsArray2[i].chCollection, i, lsArray2[i].chObjName, i,
	 lsArray2[i].isData, i, lsArray2[i].chDataSize,
	 (long) lsArray2[i].lDataSize, i);
    }

  iSame = 0;
  for (i = 0; i < iChkFailed1; i++)
    {
      for (j = 0; j < iChkFailed2; j++)
	{
	  if (strcmp (lsArray1[i].chObjName, lsArray2[j].chObjName) == 0 &&
	      strcmp (lsArray1[i].chObjName, strDataTypeInput3) == 0)
	    {
	      printf
		("GJK fake9977y  i=(%3d), j=(%3d),  lsArray1[i].chCollection=(%s), lsArray2[i].chCollection=(%s), lsArray1[i].chObjName=(%s), lsArray2[j].chObjName=(%s), strDataTypeInput3=(%s), iSame=(%d)\n",
		 i, j, lsArray1[i].chCollection, lsArray2[i].chCollection,
		 lsArray1[i].chObjName, lsArray2[j].chObjName,
		 strDataTypeInput3, iSame);
	      sprintf (strOut, "%sobject1='%s', object2='%s', count=%d\n",
		       strOut, lsArray1[i].chObjName, lsArray2[j].chObjName,
		       iSame);
	      iSame++;
	    }
	  else
	    {
	      /*
	       * printf ("GJK fake9977z  i=(%3d),
	       * j=(%d),
	       * lsArray1[i].chObjName=(%s),
	       * lsArray2[j].chObjName=(%s)\n", i,
	       * j, lsArray1[i].chObjName ,
	       * lsArray2[j].chObjName);
	       */
	    }
	}
    }

  if (iSame < 1)
    (void) snprintf (strOut, 255,
		     "%sTest OK. No file object '%s' duplicates found in '%s' and '%s' collections.\n",
		     strOut, strDataTypeInput3, collInp1.collName,
		     collInp2.collName);
  else
    {
      (void) snprintf (strOut, 255,
		       "%sTest failed. %d file object '%s' duplicates found in '%s' and '%s' collections.\n",
		       strOut, iSame, strDataTypeInput3, collInp1.collName,
		       collInp2.collName);
    }

  i = fillStrInMsParam (outParam, strOut);

  if (1 == 2)
    {
      printf ("GJK fake9999a Tue Mar  4 18:47:03 PST 2008 STOP\n");
      return (0);
    }
  else
    printf ("GJK fake9999a Tue Mar  4 18:47:03 PST 2008 OK\n");

  printf
    ("GJK end of msiChkDatNameOnceW3T1(), Fri Mar 14 18:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */

/* ****************************************************************************************** */
/*
 * \fn msiChkRechkRecompChkSum4DatObjVol2 \author  Sifang Lu \date 2007-10-01
 * \brief This microservice iterate through collection, and calls
 * rsDataObjRepl to recursively replication a collection as part of a
 * workflow  execution. \note This call should only be used through the
 * rcExecMyRule (irule) call i.e., rule execution initiated by clients and
 * should not be called internally by the server since it interacts with the
 * client through the normal client/server socket connection. \param[in] coll
 * : It can be a collInp_t or a STR_MS_T which would be taken as destination
 * collection path. destResc : STR_MS_T destination resource name options  :
 * STR_MS_T a group of options in a string delimited by '%%'. If the string
 * is empty ("\0") or null ("NULL") it will not be used.  The options can be
 * the following - "all"(ALL_KW) - "irodsAdmin" (IRODS_ADMIN_KW). -
 * "backupMode" if specified, it will try to use 'backup mode' to the
 * destination resource. Means if a good copy already exists in destination
 * resource, it will not throw an error
 * 
 * \param[out] a INT_MS_T containing the status. \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

int
msiChkDataObjAVU6Vol10 (msParam_t * coll, msParam_t * inpParam2,
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

  RE_TEST_MACRO ("    Calling msiChkDataObjAVU6Vol10")
    if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDataObjAVU6Vol10: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  /* parse inpParam1: coll */
  rei->status = parseMspForCollInp (coll, &collInp, &myCollInp, 0);
  if (rei->status < 0)
    {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			  "msiChkDataObjAVU6Vol10: input inpParam1 error. status = %d",
			  rei->status);
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam1 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkDataObjAVU6Vol10(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */

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
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkDataObjAVU6Vol10(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
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
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam3 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkDataObjAVU6Vol10(),  input inpParam3 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  printf
    ("GJK- begin 00021.1.2 in msiChkDataObjAVU6Vol10(), Fri Feb 29 22:31:47 PST 2008, status=(%d), myCollInp->collName=(%s)\n",
     status, myCollInp->collName);

  if (1 == 2)
    {
      printf ("GJK fake13 STOP\n");
      return (0);
    }
  else
    printf ("GJK fake13 OK\n");

  /*
   * (void) intChkDataType (rsComm,
   * "/tempZone/home/rods/CVS/Entries.Log", (time_t) i, rei); test
   * blbost sobota
   */

  /* iterate through all files */
  memset (&genQueryInp, 0, sizeof (genQueryInp));
  memset (&genQueryOut, 0, sizeof (genQueryOut));

  /* ./lib/core/include/rodsGenQuery. */
  /* R_DATA_MAIN: */

  addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
  addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);

  addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);

  addInxIval (&genQueryInp.selectInp, COL_D_RESC_NAME, 1);
  addInxIval (&genQueryInp.selectInp, COL_D_MODIFY_TIME, 1);

  /* ?? addInxIval (&genQueryInp->selectInp, COL_COLL_TYPE, 1); */

  /*
   * #gjk2 file size in range #gjk3 ACL user-uthorization pairs #gjk4
   * ACL contain at least #gjk5 does not ACL #gjk6 AVU contain exactly
   * #gjk7 AVU only one #gjk8 AVU does not have duplicates
   */

  printf
    ("GJK- begin 00021.1.3 in msiChkDataObjAVU6Vol10(), status=(%d), myCollInp->collName=(%s)\n",
     status, myCollInp->collName);

  if (1 == 2)
    {
      printf ("GJK fake14 STOP\n");
      return (0);
    }
  else
    printf ("GJK fake14 OK\n");

  status =
    rsQueryDataObjInCollReCur (rsComm, myCollInp->collName, &genQueryInp,
			       &genQueryOut, NULL, 1);

  printf
    ("GJK- begin 00021.1.4 in msiChkDataObjAVU6Vol10(), status=(%d), myCollInp->collName=(%s)\n",
     status, myCollInp->collName);

  if (1 == 1)
    {
      printf ("GJK fake15 STOP\n");
      return (0);
    }
  else
    printf ("GJK fake15 OK\n");

  /*
   * printf("GJK- begin 0001.1.1 in msiChkDataObjAVU6Vol10(),
   * status=(%d), myCollInp->collName=(%s)\n", status,
   * myCollInp->collName);
   */
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
			  "msiChkDataObjAVU6Vol10: msiChkDataObjAVU6Vol10 error for %s, stat=%d",
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
			  "msiChkDataObjAVU6Vol10: msiChkDataObjAVU6Vol10 error for %s, stat=%d",
			  myCollInp->collName, status);
    }
  if (status != 0)
    {
      printf
	("GJK- begin 00041.2.3z CAT_NO_ROWS_FOUND?=status=(%d), myCollInp->collName=(%s)\n",
	 status, myCollInp->collName);
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			  "msiChkDataObjAVU6Vol10: msiChkDataObjAVU6Vol10 error for %s, stat=%d",
			  myCollInp->collName, status);
      rei->status = status;
      return (rei->status);
    }
  printf ("GJK- begin 00041.2.3b status=(%d), myCollInp->collName=(%s)\n",
	  status, myCollInp->collName);
  return (0);

  while (rei->status >= 0)
    {
      sqlResult_t *subColl, *dataObj, *sqlDataSize4 = NULL;

      printf ("GJK- begin 00041.2.5 status=(%d), myCollInp->collName=(%s)\n",
	      status, myCollInp->collName);
      if (1 == 2)
	{
	  printf ("GJK fake17 STOP\n");
	  return (0);
	}
      else
	printf ("GJK fake17 OK\n");

      /* get sub coll paths in the batch */
      subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME);
      dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
      /* COL_DATA_SIZE */
      /*
       * sqlDataSize4 = getSqlResultByInx (genQueryOut,
       * COL_D_CREATE_TIME);
       */
      sqlDataSize4 = getSqlResultByInx (genQueryOut, COL_DATA_SIZE);

      if (sqlDataSize4 == NULL)
	{
	  printf
	    ("GJK-P P.00044.2.2x ERROR sqlDataSize4 == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i);
	  rodsLog (LOG_ERROR,
		   "printLsLong: getSqlResultByInx for COL_DATA_SIZE failed");
	}
      if (1 == 2)
	{
	  printf ("GJK fake7b STOP\n");
	  return (0);
	}
      else
	printf ("GJK fake7b OK\n");

#define BAD1
#ifdef BAD1
      if (sqlDataSize4 == NULL)
	{
	  printf
	    ("GJK-P P.003.2.2g ERROR sqlDataSize4 == NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i);
	}
      else
	{
	  printf
	    ("GJK-P P.003.2.2g OK sqlDataSize4 != NULL, dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
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

	  /* if (dataObj == NULL && status < 0) { */
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
	      /*
	       * rei->status = rsGenQuery (rsComm,
	       * &genQueryInp, &genQueryOut);
	       */

	      rodsLog (LOG_ERROR,
		       "msiChkDataObjAVU6Vol10: msiChkDataObjAVU6Vol10 failed, (%s) is not an iRods data object (dataObj == NULL), istatus=%d, rei->status=%d",
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
	      /*
	       * printf("GJK-P P.4001.0.2. in
	       * msiChkDataObjAVU6Vol10(),
	       * dataObjInp.objPath=(%s), i=%d\n",
	       * dataObjInp.objPath, i);
	       */
	      /* GJK return(0); */
	    }			/* dataObj == NULL) { */

	  rodsLog (LOG_ERROR,
		   "msiChkDataObjAVU6Vol10: msiChkDataObjAVU6Vol10 failed, (%s) is not an iRods collection, rei->status=%d",
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
		   "msiChkDataObjAVU6Vol10: msiChkDataObjAVU6Vol10 for COL_DATA_NAME failed");
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

      for (i = 0; i < genQueryOut->rowCnt; i++)
	{
	  char *tmpSubColl, *tmpDataName, *tmpDataSize4, strOtaznik4[3] = "?";

	  /*
	   * ./lib/api/include/objStat.h:__rsObjStat (rsComm_t
	   * *rsComm, dataObjInp_t *dataObjInp, int interFlag,
	   */
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
	  tmpSubColl = &subColl->value[subColl->len * i];
	  tmpDataName = &dataObj->value[dataObj->len * i];
	  if (sqlDataSize4 != NULL)
	    {
	      tmpDataSize4 = &sqlDataSize4->value[sqlDataSize4->len * i];
	    }
	  else
	    {
	      rstrcpy (tmpDataSize4, strOtaznik4, MAX_NAME_LEN);
	    }

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
			 "msiChkDataObjAVU6Vol10(): input rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	      }
	    memset (&genQueryInp7, 0, sizeof (genQueryInp_t));
	    memset (&myDataObjInp7, 0, sizeof (dataObjInp_t));
	    rstrcpy (myDataObjInp7.objPath, dataObjInp.objPath, MAX_NAME_LEN);	/* rstrcpy(destination,
										 * source, max_len) */
	    /*
	     * memset (&rodsObjStatOut, 0, sizeof
	     * (rodsObjStat_t)); status =
	     * rsObjStat(rsComm, myDataObjInp,
	     * &rodsObjStatOut);
	     * 
	     * if data_type only
	     * 
	     * status7 = intGetDataObjAVUsVol10 (collInp_t *
	     * ptrInpColl, UserDefinedMetadata_t *
	     * aAVUarray, int *iTotalAVUs, char *strOut,
	     * ruleExecInfo_t * rei);
	     */

	    if (1 == 3)
	      {
		printf ("GJK fake0900a STOP\n");
		return (0);
	      }
	    else
	      printf ("GJK fake0900a OK\n");

	    strncpy (ptrInpColl.collName, dataObjInp.objPath, MAX_NAME_LEN);
	    status7 =
	      intGetDataObjAVUsVol10 (&ptrInpColl, aAVUarray,
				      &iCountUserDefinedMetadata, strOut,
				      rei);

	    if (1 == 2)
	      {
		printf ("GJK fake0901b STOP\n");
		return (0);
	      }
	    else
	      printf ("GJK fake0901b OK\n");

	    /*
	     * status7 = rsObjStat(rsComm,
	     * &myDataObjInp7, &rodsObjStatOut); status7
	     * = rsObjStat(rsComm, &dataObjInp,
	     * &rodsObjStatOut);
	     * 
	     * if (status7 == 0  && status7 == 1) { if
	     * ((long)rodsObjStatOut->objSize <= lMin)
	     * iCountMin++; if
	     * ((long)rodsObjStatOut->objSize >= lMax)
	     * iCountMax++; if
	     * ((long)rodsObjStatOut->objSize > lMin &&
	     * (long)rodsObjStatOut->objSize < lMax)
	     * iCountMid++;
	     * 
	     * } printf ("GJK-P P.007.2.2c
	     * dataObjInp.objPath=(%s), size=(%ld),
	     * createTime=(%s), status7=(%d)\n",
	     * dataObjInp.objPath,
	     * (long)rodsObjStatOut->objSize,
	     * rodsObjStatOut->createTime, status7);
	     */

	    /*
	     * 2008.02.11. typedef struct DataObjInfo {
	     * char objPath[MAX_NAME_LEN]; char
	     * rescName[MAX_NAME_LEN];       / * This could
	     * be resource group * / char
	     * rescGroupName[MAX_NAME_LEN];       / * This
	     * could be resource group * / char
	     * dataType[MAX_NAME_LEN]; rodsLong_t dataSize;
	     * char chksum[MAX_NAME_LEN]; char
	     * version[MAX_NAME_LEN]; char
	     * filePath[MAX_NAME_LEN]; rescInfo_t
	     * *rescInfo; char dataOwnerName[MAX_NAME_LEN];
	     * char dataOwnerZone[MAX_NAME_LEN]; int replNum;
	     * int  replStatus;     / * isDirty flag * /
	     * char statusString[MAX_NAME_LEN]; rodsLong_t
	     * dataId; rodsLong_t  collId; int dataMapId;
	     * char dataComments[LONG_NAME_LEN]; char
	     * dataExpiry[TIME_LEN]; char
	     * dataCreate[TIME_LEN]; char
	     * dataModify[TIME_LEN]; char
	     * dataAccess[MAX_NAME_LEN]; int dataAccessInx;
	     * char destRescName[MAX_NAME_LEN]; char
	     * backupRescName[MAX_NAME_LEN]; char
	     * subPath[MAX_NAME_LEN]; specColl_t
	     * *specColl; struct DataObjInfo *next; }
	     * dataObjInfo_t;
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
			     "in msiChkDataObjAVU6Vol10(),getDataObjInfo() for (%s), status=%d",
			     myDataObjInp7.objPath, status);
		    return (status);
		  }
		else
		  {
		    printf
		      ("GJK in msiChkDataObjAVU6Vol10(), getDataObjInfo(), OK, for (%s), type=(%s), status=%d\n",
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
	   * typedef struct DataObjInp { char
	   * objPath[MAX_NAME_LEN]; int createMode; int
	   * openFlags;      / * used for specCollInx in
	   * rcQuerySpecColl * / rodsLong_t offset; rodsLong_t
	   * dataSize; int numThreads; int oprType; specColl_t
	   * *specColl; keyValPair_t condInput;   / * include
	   * chksum flag and value * / } dataObjInp_t;
	   */
	  /*
	   * typedef struct rodsObjStat { rodsLong_t objSize;
	   * / * file size * / objType_t           objType;
	   * / * DATA_OBJ_T or COLL_OBJ_T * / int numCopies;
	   * char dataId[MAX_NAME_LEN]; char chksum[MAX_NAME_LEN]; char
	   * ownerName[MAX_NAME_LEN]; char ownerZone[MAX_NAME_LEN];
	   * char createTime[TIME_LEN]; char
	   * modifyTime[TIME_LEN]; specColl_t *specColl; }
	   * rodsObjStat_t;
	   */

	  printf
	    ("GJK-P P.002.2.2b1  muj hlavni cyklus ! in msiChkDataObjAVU6Vol10(), dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i);
	  printf
	    ("GJK-P P.002.2.2b2 dataObjInp.objPath=(%s), genQueryOut->rowCnt=(%d), i=(%d), tmpDataSize4=(%s)\n",
	     dataObjInp.objPath, genQueryOut->rowCnt, i, tmpDataSize4);

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

	  /*
	   * GJK  rei->status = rsDataObjRepl (rsComm,
	   * &dataObjInp, &transStat);
	   */
	  rei->status = 0;	/* GJK */
	  if (rei->status < 0)
	    {
	      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
				  "msiChkDataObjAVU6Vol10: rsDataObjRepl failed %s, status = %d",
				  (&dataObjInp)->objPath, rei->status);
	    }
	  else
	    {
	      /*
	       * GJK fake 2 iErr = intChkDataType (rsComm,
	       * dataObjInp.objPath, t1, rei);        test
	       * blbost sobota
	       */
	      /*
	       * printf("GJK-P P.004001.0.1. in
	       * msiChkDataObjAVU6Vol10(),
	       * dataObjInp.objPath=(%s), i=%d\n",
	       * dataObjInp.objPath, i);
	       */
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
  i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse  add
						 * formated string to bytes
						 * WriteBytesBuff
						 * printMsParam.c */
  printf ("GJK end strOut=(%s)\n", strOut);

  return (rei->status);
}				/* msiChkDataObjAVU6Vol10 */

/*
 * Input : iRods absulute path of an object or collection, Unix time in
 * seconds for the time , if the file wa not checket after that input time,
 * than check it and recompute the sum and updata the AVY time stamp
 */

int
msiChkDataObjAVU6Vol10222 (msParam_t * inpParam1, msParam_t * inpParam2,
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
  RE_TEST_MACRO ("RE_TEST_MACRO, begin of msiChkDataObjAVU6Vol10");

  printf ("GJK-P P.2222.0.1. in msiChkDataObjAVU6Vol10()\n");

  rsComm = rei->rsComm;

  /* parse inpParam1 */
  rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);

  if (rei->status < 0)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDataObjAVU6Vol10(),  input inpParam1 error. status = %d",
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
	       "ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR,
	       "msiChkDataObjAVU6Vol10(),  input inpParam2 error.");
      i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  printf
    ("GJK-P P.2222.0.2. in msiChkDataObjAVU6Vol10(), ptrInpColl->collName=(%s), t1=%ld\n",
     ptrInpColl->collName, t1);

  iErr = intChkDataType (rsComm, ptrInpColl->collName, t1, rei);	/* test blbost sobota */
  /* (void) intChkDataType (rsComm, strFullDataPath, t1, rei); */

  sprintf (strOut,
	   "OK msiChkDataObjAVU6Vol10(), iCountUserDefinedMetadata=%d, t1=(%ld), iErr=%d\n",
	   iCountUserDefinedMetadata, t1, iErr);
  i = fillStrInMsParam (outParam1, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
  /* fillBuffInParam */

  printf
    ("GJK-P P.2222.0.9. in msiChkDataObjAVU6Vol10(), iCountUserDefinedMetadata=%d, iErr=%d\n",
     iCountUserDefinedMetadata, iErr);

  return (iErr);
}				/* msParam_t * outParam1, ruleExecInfo_t *
				 * rei) { */
