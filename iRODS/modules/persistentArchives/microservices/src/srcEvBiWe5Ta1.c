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

#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>

int rsDataObjCopy (rsComm_t *rsComm, dataObjCopyInp_t *dataObjCopyInp, transStat_t **transStat);

/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW5T1 \author  G.J.Kremenek \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* For every sub-collection under/in a collection check that its AVU triplets contain exactly the given list of attributes. */

/* **************************************************************************** */

int
msiW5T1 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * coll3, msParam_t * outParam,
		       ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2;
  int i, iSame = 0, j;
  char strOut[MAX_NAME_LEN255 * MAX_NAME_LEN255], *strTimeDiff;
  int iChkFailed1 = 0, iAttrValMatch;
  int intLsArraySize1 = (MAX_NAME_LEN255);
  lsInfo_t lsArray1[MAX_NAME_LEN255 + 1];
  char strDataTypeInput1[MAX_NAME_LEN255 + 1],
    strDataTypeInput2[MAX_NAME_LEN255 + 1], strDataTypeInput3[MAX_NAME_LEN255 + 1],
    chp1[MAX_NAME_LEN255], chp2[MAX_NAME_LEN255], chp3[MAX_NAME_LEN255];

  RE_TEST_MACRO ("    Calling msiW5T1")
    printf
    ("GJK- begin 000W5T101.0.0-W5T1r,Fri Mar 14 18:29:18 PDT 2008, in msiW5T1(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W5T1.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W5T1.11.b.x OK\n");

  /*        rstrcpy(collInp1.collName, strDataTypeInput1, MAX_NAME_LEN255); */

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiW5T1: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  rstrcpy (strDataTypeInput1, "1", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput2, "2", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput3, "3", MAX_NAME_LEN255);

  i =
    iParseInputParams3 (coll1, coll2, coll3, outParam, strDataTypeInput1,
			strDataTypeInput2, strDataTypeInput3, chp1, chp2,
			chp3);
  if (0 != i)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T1(), iParseInputParams2() failed.\n",
		       strOut);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      rstrcpy (strDataTypeInput1, chp1, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput2, chp2, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput3, chp3, MAX_NAME_LEN255);

      printf
	("GJK 0007.W5T1.4e i=(%d), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
	 i, strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);
      printf ("GJK 0007.W5T1.4f chp1=(%s), chp2=(%s), chp3=(%s)\n", chp1,
	      chp2, chp3);

      /*
         rstrcpy(strDataTypeInput1, chp1, MAX_NAME_LEN255);
         rstrcpy(strDataTypeInput2, chp2, MAX_NAME_LEN255);
       */
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W5T1.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W5T1.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  if (1 == 2)
    {
      printf
	("GJK fake000W5T1021xA STOP, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
	 collInp1.collName, strDataTypeInput1);
      return (0);
    }
  else
    printf
      ("GJK fake000W5T1021xA OK, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
       collInp1.collName, strDataTypeInput1);


  /*
   rei->status = rsDataObjCopy (rsComm, myDataObjCopyInp, &transStat);
    if (transStat != NULL) {
        free (transStat);
    }

    if (myDataObjCopyInp == &dataObjCopyInp) {
        clearKeyVal (&myDataObjCopyInp->destDataObjInp.condInput);
    }
  */

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 0);	/* 0 == collection name is LIKE name */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T1(), Test failed . No file object '%s' found in iRods.iChkFailed1=(%d)\n",
		       strOut, strDataTypeInput1, iChkFailed1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      for (i = 0; i < iChkFailed1; i++)
	{
	  collInp_t ptrInpColl;
	  char strFullDataPath[MAX_NAME_LEN255];
	  UserDefinedMetadata_t aAVUarray[1024];
	  int iErr = (-1), iCountUserDefinedMetadata = (-1), iTotalAVUs =
	    (-1);

	  sprintf (strFullDataPath, "%s/%s", lsArray1[i].chCollection,
		   lsArray1[i].chObjName);
	  strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN255);
	  iErr =
	    intGetDataObjChksumsTimeStampsFromAVU9 (&ptrInpColl, aAVUarray,
						    &iCountUserDefinedMetadata,
						    strOut, rei);

	  iTotalAVUs = iCountUserDefinedMetadata;
	  iAttrValMatch = 0;
	  printf
	    ("GJK-P msiW5T1(), 0008.994.7.0.G i=(%d), iChkFailed1=(%d), iTotalAVUs=(%d), iErr=(%d), strFullDataPath=(%s)\n",
	     i, iChkFailed1, iTotalAVUs, iErr, strFullDataPath);
	  for (j = 0; j < iTotalAVUs; j++)
	    {

	      printf
		("GJK fake9931N msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iTotalAVUs=(%d), aAVUarray[j].attribute=(%s), aAVUarray[j].value=(%s), aAVUarray[j].units=(%s), iSame=(%d) \n",
		 lsArray1[i].chCollection, lsArray1[i].chObjName,
		 lsArray1[i].isData, lsArray1[i].chDataSize,
		 (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
		 lsArray1[i].chDataAccessName, strDataTypeInput1,
		 lsArray1[i].chRescName, i, j, iTotalAVUs,
		 aAVUarray[j].attribute, aAVUarray[j].value,
		 aAVUarray[j].units, iSame);

	      if (strcmp (aAVUarray[j].attribute, "myattr1") == 0)
		{
		  if (strlen (aAVUarray[j].value) > 0)
		    /* I have give AVY attribute and it has non empty value */
		    iAttrValMatch = 1;
		}
	      /*
	         lTmp = strtol (aAVUarray[j].value, (char **) NULL, 10);
	         if (lMax < lTmp)
	         lMax = lTmp;
	       */
	    }

	  printf
	    ("GJK fake9922a msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iSame=(%d)\n",
	     lsArray1[i].chCollection, lsArray1[i].chObjName,
	     lsArray1[i].isData, lsArray1[i].chDataSize,
	     (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
	     lsArray1[i].chDataAccessName, strDataTypeInput1,
	     lsArray1[i].chRescName, i, j, iSame);

	  // if (iAttrValMatch != 1) break;
	}			/* for i= */


    }				/* else */

  if (iAttrValMatch == 1)
    (void) snprintf (strOut, 255,
		     "%s msiW5T1(), Test OK. Collection '%s' has given set of attributes with non-empty values.\n",
		     strOut, strDataTypeInput1);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T1(), Test failed. Collection '%s' does not have given set of attributes with non-empty values..\n",
		       strOut, strDataTypeInput1);
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
    ("GJK end of msiW5T1(), Fri Mar 21 11:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */

/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW5T2 \author  G.J.Kremenek \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* 
2. For every sub-collection under/in a collection check that its AVU triplets for a given attribute has only one of a given set of values or range of values.
*/

/* **************************************************************************** */

int
msiW5T2 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * coll3, msParam_t * outParam,
		       ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2;
  int i, iSame = 0, j;
  char strOut[MAX_NAME_LEN255 * MAX_NAME_LEN255], *strTimeDiff;
  int iChkFailed1 = 0, iAttrValMatch;
  int intLsArraySize1 = (MAX_NAME_LEN255);
  lsInfo_t lsArray1[MAX_NAME_LEN255 + 1];
  char strDataTypeInput1[MAX_NAME_LEN255 + 1],
    strDataTypeInput2[MAX_NAME_LEN255 + 1], strDataTypeInput3[MAX_NAME_LEN255 + 1],
    chp1[MAX_NAME_LEN255], chp2[MAX_NAME_LEN255], chp3[MAX_NAME_LEN255];

  RE_TEST_MACRO ("    Calling msiW5T2")
    printf
    ("GJK- begin 000W5T201.0.0-W5T2r,Fri Mar 14 18:29:18 PDT 2008, in msiW5T2(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W5T2.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W5T2.11.b.x OK\n");

  /*        rstrcpy(collInp1.collName, strDataTypeInput1, MAX_NAME_LEN255); */

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiW5T2: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  rstrcpy (strDataTypeInput1, "1", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput2, "2", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput3, "3", MAX_NAME_LEN255);

  i =
    iParseInputParams3 (coll1, coll2, coll3, outParam, strDataTypeInput1,
			strDataTypeInput2, strDataTypeInput3, chp1, chp2,
			chp3);
  if (0 != i)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T2(), iParseInputParams2() failed.\n",
		       strOut);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      rstrcpy (strDataTypeInput1, chp1, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput2, chp2, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput3, chp3, MAX_NAME_LEN255);

      printf
	("GJK 0007.W5T2.4e i=(%d), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
	 i, strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);
      printf ("GJK 0007.W5T2.4f chp1=(%s), chp2=(%s), chp3=(%s)\n", chp1,
	      chp2, chp3);

      /*
         rstrcpy(strDataTypeInput1, chp1, MAX_NAME_LEN255);
         rstrcpy(strDataTypeInput2, chp2, MAX_NAME_LEN255);
       */
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W5T2.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W5T2.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  if (1 == 2)
    {
      printf
	("GJK fake000W5T2021xA STOP, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
	 collInp1.collName, strDataTypeInput1);
      return (0);
    }
  else
    printf
      ("GJK fake000W5T2021xA OK, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
       collInp1.collName, strDataTypeInput1);


  /*
   rei->status = rsDataObjCopy (rsComm, myDataObjCopyInp, &transStat);
    if (transStat != NULL) {
        free (transStat);
    }

    if (myDataObjCopyInp == &dataObjCopyInp) {
        clearKeyVal (&myDataObjCopyInp->destDataObjInp.condInput);
    }
  */

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 0);	/* 0 == collection name is LIKE name */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T2(), Test failed . No file object '%s' found in iRods.iChkFailed1=(%d)\n",
		       strOut, strDataTypeInput1, iChkFailed1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      for (i = 0; i < iChkFailed1; i++)
	{
	  collInp_t ptrInpColl;
	  char strFullDataPath[MAX_NAME_LEN255];
	  UserDefinedMetadata_t aAVUarray[1024];
	  int iErr = (-1), iCountUserDefinedMetadata = (-1), iTotalAVUs =
	    (-1);

	  sprintf (strFullDataPath, "%s/%s", lsArray1[i].chCollection,
		   lsArray1[i].chObjName);
	  strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN255);
	  iErr =
	    intGetDataObjChksumsTimeStampsFromAVU9 (&ptrInpColl, aAVUarray,
						    &iCountUserDefinedMetadata,
						    strOut, rei);

	  iTotalAVUs = iCountUserDefinedMetadata;
	  iAttrValMatch = 0;
	  printf
	    ("GJK-P msiW5T2(), 0008.994.7.0.G i=(%d), iChkFailed1=(%d), iTotalAVUs=(%d), iErr=(%d), strFullDataPath=(%s)\n",
	     i, iChkFailed1, iTotalAVUs, iErr, strFullDataPath);
	  for (j = 0; j < iTotalAVUs; j++)
	    {

	      printf
		("GJK fake9931N msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iTotalAVUs=(%d), aAVUarray[j].attribute=(%s), aAVUarray[j].value=(%s), aAVUarray[j].units=(%s), iSame=(%d) \n",
		 lsArray1[i].chCollection, lsArray1[i].chObjName,
		 lsArray1[i].isData, lsArray1[i].chDataSize,
		 (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
		 lsArray1[i].chDataAccessName, strDataTypeInput1,
		 lsArray1[i].chRescName, i, j, iTotalAVUs,
		 aAVUarray[j].attribute, aAVUarray[j].value,
		 aAVUarray[j].units, iSame);

	      if (strcmp (aAVUarray[j].attribute, "myattr1") == 0)
		{
		  if (strlen (aAVUarray[j].value) > 0)
		    /* I have give AVY attribute and it has non empty value */
		    iAttrValMatch = 1;
		}
	      /*
	         lTmp = strtol (aAVUarray[j].value, (char **) NULL, 10);
	         if (lMax < lTmp)
	         lMax = lTmp;
	       */
	    }

	  printf
	    ("GJK fake9922a msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iSame=(%d)\n",
	     lsArray1[i].chCollection, lsArray1[i].chObjName,
	     lsArray1[i].isData, lsArray1[i].chDataSize,
	     (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
	     lsArray1[i].chDataAccessName, strDataTypeInput1,
	     lsArray1[i].chRescName, i, j, iSame);

	  // if (iAttrValMatch != 1) break;
	}			/* for i= */


    }				/* else */

  if (iAttrValMatch == 1)
    (void) snprintf (strOut, 255,
		     "%s msiW5T2(), Test OK. Collection '%s' has given set of attributes with non-empty values.\n",
		     strOut, strDataTypeInput1);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T2(), Test failed. Collection '%s' does not have given set of attributes with non-empty values..\n",
		       strOut, strDataTypeInput1);
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
    ("GJK end of msiW5T2(), Fri Mar 21 11:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */

/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW5T3 \author  G.J.Kremenek \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/*
3. For every sub-collection under/in a collection check that its AVU triplets do not have duplicates for a given attribute or lits of attributes or every attributes.
*/

/* **************************************************************************** */

int
msiW5T3 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * coll3, msParam_t * outParam,
		       ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2;
  int i, iSame = 0, j;
  char strOut[MAX_NAME_LEN255 * MAX_NAME_LEN255], *strTimeDiff;
  int iChkFailed1 = 0, iAttrValMatch;
  int intLsArraySize1 = (MAX_NAME_LEN255);
  lsInfo_t lsArray1[MAX_NAME_LEN255 + 1];
  char strDataTypeInput1[MAX_NAME_LEN255 + 1],
    strDataTypeInput2[MAX_NAME_LEN255 + 1], strDataTypeInput3[MAX_NAME_LEN255 + 1],
    chp1[MAX_NAME_LEN255], chp2[MAX_NAME_LEN255], chp3[MAX_NAME_LEN255];

  RE_TEST_MACRO ("    Calling msiW5T3")
    printf
    ("GJK- begin 000W5T301.0.0-W5T3r,Fri Mar 14 18:29:18 PDT 2008, in msiW5T3(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W5T3.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W5T3.11.b.x OK\n");

  /*        rstrcpy(collInp1.collName, strDataTypeInput1, MAX_NAME_LEN255); */

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiW5T3: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  rstrcpy (strDataTypeInput1, "1", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput2, "2", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput3, "3", MAX_NAME_LEN255);

  i =
    iParseInputParams3 (coll1, coll2, coll3, outParam, strDataTypeInput1,
			strDataTypeInput2, strDataTypeInput3, chp1, chp2,
			chp3);
  if (0 != i)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T3(), iParseInputParams2() failed.\n",
		       strOut);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      rstrcpy (strDataTypeInput1, chp1, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput2, chp2, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput3, chp3, MAX_NAME_LEN255);

      printf
	("GJK 0007.W5T3.4e i=(%d), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
	 i, strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);
      printf ("GJK 0007.W5T3.4f chp1=(%s), chp2=(%s), chp3=(%s)\n", chp1,
	      chp2, chp3);

      /*
         rstrcpy(strDataTypeInput1, chp1, MAX_NAME_LEN255);
         rstrcpy(strDataTypeInput2, chp2, MAX_NAME_LEN255);
       */
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W5T3.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W5T3.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  if (1 == 2)
    {
      printf
	("GJK fake000W5T3021xA STOP, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
	 collInp1.collName, strDataTypeInput1);
      return (0);
    }
  else
    printf
      ("GJK fake000W5T3021xA OK, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
       collInp1.collName, strDataTypeInput1);


  /*
   rei->status = rsDataObjCopy (rsComm, myDataObjCopyInp, &transStat);
    if (transStat != NULL) {
        free (transStat);
    }

    if (myDataObjCopyInp == &dataObjCopyInp) {
        clearKeyVal (&myDataObjCopyInp->destDataObjInp.condInput);
    }
  */

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 0);	/* 0 == collection name is LIKE name */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T3(), Test failed . No file object '%s' found in iRods.iChkFailed1=(%d)\n",
		       strOut, strDataTypeInput1, iChkFailed1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      for (i = 0; i < iChkFailed1; i++)
	{
	  collInp_t ptrInpColl;
	  char strFullDataPath[MAX_NAME_LEN255];
	  UserDefinedMetadata_t aAVUarray[1024];
	  int iErr = (-1), iCountUserDefinedMetadata = (-1), iTotalAVUs =
	    (-1);

	  sprintf (strFullDataPath, "%s/%s", lsArray1[i].chCollection,
		   lsArray1[i].chObjName);
	  strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN255);
	  iErr =
	    intGetDataObjChksumsTimeStampsFromAVU9 (&ptrInpColl, aAVUarray,
						    &iCountUserDefinedMetadata,
						    strOut, rei);

	  iTotalAVUs = iCountUserDefinedMetadata;
	  iAttrValMatch = 0;
	  printf
	    ("GJK-P msiW5T3(), 0008.994.7.0.G i=(%d), iChkFailed1=(%d), iTotalAVUs=(%d), iErr=(%d), strFullDataPath=(%s)\n",
	     i, iChkFailed1, iTotalAVUs, iErr, strFullDataPath);
	  for (j = 0; j < iTotalAVUs; j++)
	    {

	      printf
		("GJK fake9931N msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iTotalAVUs=(%d), aAVUarray[j].attribute=(%s), aAVUarray[j].value=(%s), aAVUarray[j].units=(%s), iSame=(%d) \n",
		 lsArray1[i].chCollection, lsArray1[i].chObjName,
		 lsArray1[i].isData, lsArray1[i].chDataSize,
		 (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
		 lsArray1[i].chDataAccessName, strDataTypeInput1,
		 lsArray1[i].chRescName, i, j, iTotalAVUs,
		 aAVUarray[j].attribute, aAVUarray[j].value,
		 aAVUarray[j].units, iSame);

	      if (strcmp (aAVUarray[j].attribute, "myattr1") == 0)
		{
		  if (strlen (aAVUarray[j].value) > 0)
		    /* I have give AVY attribute and it has non empty value */
		    iAttrValMatch = 1;
		}
	      /*
	         lTmp = strtol (aAVUarray[j].value, (char **) NULL, 10);
	         if (lMax < lTmp)
	         lMax = lTmp;
	       */
	    }

	  printf
	    ("GJK fake9922a msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iSame=(%d)\n",
	     lsArray1[i].chCollection, lsArray1[i].chObjName,
	     lsArray1[i].isData, lsArray1[i].chDataSize,
	     (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
	     lsArray1[i].chDataAccessName, strDataTypeInput1,
	     lsArray1[i].chRescName, i, j, iSame);

	  // if (iAttrValMatch != 1) break;
	}			/* for i= */


    }				/* else */

  if (iAttrValMatch == 1)
    (void) snprintf (strOut, 255,
		     "%s msiW5T3(), Test OK. Collection '%s' has given set of attributes with non-empty values.\n",
		     strOut, strDataTypeInput1);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T3(), Test failed. Collection '%s' does not have given set of attributes with non-empty values..\n",
		       strOut, strDataTypeInput1);
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
    ("GJK end of msiW5T3(), Fri Mar 21 11:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */

/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW5T4 \author  G.J.Kremenek \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* 
4. Generate authentic copy froma master (validate checksum before copy and after copy).
*/


int
msiW5T4 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * coll3, msParam_t * outParam,
		       ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2;
  int i, iSame = 0, j;
  char strOut[MAX_NAME_LEN255 * MAX_NAME_LEN255], *strTimeDiff;
  int iChkFailed1 = 0, iAttrValMatch;
  int intLsArraySize1 = (MAX_NAME_LEN255);
  lsInfo_t lsArray1[MAX_NAME_LEN255 + 1];
  char strDataTypeInput1[MAX_NAME_LEN255 + 1],
    strDataTypeInput2[MAX_NAME_LEN255 + 1], strDataTypeInput3[MAX_NAME_LEN255 + 1],
    chp1[MAX_NAME_LEN255], chp2[MAX_NAME_LEN255], chp3[MAX_NAME_LEN255];

  RE_TEST_MACRO ("    Calling msiW5T4")
    printf
    ("GJK- begin 000W5T401.0.0-W5T4r,Fri Mar 14 18:29:18 PDT 2008, in msiW5T4(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W5T4.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W5T4.11.b.x OK\n");

  /*        rstrcpy(collInp1.collName, strDataTypeInput1, MAX_NAME_LEN255); */

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiW5T4: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  { /* 
       int msiDataObjCopy (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
    */
    /*
      int msiDataObjChksum (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
      VERIFY_CHKSUM_KW
    */

    msParam_t * inpParam1, * inpParam2, * inpParam3;
    rsComm_t *rsComm;
    dataObjCopyInp_t dataObjCopyInp, *myDataObjCopyInp;
    dataObjInp_t *myDataObjInp;
    transStat_t *transStat = NULL;

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


























  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  rstrcpy (strDataTypeInput1, "1", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput2, "2", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput3, "3", MAX_NAME_LEN255);

  i =
    iParseInputParams3 (coll1, coll2, coll3, outParam, strDataTypeInput1,
			strDataTypeInput2, strDataTypeInput3, chp1, chp2,
			chp3);
  if (0 != i)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T4(), iParseInputParams2() failed.\n",
		       strOut);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      rstrcpy (strDataTypeInput1, chp1, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput2, chp2, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput3, chp3, MAX_NAME_LEN255);

      printf
	("GJK 0007.W5T4.4e i=(%d), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
	 i, strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);
      printf ("GJK 0007.W5T4.4f chp1=(%s), chp2=(%s), chp3=(%s)\n", chp1,
	      chp2, chp3);

      /*
         rstrcpy(strDataTypeInput1, chp1, MAX_NAME_LEN255);
         rstrcpy(strDataTypeInput2, chp2, MAX_NAME_LEN255);
       */
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W5T4.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W5T4.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  if (1 == 2)
    {
      printf
	("GJK fake000W5T4021xA STOP, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
	 collInp1.collName, strDataTypeInput1);
      return (0);
    }
  else
    printf
      ("GJK fake000W5T4021xA OK, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
       collInp1.collName, strDataTypeInput1);

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 0);	/* 0 == collection name is LIKE name */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T4(), Test failed . No file object '%s' found in iRods.iChkFailed1=(%d)\n",
		       strOut, strDataTypeInput1, iChkFailed1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      for (i = 0; i < iChkFailed1; i++)
	{
	  collInp_t ptrInpColl;
	  char strFullDataPath[MAX_NAME_LEN255];
	  UserDefinedMetadata_t aAVUarray[1024];
	  int iErr = (-1), iCountUserDefinedMetadata = (-1), iTotalAVUs =
	    (-1);

	  sprintf (strFullDataPath, "%s/%s", lsArray1[i].chCollection,
		   lsArray1[i].chObjName);
	  strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN255);
	  iErr =
	    intGetDataObjChksumsTimeStampsFromAVU9 (&ptrInpColl, aAVUarray,
						    &iCountUserDefinedMetadata,
						    strOut, rei);

	  iTotalAVUs = iCountUserDefinedMetadata;
	  iAttrValMatch = 0;
	  printf
	    ("GJK-P msiW5T4(), 0008.994.7.0.G i=(%d), iChkFailed1=(%d), iTotalAVUs=(%d), iErr=(%d), strFullDataPath=(%s)\n",
	     i, iChkFailed1, iTotalAVUs, iErr, strFullDataPath);
	  for (j = 0; j < iTotalAVUs; j++)
	    {

	      printf
		("GJK fake9931N msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iTotalAVUs=(%d), aAVUarray[j].attribute=(%s), aAVUarray[j].value=(%s), aAVUarray[j].units=(%s), iSame=(%d) \n",
		 lsArray1[i].chCollection, lsArray1[i].chObjName,
		 lsArray1[i].isData, lsArray1[i].chDataSize,
		 (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
		 lsArray1[i].chDataAccessName, strDataTypeInput1,
		 lsArray1[i].chRescName, i, j, iTotalAVUs,
		 aAVUarray[j].attribute, aAVUarray[j].value,
		 aAVUarray[j].units, iSame);

	      if (strcmp (aAVUarray[j].attribute, "myattr1") == 0)
		{
		  if (strlen (aAVUarray[j].value) > 0)
		    /* I have give AVY attribute and it has non empty value */
		    iAttrValMatch = 1;
		}
	      /*
	         lTmp = strtol (aAVUarray[j].value, (char **) NULL, 10);
	         if (lMax < lTmp)
	         lMax = lTmp;
	       */
	    }

	  printf
	    ("GJK fake9922a msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iSame=(%d)\n",
	     lsArray1[i].chCollection, lsArray1[i].chObjName,
	     lsArray1[i].isData, lsArray1[i].chDataSize,
	     (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
	     lsArray1[i].chDataAccessName, strDataTypeInput1,
	     lsArray1[i].chRescName, i, j, iSame);

	  // if (iAttrValMatch != 1) break;
	}			/* for i= */


    }				/* else */

  if (iAttrValMatch == 1)
    (void) snprintf (strOut, 255,
		     "%s msiW5T4(), Test OK. Collection '%s' has given set of attributes with non-empty values.\n",
		     strOut, strDataTypeInput1);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T4(), Test failed. Collection '%s' does not have given set of attributes with non-empty values..\n",
		       strOut, strDataTypeInput1);
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
    ("GJK end of msiW5T4(), Fri Mar 21 11:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */

/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW5T5 \author  G.J.Kremenek \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* For every sub-collection under/in a collection check that its AVU triplets contain exactly the given list of attributes. */

/* **************************************************************************** */

int
msiW5T5 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * coll3, msParam_t * outParam,
		       ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2;
  int i, iSame = 0, j;
  char strOut[MAX_NAME_LEN255 * MAX_NAME_LEN255], *strTimeDiff;
  int iChkFailed1 = 0, iAttrValMatch;
  int intLsArraySize1 = (MAX_NAME_LEN255);
  lsInfo_t lsArray1[MAX_NAME_LEN255 + 1];
  char strDataTypeInput1[MAX_NAME_LEN255 + 1],
    strDataTypeInput2[MAX_NAME_LEN255 + 1], strDataTypeInput3[MAX_NAME_LEN255 + 1],
    chp1[MAX_NAME_LEN255], chp2[MAX_NAME_LEN255], chp3[MAX_NAME_LEN255];

  RE_TEST_MACRO ("    Calling msiW5T5")
    printf
    ("GJK- begin 000W5T501.0.0-W5T5r,Fri Mar 14 18:29:18 PDT 2008, in msiW5T5(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W5T5.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W5T5.11.b.x OK\n");

  /*        rstrcpy(collInp1.collName, strDataTypeInput1, MAX_NAME_LEN255); */

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiW5T5: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  rstrcpy (strDataTypeInput1, "1", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput2, "2", MAX_NAME_LEN255);
  rstrcpy (strDataTypeInput3, "3", MAX_NAME_LEN255);

  i =
    iParseInputParams3 (coll1, coll2, coll3, outParam, strDataTypeInput1,
			strDataTypeInput2, strDataTypeInput3, chp1, chp2,
			chp3);
  if (0 != i)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T5(), iParseInputParams2() failed.\n",
		       strOut);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      rstrcpy (strDataTypeInput1, chp1, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput2, chp2, MAX_NAME_LEN255);
      rstrcpy (strDataTypeInput3, chp3, MAX_NAME_LEN255);

      printf
	("GJK 0007.W5T5.4e i=(%d), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
	 i, strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);
      printf ("GJK 0007.W5T5.4f chp1=(%s), chp2=(%s), chp3=(%s)\n", chp1,
	      chp2, chp3);

      /*
         rstrcpy(strDataTypeInput1, chp1, MAX_NAME_LEN255);
         rstrcpy(strDataTypeInput2, chp2, MAX_NAME_LEN255);
       */
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W5T5.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W5T5.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  if (1 == 2)
    {
      printf
	("GJK fake000W5T5021xA STOP, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
	 collInp1.collName, strDataTypeInput1);
      return (0);
    }
  else
    printf
      ("GJK fake000W5T5021xA OK, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
       collInp1.collName, strDataTypeInput1);


  /*
   rei->status = rsDataObjCopy (rsComm, myDataObjCopyInp, &transStat);
    if (transStat != NULL) {
        free (transStat);
    }

    if (myDataObjCopyInp == &dataObjCopyInp) {
        clearKeyVal (&myDataObjCopyInp->destDataObjInp.condInput);
    }
  */

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 0);	/* 0 == collection name is LIKE name */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T5(), Test failed . No file object '%s' found in iRods.iChkFailed1=(%d)\n",
		       strOut, strDataTypeInput1, iChkFailed1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      for (i = 0; i < iChkFailed1; i++)
	{
	  collInp_t ptrInpColl;
	  char strFullDataPath[MAX_NAME_LEN255];
	  UserDefinedMetadata_t aAVUarray[1024];
	  int iErr = (-1), iCountUserDefinedMetadata = (-1), iTotalAVUs =
	    (-1);

	  sprintf (strFullDataPath, "%s/%s", lsArray1[i].chCollection,
		   lsArray1[i].chObjName);
	  strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN255);
	  iErr =
	    intGetDataObjChksumsTimeStampsFromAVU9 (&ptrInpColl, aAVUarray,
						    &iCountUserDefinedMetadata,
						    strOut, rei);

	  iTotalAVUs = iCountUserDefinedMetadata;
	  iAttrValMatch = 0;
	  printf
	    ("GJK-P msiW5T5(), 0008.994.7.0.G i=(%d), iChkFailed1=(%d), iTotalAVUs=(%d), iErr=(%d), strFullDataPath=(%s)\n",
	     i, iChkFailed1, iTotalAVUs, iErr, strFullDataPath);
	  for (j = 0; j < iTotalAVUs; j++)
	    {

	      printf
		("GJK fake9931N msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iTotalAVUs=(%d), aAVUarray[j].attribute=(%s), aAVUarray[j].value=(%s), aAVUarray[j].units=(%s), iSame=(%d) \n",
		 lsArray1[i].chCollection, lsArray1[i].chObjName,
		 lsArray1[i].isData, lsArray1[i].chDataSize,
		 (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
		 lsArray1[i].chDataAccessName, strDataTypeInput1,
		 lsArray1[i].chRescName, i, j, iTotalAVUs,
		 aAVUarray[j].attribute, aAVUarray[j].value,
		 aAVUarray[j].units, iSame);

	      if (strcmp (aAVUarray[j].attribute, "myattr1") == 0)
		{
		  if (strlen (aAVUarray[j].value) > 0)
		    /* I have give AVY attribute and it has non empty value */
		    iAttrValMatch = 1;
		}
	      /*
	         lTmp = strtol (aAVUarray[j].value, (char **) NULL, 10);
	         if (lMax < lTmp)
	         lMax = lTmp;
	       */
	    }

	  printf
	    ("GJK fake9922a msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), .chRescName=(%s), i=(%3d), j=(%d), iSame=(%d)\n",
	     lsArray1[i].chCollection, lsArray1[i].chObjName,
	     lsArray1[i].isData, lsArray1[i].chDataSize,
	     (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
	     lsArray1[i].chDataAccessName, strDataTypeInput1,
	     lsArray1[i].chRescName, i, j, iSame);

	  // if (iAttrValMatch != 1) break;
	}			/* for i= */


    }				/* else */

  if (iAttrValMatch == 1)
    (void) snprintf (strOut, 255,
		     "%s msiW5T5(), Test OK. Collection '%s' has given set of attributes with non-empty values.\n",
		     strOut, strDataTypeInput1);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiW5T5(), Test failed. Collection '%s' does not have given set of attributes with non-empty values..\n",
		       strOut, strDataTypeInput1);
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
    ("GJK end of msiW5T5(), Fri Mar 21 11:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */

/*
5. Generate report listing all preservation attributes.
6. Check whether file is master copy and turn off deletion, turn off user access
, turn on versioning.
7. Override access restriction if dispute flag is set and access is by data grid
 administrator.

*/
