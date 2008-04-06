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

#ifdef LIBBB
#include "libbb.h"

extern int
copyfd (int fd1, int fd2)
{
  char buf[8192];
  ssize_t nread, nwrote;

  while (1)
    {
      nread = safe_read (fd1, buf, sizeof (buf));
      if (nread == 0)
	break;
      if (nread == -1)
	{
	  perror_msg ("read");
	  return -1;
	}

      nwrote = full_write (fd2, buf, nread);
      if (nwrote == -1)
	{
	  perror_msg ("write");
	  return -1;
	}
    }

  return 0;
}
#else
int
copyfd (int fd1, int fd2)
{
  char buf[8192];
  ssize_t nread, nwrote;

  while (1)
    {
      nread = read (fd1, buf, sizeof (buf));
      if (nread == 0)
	break;
      if (nread == -1)
	{
	  printf ("GJK copyfd() read rERROR nread=(%d)\n", nread);
	  return -1;
	}

      nwrote = write (fd2, buf, nread);
      if (nwrote == -1)
	{
	  printf ("GJK copyfd() nwrote rERROR nread=(%d)\n", nwrote);
	  return -1;
	}
    }

  return 0;
}
#endif

int
iParseInputParams2 (msParam_t * coll1, msParam_t * coll2,
		    msParam_t * outParam, char *strDataTypeInput1,
		    char *strDataTypeInput2, char *chp1, char *chp2)
{
  int i;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN];


  printf
    ("GJK iParseInputParams2 1A, chp1=(%s), chp2=(%s), chp1=(%p), chp2=(%p)\n",
     chp1, chp2, chp1, chp2);
  printf
    ("GJK iParseInputParams2 1A, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput1=(%p), strDataTypeInput2=(%p)\n",
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput1,
     strDataTypeInput2);

  /* parse input parameter 1 'coll1' , iRods directory or file name */
  if ((strDataTypeInput1 = parseMspForStr (coll1)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "iParseInputParams2(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############0iParseInputParams2.0001a STOP\n");
      return (0);
    }
  else
    {
      printf
	("GJK ############### fake00 ##############0iParseInputParams2.0001a OK, strDataTypeInput1=(%s)\n",
	 strDataTypeInput1);
      sleep (1);
    }

  /* parse input parameter 2 'coll2' , expiry date */
  if ((strDataTypeInput2 = parseMspForStr (coll2)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "iParseInputParams2(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############0iParseInputParams2.2aK STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0iParseInputParams2.2aK OK, strDataTypeInput2=(%s)\n",
       strDataTypeInput2);

#define noPAR3
#ifdef PAR3
  /* parse input parameter 3 'coll3' */
  if ((strDataTypeInput3 = parseMspForStr (coll3)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "iParseInputParams2(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }
#endif

  printf
    ("GJK 1M iParseInputParams2 1Z, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput1=(%p), strDataTypeInput2=(%p)\n",
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput1,
     strDataTypeInput2);

  strncpy (chp1, strDataTypeInput1, MAX_NAME_LEN);
  strncpy (chp2, strDataTypeInput2, MAX_NAME_LEN);

  printf
    ("GJK 1X iParseInputParams2 strDataTypeInput1=(%s), strDataTypeInput2=(%s))\n",
     strDataTypeInput1, strDataTypeInput2);
  printf ("GJK 1Y iParseInputParams2 chp1=(%s), chp2=(%s))\n", chp1, chp2);
  printf
    ("GJK 1Y iParseInputParams2 1Z, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput1=(%p), strDataTypeInput2=(%p)\n",
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput1,
     strDataTypeInput2);

  printf
    ("GJK iParseInputParams2 1Z, chp1=(%s), chp2=(%s), chp1=(%p), chp2=(%p)\n",
     chp1, chp2, chp1, chp2);
  printf
    ("GJK iParseInputParams2 1Z, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput1=(%p), strDataTypeInput2=(%p)\n",
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput1,
     strDataTypeInput2);
  return (0);
}

int
iParseInputParams3 (msParam_t * coll1, msParam_t * coll2, msParam_t * coll3,
		    msParam_t * outParam, char *strDataTypeInput1,
		    char *strDataTypeInput2, char *strDataTypeInput3,
		    char *chp1, char *chp2, char *chp3)
{
  int i;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN];


  printf
    ("GJK iParseInputParams3 1A, chp1=(%s), chp2=(%s), chp3=(%s), chp1=(%p), chp2=(%p), chp3=(%p)\n",
     chp1, chp2, chp3, chp1, chp2, chp3);
  printf
    ("GJK iParseInputParams3 1A, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), strDataTypeInput1=(%p), strDataTypeInput2=(%p), strDataTypeInput3=(%p)\n",
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput3,
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);

  /* parse input parameter 1 'coll1' , iRods directory or file name */
  if ((strDataTypeInput1 = parseMspForStr (coll1)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "iParseInputParams3(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############0iParseInputParams3.0001a STOP\n");
      return (0);
    }
  else
    {
      printf
	("GJK ############### fake00 ##############0iParseInputParams3.0001a OK, strDataTypeInput1=(%s)\n",
	 strDataTypeInput1);
      sleep (1);
    }

  /* parse input parameter 2 'coll2' , expiry date */
  if ((strDataTypeInput2 = parseMspForStr (coll2)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "iParseInputParams3(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############0iParseInputParams3.2aK STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0iParseInputParams3.2aK OK, strDataTypeInput2=(%s)\n",
       strDataTypeInput2);

  /* parse input parameter 3 'coll3' , expiry date */
  if ((strDataTypeInput3 = parseMspForStr (coll3)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam3 error\n");
      rodsLog (LOG_ERROR, "iParseInputParams3(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 3)
    {
      printf
	("GJK ############### fake00 ##############0iParseInputParams3.3aK STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0iParseInputParams3.3aK OK, strDataTypeInput3=(%s)\n",
       strDataTypeInput3);

#define noPAR3
#ifdef PAR3
  /* parse input parameter 3 'coll3' */
  if ((strDataTypeInput3 = parseMspForStr (coll3)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "iParseInputParams3(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }
#endif

  printf
    ("GJK iParseInputParams3 1M, chp1=(%s), chp2=(%s), chp3=(%s), chp1=(%p), chp2=(%p), chp3=(%p)\n",
     chp1, chp2, chp3, chp1, chp2, chp3);
  printf
    ("GJK iParseInputParams3 1M, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), strDataTypeInput1=(%p), strDataTypeInput2=(%p), strDataTypeInput3=(%p)\n",
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput3,
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);

  strncpy (chp1, strDataTypeInput1, MAX_NAME_LEN);
  strncpy (chp2, strDataTypeInput2, MAX_NAME_LEN);
  strncpy (chp3, strDataTypeInput3, MAX_NAME_LEN);

  printf
    ("GJK iParseInputParams3 1X, chp1=(%s), chp2=(%s), chp3=(%s), chp1=(%p), chp2=(%p), chp3=(%p)\n",
     chp1, chp2, chp3, chp1, chp2, chp3);
  printf
    ("GJK iParseInputParams3 1X, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), strDataTypeInput1=(%p), strDataTypeInput2=(%p), strDataTypeInput3=(%p)\n",
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput3,
     strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);

  return (0);
}

/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW4T1 \author  Sifang Lu \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* Check if file has expiry date equal to input */
/* **************************************************************************** */

int
msiChkDatNameOnceW4T1 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * outParam, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2, *myCollInp1, *myCollInp2;
  int i, iSame = (-1);
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  int iChkFailed1 = 0;
  int intLsArraySize1 = (MAX_NAME_LEN);
  lsInfo_t lsArray1[MAX_NAME_LEN + 1];

  char *strDataTypeInput1, *strDataTypeInput2, *strDataTypeInput3;

  RE_TEST_MACRO ("    Calling msiChkDatNameOnceW4T1")
    printf
    ("GJK- begin 000W4T101.0.0-W4T1r,Fri Mar 14 18:29:18 PDT 2008, in msiChkDatNameOnceW4T1(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W4T1.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W4T1.11.b.x OK\n");

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDatNameOnceW4T1: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  /* parse input parameter 1 'coll1' , iRods directory or file name */
  if ((strDataTypeInput1 = parseMspForStr (coll1)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDatNameOnceW4T1(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T1.0001a STOP\n");
      return (0);
    }
  else
    {
      printf
	("GJK ############### fake00 ##############0W4T1.0001a OK, myCollInp1->collName=(%s), strDataTypeInput1=(%s)\n",
	 myCollInp1->collName, strDataTypeInput1);
      sleep (1);
    }

  /* parse input parameter 2 'coll2' , expiry date */
  if ((strDataTypeInput2 = parseMspForStr (coll2)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDatNameOnceW4T1(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T1.2aK STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W4T1.2aK OK,  myCollInp2->collName=(%s), strDataTypeInput2=(%s)\n",
       myCollInp2->collName, strDataTypeInput2);

#define noPAR3
#ifdef PAR3
  /* parse input parameter 3 'coll3' */
  if ((strDataTypeInput3 = parseMspForStr (coll3)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDatNameOnceW4T1(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }
#endif

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T1.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W4T1.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  rstrcpy (collInp1.collName, strDataTypeInput1, MAX_NAME_LEN);
  if (1 == 2)
    {
      printf ("GJK fake000W4T1021xA STOP\n");
      return (0);
    }
  else
    printf
      ("GJK fake000W4T1021xA OK, before intDescColTree(), collInp1.collName=(%s)\n",
       collInp1.collName);

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 1);	/* 1 == collection name is equal */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T1 Test failed . No file object '%s' found in iRods.\n",
		       strOut, strDataTypeInput1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {
      for (i = 0; i < iChkFailed1; i++)
	{
	  char strTmp33[MAX_NAME_LEN + 1] = "";

	  printf
	    ("GJK fake9998x msiChkDatNameOnceW4T1(), .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chModifyTime=(%s), .chDataExpiry=(%s), i=(%3d)\n",
	     lsArray1[i].chCollection, lsArray1[i].chObjName,
	     lsArray1[i].isData, lsArray1[i].chDataSize,
	     (long) lsArray1[i].lDataSize, lsArray1[i].chModifyTime,
	     lsArray1[i].chDataExpiry, i);

	  sprintf (strTmp33, "%s/%s", lsArray1[i].chCollection,
		   lsArray1[i].chObjName);

	  if (strcmp (strDataTypeInput1, strTmp33) == 0)
	    {
	      /* the name is the same */
	      printf
		("GJK fake9922a msiChkDatNameOnceW4T1(), .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chModifyTime=(%s), .chDataExpiry=(%s), strDataTypeInput1=(%s), strTmp33=(%s), i=(%3d), iSame=(%d)\n",
		 lsArray1[i].chCollection, lsArray1[i].chObjName,
		 lsArray1[i].isData, lsArray1[i].chDataSize,
		 (long) lsArray1[i].lDataSize, lsArray1[i].chModifyTime,
		 lsArray1[i].chDataExpiry, strDataTypeInput1, strTmp33, i,
		 iSame);

	      if (strcmp (lsArray1[i].chDataExpiry, strDataTypeInput2) == 0)
		{
		  /* the name and the expiration stamp is the same */
		  iSame = 0;
		  printf
		    ("GJK fake9923b msiChkDatNameOnceW4T1(), .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chModifyTime=(%s), .chDataExpiry=(%s), i=(%3d), strDataTypeInput1=(%s), strTmp33=(%s), strDataTypeInput2=(%s), iSame=(%d)\n",
		     lsArray1[i].chCollection, lsArray1[i].chObjName,
		     lsArray1[i].isData, lsArray1[i].chDataSize,
		     (long) lsArray1[i].lDataSize, lsArray1[i].chModifyTime,
		     lsArray1[i].chDataExpiry, i, strDataTypeInput1, strTmp33,
		     strDataTypeInput2, iSame);
		}
	      else
		{
		  /* the name is the same but the expiration stamp is not */
		  iSame = (-2);
		  printf
		    ("GJK fake9924c msiChkDatNameOnceW4T1(), .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chModifyTime=(%s), .chDataExpiry=(%s), i=(%3d), strDataTypeInput1=(%s), strTmp33=(%s), strDataTypeInput2=(%s), iSame=(%d)\n",
		     lsArray1[i].chCollection, lsArray1[i].chObjName,
		     lsArray1[i].isData, lsArray1[i].chDataSize,
		     (long) lsArray1[i].lDataSize, lsArray1[i].chModifyTime,
		     lsArray1[i].chDataExpiry, i, strDataTypeInput1, strTmp33,
		     strDataTypeInput2, iSame);
		}
	    }
	}
    }

  if (iSame == 0)
    (void) snprintf (strOut, 255,
		     "%s msiChkDatNameOnceW4T1(), Test OK. File object '%s' has the required expiration date '%s'.\n",
		     strOut, strDataTypeInput1, strDataTypeInput2);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T1() Test failed. File object '%s' has not the required expiration date '%s'.\n",
		       strOut, strDataTypeInput1, strDataTypeInput2);
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
    ("GJK end of msiChkDatNameOnceW4T1(), Tue Mar 18 18:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */


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


/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW4T3 \author  Sifang Lu \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* Check if file(s) are not duplicated in the smae resource */
/* **************************************************************************** */

int
msiChkDatNameOnceW4T3 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * outParam, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2, *myCollInp1, *myCollInp2;
  int i, j, iSame = 0;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  int iChkFailed1 = 0;
  int intLsArraySize1 = (MAX_NAME_LEN);
  lsInfo_t lsArray1[MAX_NAME_LEN + 1];

  char *strDataTypeInput1, *strDataTypeInput2, *strDataTypeInput3;

  RE_TEST_MACRO ("    Calling msiChkDatNameOnceW4T3")
    printf
    ("GJK- begin 000W4T301.0.0-W4T3r,Fri Mar 14 18:29:18 PDT 2008, in msiChkDatNameOnceW4T3(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W4T3.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W4T3.11.b.x OK\n");

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDatNameOnceW4T3: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  /* parse input parameter 1 'coll1' , iRods directory or file name */
  if ((strDataTypeInput1 = parseMspForStr (coll1)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDatNameOnceW4T3(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T3.0001a STOP\n");
      return (0);
    }
  else
    {
      printf
	("GJK ############### fake00 ##############0W4T3.0001a OK, myCollInp1->collName=(%s), strDataTypeInput1=(%s)\n",
	 myCollInp1->collName, strDataTypeInput1);
      sleep (1);
    }

  /* parse input parameter 2 'coll2' , expiry date */
  if ((strDataTypeInput2 = parseMspForStr (coll2)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDatNameOnceW4T3(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T3.2aK STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W4T3.2aK OK,  myCollInp2->collName=(%s), strDataTypeInput2=(%s)\n",
       myCollInp2->collName, strDataTypeInput2);

#define noPAR3
#ifdef PAR3
  /* parse input parameter 3 'coll3' */
  if ((strDataTypeInput3 = parseMspForStr (coll3)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDatNameOnceW4T3(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }
#endif

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T3.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W4T3.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  rstrcpy (collInp1.collName, strDataTypeInput1, MAX_NAME_LEN);
  if (1 == 2)
    {
      printf ("GJK fake000W4T3021xA STOP\n");
      return (0);
    }
  else
    printf
      ("GJK fake000W4T3021xA OK, before intDescColTree(), collInp1.collName=(%s)\n",
       collInp1.collName);

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 1);	/* 0 == collection name is LIKE name */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T3(), Test failed . No file object '%s' found in iRods.iChkFailed1=(%d)\n",
		       strOut, strDataTypeInput1, iChkFailed1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {
      for (i = 0; i < iChkFailed1; i++)
	{
	  for (j = 0; j < iChkFailed1; j++)
	    {
	      char strTmp33[MAX_NAME_LEN + 1] = "";

	      printf
		("GJK fake9998x msiChkDatNameOnceW4T3(), .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chModifyTime=(%s), .chDataExpiry=(%s), i=(%3d)\n",
		 lsArray1[i].chCollection, lsArray1[i].chObjName,
		 lsArray1[i].isData, lsArray1[i].chDataSize,
		 (long) lsArray1[i].lDataSize, lsArray1[i].chModifyTime,
		 lsArray1[i].chDataExpiry, i);

	      sprintf (strTmp33, "%s/%s", lsArray1[i].chCollection,
		       lsArray1[i].chObjName);

	      if (i != j &&
		  strcmp (lsArray1[i].chCollection,
			  lsArray1[j].chCollection) == 0
		  && strcmp (lsArray1[i].chObjName,
			     lsArray1[j].chObjName) == 0
		  && strcmp (lsArray1[i].chRescName,
			     lsArray1[j].chRescName) == 0)
		{
		  /* the name is the same and resotce is the same */
		  printf
		    ("GJK fake9922a msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chModifyTime=(%s), .chDataExpiry=(%s), strDataTypeInput1=(%s), strTmp33=(%s), .chRescName=(%s), i=(%3d), j=(%d), iSame=(%d)\n",
		     lsArray1[i].chCollection, lsArray1[i].chObjName,
		     lsArray1[i].isData, lsArray1[i].chDataSize,
		     (long) lsArray1[i].lDataSize, lsArray1[i].chModifyTime,
		     lsArray1[i].chDataExpiry, strDataTypeInput1, strTmp33,
		     lsArray1[i].chRescName, i, j, iSame);
		}
	      else
		{
		  printf
		    ("GJK fake9922a msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chModifyTime=(%s), .chDataExpiry=(%s), strDataTypeInput1=(%s), strTmp33=(%s), .chRescName=(%s), i=(%3d), j=(%d), iSame=(%d)\n",
		     lsArray1[i].chCollection, lsArray1[i].chObjName,
		     lsArray1[i].isData, lsArray1[i].chDataSize,
		     (long) lsArray1[i].lDataSize, lsArray1[i].chModifyTime,
		     lsArray1[i].chDataExpiry, strDataTypeInput1, strTmp33,
		     lsArray1[i].chRescName, i, j, iSame);
		}		/* else */
	    }			/* for j= */
	}			/* for i= */
    }				/* else */

  if (iSame == 0)
    (void) snprintf (strOut, 255,
		     "%s msiChkDatNameOnceW4T3(), Test OK. File object '%s' has the required expiration date '%s'.\n",
		     strOut, strDataTypeInput1, strDataTypeInput2);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T3(), Test failed. oRods objects in '%s' collection are not replicated in the '%s' resource\n",
		       strOut, strDataTypeInput1, strDataTypeInput2);
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
    ("GJK end of msiChkDatNameOnceW4T3(), Tue Mar 18 18:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */


/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW4T5 \author  Sifang Lu \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* Check if the collection has N files */
/* **************************************************************************** */

int
msiChkDatNameOnceW4T5 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * outParam, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2, *myCollInp1, *myCollInp2;
  int i, iSame = 0;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  int iChkFailed1 = 0;
  int intLsArraySize1 = (MAX_NAME_LEN);
  lsInfo_t lsArray1[MAX_NAME_LEN + 1];

  char *strDataTypeInput1, *strDataTypeInput2, *strDataTypeInput3;
  int iTmp3 = 0;

  RE_TEST_MACRO ("    Calling msiChkDatNameOnceW4T5")
    printf
    ("GJK- begin 000W4T501.0.0-W4T5r,Fri Mar 14 18:29:18 PDT 2008, in msiChkDatNameOnceW4T5(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W4T5.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W4T5.11.b.x OK\n");

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDatNameOnceW4T5: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  /* parse input parameter 1 'coll1' , iRods directory or file name */
  if ((strDataTypeInput1 = parseMspForStr (coll1)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDatNameOnceW4T5(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T5.0001a STOP\n");
      return (0);
    }
  else
    {
      printf
	("GJK ############### fake00 ##############0W4T5.0001a OK, myCollInp1->collName=(%s), strDataTypeInput1=(%s)\n",
	 myCollInp1->collName, strDataTypeInput1);
      sleep (1);
    }

  /* parse input parameter 2 'coll2' , expiry date */
  if ((strDataTypeInput2 = parseMspForStr (coll2)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDatNameOnceW4T5(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T5.2aK STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W4T5.2aK OK,  myCollInp2->collName=(%s), strDataTypeInput2=(%s)\n",
       myCollInp2->collName, strDataTypeInput2);

#define noPAR3
#ifdef PAR3
  /* parse input parameter 3 'coll3' */
  if ((strDataTypeInput3 = parseMspForStr (coll3)) != NULL)
    {
    }
  else
    {
      sprintf (strOut,
	       "usage 'foo iRods_collection size_min size_max', ERROR:  msiChkDataObjAVU6Vol10(), input inpParam2 error\n");
      rodsLog (LOG_ERROR, "msiChkDatNameOnceW4T5(),  input inpParam1 error.");
      i = fillStrInMsParam (outParam, strOut);	/* MsParam.c parse
						 * addformatedtrsing to
						 * bytes WriteBytesBuff
						 * printMsParam.c */
      return (-1);
    }
#endif

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T5.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W4T5.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  rstrcpy (collInp1.collName, strDataTypeInput1, MAX_NAME_LEN);
  if (1 == 2)
    {
      printf ("GJK fake000W4T5021xA STOP\n");
      return (0);
    }
  else
    printf
      ("GJK fake000W4T5021xA OK, before intDescColTree(), collInp1.collName=(%s)\n",
       collInp1.collName);

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 1);	/* 0 == collection name is LIKE name */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T5(), Test failed . No file object '%s' found in iRods.iChkFailed1=(%d)\n",
		       strOut, strDataTypeInput1, iChkFailed1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {
      iTmp3 = atoi (strDataTypeInput2);
      if (iTmp3 == iChkFailed1)
	iSame = 0;
      else
	iSame = 1;

    }				/* else */

  if (iSame == 0)
    (void) snprintf (strOut, 255,
		     "%s msiChkDatNameOnceW4T5(), Test OK. Collection '%s' has exactly %d files\n",
		     strOut, strDataTypeInput1, iChkFailed1);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T5(), Test failed. Collection '%s' has %d files but it should have contain %s files.\n",
		       strOut, strDataTypeInput1, iChkFailed1,
		       strDataTypeInput2);
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
    ("GJK end of msiChkDatNameOnceW4T5(), Tue Mar 18 18:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */


/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW4T7 \author  Sifang Lu \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* Check if the collection has N files */
/* **************************************************************************** */

int
msiChkDatNameOnceW4T7 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * outParam, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2;
  int i, iSame = 0, j;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  int iChkFailed1 = 0;
  int intLsArraySize1 = (MAX_NAME_LEN);
  lsInfo_t lsArray1[MAX_NAME_LEN + 1];
  char strDataTypeInput1[MAX_NAME_LEN + 1],
    strDataTypeInput2[MAX_NAME_LEN + 1], strDataTypeInput3[MAX_NAME_LEN + 1],
    chp1[MAX_NAME_LEN], chp2[MAX_NAME_LEN];

  RE_TEST_MACRO ("    Calling msiChkDatNameOnceW4T7")
    printf
    ("GJK- begin 000W4T701.0.0-W4T7r,Fri Mar 14 18:29:18 PDT 2008, in msiChkDatNameOnceW4T7(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W4T7.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W4T7.11.b.x OK\n");

  rstrcpy (collInp1.collName, strDataTypeInput1, MAX_NAME_LEN);

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDatNameOnceW4T7: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  rstrcpy (strDataTypeInput1, "1", MAX_NAME_LEN);
  rstrcpy (strDataTypeInput2, "2", MAX_NAME_LEN);
  rstrcpy (strDataTypeInput3, "3", MAX_NAME_LEN);

  i =
    iParseInputParams2 (coll1, coll2, outParam, strDataTypeInput1,
			strDataTypeInput2, chp1, chp2);
  if (0 != i)
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T7(), iParseInputParams2() failed.\n",
		       strOut);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      rstrcpy (strDataTypeInput1, chp1, MAX_NAME_LEN);
      rstrcpy (strDataTypeInput2, chp2, MAX_NAME_LEN);

      printf
	("GJK 0007.W4T7.4e i=(%d), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
	 i, strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);
      printf ("GJK 0007.W4T7.4f chp1=(%s), chp2=(%s))\n", chp1, chp2);

      /*
         rstrcpy(strDataTypeInput1, chp1, MAX_NAME_LEN);
         rstrcpy(strDataTypeInput2, chp2, MAX_NAME_LEN);
       */
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T7.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W4T7.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  if (1 == 2)
    {
      printf
	("GJK fake000W4T7021xA STOP, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
	 collInp1.collName, strDataTypeInput1);
      return (0);
    }
  else
    printf
      ("GJK fake000W4T7021xA OK, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
       collInp1.collName, strDataTypeInput1);

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 0);	/* 0 == collection name is LIKE name */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T7(), Test failed . No file object '%s' found in iRods.iChkFailed1=(%d)\n",
		       strOut, strDataTypeInput1, iChkFailed1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      for (i = 0; i < iChkFailed1; i++)
	{
	  for (j = 0; j < iChkFailed1; j++)
	    {
	      char strTmp33[MAX_NAME_LEN + 1] = "";

	      printf
		("GJK fake9998x msiChkDatNameOnceW4T3(), .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), i=(%3d)\n",
		 lsArray1[i].chCollection, lsArray1[i].chObjName,
		 lsArray1[i].isData, lsArray1[i].chDataSize,
		 (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
		 lsArray1[i].chDataAccessName, i);

	      sprintf (strTmp33, "%s/%s", lsArray1[i].chCollection,
		       lsArray1[i].chObjName);

	      if (i != j &&
		  strcmp (lsArray1[i].chCollection,
			  lsArray1[j].chCollection) == 0
		  && strcmp (lsArray1[i].chObjName,
			     lsArray1[j].chObjName) == 0
		  && strcmp (lsArray1[i].chRescName,
			     lsArray1[j].chRescName) == 0)
		{
		  /* the name is the same and resotce is the same */
		  printf
		    ("GJK fake9922a msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), strTmp33=(%s), .chRescName=(%s), i=(%3d), j=(%d), iSame=(%d)\n",
		     lsArray1[i].chCollection, lsArray1[i].chObjName,
		     lsArray1[i].isData, lsArray1[i].chDataSize,
		     (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
		     lsArray1[i].chDataAccessName, strDataTypeInput1,
		     strTmp33, lsArray1[i].chRescName, i, j, iSame);
		}
	      else
		{
		  printf
		    ("GJK fake9922a msiChkDatNameOnceW4T3(), is same, .chCollection=(%s), .chObjName=(%s), .isData=(%d), .chDataSize=(%s), .lDataSize=(%ld), .chUserName=(%s), .chDataAccessName=(%s), strDataTypeInput1=(%s), strTmp33=(%s), .chRescName=(%s), i=(%3d), j=(%d), iSame=(%d)\n",
		     lsArray1[i].chCollection, lsArray1[i].chObjName,
		     lsArray1[i].isData, lsArray1[i].chDataSize,
		     (long) lsArray1[i].lDataSize, lsArray1[i].chUserName,
		     lsArray1[i].chDataAccessName, strDataTypeInput1,
		     strTmp33, lsArray1[i].chRescName, i, j, iSame);
		}		/* else */
	    }			/* for j= */
	}			/* for i= */


    }				/* else */

  if (iSame == 0)
    (void) snprintf (strOut, 255,
		     "%s msiChkDatNameOnceW4T7(), Test OK. Collection '%s' has exactly %d files\n",
		     strOut, strDataTypeInput1, iChkFailed1);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T7(), Test failed. Collection '%s' has %d files but it should have contain %s files.\n",
		       strOut, strDataTypeInput1, iChkFailed1,
		       strDataTypeInput2);
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
    ("GJK end of msiChkDatNameOnceW4T7(), Tue Mar 18 18:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */


/* **************************************************************************** */
/*
 * \fn msiChkDatNameOnceW4T9 \author  Sifang Lu \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* Check if the collection/sybcollections have given set of metadata attributes */
/* **************************************************************************** */

int
msiChkDatNameOnceW4T9 (msParam_t * coll1, msParam_t * coll2,
		       msParam_t * coll3, msParam_t * outParam,
		       ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2;
  int i, iSame = 0, j;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN], *strTimeDiff;
  int iChkFailed1 = 0, iAttrValMatch;
  int intLsArraySize1 = (MAX_NAME_LEN);
  lsInfo_t lsArray1[MAX_NAME_LEN + 1];
  char strDataTypeInput1[MAX_NAME_LEN + 1],
    strDataTypeInput2[MAX_NAME_LEN + 1], strDataTypeInput3[MAX_NAME_LEN + 1],
    chp1[MAX_NAME_LEN], chp2[MAX_NAME_LEN], chp3[MAX_NAME_LEN];

  RE_TEST_MACRO ("    Calling msiChkDatNameOnceW4T9")
    printf
    ("GJK- begin 000W4T901.0.0-W4T9r,Fri Mar 14 18:29:18 PDT 2008, in msiChkDatNameOnceW4T9(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf
	("GJK ############### fake00 ##############01.W4T9.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.W4T9.11.b.x OK\n");

  /*        rstrcpy(collInp1.collName, strDataTypeInput1, MAX_NAME_LEN); */

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR,
	       "msiChkDatNameOnceW4T9: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  rstrcpy (strDataTypeInput1, "1", MAX_NAME_LEN);
  rstrcpy (strDataTypeInput2, "2", MAX_NAME_LEN);
  rstrcpy (strDataTypeInput3, "3", MAX_NAME_LEN);

  i =
    iParseInputParams3 (coll1, coll2, coll3, outParam, strDataTypeInput1,
			strDataTypeInput2, strDataTypeInput3, chp1, chp2,
			chp3);
  if (0 != i)
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T9(), iParseInputParams2() failed.\n",
		       strOut);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      rstrcpy (strDataTypeInput1, chp1, MAX_NAME_LEN);
      rstrcpy (strDataTypeInput2, chp2, MAX_NAME_LEN);
      rstrcpy (strDataTypeInput3, chp3, MAX_NAME_LEN);

      printf
	("GJK 0007.W4T9.4e i=(%d), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
	 i, strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);
      printf ("GJK 0007.W4T9.4f chp1=(%s), chp2=(%s), chp3=(%s)\n", chp1,
	      chp2, chp3);

      /*
         rstrcpy(strDataTypeInput1, chp1, MAX_NAME_LEN);
         rstrcpy(strDataTypeInput2, chp2, MAX_NAME_LEN);
       */
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0W4T9.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0W4T9.3c OK, strTimeDiff=(%s), strDataTypeInput3=(%s)\n",
       strTimeDiff, strDataTypeInput3);

  if (1 == 2)
    {
      printf
	("GJK fake000W4T9021xA STOP, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
	 collInp1.collName, strDataTypeInput1);
      return (0);
    }
  else
    printf
      ("GJK fake000W4T9021xA OK, before intDescColTree(), collInp1.collName=(%s), strDataTypeInput1=(%s)\n",
       collInp1.collName, strDataTypeInput1);

  iChkFailed1 = intDescColTree (rsComm, &collInp1, rei, lsArray1, intLsArraySize1, 0);	/* 0 == collection name is LIKE name */
  rstrcpy (strOut, "", 2);

  if (iChkFailed1 < 1)
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T9(), Test failed . No file object '%s' found in iRods.iChkFailed1=(%d)\n",
		       strOut, strDataTypeInput1, iChkFailed1);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      for (i = 0; i < iChkFailed1; i++)
	{
	  collInp_t ptrInpColl;
	  char strFullDataPath[MAX_NAME_LEN];
	  UserDefinedMetadata_t aAVUarray[1024];
	  int iErr = (-1), iCountUserDefinedMetadata = (-1), iTotalAVUs =
	    (-1);

	  sprintf (strFullDataPath, "%s/%s", lsArray1[i].chCollection,
		   lsArray1[i].chObjName);
	  strncpy (ptrInpColl.collName, strFullDataPath, MAX_NAME_LEN);
	  iErr =
	    intGetDataObjChksumsTimeStampsFromAVU9 (&ptrInpColl, aAVUarray,
						    &iCountUserDefinedMetadata,
						    strOut, rei);

	  iTotalAVUs = iCountUserDefinedMetadata;
	  iAttrValMatch = 0;
	  printf
	    ("GJK-P msiChkDatNameOnceW4T9(), 0008.994.7.0.G i=(%d), iChkFailed1=(%d), iTotalAVUs=(%d), iErr=(%d), strFullDataPath=(%s)\n",
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
		     "%s msiChkDatNameOnceW4T9(), Test OK. Collection '%s' has given set of attributes with non-empty values.\n",
		     strOut, strDataTypeInput1);
  else
    {
      (void) snprintf (strOut, 255,
		       "%s msiChkDatNameOnceW4T9(), Test failed. Collection '%s' does not have given set of attributes with non-empty values..\n",
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
    ("GJK end of msiChkDatNameOnceW4T9(), Fri Mar 21 11:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
     strOut);
  return (rei->status);
}				/* msiChkDatNameOnce11b(msParam... */

/* **************************************************************************** */
/*
 * \fn msiPhp \author  Sifang Lu \date   2008.03.18. \brief This
 * microservice iterate through collection,  \return integer \retval 0 on
 * success \sa \post \pre \bug  no known bugs
 */

/* Check if the collection/sybcollections have given set of metadata attributes */
/* **************************************************************************** */
/* http://www-h.eng.cam.ac.uk/help/tpl/unix/fork.html */
/*
in file  reSysDataObjOpr.c
*/
int
msiPhp (msParam_t * coll1, msParam_t * coll2, msParam_t * coll3,
	msParam_t * outParam, ruleExecInfo_t * rei)
{
  rsComm_t *rsComm;
  collInp_t collInp1, collInp2;
  int i;
  char strOut[MAX_NAME_LEN * MAX_NAME_LEN] = "";
  char strDataTypeInput1[MAX_NAME_LEN + 1],
    strDataTypeInput2[MAX_NAME_LEN + 1], strDataTypeInput3[MAX_NAME_LEN + 1],
    chp1[MAX_NAME_LEN], chp2[MAX_NAME_LEN], chp3[MAX_NAME_LEN], cmdLine[10*MAX_NAME_LEN]="";

  dataObjInfo_t *dataObjInfoHead;

  RE_TEST_MACRO ("    Calling msiPhp")

    printf
    ("GJK- begin 000Php01.0.0-Phpr,Fri Mar 14 18:29:18 PDT 2008, in msiPhp(), bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############01.Php.11.bx STOP\n");
      return (0);
    }
  else
    printf ("GJK ############### fake00 ##############01.Php.11.b.x OK\n");

  /*        rstrcpy(collInp1.collName, strDataTypeInput1, MAX_NAME_LEN); */

  if (rei == NULL || rei->rsComm == NULL)
    {
      rodsLog (LOG_ERROR, "msiPhp: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
  rsComm = rei->rsComm;

  memset (&collInp1, 0, sizeof (collInp1));
  memset (&collInp2, 0, sizeof (collInp2));
  /*
     memset(myCollInp1, 0, sizeof(*myCollInp1));
     memset(myCollInp2, 0, sizeof(*myCollInp2));
   */

  rstrcpy (strDataTypeInput1, "1", MAX_NAME_LEN);
  rstrcpy (strDataTypeInput2, "2", MAX_NAME_LEN);
  rstrcpy (strDataTypeInput3, "3", MAX_NAME_LEN);

  i =
    iParseInputParams3 (coll1, coll2, coll3, outParam, strDataTypeInput1,
			strDataTypeInput2, strDataTypeInput3, chp1, chp2,
			chp3);
  if (0 != i)
    {
      (void) snprintf (strOut, 255,
		       "%s msiPhp(), iParseInputParams2() failed.\n", strOut);
      i = fillStrInMsParam (outParam, strOut);
      return (1);
    }
  else
    {

      rstrcpy (strDataTypeInput1, chp1, MAX_NAME_LEN);
      rstrcpy (strDataTypeInput2, chp2, MAX_NAME_LEN);
      rstrcpy (strDataTypeInput3, chp3, MAX_NAME_LEN);

      dataObjInfoHead = rei->doi;

      if (dataObjInfoHead == NULL) {
        return (rei->status);
      }

      rstrcpy (strDataTypeInput3, dataObjInfoHead->objPath, MAX_NAME_LEN);
      sprintf(cmdLine, "%s %s", strDataTypeInput1, strDataTypeInput3);

      printf
	("GJK 0007.Php.4e i=(%d), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
	 i, strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);
      printf ("GJK 0007.Php.4f chp1=(%s), chp2=(%s), chp3=(%s)\n", chp1, chp2, chp3);
      printf ("GJK 0007.Php.4g cmdLine=(%s), cstrDataTypeInput1=(%s)\n", cmdLine, strDataTypeInput1);
      rstrcpy (strDataTypeInput1, cmdLine, MAX_NAME_LEN);

      /*
         rstrcpy(strDataTypeInput1, chp1, MAX_NAME_LEN);
         rstrcpy(strDataTypeInput2, chp2, MAX_NAME_LEN);
       */
    }

  if (1 == 2)
    {
      printf ("GJK ############### fake00 ##############0Php.3c STOP\n");
      return (0);
    }
  else
    printf
      ("GJK ############### fake00 ##############0Php.3c OK, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
       strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);

  { /* block 1 */
    FILE *fp;
    int status;
    char path[PATH_MAX];


    /* fp = popen ("/bin/ls *", "r"); */
    /* execl(shell path, "sh", "-c", command, (char *)0); */
    /*
       #include <unistd.h>
       main()
       {
       execlp("/bin/ls", "-r", "-t", "-l", (char *) 0);
       }
     */
    /*

       int main(int argc, char *argv[]) {
       char *path = "/bin/ls";
       char *arg0 = "ls";
       pid_t pid;
       int pipefd[2];
       int status;
       pipe(pipefd);
       pid = fork();
       if (pid == 0) {
       dup2(pipefd[1], STDOUT_FILENO);
       close(pipefd[0]);
       close(pipefd[1]);
       if (execl(path, arg0, NULL) == -1)
       perror("execl");
       } else {
       close(pipefd[1]);
       copyfd(pipefd[0], STDOUT_FILENO);
       wait(&status);
       }
       return 0;
       }
     */
    /*

       int main(){
       pid_t pid;
       int status, died;
       switch(pid=fork()){
       case -1: cout << "can't fork\n";
       exit(-1);
       case 0 : execl("/usr/bin/date","date",0); // this is the code the child runs 
       default: died= wait(&status); // this is the code the parent runs
       }
       }

     */

#define noEXECL
#ifdef noEXECL
    fp = popen (strDataTypeInput1, "r");
    if (fp == NULL)
      {
	/* Handle error */
	printf ("GJK Php.34c ERROR fp == NULL\n");
	return (-1);
      }
    //rstrcpy(strOut, "", MAX_NAME_LEN);
    i = 1;
    while (fgets (path, PATH_MAX, fp) != NULL)
      {
	//char chBuff7[MAX_NAME_LEN];
	printf ("GJK PHP i=(%d), /bin/ls (%s)", i, path);
	//fprintf (chBuff7, "%s %s", strOut, path);
	//strncat(strOut, chBuff7, MAX_NAME_LEN);
      }
    
    status = pclose (fp);
#else
    {				/* execl() */
      pid_t pid, w;
      int pipefd[2];
      int status;

      char buf[8192];
      ssize_t nread, nwrote;

      pipe (pipefd);
      pid = fork ();
      if (pid == 0)
	{
	  dup2 (pipefd[1], STDOUT_FILENO);
	  close (pipefd[0]);
	  close (pipefd[1]);
	  status =
	    execlp (strDataTypeInput1, strDataTypeInput2, strDataTypeInput3,
		    (char *) 0);
	  if (status == -1)
	    {
	      /* I am the child process now */
	      printf
		("GJK i am the child in msiPhp(), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), errno=(%d), status=(%d)\n",
		 strDataTypeInput1, strDataTypeInput2, strDataTypeInput3,
		 errno, status);
	    }
	  else
	    {
	      printf
		("GJK i am the child in msiPhp(), strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), errno=(%d), status=(%d)\n",
		 strDataTypeInput1, strDataTypeInput2, strDataTypeInput3,
		 errno, status);
	    }
	}
      else
	{
	  /* I am the parent now */
	  close (pipefd[1]);
	  copyfd (pipefd[0], STDOUT_FILENO);
	  w = waitpid (-1, &status, 0);	/* wait (&status); */
	  /* return the PID of the child or error == (-1) */
	  if (w == -1)
	    {
	      printf
		("GJK i am the parent in msiPhp(), wait() ERROR, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), errno=(%d), status=(%d), w=(%d)\n",
		 strDataTypeInput1, strDataTypeInput2, strDataTypeInput3,
		 errno, status, w);

	      //      exit(EXIT_FAILURE); 
	    }
	  if (WIFEXITED (status))
	    {
	      printf
		("GJK i am the parent in msiPhp(), child exited, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), errno=(%d), status=(%d), w=(%d)\n",
		 strDataTypeInput1, strDataTypeInput2, strDataTypeInput3,
		 errno, WEXITSTATUS (status), w);
	    }
	  else
	    {
	      if (WIFSIGNALED (status))
		{
		  printf
		    ("GJK i am the parent in msiPhp(), child signalled, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), errno=(%d), status=(%d), w=(%d)\n",
		     strDataTypeInput1, strDataTypeInput2, strDataTypeInput3,
		     errno, WTERMSIG (status), w);
		}
	      else
		{
		  if (WIFSTOPPED (status))
		    {
		      printf
			("GJK i am the parent in msiPhp(), child stopped by signal, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), errno=(%d), status=(%d), w=(%d)\n",
			 strDataTypeInput1, strDataTypeInput2,
			 strDataTypeInput3, errno, WSTOPSIG (status), w);
		    }
		  else
		    {
		      if (WIFCONTINUED (status))
			{
			  printf
			    ("GJK i am the parent in msiPhp(), child continued, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s), errno=(%d), status=(%d), w=(%d)\n",
			     strDataTypeInput1, strDataTypeInput2,
			     strDataTypeInput3, errno, WSTOPSIG (status), w);
			}
		    }
		}
	    }
	}

      status = 0;
      /* get the standart output */
      while (1)
	{
	  nread = read (pipefd[0], buf, sizeof (buf));
	  if (nread == 0)
	    break;
	  if (nread == -1)
	    {
	      printf
		("GJK read(pipefd[0], buf, sizeof(buf),  read ERROR nread=(%d)\n",
		 nread);
	      status = (-1);
	    }
	  printf ("GJK buf=(%s), nread=(%d)", buf, nread);
	}

    }				/* pid != 0 */
#endif
    if (1 == 2)
      {
	printf ("GJK ############### fake00 ##############0Php.11g STOP\n");
	return (0);
      }
    else
      printf
	("GJK ############### fake00 ##############0Php.11g OK, strDataTypeInput1=(%s), strDataTypeInput2=(%s), strDataTypeInput3=(%s)\n",
	 strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);

    if (status == -1)
      {
	/* Error reported by pclose() */
	printf ("GJK Php.35d ERROR status=(%d)\n", status);
	//return (-2);
      }
    else
      {
	/* Use macros described under wait() to inspect `status' in order
	   to determine success/failure of command executed by popen() */
	printf ("GJK Php.36e OK status=(%d)\n", status);
	//return (-3);
      }

    sprintf (strOut, "Done OK, s1=(%s), s2=(%s), s3=(%s),\n",
	     strDataTypeInput1, strDataTypeInput2, strDataTypeInput3);

    i = fillStrInMsParam (outParam, strOut);

    if (1 == 2)
      {
	printf ("GJK fake9999a Tue Mar  4 18:47:03 PST 2008 STOP\n");
	return (0);
      }
    else
      printf ("GJK fake9999a Tue Mar  4 18:47:03 PST 2008 OK\n");

    printf
      ("GJK end of msiPhp(), Fri Mar 21 11:29:18 PDT 2008, EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE , strOut=(%s)\n",
       strOut);
    return (rei->status);
  } /* block 1 */
}				/* msiChkDatNameOnce11b(msParam... */
