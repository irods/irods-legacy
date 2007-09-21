/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include <stdarg.h>
#include "apiHeaderAll.h"
#include "objStat.h"
#include "reDataObjOpr.h"
#include "reGlobalsExtern.h"
#include "reDataRel.h"


int
iAddChkSumDateAvuMetadata (rsComm_t * rsComm, char *objPath, int status)
{
  modAVUMetadataInp_t modAVUMetadataInp;
  char mytime[256];
  time_t t1;

  // mam check sum hodnotu
  // nemam check sum cas
  // insert check sum cas in Unix number
  /*Call the function to insert metadata here. */

  memset (&modAVUMetadataInp, 0, sizeof (modAVUMetadataInp));
  /*
     rstrcpy (dataObjInp.objPath, path, MAX_NAME_LEN);
     objStatus = rsObjStat (rsComm, &dataObjInp, &rodsObjStatOut);

     if (objStatus) {


     switch (objStatus) {
     case 1:
     modAVUMetadataInp.arg1 = "-d"; // dsts == filr 
     break;
     case 2:
     modAVUMetadataInp.arg1 = "-c"; // collection
     break;
     default:
     rodsLog (LOG_ERROR, "loadMetadataFromFile error: Unsupported Object Type");
     return (INVALID_OBJECT_TYPE);
     break;
     }
   */


  modAVUMetadataInp.arg0 = "add";
  modAVUMetadataInp.arg1 = "-d";	// data
  modAVUMetadataInp.arg2 = objPath;
  modAVUMetadataInp.arg3 = "MD5checkSumDataStamp";

  (void)
    printf
    ("GJK-P P.123.0.1. in iAddChkSumDateAvuMetadata(), after rsModAVUMetadata((%s), rsComm\n",
     objPath);
  (void) time (&t1);
  (void) snprintf (mytime, 255, "%d", (int) t1);
  modAVUMetadataInp.arg4 = mytime;
  modAVUMetadataInp.arg5 = "UnixTimeInSeconds";
  status = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
  (void)
    printf
    ("GJK-P P.123.0.2. in iAddChkSumDateAvuMetadata(), after rsModAVUMetadata((%s), rsComm\n",
     objPath);
  if (0 > status)
    (void) rodsLog (LOG_ERROR,
		    "iAddChkSumDateAvuMetadata() rsModAVUMetadata failed objPath=(%s)",
		    objPath);
  else
    (void)
      printf
      ("GJK-P P.10.0.3. in iAddChkSumDateAvuMetadata(), after rsModAVUMetadata((%s), sComm, status=%d\n",
       objPath, status);
  (void)
    printf
    ("GJK-P P.123.0.3. OK in iAddChkSumDateAvuMetadata(), after rsModAVUMetadata((%s), rsComm, status=%d\n",
     objPath, status);

  return (0);
}


int
iFindChkSumDateAvuMetadata (int status, genQueryOut_t * genQueryOut,
			    char *fullName, UserDefinedMetadata_t UAVArray[],
			    int *iCountUserDefinedMetadata)
{
  int i = 0, j = 0, iErr = 0;
  size_t size;

  *iCountUserDefinedMetadata = 0;

  printf
    ("GJK 300.0.0. iFindChkSumDateAvuMetadata, fullName=(%s), i=%d, status=%d\n",
     fullName, i, status);

  //return (0);

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

	      // appendStrToBBuf(mybuf, strlen(fullName)+1, fullName);
	      printf
		("GJK 300.0.1. iFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->rowCnt=%d, genQueryOut->attriCnt=%d, iCountUserDefinedMetadata=%d\n",
		 fullName, i, j, genQueryOut->rowCnt, genQueryOut->attriCnt,
		 *iCountUserDefinedMetadata);
	      // return (0);

	      for (j = 0; j < (genQueryOut->attriCnt - 0); j++)
		{
		  char *tResult;

		  printf
		    ("GJK 300.0.2. iFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n",
		     fullName, i, j, genQueryOut->attriCnt);
		  // return (0);

		  tResult = genQueryOut->sqlResult[j].value;
		  printf
		    ("GJK 300.0.3. iFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n",
		     fullName, i, j, genQueryOut->attriCnt);
		  //return (0);

		  tResult += i * genQueryOut->sqlResult[j].len;
		  printf
		    ("GJK 300.0.4. iFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n",
		     fullName, i, j, genQueryOut->attriCnt);
		  //return (0);

		  /* skip final | if no units were defined */
		  if (j < 2 || strlen (tResult))
		    {
		      size = genQueryOut->sqlResult[j].len + 2;
		      // appendStrToBBuf(mybuf, size, "%s",tResult);
		      printf
			("GJK 300.1.2. iFindChkSumDateAvuMetadata, tResult=(%s), i=%d, j=%d\n",
			 tResult, i, j);
		    }

		  printf
		    ("GJK 300.0.5. iFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, iCountUserDefinedMetadata=%d\n",
		     fullName, i, j, *iCountUserDefinedMetadata);

		  //return 0;

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

		  printf
		    ("GJK 300.0.6. iFindChkSumDateAvuMetadata, fullName=(%s), i=%d, j=%d, genQueryOut->attriCnt=%d\n",
		     fullName, i, j, genQueryOut->attriCnt);
		  //return 0;
		}		// j=0
	      // appendStrToBBuf(mybuf, 2, "\n");
	      *iCountUserDefinedMetadata = *iCountUserDefinedMetadata + 1;
	    }			// i=0
	}
    }
  iErr = *iCountUserDefinedMetadata;
  printf
    ("GJK 333.3.3. iFindChkSumDateAvuMetadata, i=%d, j=%d, iCountUserDefinedMetadata=%d, iErr=%d\n",
     i, j, *iCountUserDefinedMetadata, iErr);
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

  //   sprintf(strOut, "#1\n#2\n\n#3 lines gjk\n");
  i = fillStrInMsParam (outParam1, strOut);	// MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c
  // fillBuffInParam


  printf
    ("GJK-P P.111.0.9. in msiGetDataObjChksumsTimeStampsFromAVU(), GJK msiGetDataObjChksumsTimeStampsFromAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU, iCountUserDefinedMetadata=%d\n",
     iCountUserDefinedMetadata);

  return (iErr);
}

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


  /* Make sure input is a collection */
  chrPtr1 = strrchr (ptrInpColl->collName, '/');
  printf
    ("GJK-P P.1.0.1. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n",
     chrPtr1, ptrInpColl->collName);
  if (chrPtr1 != NULL && *chrPtr1 == '/'
      && ((chrPtr1 - ptrInpColl->collName) <=
	  (strlen (ptrInpColl->collName - 1))))
    *chrPtr1 = 0;		// replace '/' in /myzone/foo/'
  printf
    ("GJK-P P.1.0.2. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n",
     chrPtr1, ptrInpColl->collName);

  if ((iI = isData (rei->rsComm, ptrInpColl->collName, NULL)) >= 0)
    {
      rodsLog (LOG_NOTICE,
	       "GJK intGetDataObjChksumsTimeStampsFromAVU: input (%s) is data.",
	       ptrInpColl->collName);
      //iI=iAddChkSumDateAvuMetadata (rei->rsComm, ptrInpColl->collName, v3, iI);
    }
  else
    {
      printf
	("GJK-P P.21.0.2. in intGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s), iI=%d\n",
	 chrPtr1, ptrInpColl->collName, iI);
      if ((iI = isColl (rei->rsComm, ptrInpColl->collName, NULL)) < 0)
	{
	  rodsLog (LOG_ERROR,
		   "iGetDataObjChksumsTimeStampsFromAVU: input object=(%s) is not data or collection. Exiting!",
		   ptrInpColl->collName);
	  //return (rei->status);
	}
      else
	{
	  rodsLog (LOG_ERROR,
		   "GJK intGetDataObjChksumsTimeStampsFromAVU: input (%s) is a collection.",
		   ptrInpColl->collName);
	  return (rei->status);
	}
    }

  // printf ("GJK-P P.1.0.3. intGetDataObjChksumsTimeStampsFromAVU : input (%s)", ptrInpColl->collName);

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
      printCount += iFindChkSumDateAvuMetadata (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	// proc??
    }
  else
    {
      printCount += iFindChkSumDateAvuMetadata (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	// proc??
    }

  while (iErr == 0 && genQueryOut->continueInx > 0)
    {
      genQueryInp.continueInx = genQueryOut->continueInx;
      iErr = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);
      printCount += iFindChkSumDateAvuMetadata (iErr, genQueryOut, strAbsPath, aAVUarray, iTotalAVUs);	// proc ??
    }

  printf
    ("GJK-P P.14.1.15. in intGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d\n",
     strAbsPath, ptrInpColl->collName, iErr);

  return (iErr);
}
