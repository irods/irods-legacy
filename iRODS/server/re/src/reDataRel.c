/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include <stdarg.h>
#include "apiHeaderAll.h"
#include "objStat.h"
#include "reDataObjOpr.h"
#include "reGlobalsExtern.h"
#include "reDataRel.h"


/*
 * Get all Dates of Performed Checksum Operations from metadata AVUs for a given iRods data object.
 * 
 */

int
msiGetDataObjChksumsTimeStampsFromAVU(msParam_t *inpParam1, msParam_t *outParam1, ruleExecInfo_t *rei)
{
 rsComm_t *rsComm; 
 collInp_t collInpCache, *ptrInpColl;
 int iErr=0, iI=0, i=0;
 UserDefinedMetadata_t aAVUarray[1024];
 int countUserDefinedMetadata;
 char strOut[MAX_NAME_LEN*MAX_NAME_LEN], strTmp[1024];

 /* For testing mode when used with irule --test */
 RE_TEST_MACRO ("RE_TEST_MACRO, begin of msiGetDataObjChksumsTimeStampsFromAVU") ;
 
   printf ("GJK-P P.1.0.0. in msiGetDataObjChksumsTimeStampsFromAVU(), GJK msiGetDataObjChksumsTimeStampsFromAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU\n");  
   
   rsComm = rei->rsComm;

   /* parse inpParam11 */
   rei->status = parseMspForCollInp (inpParam1, &collInpCache, &ptrInpColl, 0);
   
   if (rei->status < 0) {
     rodsLog (LOG_ERROR, "msiGetDataObjChksumsTimeStampsFromAVU(),  input inpParam1 error. status = %d", rei->status);
     return (rei->status);
   }

   iErr = iGetDataObjChksumsTimeStampsFromAVU (ptrInpColl, aAVUarray, &countUserDefinedMetadata, strOut, rei);

   for (iI = 0; iI < countUserDefinedMetadata; iI++) {
     
     snprintf (strTmp, MAX_NAME_LEN,"|%s|%s|%s|\n",
	       aAVUarray[iI].attribute, aAVUarray[iI].value, aAVUarray[iI].units);
     strncat(strOut, strTmp, MAX_NAME_LEN);
   }
   
   //   sprintf(strOut, "#1\n#2\n\n#3 lines gjk\n");
   i = fillStrInMsParam (outParam1, strOut);  // MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c
   // fillBuffInParam
   
   
   return (iErr);
}

/* ****************************************************************************************** */
int iGetDataObjChksumsTimeStampsFromAVU (collInp_t* ptrInpColl, UserDefinedMetadata_t* aAVUarray, int* countUserDefinedMetadata, char* strOut, ruleExecInfo_t *rei)
{
  char *chrPtr1 = NULL, strAbsPath[MAX_NAME_LEN], v1[1024], v2[1024], v3[1024], strDirName[MAX_NAME_LEN], strFileName[MAX_NAME_LEN], *condVal[10], attrName[256]="MD5checkSumDataStamp";;
  rsComm_t *rsComm;
  genQueryInp_t genQueryInp;
  int i1a[10], i1b[10], i2a[10], iI=0, iErr=0, printCount=0;
  bytesBuf_t *byteBuf;
  genQueryOut_t *genQueryOut;


  /* Make sure input is a collection */
   chrPtr1 = strrchr(ptrInpColl->collName, '/');
   printf ("GJK-P P.1.0.1. in iGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n", chrPtr1, ptrInpColl->collName);  
   if (chrPtr1 != NULL && *chrPtr1 == '/' && ((chrPtr1 - ptrInpColl->collName) <= (strlen(ptrInpColl->collName - 1)))) 
     *chrPtr1 = 0; // replace '/' in /myzone/foo/'
   printf ("GJK-P P.1.0.2. in iGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s)\n", chrPtr1, ptrInpColl->collName);  

   if ((iI = isData(rei->rsComm, ptrInpColl->collName, NULL)) >= 0) {
     rodsLog (LOG_NOTICE, "GJK iGetDataObjChksumsTimeStampsFromAVU: input (%s) is data.", ptrInpColl->collName); 
   }
   else 
     {
       printf ("GJK-P P.21.0.2. in iGetDataObjChksumsTimeStampsFromAVU(), chrPtr1=(%s), ptrInpColl->collName=(%s), iI=%d\n", chrPtr1, ptrInpColl->collName, iI);  
       if ((iI = isColl(rei->rsComm, ptrInpColl->collName, NULL)) < 0) {
	 rodsLog (LOG_ERROR, 
		  "iGetDataObjChksumsTimeStampsFromAVU: input object=(%s) is not data or collection. Exiting!", ptrInpColl->collName);
	 //return (rei->status);
       }
       else {
	 rodsLog (LOG_ERROR, 
		  "GJK iGetDataObjChksumsTimeStampsFromAVU: input (%s) is a collection.", ptrInpColl->collName);
	 return (rei->status);
       }
     }

   // printf ("GJK-P P.1.0.3. iGetDataObjChksumsTimeStampsFromAVU : input (%s)", ptrInpColl->collName);
   
   if (rei->rsComm == NULL) {
     rodsLog (LOG_ERROR,
	      "GJKgetDataObjPSmeta: input rsComm is NULL");
     return (SYS_INTERNAL_NULL_INPUT_ERR);
   }
   
   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   i1a[0]=COL_META_DATA_ATTR_NAME;
   i1b[0]=0; /* currently unused */
   i1a[1]=COL_META_DATA_ATTR_VALUE;
   i1b[1]=0; /* currently unused */
   i1a[2]=COL_META_DATA_ATTR_UNITS;
   i1b[2]=0; /* currently unused */
   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = 3;

   strncpy(strAbsPath, ptrInpColl->collName, MAX_NAME_LEN);
   printf ("GJK-P P.14.0.11. in iGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s)\n", 
	   strAbsPath, ptrInpColl->collName);  

   iErr = splitPathByKey(strAbsPath, strDirName, strFileName, '/');

   printf ("GJK-P P.14.0.12. in iGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), strDirName=(%s), strFileName=(%s), iErr=%d\n", 
	   strAbsPath, ptrInpColl->collName, strDirName, strFileName, iErr);  

   i2a[0]=COL_COLL_NAME;
   sprintf(v1,"='%s'",strDirName);
   condVal[0]=v1;

   i2a[1]=COL_DATA_NAME;
   sprintf(v2,"='%s'",strFileName);
   condVal[1]=v2;

   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=2;

   if (attrName != NULL && *attrName!='\0') {
     i2a[2]=COL_META_DATA_ATTR_NAME;
     sprintf(v3,"= '%s'",attrName);
     condVal[2]=v3;
     genQueryInp.sqlCondInp.len++;
   }
   
   genQueryInp.maxRows=100;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;

   printf ("GJK-P P.14.0.13. in iGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), v3=(%s), iErr=%d\n", strAbsPath, ptrInpColl->collName, v3, iErr);  

   /* Actual query happens here */
   iErr = rsGenQuery(rei->rsComm, &genQueryInp, &genQueryOut);

   printf ("GJK-P P.14.0.14. in iGetDataObjChksumsTimeStampsFromAVU(), strAbsPath=(%s), ptrInpColl->collName=(%s), iErr=%d\n", strAbsPath, ptrInpColl->collName, iErr);  

   if (iErr == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      iErr = rsGenQuery(rei->rsComm, &genQueryInp, &genQueryOut);
      if (iErr == 0) {
	 printf("GJK GJKgetDataObjPSmeta(),  iErr=%d, None\n", iErr); 
	 return(0);
      }
      if (iErr == CAT_NO_ROWS_FOUND) {

	rodsLog (LOG_NOTICE, "GJKgetDataObjPSmeta: DataObject %s not found. iErr = %d", strAbsPath, iErr);
	countUserDefinedMetadata = 0;
	return (0);
      }
       printCount+=extractPSQueryResults(rei->rsComm, iErr, genQueryOut, byteBuf, strAbsPath); // proc??
   }
   else {
     // printCount+=extractPSQueryResults(rei->rsComm, iErr, genQueryOut, byteBuf, strAbsPath); // proc??
   }

   while (iErr==0 && genQueryOut->continueInx > 0) {
     genQueryInp.continueInx=genQueryOut->continueInx;
     iErr = rsGenQuery(rei->rsComm, &genQueryInp, &genQueryOut);
     printCount+= extractPSQueryResults(rei->rsComm, iErr, genQueryOut, byteBuf, strAbsPath); // proc ??
   }
   
   return (iErr);
}

#define GJK44
#ifdef GJK44
/*
 * Gets pipe separated metadata AVUs fgoor a data object.
 * 
 */
int
m2siGetDataObjChksumsTimeStampsFromAVU(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int i1a[10];
   int i1b[10];
   int i2a[10];
   char *condVal[10];
   char v1[100*MAX_NAME_LEN];
   char v2[100*MAX_NAME_LEN], v3[100*MAX_NAME_LEN];
   char fullName[MAX_NAME_LEN];
   char myDirName[MAX_NAME_LEN];
   char myFileName[MAX_NAME_LEN];
   int printCount=0;
   int status, wild=1;
   char attrName[256]="MD5checkSumDataStamp";

   rsComm_t *rsComm;
   collInp_t collInpCache, *outCollInp;
   char objPath[MAX_NAME_LEN], strOut[MAX_NAME_LEN*MAX_NAME_LEN];
   int i=0, iGjk22=0, iGjk=0;
   char *iGjk2 = NULL, Gjk3chksumStr = NULL;	
   bytesBuf_t *mybuf;

 struct UserDefinedMetadata
{
  char attribute[NAME_MAX+1];
  char value[NAME_MAX+1];
  char units[NAME_MAX+1];
};

 static struct UserDefinedMetadata aAVUarray[1024];
 static int countUserDefinedMetadata;

   /* For testing mode when used with irule --test */
   RE_TEST_MACRO ("    GJK M.1.0.0 Calling msiGetDataObjChksumsTimeStampsFromAVU") ;
   
   printf ("GJK-P P.1.0.0. in msiGetDataObjChksumsTimeStampsFromAVU(), GJK msiGetDataObjChksumsTimeStampsFromAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU\n");  
   
   // does NOT work !!! rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status, "GJK msiGetDataObjChksumsTimeStampsFromAVU: GJK Calling msiGetDataObjChksumsTimeStampsFromAVU");
   
   rsComm = rei->rsComm;
   
   /* parse inpParam1 */
   rei->status = parseMspForCollInp (inpParam, &collInpCache, &outCollInp, 0);
   

   if (rei->status < 0) {
     rodsLog (LOG_ERROR, &rsComm->rError, rei->status,
	      "GJK msiGetDataObjChksumsTimeStampsFromAVU: input inpParam1 error. status = %d", rei->status);
     return (rei->status);
   }
   
   /* Make sure input is a collection */
   iGjk2 = strrchr(outCollInp->collName, '/');
   printf ("GJK-P P.1.0.1. in msiGetDataObjChksumsTimeStampsFromAVU(), iGjk2=(%s), outCollInp->collName=(%s)\n", iGjk2, outCollInp->collName);  
   if (iGjk2 != NULL && *iGjk2 == '/' && ((iGjk2 - outCollInp->collName) <= (strlen(outCollInp->collName - 1)))) 
     *iGjk2 = 0; // replace '/' in /myzone/foo/'
   printf ("GJK-P P.1.0.2. in msiGetDataObjChksumsTimeStampsFromAVU(), iGjk2=(%s), outCollInp->collName=(%s)\n", iGjk2, outCollInp->collName);  

   if ((iGjk = isColl(rei->rsComm, outCollInp->collName, NULL)) < 0) {
     rodsLog (LOG_ERROR, &rsComm->rError, rei->status,
	      "GJK msiGetDataObjChksumsTimeStampsFromAVU: input (%s) is not a collection.", outCollInp->collName);
     if ((iGjk = isData(rei->rsComm, outCollInp->collName, NULL)) >= 0) {
       rodsLog (LOG_ERROR, &rsComm->rError, rei->status,
		"GJK msiGetDataObjChksumsTimeStampsFromAVU: input (%s) is data.", outCollInp->collName);
     }
     else {
       rodsLog (LOG_ERROR, &rsComm->rError, rei->status,
		"GJK msiGetDataObjChksumsTimeStampsFromAVU: input (%s) is not data or collection. Exiting!", outCollInp->collName);
       return (rei->status);
     }
   }	
   else {
     rodsLog (LOG_ERROR, &rsComm->rError, rei->status,
	      "GJK msiGetDataObjChksumsTimeStampsFromAVU: input (%s) is a collection.", outCollInp->collName);
   }
   

   printf ("GJK-P P.1.0.3. in msiGetDataObjChksumsTimeStampsFromAVU: input (%s)", outCollInp->collName);
   
   if (rsComm == NULL) {
     rodsLog (LOG_ERROR,
	      "GJKgetDataObjPSmeta: input rsComm is NULL");
     return (SYS_INTERNAL_NULL_INPUT_ERR);
   }
   
   printf ("GJK-P P.1.0.4. in msiGetDataObjChksumsTimeStampsFromAVU: input (%s)", outCollInp->collName);

   
   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   i1a[0]=COL_META_DATA_ATTR_NAME;
   i1b[0]=0; /* currently unused */
   i1a[1]=COL_META_DATA_ATTR_VALUE;
   i1b[1]=0; /* currently unused */
   i1a[2]=COL_META_DATA_ATTR_UNITS;
   i1b[2]=0; /* currently unused */
   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = 3;

   /*
     if (testMode) {
     genQueryInp.selectInp.len = 4;
     }
   */

   /* Extract cwd name and object name */
   //strncpy(fullName, objPath, MAX_NAME_LEN);
   strncpy(fullName, outCollInp->collName, MAX_NAME_LEN);
   strncpy(objPath, outCollInp->collName, MAX_NAME_LEN);
   printf ("GJK-P P.14.0.11. in msiGetDataObjChksumsTimeStampsFromAVU(), fullName=(%s), outCollInp->collName=(%s), objPath=(%s), status=%d\n", 
	   fullName, outCollInp->collName, objPath, status);  

   status = splitPathByKey(fullName, myDirName, myFileName, '/');

   printf ("GJK-P P.14.0.12. in msiGetDataObjChksumsTimeStampsFromAVU(), fullName=(%s), outCollInp->collName=(%s), myDirName=(%s), myFileName=(%s), status=%d\n", 
	   fullName, outCollInp->collName, myDirName, myFileName, status);  

   i2a[0]=COL_COLL_NAME;
   sprintf(v1,"='%s'",myDirName);
   condVal[0]=v1;

   i2a[1]=COL_DATA_NAME;
   sprintf(v2,"='%s'",myFileName);
   condVal[1]=v2;


   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=2;

   wild = 0;
   if (attrName != NULL && *attrName!='\0') {
     i2a[2]=COL_META_DATA_ATTR_NAME;
     if (wild) {
       sprintf(v3,"like '%s'",attrName);
     }
      else {
	sprintf(v3,"= '%s'",attrName);
      }
     condVal[2]=v3;
     genQueryInp.sqlCondInp.len++;
   }
   
   genQueryInp.maxRows=100;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;

   printf ("GJK-P P.14.0.13. in msiGetDataObjChksumsTimeStampsFromAVU(), fullName=(%s), outCollInp->collName=(%s), v3=(%s), status=%d\n", fullName, outCollInp->collName, v3, status);  
   /* Actual query happens here */
   status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);

   printf ("GJK-P P.14.0.14. in msiGetDataObjChksumsTimeStampsFromAVU(), fullName=(%s), outCollInp->collName=(%s), status=%d\n", fullName, outCollInp->collName, status);  

   if (status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      if (status == 0) {
	 printf("GJK GJKgetDataObjPSmeta(),  status=%d, None\n", status); 
	 return(0);
      }
      if (status == CAT_NO_ROWS_FOUND) {

	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, status,
          "GJKgetDataObjPSmeta: DataObject %s not found. status = %d", fullName, status);
	return (status);
      }
      printCount+=extractPSQueryResults(rsComm, status, genQueryOut, mybuf, fullName);
   }
   else {
      printCount+=extractPSQueryResults(rsComm, status, genQueryOut, mybuf, fullName);
   }

   while (status==0 && genQueryOut->continueInx > 0) {
     genQueryInp.continueInx=genQueryOut->continueInx;
     status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
     printCount+= extractPSQueryResults(rsComm, status, genQueryOut, mybuf, fullName);
   }
   
   for (iGjk22 = 0; iGjk22 < countUserDefinedMetadata; iGjk22++) {
     
     snprintf (objPath, MAX_NAME_LEN,"|%s|%s|%s|\n",
	       aAVUarray[iGjk22].attribute, aAVUarray[iGjk22].value, aAVUarray[iGjk22].units);
     strncat(strOut, objPath,MAX_NAME_LEN);
   }
   
   //   sprintf(strOut, "#1\n#2\n\n#3 lines gjk\n");
   i = fillStrInMsParam (outParam, strOut);  // MsParam.c parse  addformatedtrsing to bytes WriteBytesBuff printMsParam.c
   // fillBuffInParam
   
   //return(i);
   
   return (status);
}

#endif

