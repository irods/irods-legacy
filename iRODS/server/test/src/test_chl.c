/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
  ICAT test program.
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "readServerConfig.h"

#include "rodsUser.h"

#include "icatHighLevelRoutines.h"
#include "icatMidLevelRoutines.h"

#include <string.h>

extern icatSessionStruct *chlGetRcs();


/*
int testCml(rsComm_t *rsComm)
{
   return(cmlTest(rsComm));
}
*/

rodsEnv myEnv;

int testRegUser(rsComm_t *rsComm, char *name, 
		   char *zone, char *userType, char *dn) {
   userInfo_t userInfo;

   strncpy(userInfo.userName, name, NAME_LEN);
   strncpy(userInfo.rodsZone, zone, NAME_LEN);
   strncpy(userInfo.userType, userType, NAME_LEN);
   strncpy(userInfo.authInfo.authStr, dn, NAME_LEN);

   /* no longer used   return(chlRegUser(rsComm, &userInfo)); */
   return(0);
}

int testRegRule(rsComm_t *rsComm, char *name) {
   ruleExecSubmitInp_t ruleInfo;

   memset(&ruleInfo,0,sizeof(ruleInfo));

   strncpy(ruleInfo.ruleName, name, MAX_NAME_LEN);

   strncpy(ruleInfo.reiFilePath,"../config/packedRei/rei.file1", NAME_LEN);

   strncpy(ruleInfo.userName,"Wayne", NAME_LEN);
   strncpy(ruleInfo.exeAddress, "Bermuda", NAME_LEN);
   strncpy(ruleInfo.exeTime,"whenEver", NAME_LEN);
   strncpy(ruleInfo.exeFrequency,"every 2 days", NAME_LEN);
   strncpy(ruleInfo.priority,"high", NAME_LEN);
   strncpy(ruleInfo.estimateExeTime,"2 hours", NAME_LEN);
   strncpy(ruleInfo.notificationAddr,"noone@nowhere.com", NAME_LEN);

   return(chlRegRuleExec(rsComm, &ruleInfo));
}

int testRename(rsComm_t *rsComm, char *id, char *newName) {
   rodsLong_t intId;
   int status;
   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   rsComm->proxyUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   intId = strtoll(id, 0, 0);
   status = chlRenameObject(rsComm, intId, newName);
   if (status) return(status);
   return(chlCommit(rsComm));
}

int testLogin(rsComm_t *rsComm, char *User, char *pw, char *pw1) {
   int status, stat2;
   rcComm_t *Conn;
   rErrMsg_t errMsg;

   Conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
                     myEnv.rodsZone, 0, &errMsg);
   if (Conn == NULL) {
      printf("rcConnect failure");
      return -1;
   }

#if 0
     Conn->clientUser.authInfo.authFlag = 0;
     Conn->proxyUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
     rstrcpy (Conn->clientUser.userName, privUser, NAME_LEN);
#endif

   status = clientLoginWithPassword(Conn, pw1);  /* first login as self */
   if (status ==0) {
      rstrcpy (Conn->clientUser.userName, User, NAME_LEN);
      rstrcpy (Conn->clientUser.rodsZone, myEnv.rodsZone, NAME_LEN); /* default
								to our zone */
      status = clientLoginWithPassword(Conn, pw);  /* then try other user */
   }

   stat2 = rcDisconnect(Conn);

   return(status);
}

int testMove(rsComm_t *rsComm, char *id, char *destId) {
   rodsLong_t intId, intDestId;
   int status;
   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   rsComm->proxyUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   intId = strtoll(id, 0, 0);
   intDestId = strtoll(destId, 0, 0);
   status = chlMoveObject(rsComm, intId, intDestId);
   if (status) return(status);
   return(chlCommit(rsComm));
}

int testTempPw(rsComm_t *rsComm) {
   int status;
   char pwValueToHash[500];
   status = chlMakeTempPw(rsComm, pwValueToHash);
   printf("pwValueToHash: %s\n", pwValueToHash);

   return(status);
}

int testTempPwConvert(char *s1, char *s2) {
   char md5Buf[100];
   unsigned char digest[RESPONSE_LEN+2];
   char digestStr[100];
   MD5_CTX context;

   /* 
      Calcuate the temp password: a hash of s1 (the user's main
      password) and s2 (the value returned by chlGenTempPw).
   */

   memset(md5Buf, 0, sizeof(md5Buf));
   strncpy(md5Buf, s2, 100);
   strncat(md5Buf, s1, 100);

   MD5Init (&context);
   MD5Update (&context, md5Buf, 100);
   MD5Final (digest, &context);

   md5ToStr(digest, digestStr);
   printf("digestStr (derived temp pw)=%s\n", digestStr);

   return(0);
}


int testTempPwCombined(rsComm_t *rsComm, char *s1) {
   int status;
   char pwValueToHash[500];
   char md5Buf[100];
   unsigned char digest[RESPONSE_LEN+2];
   char digestStr[100];
   MD5_CTX context;

   status = chlMakeTempPw(rsComm, pwValueToHash);
   if (status) return(status);

   printf("pwValueToHash: %s\n", pwValueToHash);

   /* 
      Calcuate the temp password: a hash of s1 (the user's main
      password) and the value returned by chlGenTempPw.
   */

   memset(md5Buf, 0, sizeof(md5Buf));
   strncpy(md5Buf, pwValueToHash, 100);
   strncat(md5Buf, s1, 100);

   MD5Init (&context);
   MD5Update (&context, md5Buf, 100);
   MD5Final (digest, &context);

   md5ToStr(digest, digestStr);
   printf("digestStr (derived temp pw)=%s\n", digestStr);

   return(0);
}
int testCheckAuth(rsComm_t *rsComm, char *testAdminUser,  char *testUser,
		  char *testUserZone) {
   /* Use an pre-determined user, challenge and resp */

   char response[RESPONSE_LEN+2];
   char challenge[CHALLENGE_LEN+2];
   int userPrivLevel;
   int clientPrivLevel;
   int status, i;
   char userNameAndZone[NAME_LEN*2];

   strncpy(rsComm->clientUser.userName, testUser, NAME_LEN);
   strncpy(rsComm->clientUser.rodsZone, testUserZone, NAME_LEN);

   for (i=0;i<CHALLENGE_LEN+2;i++) challenge[i]=' ';

   i=0;
   response[i++]=0xd6; /* found to be a valid response */
   response[i++]=0x8a;
   response[i++]=0xaf;
   response[i++]=0xc4;
   response[i++]=0x83;
   response[i++]=0x46;
   response[i++]=0x1b;
   response[i++]=0xa2;
   response[i++]=0x5c;
   response[i++]=0x8c;
   response[i++]=0x6d;
   response[i++]=0xc5;
   response[i++]=0xb1;
   response[i++]=0x41;
   response[i++]=0x84;
   response[i++]=0xeb;
   response[i++]=0x00;

   strncpy(userNameAndZone, testAdminUser, NAME_LEN);
   strncat(userNameAndZone, "#", NAME_LEN);
   strncat(userNameAndZone, testUserZone, NAME_LEN);

   status = chlCheckAuth(rsComm, challenge, response,
			 userNameAndZone,
			 &userPrivLevel, &clientPrivLevel);

   if (status == 0) {
      printf("clientPrivLevel=%d\n",clientPrivLevel);
   }
   return(status);

}

int testDelUser(rsComm_t *rsComm, char *name, char *zone) {
   userInfo_t userInfo;

   strncpy(userInfo.userName, name, NAME_LEN);
   strncpy(userInfo.rodsZone, zone, NAME_LEN);

   /* no more  return(chlDelUser(rsComm, &userInfo)); */
   return(0);
}

int testDelFile(rsComm_t *rsComm, char *name, char *replica) {
   dataObjInfo_t dataObjInfo;
   keyValPair_t *condInput;

   memset(&dataObjInfo, 0, sizeof(dataObjInfo));
   if (replica != NULL && *replica != 0) {
      int ireplica;
      ireplica = atoi(replica); 
      if (ireplica >= 0) {
	 dataObjInfo.replNum = ireplica;
      }
      if (ireplica == 999999) {
	 dataObjInfo.replNum = -1;
      }
   }
   strncpy(dataObjInfo.objPath, name, NAME_LEN);

   memset (&condInput, 0, sizeof (condInput));

   return(chlUnregDataObj(rsComm, &dataObjInfo, condInput));
}

int testDelFilePriv(rsComm_t *rsComm, char *name, char *dataId, 
		    char *replica) {
   dataObjInfo_t dataObjInfo;
   keyValPair_t condInput;

   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   memset (&condInput, 0, sizeof (condInput));
   addKeyVal(&condInput, IRODS_ADMIN_KW, " ");

   memset(&dataObjInfo, 0, sizeof(dataObjInfo));

   if (dataId != NULL && *dataId != 0) {
      rodsLong_t idataId;
      idataId = strtoll(dataId, NULL, 0); 
      if (idataId >= 0) {
	 dataObjInfo.dataId = idataId;
      }
   }
   dataObjInfo.replNum = -1;
   if (replica != NULL && *replica != 0) {
      int ireplica;
      ireplica = atoi(replica); 
      if (ireplica >= 0) {
	 dataObjInfo.replNum = ireplica;
      }
   }
   strncpy(dataObjInfo.objPath, name, NAME_LEN);

   return(chlUnregDataObj(rsComm, &dataObjInfo, &condInput));
}

int testDelFileTrash(rsComm_t *rsComm, char *name, char *dataId) {
   dataObjInfo_t dataObjInfo;
   keyValPair_t condInput;

   char *replica=0;

   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   memset (&condInput, 0, sizeof (condInput));
   addKeyVal(&condInput, IRODS_ADMIN_RMTRASH_KW, " ");

   memset(&dataObjInfo, 0, sizeof(dataObjInfo));

   if (dataId != NULL && *dataId != 0) {
      rodsLong_t idataId;
      idataId = strtoll(dataId, NULL, 0); 
      if (idataId >= 0) {
	 dataObjInfo.dataId = idataId;
      }
   }
   dataObjInfo.replNum = -1;
   if (replica != NULL && *replica != 0) {
      int ireplica;
      ireplica = atoi(replica); 
      if (ireplica >= 0) {
	 dataObjInfo.replNum = ireplica;
      }
   }
   strncpy(dataObjInfo.objPath, name, NAME_LEN);

   return(chlUnregDataObj(rsComm, &dataObjInfo, &condInput));
}

int testRegColl(rsComm_t *rsComm, char *name) {

   collInfo_t collInp;

   strncpy(collInp.collName, name, MAX_NAME_LEN);

   return(chlRegColl(rsComm, &collInp));
}

int testDelColl(rsComm_t *rsComm, char *name) {

   collInfo_t collInp;

   strncpy(collInp.collName, name, MAX_NAME_LEN);

   return(chlDelColl(rsComm, &collInp));
}

int testDelRule(rsComm_t *rsComm, char *ruleName, char *userName) {
   if (userName!=NULL && strlen(userName)>0) {
      rsComm->clientUser.authInfo.authFlag = LOCAL_USER_AUTH;
      rsComm->proxyUser.authInfo.authFlag = LOCAL_USER_AUTH;
      strncpy(rsComm->clientUser.userName, userName, MAX_NAME_LEN);
   }
   else {
      rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
      rsComm->proxyUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   }
   return(chlDelRuleExec(rsComm, ruleName));
}

int testRegDataObj(rsComm_t *rsComm, char *name, 
		   char *dataType, char *filePath) {
   dataObjInfo_t dataObjInfo;
   memset(&dataObjInfo,0,sizeof(dataObjInfo_t));

   strcpy(dataObjInfo.objPath, name);
   dataObjInfo.replNum=1;
   strcpy(dataObjInfo.version, "12");
   strcpy(dataObjInfo.dataType, dataType);
   dataObjInfo.dataSize=42;

   strcpy(dataObjInfo.rescName, "demoResc");
   strcpy(dataObjInfo.filePath, filePath);

   dataObjInfo.replStatus=5;

   return (chlRegDataObj(rsComm, &dataObjInfo));
}

/*
 Do multiple data registrations.  If you comment out the commit in
 chlRegDataObj and then build this, it can add phony data-objects at
 about 8 times the speed of lots of iput's of small files.  This can
 come in handy for creating simulated large instances for DBMS
 performance testing and tuning.  In this source file, you might also
 want to change rodsLogLevel(LOG_NOTICE) to rodsLogLevel(LOG_ERROR)
 and comment out rodsLogSqlReq(1);.
 */
int testRegDataMulti(rsComm_t *rsComm, char *count, 
		     char *nameBase,  char *dataType, char *filePath) {
   int status;
   int myCount=100;
   int i;
   char myName[MAX_NAME_LEN];

   myCount = atoi(count);
   if (myCount <=0) {
      printf("Invalid input: count\n");
      return(USER_INPUT_OPTION_ERR);
   }

   for (i=0;i<myCount;i++) {
      snprintf (myName, MAX_NAME_LEN, "%s.%d", nameBase, i);
      status = testRegDataObj(rsComm, myName, dataType, filePath);
      if (status) return(status);
   }

   status = chlCommit(rsComm);
   return(status);
}

int testModDataObjMeta(rsComm_t *rsComm, char *name, 
		       char *dataType, char *filePath) {
   dataObjInfo_t dataObjInfo;
   int status;
   keyValPair_t regParam;
   char tmpStr[LONG_NAME_LEN], tmpStr2[LONG_NAME_LEN];
   /*   int replStatus; */

   memset(&dataObjInfo,0,sizeof(dataObjInfo_t));

   memset (&regParam, 0, sizeof (regParam));

   /*
   replStatus=1;
   snprintf (tmpStr, LONG_NAME_LEN, "%d", replStatus);
   addKeyVal (&regParam, "replStatus", tmpStr);
   */
   snprintf (tmpStr, LONG_NAME_LEN, "'now'");
   addKeyVal (&regParam, "dataCreate", tmpStr);

   snprintf (tmpStr2, LONG_NAME_LEN, "'test comment'");
   addKeyVal (&regParam, "dataComments", tmpStr2);

   strcpy(dataObjInfo.objPath, name);
   /*   dataObjInfo.replNum=1; */
   dataObjInfo.replNum=0;

   strcpy(dataObjInfo.version, "12");
   strcpy(dataObjInfo.dataType, dataType);
   dataObjInfo.dataSize=42;

   strcpy(dataObjInfo.rescName, "resc A");

   strcpy(dataObjInfo.filePath, filePath);

   dataObjInfo.replStatus=5;

   status = chlModDataObjMeta(rsComm, &dataObjInfo, &regParam);


   return(status);
}

int testModDataObjMeta2(rsComm_t *rsComm, char *name, 
		       char *dataType, char *filePath) {
   dataObjInfo_t dataObjInfo;
   int status;
   keyValPair_t regParam;
   char tmpStr[LONG_NAME_LEN], tmpStr2[LONG_NAME_LEN];

   memset(&dataObjInfo,0,sizeof(dataObjInfo_t));

   memset (&regParam, 0, sizeof (regParam));

   snprintf (tmpStr, LONG_NAME_LEN, "whatever");
   addKeyVal (&regParam, "all", tmpStr);

   snprintf (tmpStr2, LONG_NAME_LEN, "42");
   addKeyVal (&regParam, "dataSize", tmpStr2);

   strcpy(dataObjInfo.objPath, name);
   dataObjInfo.replNum=0;
   strcpy(dataObjInfo.version, "12");
   strcpy(dataObjInfo.dataType, dataType);
   dataObjInfo.dataSize=42;

   strcpy(dataObjInfo.rescName, "resc A");

   strcpy(dataObjInfo.filePath, filePath);

   dataObjInfo.replStatus=5;

   status = chlModDataObjMeta(rsComm, &dataObjInfo, &regParam);


   return(status);
}

int testModColl(rsComm_t *rsComm, char *name, char *type, 
		       char *info1, char *info2) {
   int status;
   collInfo_t collInp;

   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   rsComm->proxyUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;

   memset(&collInp,0,sizeof(collInp));

   if (name!=NULL && strlen(name)>0) {
      strncpy(collInp.collName, name, MAX_NAME_LEN);
   }
   if (type!=NULL && strlen(type)>0) {
      strncpy(collInp.collType, type, MAX_NAME_LEN);
   }
   if (info1!=NULL && strlen(info1)>0) {
      strncpy(collInp.collInfo1, info1, MAX_NAME_LEN);
   }
   if (info2!=NULL && strlen(info2)>0) {
      strncpy(collInp.collInfo2, info2, MAX_NAME_LEN);
   }

   status = chlModColl(rsComm, &collInp);

   if (status != 0)  return(status);

   status = chlCommit(rsComm);
   return(status);
}

int testModRuleMeta(rsComm_t *rsComm, char *id, 
 		       char *attrName, char *attrValue) {
   /*   ruleExecSubmitInp_t ruleInfo; */
   char ruleId[100];
   int status;
   keyValPair_t regParam;
   char tmpStr[LONG_NAME_LEN];

   /*   memset(&ruleInfo,0,sizeof(ruleExecSubmitInp_t)); */

   memset (&regParam, 0, sizeof (regParam));

   rstrcpy (tmpStr, attrValue, LONG_NAME_LEN);

   addKeyVal (&regParam, attrName, tmpStr);

   strcpy(ruleId, id);

   status = chlModRuleExec(rsComm, ruleId, &regParam);

   return(status);
}

int testModResourceFreeSpace(rsComm_t *rsComm, char *rescName, 
		       char *numberString, char *option) {
   int number, status;
   if (*numberString=='\\') numberString++;
   number = atoi(numberString);
   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   rsComm->proxyUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   status = chlModRescFreeSpace(rsComm, rescName, number);
   if (status != 0)  return(status);
   if (option != NULL && strcmp(option, "rollback")==0 ) {
      status = chlRollback(rsComm);
   }
   if (option != NULL && strcmp(option, "close")==0 ) {
      status = chlClose();
      return(status);
   }
   status = chlCommit(rsComm);
   return(status);
}

int testRegReplica(rsComm_t *rsComm, char *srcPath, char *srcDataId, 
		       char *srcReplNum, char *dstPath) {
   dataObjInfo_t srcDataObjInfo;
   dataObjInfo_t dstDataObjInfo;
   keyValPair_t condInput;
   int status;

   memset(&srcDataObjInfo,0,sizeof(dataObjInfo_t));
   memset(&dstDataObjInfo,0,sizeof(dataObjInfo_t));
   memset(&condInput,0,sizeof(condInput));

   strcpy(srcDataObjInfo.objPath, srcPath);
   srcDataObjInfo.dataId=atoi(srcDataId);
   srcDataObjInfo.replNum=atoi(srcReplNum);


   strcpy(dstDataObjInfo.rescName, "resc A");
   strcpy(dstDataObjInfo.rescGroupName, "resc A");
   strcpy(dstDataObjInfo.filePath, dstPath);

   dstDataObjInfo.replStatus=5;

   status = chlRegReplica(rsComm, &srcDataObjInfo, &dstDataObjInfo,
    &condInput);
   return(status);
}

int testSimpleQ(rsComm_t *rsComm, char *sql, char *arg1, char *format) {
   char bigBuf[1000];
   int status;
   int control;
   int form;

   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
   rsComm->proxyUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;

   control=0;
   form = 1;
   if (format != NULL) form = atoi(format);

   status = chlSimpleQuery(rsComm, sql, arg1, 0, 0, 0, 
                           form, &control, bigBuf, 1000);
   if (status==0) printf("%s",bigBuf);

   while (control && (status==0) ) {
      status = chlSimpleQuery(rsComm, sql, 0, 0, 0, 0,
			      form, &control, bigBuf, 1000);
      if (status==0) printf("%s",bigBuf);
   }
   return(status);
}

int testChmod(rsComm_t *rsComm, char *user, char *zone, 
	      char *access, char *path) {
   int status;
   status = chlModAccessControl(rsComm, 0, user, zone, access, path);

   return(status);
}

int testServerLoad(rsComm_t *rsComm, char *option) {
   int status;

   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;

   status = chlRegServerLoad(rsComm, "host", "resc", option, "2", "3", 
			     "4", "5", "6", "7");
   return(status);
}

int testPurgeServerLoad(rsComm_t *rsComm, char *option) {
   int status;

   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;

   if (option == NULL) {
      status = chlPurgeServerLoad(rsComm, "2000");
   }
   else {
      status = chlPurgeServerLoad(rsComm, option);
   }

   return(status);
}

int testServerLoadDigest(rsComm_t *rsComm, char *option) {
   int status;

   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;

   status = chlRegServerLoadDigest(rsComm, "resc", option);
   return(status);
}

int testPurgeServerLoadDigest(rsComm_t *rsComm, char *option) {
   int status;

   rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;

   if (option == NULL) {
      status = chlPurgeServerLoadDigest(rsComm, "2000");
   }
   else {
      status = chlPurgeServerLoadDigest(rsComm, option);
   }

   return(status);
}

rodsLong_t
testCurrent(rsComm_t *rsComm) {
   rodsLong_t status;
   icatSessionStruct *icss;

   icss = chlGetRcs();

   status = cmlGetCurrentSeqVal(icss);
   return(status);
}

int
main(int argc, char **argv) {
   int status;
   rsComm_t *Comm;
   /*   rErrMsg_t errMsg;*/
   rodsArguments_t myRodsArgs;
   char *mySubName;
   char *myName;
   int didOne;
   rodsServerConfig_t serverConfig;

   Comm = malloc (sizeof (rsComm_t));
   memset (Comm, 0, sizeof (rsComm_t));

   parseCmdLineOpt(argc, argv, "", 0, &myRodsArgs);

   rodsLogLevel(LOG_NOTICE);

   rodsLogSqlReq(1);

   if (argc < 2) {
      printf("Usage: test_chl testName [args...]\n");
      exit(3);
   }

   status = getRodsEnv (&myEnv);
   if (status < 0) {
      rodsLog (LOG_ERROR, "main: getRodsEnv error. status = %d",
	       status);
      exit (1);
   }

   if (strstr(myEnv.rodsDebug, "CAT") != NULL) {
      chlDebug(myEnv.rodsDebug);
   }

   memset(&serverConfig, 0, sizeof(serverConfig));
   status = readServerConfig(&serverConfig);

   strncpy(Comm->clientUser.userName, myEnv.rodsUserName, NAME_LEN);
   strncpy(Comm->clientUser.rodsZone, myEnv.rodsZone, NAME_LEN);

   /*
   char rodsUserName[NAME_LEN];
   char rodsZone[NAME_LEN];

     userInfo_t clientUser;
    char userName[NAME_LEN];
    char rodsZone[NAME_LEN];
   */
   if ((status = chlOpen(serverConfig.DBUsername,
			 serverConfig.DBPassword)) != 0) {
        rodsLog (LOG_SYS_FATAL,
		 "initInfoWithRcat: chlopen Error. Status = %d",
		 status);
        return (status);
   }

   didOne=0;
   if (strcmp(argv[1],"reg")==0) {
      status = testRegDataObj(Comm, argv[2], argv[3], argv[4]);
      didOne=1;
   }
   if (strcmp(argv[1],"regmulti")==0) {
      status = testRegDataMulti(Comm, argv[2], argv[3], argv[4], argv[5]);
      didOne=1;
   }

   if (strcmp(argv[1],"mod")==0) {
      status = testModDataObjMeta(Comm, argv[2], argv[3], argv[4]);
      didOne=1;
   }

   if (strcmp(argv[1],"mod2")==0) {
      status = testModDataObjMeta2(Comm, argv[2], argv[3], argv[4]);
      didOne=1;
   }

   if (strcmp(argv[1],"modr")==0) {
      status = testModRuleMeta(Comm, argv[2], argv[3], argv[4]);
      didOne=1;
   }

   if (strcmp(argv[1],"modc")==0) {
      status = testModColl(Comm, argv[2], argv[3], argv[4], argv[5]);
      didOne=1;
   }

   if (strcmp(argv[1],"rmrule")==0) {
      status = testDelRule(Comm, argv[2], argv[3]);
      didOne=1;
   }

   if (strcmp(argv[1],"modrfs")==0) { 
      status = testModResourceFreeSpace(Comm, argv[2], argv[3], argv[4]);
      didOne=1;
   }

   if (strcmp(argv[1],"rep")==0) {
      if (argc < 6) {
	 printf("too few arguments\n");
	 exit(1);
      }
      status = testRegReplica(Comm, argv[2], argv[3], argv[4], argv[5]);
      didOne=1;
   }

   /*
   if (strcmp(argv[1],"cml")==0) {
      status = testCml(Comm);
      didOne=1;
   }
   */

   if (strcmp(argv[1],"rmuser")==0) {
      status = testDelUser(Comm, argv[2], argv[3]);
      didOne=1;
   }

   if (strcmp(argv[1],"mkdir")==0) {
      status = testRegColl(Comm, argv[2]);
      didOne=1;
   }

   if (strcmp(argv[1],"rmdir")==0) {
      status = testDelColl(Comm, argv[2]);
      didOne=1;
   }

   if (strcmp(argv[1],"sql")==0) {
      status = testSimpleQ(Comm, argv[2], argv[3], argv[4]);
      didOne=1;
   }

   if (strcmp(argv[1],"rm")==0) {
      status = testDelFile(Comm, argv[2], argv[3]);
      didOne=1;
   }

   if (strcmp(argv[1],"rmtrash")==0) {
      status = testDelFileTrash(Comm, argv[2], argv[3]);
      didOne=1;
   }

   if (strcmp(argv[1],"rmpriv")==0) {
      status = testDelFilePriv(Comm, argv[2], argv[3], argv[4]);
      didOne=1;
   }

   if (strcmp(argv[1],"chmod")==0) {
      status = testChmod(Comm, argv[2], argv[3], argv[4], argv[5]);
      didOne=1;
   }

   if (strcmp(argv[1],"regrule")==0) {
      status = testRegRule(Comm, argv[2]);
      didOne=1;
   }

   if (strcmp(argv[1],"rename")==0) {
      status = testRename(Comm, argv[2], argv[3]);
      testCurrent(Comm);  /* exercise this as part of rename;
                             testCurrent needs a SQL context */
      didOne=1;
   }

   if (strcmp(argv[1],"login")==0) {
      status = testLogin(Comm, argv[2], argv[3], argv[4]);
      didOne=1;
   }

   if (strcmp(argv[1],"move")==0) {
      status = testMove(Comm, argv[2], argv[3]);
      didOne=1;
   }

   if (strcmp(argv[1],"checkauth")==0) {
      status = testCheckAuth(Comm, argv[2], argv[3], argv[4]);
      didOne=1;
   }

   if (strcmp(argv[1],"temppw")==0) {
      status = testTempPw(Comm);
      didOne=1;
   }

   if (strcmp(argv[1],"tpc")==0) {
      status = testTempPwConvert(argv[2], argv[3]);
      didOne=1;
   }

   if (strcmp(argv[1],"tpw")==0) {
      status = testTempPwCombined(Comm, argv[2]);
      didOne=1;
   }

   if (strcmp(argv[1],"serverload")==0) {
      status = testServerLoad(Comm, argv[2]);
      didOne=1;
   }

   if (strcmp(argv[1],"purgeload")==0) {
      status = testPurgeServerLoad(Comm, argv[2]);
      didOne=1;
   }

   if (strcmp(argv[1],"serverdigest")==0) {
      status = testServerLoadDigest(Comm, argv[2]);
      didOne=1;
   }

   if (strcmp(argv[1],"purgedigest")==0) {
      status = testPurgeServerLoadDigest(Comm, argv[2]);
      didOne=1;
   }

   if (strcmp(argv[1],"open")==0) {
      int i;
      for (i=0;i<3;i++) {
	 status = chlClose();
	 if (status) {
	    printf ("close %d error", i);
	 }
      
	 if ((status = chlOpen(serverConfig.DBUsername,
			       serverConfig.DBPassword)) != 0) {
	    rodsLog (LOG_SYS_FATAL,
		     "initInfoWithRcat: chlopen %d Error. Status = %d",
		     i, status);
	    return (status);
	 }
      }
      didOne=1;
   }


   if (status != 0) {
      /*
      if (Comm->rError) {
	 rError_t *Err;
         rErrMsg_t *ErrMsg;
	 int i, len;
	 Err = Comm->rError;
	 len = Err->len;
	 for (i=0;i<len;i++) {
	    ErrMsg = Err->errMsg[i];
	    rodsLog(LOG_ERROR, "Level %d: %s",i, ErrMsg->msg);
	 }
      }
      */
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "%s failed with error %d %s %s", argv[1],
	       status, myName, mySubName);
   }
   else {
      if (didOne) printf("Completed successfully\n");
   }

   if (didOne==0) {
      printf("Unknown test type: %s\n", argv[1]);
   }

   exit(status);
}
