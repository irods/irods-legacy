/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/*
 Microservices for the Rule-based Database Access (RDA).
 */
#include "reGlobalsExtern.h"

#include "rdaHighLevelRoutines.h"
#include "dataObjWrite.h"

/*
 * \fn msiRdaStdout
 * \author Wayne Schroeder
 * \date   2007-05-12
 * \brief This microservice calls new RDA functions to interface
 * to an arbitrary database (under iRODS access control), getting results
 * (i.e. from a query) and returning them in stdout.
 * \note 
 * \param[in]
 *    inpRdaName - string, name of the RDA being used
 *    inpSQL - string, the SQL to use
 *    inpParam1-4 optional - STR_MS_T parameters (bind variables) to
 *    the SQL.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
 */

int
msiRdaToStdout (msParam_t *inpRdaName, msParam_t *inpSQL,
	      msParam_t *inpParam1, msParam_t *inpParam2, 
	      msParam_t *inpParam3, msParam_t *inpParam4, 
	      ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    char *rdaName;
    char *sql;
    char *p1;
    char *p2;
    char *p3;
    char *p4;
    int status;
    char *parms[20];
    int nParm=0;
    char *outBuf=0;

    RE_TEST_MACRO ("    Calling msiRdaToStdout")

    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiRdaToStdout rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    rsComm = rei->rsComm;


    rdaName = parseMspForStr(inpRdaName);
    if (rdaName == NULL) {
       rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			   "msiRdaToStdout: input inpRdaName is NULL");
       return(USER__NULL_INPUT_ERR);
    }

    sql = parseMspForStr(inpSQL);
    if (sql == NULL) {
       rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			   "msiRdaToStdout: input inpSQL is NULL");
       return(USER__NULL_INPUT_ERR);
    }

    p1 = parseMspForStr(inpParam1);
    p2 = parseMspForStr(inpParam2);
    p3 = parseMspForStr(inpParam3);
    p4 = parseMspForStr(inpParam4);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiRdaToStdout: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    status = rdaCheckAccess(rdaName, rsComm);
    if (status) return(status);

    status = rdaOpen(rdaName);
    if (status) return(status);

    if (p1!=NULL) {
       parms[nParm++]=p1;
    }
    if (p2!=NULL) {
       parms[nParm++]=p2;
    }
    if (p3!=NULL) {
       parms[nParm++]=p3;
    }
    if (p4!=NULL) {
       parms[nParm++]=p4;
    }


    status = rdaSqlWithResults(sql, parms, nParm, &outBuf);
    if (status) {
       return(status);
    }

    if (outBuf != NULL) {
       _writeString("stdout",outBuf,rei);
       free(outBuf);
    }

    return (0);
}


/*
 * \fn msiRdaToDataObj
 * \author Wayne Schroeder
 * \date   2007-05-15
 * \brief This microservice calls new RDA functions to interface
 * to an arbitrary database (under iRODS access control), getting results
 * (i.e. from a query) and writing them to an open DataObject.
 * \note 
 * \param[in]
 *    inpRdaName - string, name of the RDA being used
 *    inpSQL - STR_MS_T which is the SQL.
 *    inpParam2-5 optional - STR_MS_T parameters (bind variables) to the SQL,
 *    inpOutObj - open descriptor to write results to.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
 */
int
msiRdaToDataObj (msParam_t *inpRdaName, msParam_t *inpSQL, 
	      msParam_t *inpParam1, msParam_t *inpParam2, 
	      msParam_t *inpParam3, msParam_t *inpParam4, 
	      msParam_t *inpOutObj, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm; 
    char *rdaName;
    char *sql;
    char *p1;
    char *p2;
    char *p3;
    char *p4;
    int status;
    char *parms[20];
    int nParm=0;
    char *outBuf=0;
    int myInt;
    dataObjWriteInp_t myDataObjWriteInp;
    bytesBuf_t dataObjWriteInpBBuf;

    RE_TEST_MACRO ("    Calling msiRdaToDataObj")

    if (inpOutObj == NULL) {
	rei->status = SYS_INTERNAL_NULL_INPUT_ERR;
	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiRdaToDataObje: input inpOutBuf is NULL");
        return (rei->status);
    }
    if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiRdaToDataObj rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    rsComm = rei->rsComm;

    rdaName = parseMspForStr(inpRdaName);
    if (rdaName == NULL) {
       rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			   "msiRdaToStdout: input inpRdaName is NULL");
       return(USER__NULL_INPUT_ERR);
    }

    sql = parseMspForStr(inpSQL);
    if (sql == NULL) {
       rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			   "msiRDAToDataObj: input inpParam1 is NULL");
       return(USER__NULL_INPUT_ERR);
    }

    p1 = parseMspForStr(inpParam1);
    p2 = parseMspForStr(inpParam2);
    p3 = parseMspForStr(inpParam3);
    p4 = parseMspForStr(inpParam4);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiRdaToDataObj: input inpParam1 error. status = %d", rei->status);
        return (rei->status);
    }

    status = rdaCheckAccess(rdaName, rsComm);
    if (status) return(status);

    status = rdaOpen(rdaName);
    if (status) return(status);

    if (p1!=NULL) {
       parms[nParm++]=p1;
    }
    if (p2!=NULL) {
       parms[nParm++]=p2;
    }
    if (p3!=NULL) {
       parms[nParm++]=p3;
    }
    if (p4!=NULL) {
       parms[nParm++]=p4;
    }

    myInt = parseMspForPosInt (inpOutObj);
    if (myInt >= 0) {
       myDataObjWriteInp.l1descInx = myInt;
    } else {
       rei->status = myInt;
       rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
		   "msiRdaToDataObj: parseMspForPosInt error for inpOutObj");
            return (rei->status);
    }

    status = rdaSqlWithResults(sql, parms, nParm, &outBuf);
    if (status) {
       return(status);
    }

    if (outBuf != NULL) {
       int len;
       len = strlen(outBuf);
       dataObjWriteInpBBuf.buf = outBuf;
       myDataObjWriteInp.len = len;
       status = rsDataObjWrite (rsComm, &myDataObjWriteInp, 
       				&dataObjWriteInpBBuf);
       free(outBuf);
    }
    return (status);
}
