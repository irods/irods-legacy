
/*-------------------------------------------------------------------------
 *
 * irodsReServer.c--The irods Rule Execution server
 * 
 *
 *-------------------------------------------------------------------------
 */

#include "irodsReServer.h"
#include "objMetaOpr.h"
#include "rsApiHandler.h"
#include "rsIcatOpr.h"

extern int msiAdmClearAppRuleStruct(ruleExecInfo_t *rei);

int usage (char *prog);

int
main(int argc, char **argv)
{
    int status;
    int c;
    rsComm_t rsComm;
    int runMode = IRODS_SERVER;
    int flagval = 0;
    char *logDir = NULL;
    char *tmpStr;
    int logFd;

    ProcessType = RE_SERVER_PT;

#ifndef _WIN32
    signal(SIGINT, signalExit);
    signal(SIGHUP, signalExit);
    signal(SIGTERM, signalExit);
    signal(SIGUSR1, signalExit);
    signal(SIGPIPE, rsPipSigalHandler);

#endif

    /* Handle option to log sql commands */
    tmpStr = getenv (SP_LOG_SQL);
    if (tmpStr != NULL) {
       rodsLogSqlReq(1);
    }

    /* Set the logging level */
    tmpStr = getenv (SP_LOG_LEVEL);
    if (tmpStr != NULL) {
       int i;
       i = atoi(tmpStr);
       rodsLogLevel(i);
    } else {
         rodsLogLevel(LOG_NOTICE); /* default */
    }

    while ((c=getopt(argc, argv,"sScvD:")) != EOF) {
        switch (c) {
	    case 's':
		runMode = SINGLE_PASS;
		break;
	    case 'S':
		runMode = STANDALONE_SERVER;
		break;
            case 'v':   /* verbose */
                flagval |= v_FLAG;
                break;
            case 'D':   /* user specified a log directory */
		logDir = strdup (optarg);
		break;
            default:
                usage (argv[0]);
                exit (1);
        }
    }

    status = initRsComm (&rsComm);

    if (status < 0) {
        cleanupAndExit (status);
    }

    if ((logFd = logFileOpen (runMode, logDir, RULE_EXEC_LOGFILE)) < 0) {
        exit (1);
    }

    daemonize (runMode, logFd);

    status = initAgent (&rsComm);
    if (status < 0) {
        cleanupAndExit (status);
    }

    status = reServerMain (&rsComm);

    cleanupAndExit (status);

    exit (0);
}

int usage (char *prog)
{
    fprintf(stderr, "Usage: %s [-scva] [-D logDir] \n",prog);
    return 0;
}

int
reServerMain (rsComm_t *rsComm)
{
    int status = 0;
    genQueryOut_t *genQueryOut = NULL;
    time_t endTime;
    int runCnt;
    reExec_t reExec;
   
    initReExec (rsComm, &reExec);
    while (1) {
	chkAndResetRule (rsComm);
        rodsLog (LOG_NOTICE,
          "reServerMain: checking the queue for jobs");
        status = getReInfo (rsComm, &genQueryOut);
        if (status < 0) {
            if (status != CAT_NO_ROWS_FOUND) {
                rodsLog (LOG_ERROR,
                  "reServerMain: getReInfo error. status = %d", status);
            }
	    reSvrSleep (rsComm);
            continue;
        }
        endTime = time (NULL) + RE_SERVER_EXEC_TIME;
	runCnt = runQueuedRuleExec (rsComm, &reExec, &genQueryOut, endTime, 0);
	if (runCnt > 0 || 
	  (genQueryOut != NULL && genQueryOut->continueInx > 0)) {
	    /* need to refresh */
	    svrCloseQueryOut (rsComm, genQueryOut);
            freeGenQueryOut (&genQueryOut);
            status = getReInfo (rsComm, &genQueryOut);
            if (status < 0) {
		reSvrSleep (rsComm);
                continue;
	    }
        }

	/* run the failed job */

	runCnt = 
	  runQueuedRuleExec (rsComm, &reExec, &genQueryOut, endTime, 
	  RE_FAILED_STATUS);
	svrCloseQueryOut (rsComm, genQueryOut);
        freeGenQueryOut (&genQueryOut);
	if (runCnt > 0 ||
	  (genQueryOut != NULL && genQueryOut->continueInx > 0)) {
	    continue;
	} else {
	    /* nothing got run */
	    reSvrSleep (rsComm);
	}
    }
}

int
reSvrSleep (rsComm_t *rsComm)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;

    if ((status = disconnRcatHost (rsComm, MASTER_RCAT, 
      rsComm->myEnv.rodsZone)) == LOCAL_HOST) {
#ifdef RODS_CAT
#ifndef ORA_ICAT
       /* For Oracle, we don't disconnect.  This is to avoid a 
          memory leak in the OCI library */
        disconnectRcat (rsComm);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "reSvrSleep: disconnectRcat error. status = %d", status);
        }
#endif
#endif
    }
    rodsSleep (RE_SERVER_SLEEP_TIME, 0);

    if ((status = getAndConnRcatHost (rsComm, MASTER_RCAT, 
      rsComm->myEnv.rodsZone, &rodsServerHost)) == LOCAL_HOST) {
#ifdef RODS_CAT
        status = connectRcat (rsComm);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "reSvrSleep: connectRcat error. status = %d", status);
	}
#endif
    }
    return (status);
}

int
chkAndResetRule (rsComm_t *rsComm)
{
    char *configDir;
    char rulesFileName[MAX_NAME_LEN];
    struct stat statbuf;
    int status;
    ruleExecInfo_t rei;

    configDir = getConfigDir ();
    snprintf (rulesFileName, MAX_NAME_LEN, "%s/reConfigs/core.irb", 
      configDir); 
    status = stat (rulesFileName, &statbuf);

    if (status != 0) {
	status = UNIX_FILE_STAT_ERR - errno;
        rodsLog (LOG_ERROR,
          "chkAndResetRule: unable to read rule config file %s, status = %d",
	  rulesFileName, status);
	return (status);
    }

    if (CoreIrbTimeStamp == 0) {
	/* first time */
	CoreIrbTimeStamp = (uint) statbuf.st_mtime;
	return (0);
    }

    if ((uint) statbuf.st_mtime > CoreIrbTimeStamp) {
	/* file has been changed */
        rodsLog (LOG_NOTICE,
          "chkAndResetRule: reconf file %s has been changed. re-initializing",
	  rulesFileName);
	CoreIrbTimeStamp = (uint) statbuf.st_mtime;
	rei.rsComm = rsComm;
	msiAdmClearAppRuleStruct (&rei);
	status = initRuleEngine(reRuleStr, reFuncMapStr, reVariableMapStr);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "chkAndResetRule: initRuleEngine error, status = %d", status);
        }
    }
    return status;
}

