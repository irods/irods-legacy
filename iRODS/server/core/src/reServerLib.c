/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* reServerLib.c - functions for the irodsReServer */

#ifndef windows_platform
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include "reServerLib.h"
#if 0
#include "resource.h"
#include "collection.h"
#include "specColl.h"
#include "modDataObjMeta.h"
#endif
#include "ruleExecSubmit.h"
#include "ruleExecDel.h"
#include "genQuery.h"
#include "icatHighLevelRoutines.h"
#include "reSysDataObjOpr.h"
#include "miscUtil.h"
#include "rodsClient.h"
#include "rsIcatOpr.h"

int 
getReInfo (rsComm_t *rsComm, genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    int status;

    *genQueryOut = NULL;
    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    /*    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ID, 1); Raja Sep 8 2010 */
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ID, ORDER_BY);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_REI_FILE_PATH, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_USER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ADDRESS, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_FREQUENCY, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_PRIORITY, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ESTIMATED_EXE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_NOTIFICATION_ADDR, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_LAST_EXE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_STATUS, 1);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, genQueryOut);

    clearGenQueryInp (&genQueryInp);
    /* take care of mem leak */
    if (*genQueryOut != NULL) {
        if (status < 0) {
	    free (*genQueryOut);
	    *genQueryOut = NULL;
        } else {
            svrCloseQueryOut (rsComm, *genQueryOut);
	}
    }
    return (status);
}

int 
getReInfoById (rsComm_t *rsComm, char *ruleExecId, genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    char tmpStr[NAME_LEN];
    int status;

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_REI_FILE_PATH, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_USER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ADDRESS, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_FREQUENCY, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_PRIORITY, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ESTIMATED_EXE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_LAST_EXE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_STATUS, 1);

    snprintf (tmpStr, NAME_LEN, "='%s'", ruleExecId);
    addInxVal (&genQueryInp.sqlCondInp, COL_RULE_EXEC_ID, tmpStr);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, genQueryOut);

    clearGenQueryInp (&genQueryInp);

    return (status);
}

/* getNextQueuedRuleExec - get the next RuleExec in queue to run
 * jobType -   0 - run exeStatus = RE_IN_QUEUE and RE_RUNNING
 * 		  RE_FAILED_STATUS -  run the RE_FAILED too
 */
  
int
getNextQueuedRuleExec (rsComm_t *rsComm, genQueryOut_t **inGenQueryOut, 
int startInx, ruleExecSubmitInp_t *queuedRuleExec, 
reExec_t *reExec, int jobType)
{
    sqlResult_t *ruleExecId, *ruleName, *reiFilePath, *userName, *exeAddress,
      *exeTime, *exeFrequency, *priority, *lastExecTime, *exeStatus,
      *estimateExeTime, *notificationAddr;
    int i, status;
    genQueryOut_t *genQueryOut;

    if (inGenQueryOut == NULL || *inGenQueryOut == NULL ||
      queuedRuleExec == NULL || queuedRuleExec->packedReiAndArgBBuf == NULL ||
      queuedRuleExec->packedReiAndArgBBuf->buf == NULL) {
        rodsLog (LOG_ERROR,
          "getNextQueuedRuleExec: NULL input");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    genQueryOut = *inGenQueryOut;
    if ((ruleExecId = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_ID)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_ID failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((ruleName = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((reiFilePath = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_REI_FILE_PATH)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for REI_FILE_PATH failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((userName = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_USER_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for USER_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((exeAddress = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_ADDRESS)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_ADDRESS failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((exeTime = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_TIME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((exeFrequency = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_FREQUENCY)) == NULL) {
        rodsLog (LOG_NOTICE,
         "getNextQueuedRuleExec:getResultByInx for RULE_EXEC_FREQUENCY failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((priority = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_PRIORITY)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for PRIORITY failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((lastExecTime = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_LAST_EXE_TIME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for LAST_EXE_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((exeStatus = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_STATUS)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_STATUS failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((estimateExeTime = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_ESTIMATED_EXE_TIME)) == NULL) {
        rodsLog (LOG_NOTICE,
         "getNextQueuedRuleExec: getResultByInx for ESTIMATED_EXE_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((notificationAddr = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_NOTIFICATION_ADDR)) == NULL) {
        rodsLog (LOG_NOTICE,
         "getNextQueuedRuleExec:getResultByInx for NOTIFICATION_ADDR failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    for (i = startInx; i < genQueryOut->rowCnt; i++) {
	char *exeStatusStr, *exeTimeStr, *ruleExecIdStr;
        struct stat statbuf;
        int fd;
	

	exeStatusStr = &exeStatus->value[exeStatus->len * i];
	exeTimeStr = &exeTime->value[exeTime->len * i];
	ruleExecIdStr =  &ruleExecId->value[ruleExecId->len * i];
	
	if ((jobType & RE_FAILED_STATUS) == 0 && 
	  strcmp (exeStatusStr, RE_FAILED) == 0) {
	    /* failed request */
	    continue;
	} else if (atoi (exeTimeStr) > time (0)) {
	    /* not time yet */
	    continue;
        } else if (strcmp (exeStatusStr, RE_RUNNING) == 0) {
            /* is already running */
	    if (reExec->doFork == 1 &&	/* multiProc */
	     matchRuleExecId (reExec, ruleExecIdStr, RE_PROC_RUNNING)) {
	        /* the job is running in multiProc env */
		continue;
	    } else {
                rodsLog (LOG_NOTICE,
                 "getNextQueuedRuleExec: reId %s in RUNNING state. Run again",
                  ruleExecIdStr);
	    }
	}

        rstrcpy (queuedRuleExec->reiFilePath,
          &reiFilePath->value[reiFilePath->len * i], MAX_NAME_LEN);
	if (stat (queuedRuleExec->reiFilePath, &statbuf) < 0) {
	    status = UNIX_FILE_STAT_ERR - errno;
	    rodsLog (LOG_ERROR,
             "getNextQueuedRuleExec: stat error for rei file %s, status = %d",
	     queuedRuleExec->reiFilePath, status);
	    continue;
	}

        if (statbuf.st_size > queuedRuleExec->packedReiAndArgBBuf->len) {
	    free (queuedRuleExec->packedReiAndArgBBuf->buf);
	    queuedRuleExec->packedReiAndArgBBuf->buf = 
	      malloc ((int) statbuf.st_size);
	    queuedRuleExec->packedReiAndArgBBuf->len = statbuf.st_size;
	}

	fd = open (queuedRuleExec->reiFilePath, O_RDONLY,0);
	if (fd < 0) {
            status = UNIX_FILE_OPEN_ERR - errno;
            rodsLog (LOG_ERROR,
             "getNextQueuedRuleExec: open error for rei file %s, status = %d",
             queuedRuleExec->reiFilePath, status);
            return (status);
        }
 
	status = read (fd, queuedRuleExec->packedReiAndArgBBuf->buf, 
	  queuedRuleExec->packedReiAndArgBBuf->len);

	close (fd);
        if (status != statbuf.st_size) {
	    if (status < 0) {
                status = UNIX_FILE_READ_ERR - errno;
                rodsLog (LOG_ERROR,
                 "getNextQueuedRuleExec: read error for file %s, status = %d",
                 queuedRuleExec->reiFilePath, status);
	    } else {
                rodsLog (LOG_ERROR,
                 "getNextQueuedRuleExec:read error for %s,toRead %d, read %d",
                  queuedRuleExec->reiFilePath, 
	          queuedRuleExec->packedReiAndArgBBuf->len, status);
                return (SYS_COPY_LEN_ERR);
	    }
        }

	rstrcpy (queuedRuleExec->exeTime, exeTimeStr, NAME_LEN);
	rstrcpy (queuedRuleExec->exeStatus, exeStatusStr, NAME_LEN);
        rstrcpy (queuedRuleExec->ruleExecId, ruleExecIdStr, NAME_LEN);

	rstrcpy (queuedRuleExec->ruleName, 
	  &ruleName->value[ruleName->len * i], META_STR_LEN);
	rstrcpy (queuedRuleExec->userName, 
	  &userName->value[userName->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->exeAddress, 
	  &exeAddress->value[exeAddress->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->exeFrequency, 
	  &exeFrequency->value[exeFrequency->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->priority, 
	  &priority->value[priority->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->estimateExeTime, 
	  &estimateExeTime->value[estimateExeTime->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->notificationAddr, 
	  &notificationAddr->value[notificationAddr->len * i], NAME_LEN);
	return (i);
    }
    return (-1);
}

int
modExeInfoForRepeat(rsComm_t *rsComm, char *ruleExecId, char* pastTime, 
		      char *delay, int opStatus) {
    keyValPair_t *regParam;
    int status, status1;
    char myTimeNow[200];
    char myTimeNext[200];
	ruleExecModInp_t ruleExecModInp;
    ruleExecDelInp_t ruleExecDelInp;

    if (opStatus > 0) opStatus = 0;
  
    rstrcpy (myTimeNext, pastTime, 200);
    getOffsetTimeStr((char*)&myTimeNow, "                      ");

    status1 = getNextRepeatTime(myTimeNow, delay,myTimeNext);

    /***
    if (status != 0)
      return(status);
    rDelay = (atol(delay) * 60)  + atol(myTimeNow);
    sprintf(myTimeNext,"%lld", rDelay);
    ***/
    rodsLog (LOG_NOTICE,"modExeInfoForRepeat: rulId=%s,opStatus=%d,nextRepeatStatus=%d",ruleExecId,opStatus,status1);
    regParam = &(ruleExecModInp.condInput);
    rstrcpy (ruleExecModInp.ruleId, ruleExecId, NAME_LEN);
    memset (regParam, 0, sizeof (keyValPair_t));
    if (status1 == 0) {
      addKeyVal (regParam, RULE_EXE_STATUS_KW, "");
      addKeyVal (regParam, RULE_LAST_EXE_TIME_KW, myTimeNow);
      addKeyVal (regParam, RULE_EXE_TIME_KW, myTimeNext);
      status = rsRuleExecMod(rsComm, &ruleExecModInp);
    }
    else if (status1 == 1) {
      if (opStatus == 0) {
	/* entry remove  */
	rstrcpy (ruleExecDelInp.ruleExecId, ruleExecId, NAME_LEN);
	status = rsRuleExecDel (rsComm, &ruleExecDelInp);
      }
      else {
	addKeyVal (regParam, RULE_EXE_STATUS_KW, "");
	addKeyVal (regParam, RULE_LAST_EXE_TIME_KW, myTimeNow);
	addKeyVal (regParam, RULE_EXE_TIME_KW, myTimeNext);
	status = rsRuleExecMod(rsComm, &ruleExecModInp);
      }
    }
    else if (status1 == 2 ) {
      /* entry remove  */
      rstrcpy (ruleExecDelInp.ruleExecId, ruleExecId, NAME_LEN);
      status = rsRuleExecDel (rsComm, &ruleExecDelInp);
    }
    else if (status1 == 3 ) {
	addKeyVal (regParam, RULE_EXE_STATUS_KW, "");
	addKeyVal (regParam, RULE_LAST_EXE_TIME_KW, myTimeNow);
	addKeyVal (regParam, RULE_EXE_TIME_KW, myTimeNext);
	addKeyVal (regParam, RULE_EXE_FREQUENCY_KW,delay);
	status = rsRuleExecMod(rsComm, &ruleExecModInp);
    }
    else if (status1 == 4 ) {
      if (opStatus == 0) {
	/* entry remove  */
	rstrcpy (ruleExecDelInp.ruleExecId, ruleExecId, NAME_LEN);
	status = rsRuleExecDel (rsComm, &ruleExecDelInp);
      }
      else {
	addKeyVal (regParam, RULE_EXE_STATUS_KW, "");
	addKeyVal (regParam, RULE_LAST_EXE_TIME_KW, myTimeNow);
	addKeyVal (regParam, RULE_EXE_TIME_KW, myTimeNext);
	addKeyVal (regParam, RULE_EXE_FREQUENCY_KW,delay);
	status = rsRuleExecMod(rsComm, &ruleExecModInp);
      }
    }
    if (regParam->len > 0)
      clearKeyVal (regParam); 

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "modExeInfoForRepeat: rsRuleExecMod/rsRuleExecDel Error of id %s failed, status = %d",
          ruleExecId, status);
    }
    else {
      if (status1 == 3 || (status1 != 2 && opStatus != 0))
        rodsLog (LOG_NOTICE,
	      "Rule id %s set to run again at %s (frequency %s seconds)", 
		 ruleExecId, myTimeNext, delay);
    }

    return (status);
}

int
regExeStatus (rsComm_t *rsComm, char *ruleExecId, char *exeStatus)
{
    keyValPair_t *regParam;
    ruleExecModInp_t ruleExecModInp;
    int status;
    /*** RAJA July 24, 2007 changed chl call to rs call ***/
    regParam = &(ruleExecModInp.condInput);
    memset (regParam, 0, sizeof (keyValPair_t));
    rstrcpy (ruleExecModInp.ruleId, ruleExecId, NAME_LEN);
    addKeyVal (regParam, RULE_EXE_STATUS_KW, exeStatus);
    status = rsRuleExecMod(rsComm, &ruleExecModInp);


    clearKeyVal (regParam);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regExeStatus: rsRuleExecMod of id %s failed, status = %d",
          ruleExecId, status);
    }

    return (status);
}

/* runQueuedRuleExec - given the job queue given in genQueryOut (from
 * a getReInfo call), run the jobs with the input jobType.
 * Valid jobType = 0 ==> normal job. 
 * jobType = RE_FAILED_STATUS ==> job have failed as least once
 */

int
runQueuedRuleExec (rsComm_t *rsComm, reExec_t *reExec, 
genQueryOut_t **genQueryOut, time_t endTime, int jobType)
{
    int inx, status;
    ruleExecSubmitInp_t *myRuleExecInp;
    int runCnt = 0;
    int thrInx;

    inx = -1;
    while (time(NULL) <= endTime && (thrInx = allocReThr (reExec)) >= 0) {
	myRuleExecInp = &reExec->reExecProc[thrInx].ruleExecSubmitInp;
        if ((inx = getNextQueuedRuleExec (rsComm, genQueryOut, inx + 1,
          myRuleExecInp, reExec, jobType)) < 0) {
	    /* no job to run */
	    freeReThr (reExec, thrInx);
	    break;
	} else {
	    reExec->reExecProc[thrInx].jobType = jobType;
	}

        /* mark running */
        status = regExeStatus (rsComm, myRuleExecInp->ruleExecId, 
	  RE_RUNNING);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "runQueuedRuleExec: regExeStatus of id %s failed,stat = %d",
              myRuleExecInp->ruleExecId, status);
	    freeReThr (reExec, thrInx);
            continue;
        }

	runCnt ++;
	if (reExec->doFork == 0) {
	    /* single thread. Just call runRuleExec */  
	    status = runRuleExec (&reExec->reExecProc[thrInx]);
            postProcRunRuleExec (rsComm, &reExec->reExecProc[thrInx]);
            freeReThr (reExec, thrInx);
            continue;
	} else {
	    if ((reExec->reExecProc[thrInx].pid = fork()) == 0) {
		/* child. need to disconnect Rcat */ 
		rodsServerHost_t *rodsServerHost = NULL;

                if ((status = resetRcatHost (rsComm, MASTER_RCAT,
                 rsComm->myEnv.rodsZone)) == LOCAL_HOST) {
#ifdef RODS_CAT
                    resetRcat (rsComm);
#endif
		}
        	if ((status = getAndConnRcatHost (rsComm, MASTER_RCAT,
                 rsComm->myEnv.rodsZone, &rodsServerHost)) == LOCAL_HOST) {
#ifdef RODS_CAT
                    status = connectRcat (rsComm);
                    if (status < 0) {
                        rodsLog (LOG_ERROR,
                          "runQueuedRuleExec: connectRcat error. status=%d",
			  status);
        	    }
#endif
    		}
		seedRandom ();
		status = runRuleExec (&reExec->reExecProc[thrInx]);
                postProcRunRuleExec (rsComm, &reExec->reExecProc[thrInx]);
#ifdef RE_SERVER_DEBUG
		rodsLog (LOG_NOTICE,
		  "runQueuedRuleExec: process %d exiting", getpid ());
#endif
		if (reExec->reExecProc[thrInx].status >= 0) {
		    exit (0);
		} else {
		    exit (1);
		}
	    } else { 
#ifdef RE_SERVER_DEBUG
		rodsLog (LOG_NOTICE,
		  "runQueuedRuleExec: started proc %d, thrInx %d",
		  reExec->reExecProc[thrInx].pid, thrInx); 
#endif
	        /* parent fall through here */
	        reExec->runCnt++;
	        continue;
	    }
	}
    }
    if (reExec->doFork == 1) {
	/* wait for all jobs to finish */
	while (reExec->runCnt + 1 >= reExec->maxRunCnt && 
	  waitAndFreeReThr (reExec) >= 0);
    }

    return (runCnt);
}

#ifndef windows_platform
int
initReExec (rsComm_t *rsComm, reExec_t *reExec)
{
    int i;
    ruleExecInfo_t rei;
    int status;

    if (reExec == NULL) return SYS_INTERNAL_NULL_INPUT_ERR;

    bzero (reExec, sizeof (reExec_t));
    bzero (&rei, sizeof (ruleExecInfo_t)); /* RAJA ADDED June 17. 2009 */
    
    rei.rsComm = rsComm;
    status = applyRule ("acSetReServerNumProc", NULL, &rei, NO_SAVE_REI);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "initReExec: rule acSetReServerNumProc error, status = %d",
          status);
        reExec->maxRunCnt = 1;
        reExec->doFork = 0;
    } else {
	reExec->maxRunCnt = rei.status;
        if (reExec->maxRunCnt <= 0) {
	    reExec->maxRunCnt = 1;
            reExec->doFork = 0;
	} else {
            if (reExec->maxRunCnt > MAX_RE_PROCS) 
	        reExec->maxRunCnt = MAX_RE_PROCS;
	    reExec->doFork = 1;
        }
    }
    for (i = 0; i < reExec->maxRunCnt; i++) {
	reExec->reExecProc[i].procExecState = RE_PROC_IDLE;
        reExec->reExecProc[i].ruleExecSubmitInp.packedReiAndArgBBuf =
          (bytesBuf_t *) malloc (sizeof (bytesBuf_t));
        reExec->reExecProc[i].ruleExecSubmitInp.packedReiAndArgBBuf->buf = 
	  malloc (REI_BUF_LEN);
        reExec->reExecProc[i].ruleExecSubmitInp.packedReiAndArgBBuf->len = 
	  REI_BUF_LEN;

        /* init reComm */
        reExec->reExecProc[i].reComm.proxyUser = rsComm->proxyUser;
        reExec->reExecProc[i].reComm.myEnv = rsComm->myEnv;
    }
    return 0;
}

int 
allocReThr (reExec_t *reExec)
{
    int i;
    int thrInx = SYS_NO_FREE_RE_THREAD;

    if (reExec == NULL) return SYS_INTERNAL_NULL_INPUT_ERR;

    if (reExec->doFork == 0) {
	/* single thread */
	reExec->runCnt = 1;
	return 0;	
    }

    reExec->runCnt = 0;		/* reset each time */
    for (i = 0; i < reExec->maxRunCnt; i++) {
	if (reExec->reExecProc[i].procExecState == RE_PROC_IDLE) {
	    if (thrInx == SYS_NO_FREE_RE_THREAD) {
		thrInx = i;
	    }
	} else {
	    reExec->runCnt++;
	}
    }
    if (thrInx == SYS_NO_FREE_RE_THREAD) {
	thrInx = waitAndFreeReThr (reExec);
    }
    if (thrInx >= 0) 
        reExec->reExecProc[thrInx].procExecState = RE_PROC_RUNNING;

    return (thrInx);
}

int
waitAndFreeReThr (reExec_t *reExec)
{
    pid_t childPid;
    int status = 0;
    int thrInx = SYS_NO_FREE_RE_THREAD;

    childPid = waitpid (-1, &status, WUNTRACED);
    if (childPid < 0) {
	if (reExec->runCnt > 0) {
	    int i;
            rodsLog (LOG_NOTICE,
             "waitAndFreeReThr: no outstanding child. but runCnt=%d",
              reExec->runCnt);
	    for (i = 0; i < reExec->maxRunCnt; i++) {
		if (reExec->reExecProc[i].procExecState != RE_PROC_IDLE) {
		    freeReThr (reExec, i);
		}
	    }
	    reExec->runCnt = 0;
	    thrInx = 0;
	}
    } else {
        thrInx = matchPidInReExec (reExec, childPid);
        if (thrInx >= 0) freeReThr (reExec, thrInx);
    }
    return thrInx;
}
#endif

int
matchPidInReExec (reExec_t *reExec, pid_t pid)
{
    int i;

    for (i = 0; i < reExec->maxRunCnt; i++) {
	if (reExec->reExecProc[i].pid == pid) return i;
    }
    rodsLog (LOG_ERROR,
      "matchPidInReExec: no match for pid %d", pid);

    return SYS_NO_FREE_RE_THREAD;
}

int
freeReThr (reExec_t *reExec, int thrInx)
{
    bytesBuf_t *packedReiAndArgBBuf;

#ifdef RE_SERVER_DEBUG
    rodsLog (LOG_NOTICE,
      "freeReThr: thrInx %d, pid %d",thrInx, reExec->reExecProc[thrInx].pid);
#endif
    if (reExec == NULL) return SYS_INTERNAL_NULL_INPUT_ERR;
    if (thrInx < 0 || thrInx >= reExec->maxRunCnt) {
        rodsLog (LOG_ERROR, "freeReThr: Bad input thrInx %d", thrInx);
	return (SYS_BAD_RE_THREAD_INX);
    }
    reExec->runCnt--;
    reExec->reExecProc[thrInx].procExecState = RE_PROC_IDLE;
    reExec->reExecProc[thrInx].status = 0;
    reExec->reExecProc[thrInx].jobType = 0;
    reExec->reExecProc[thrInx].pid = 0;
    /* save the packedReiAndArgBBuf */
    packedReiAndArgBBuf = 
    reExec->reExecProc[thrInx].ruleExecSubmitInp.packedReiAndArgBBuf;

    bzero (packedReiAndArgBBuf->buf, REI_BUF_LEN);
    bzero (&reExec->reExecProc[thrInx].ruleExecSubmitInp, 
      sizeof (ruleExecSubmitInp_t));
    reExec->reExecProc[thrInx].ruleExecSubmitInp.packedReiAndArgBBuf = 
      packedReiAndArgBBuf;

    return 0;
}

int
runRuleExec (reExecProc_t *reExecProc)
{
    ruleExecSubmitInp_t *myRuleExec;
    ruleExecInfoAndArg_t *reiAndArg = NULL;
    rsComm_t *reComm;

    if (reExecProc == NULL) {
	rodsLog (LOG_ERROR, "runRuleExec: NULL reExecProc input");
	reExecProc->status = SYS_INTERNAL_NULL_INPUT_ERR;
	return reExecProc->status;
    }
	
    reComm = &reExecProc->reComm;
    myRuleExec = &reExecProc->ruleExecSubmitInp;

    reExecProc->status = unpackReiAndArg (reComm, &reiAndArg,
      myRuleExec->packedReiAndArgBBuf);

    if (reExecProc->status < 0) {
        rodsLog (LOG_ERROR,
          "runRuleExec: unpackReiAndArg of id %s failed, status = %d",
              myRuleExec->ruleExecId, reExecProc->status);
        return reExecProc->status;
    }

    /* execute the rule */
    reExecProc->status = applyRule (myRuleExec->ruleName,
      reiAndArg->rei->msParamArray,
      reiAndArg->rei, SAVE_REI);

    if (reiAndArg->rei->status < 0) {
        reExecProc->status = reiAndArg->rei->status;
    }
    freeRuleExecInfoStruct (reiAndArg->rei, FREE_MS_PARAM | FREE_DOINP);
    free (reiAndArg);

    return reExecProc->status;
}

int
postProcRunRuleExec (rsComm_t *rsComm, reExecProc_t *reExecProc)
{
    int status;
    int savedStatus = 0;
    ruleExecDelInp_t ruleExecDelInp;
    ruleExecSubmitInp_t *myRuleExecInp;

    myRuleExecInp = &reExecProc->ruleExecSubmitInp;

    if (strlen (myRuleExecInp->exeFrequency) > 0 ) {
        rodsLog(LOG_NOTICE, "postProcRunRuleExec: exec of freq: %s",
          myRuleExecInp->exeFrequency);

        savedStatus = modExeInfoForRepeat (rsComm, myRuleExecInp->ruleExecId,
          myRuleExecInp->exeTime, myRuleExecInp->exeFrequency, 
	  reExecProc->status);
    }
    else if (reExecProc->status < 0) {
        rodsLog (LOG_ERROR,
          "postProcRunRuleExec: ruleExec of id %s failed, status = %d",
          myRuleExecInp->ruleExecId, reExecProc->status);
        if ((reExecProc->jobType & RE_FAILED_STATUS) == 0) {
            /* first time. just mark it RE_FAILED */
            regExeStatus (rsComm, myRuleExecInp->ruleExecId, RE_FAILED);
        } else {

            /* failed once already. delete the ruleExecId */
             rodsLog (LOG_ERROR,
               "postProcRunRuleExec: ruleExec of %s: %s failed again.Removed",
                   myRuleExecInp->ruleExecId, myRuleExecInp->ruleName);
             rstrcpy (ruleExecDelInp.ruleExecId, myRuleExecInp->ruleExecId,
                   NAME_LEN);
             status = rsRuleExecDel (rsComm, &ruleExecDelInp);
             if (status < 0) {
                 rodsLog (LOG_ERROR,
                  "postProcRunRuleExec: rsRuleExecDel failed for %s, stat=%d",
                   myRuleExecInp->ruleExecId, status);
             }
        }
    } else {
        rstrcpy (ruleExecDelInp.ruleExecId, myRuleExecInp->ruleExecId,
                 NAME_LEN);
        rodsLog (LOG_NOTICE,
          "postProcRunRuleExec: exec of %s done", myRuleExecInp->ruleExecId);
        status = rsRuleExecDel (rsComm, &ruleExecDelInp);
        if (status < 0) {
           rodsLog (LOG_ERROR,
            "postProcRunRuleExec: rsRuleExecDel failed for %s, status = %d",
             myRuleExecInp->ruleExecId, status);
        }
    }
    if (status >= 0 && savedStatus < 0) return savedStatus;

    return status;
}

int
matchRuleExecId (reExec_t *reExec, char *ruleExecIdStr, 
procExecState_t execState)
{
    int i;

    if (reExec == NULL || ruleExecIdStr == NULL ||
      execState == RE_PROC_IDLE) return 0;

    for (i = 0; i < reExec->maxRunCnt; i++) {
        if (reExec->reExecProc[i].procExecState == execState &&
	  strcmp (reExec->reExecProc[i].ruleExecSubmitInp.ruleExecId,
	  ruleExecIdStr) == 0) {
	    return 1;
	}
    }
    return 0;
}

