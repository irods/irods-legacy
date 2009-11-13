/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
  ICAT test program for the GeneralUpdate.
*/

#include "rodsClient.h"
#include "readServerConfig.h"

#include "icatHighLevelRoutines.h"
#include "rodsGeneralUpdate.h"

int sTest(int i1, int i2);
int sTest2(int i1, int i2, int i3);
int findCycles(int startTable);

int
doTest1(char *arg1, char *arg2, char *arg3, char *arg4) {
    generalUpdateInp_t generalUpdateInp;
    int status;

    printf("dotest1\n");
    rodsLogSqlReq(1);

    memset (&generalUpdateInp, 0, sizeof (generalUpdateInp));

    addInxVal (&generalUpdateInp.values, COL_TOKEN_NAMESPACE, arg1);
    addInxVal (&generalUpdateInp.values, COL_TOKEN_ID, arg2);
    addInxVal (&generalUpdateInp.values, COL_TOKEN_NAME, arg3);

    generalUpdateInp.type = GENERAL_UPDATE_INSERT;

    status  = chlGeneralUpdate(generalUpdateInp);
    printf("chlGenUpdate status=%d\n",status);

    return(status);
}

int
doTest2(char *arg1, char *arg2, char *arg3, char *arg4) {
    generalUpdateInp_t generalUpdateInp;
    int status;

    printf("dotest2\n");
    rodsLogSqlReq(1);

    memset (&generalUpdateInp, 0, sizeof (generalUpdateInp));

    addInxVal (&generalUpdateInp.values, COL_TOKEN_NAMESPACE, arg1);
    addInxVal (&generalUpdateInp.values, COL_TOKEN_ID, arg2);
    addInxVal (&generalUpdateInp.values, COL_TOKEN_NAME, arg3);

    generalUpdateInp.type = GENERAL_UPDATE_DELETE;

    status  = chlGeneralUpdate(generalUpdateInp);
    printf("chlGenUpdate status=%d\n",status);

    return(status);
}

int
doTest3(char *arg1, char *arg2, char *arg3, char *arg4) {
    generalUpdateInp_t generalUpdateInp;
    int status;

    printf("dotest3\n");
    rodsLogSqlReq(1);

    memset (&generalUpdateInp, 0, sizeof (generalUpdateInp));

    addInxVal (&generalUpdateInp.values, COL_RULE_ID, GU_NEXT_SEQ_VALUE);
    addInxVal (&generalUpdateInp.values, COL_RULE_NAME, arg1);
    addInxVal (&generalUpdateInp.values, COL_RULE_BASE_NAME, arg2);
    addInxVal (&generalUpdateInp.values, COL_RULE_BODY, arg3);

    addInxVal (&generalUpdateInp.values, COL_RULE_EVENT, "e");
    addInxVal (&generalUpdateInp.values, COL_RULE_RECOVERY, "r");
    addInxVal (&generalUpdateInp.values, COL_RULE_OWNER_NAME, "on");
    addInxVal (&generalUpdateInp.values, COL_RULE_OWNER_ZONE, "oz");

    addInxVal (&generalUpdateInp.values, COL_RULE_CREATE_TIME, GU_NOW_TIME);
    addInxVal (&generalUpdateInp.values, COL_RULE_MODIFY_TIME, GU_NOW_TIME);

    generalUpdateInp.type = GENERAL_UPDATE_INSERT;

    status  = chlGeneralUpdate(generalUpdateInp);
    printf("chlGenUpdate status=%d\n",status);

    return(status);
}

int
doTest4(char *arg1, char *arg2, char *arg3, char *arg4) {
    generalUpdateInp_t generalUpdateInp;
    int status;

    printf("dotest2\n");
    rodsLogSqlReq(1);

    memset (&generalUpdateInp, 0, sizeof (generalUpdateInp));

    addInxVal (&generalUpdateInp.values, COL_RULE_ID, arg1);
    if (arg2 !=0 && *arg2 !='\0') {
       addInxVal (&generalUpdateInp.values, COL_RULE_NAME, arg2);
    }

    generalUpdateInp.type = GENERAL_UPDATE_DELETE;

    status  = chlGeneralUpdate(generalUpdateInp);
    printf("chlGenUpdate status=%d\n",status);

    return(status);
}

int
main(int argc, char **argv) {
   int mode;
   rodsServerConfig_t serverConfig;
   int status;
   rodsEnv myEnv;

   /* remove this call or change to LOG_NOTICE for more verbosity */
   rodsLogLevel(LOG_ERROR);

   /* this will cause the sql to be printed, comment this out to skip it  */
   rodsLogSqlReq(1);

   mode = 0;
   if (strcmp(argv[1],"1")==0) mode=1;
   if (strcmp(argv[1],"2")==0) mode=2;
   if (strcmp(argv[1],"3")==0) mode=3;
   if (strcmp(argv[1],"4")==0) mode=4;

   memset((char *)&myEnv, 0, sizeof(myEnv));
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

   if ((status = chlOpen(serverConfig.DBUsername,
			 serverConfig.DBPassword)) != 0) {
      rodsLog (LOG_SYS_FATAL,
	       "chlopen Error. Status = %d",
	       status);
      return (status);
   }
   if (mode==1) {
      status = doTest1(argv[2], argv[3], argv[4], argv[5]);
      if (status <0) exit(2);
      exit(0);
   }
   if (mode==2) {
      status = doTest2(argv[2], argv[3], argv[4], argv[5]);
      if (status <0) exit(2);
      exit(0);
   }
   if (mode==3) {
      status = doTest3(argv[2], argv[3], argv[4], argv[5]);
      if (status <0) exit(2);
      exit(0);
   }
   if (mode==4) {
      status = doTest4(argv[2], argv[3], argv[4], argv[5]);
      if (status <0) exit(2);
      exit(0);
   }
   exit(0);
}
