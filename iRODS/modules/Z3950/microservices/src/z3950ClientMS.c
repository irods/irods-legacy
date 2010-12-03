/**
 * @file  z3950ClientMS.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/


#include "yaz/zoom.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "z3950ClientMS.h"


/**
 * \fn msiz3950Submit(msParam_t *serverName,msParam_t *query, msParam_t *recordSyntax,
 *                    msParam_t *outParam, ruleExecInfo_t *rei )
 *
 * \brief   Retrieves a record from a Z39.50 server
 *
 * \module  Z3950
 *
 * \since   after 2.4.1
 *
 * \author  Rahul Deshmukh and Terrell Russell, SILS at UNC-Chapel Hill
 * \date    Summer 2010
 *
 * \remark Terrell Russell - msi documentation, 2010-12-03
 *
 * \note 
 *
 * \usage  See iRODS/modules/Z3950/z3950rule.ir
 *
 * \param[in]       serverName    - a STR_MS_T containing the name of the Z39.50 server
 * \param[in]       query         - a STR_MS_T containing the input query to the Z39.50 server
 * \param[in]       recordSyntax  - a STR_MS_T containing the preferred syntax for the returned record
 * \param[out]      outParam      - a STR_MS_T containing the retrieved record 
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/

int
msiz3950Submit(msParam_t *serverName,msParam_t *query, msParam_t *recordSyntax, msParam_t *outParam, ruleExecInfo_t *rei )
{

  const char *rec;
  char outStr[MAX_NAME_LEN];
  const char *sName = serverName->inOutStruct ; 
  const char *zquery = query->inOutStruct;  
  const char *rSyntax = recordSyntax->inOutStruct;

  ZOOM_connection z = ZOOM_connection_new (sName, 0);   // 0 denotes port number
    ZOOM_resultset r;
      
    ZOOM_connection_option_set(z, "preferredRecordSyntax", rSyntax);
    r = ZOOM_connection_search_pqf(z,zquery);


    rec =  ZOOM_record_get (ZOOM_resultset_record (r, 0), "render", 0);

    snprintf (outStr, MAX_NAME_LEN, "Here is the record: \n %s", rec);
    fillStrInMsParam (outParam, outStr);
    rei->status = 0;

  ZOOM_connection_destroy(z);

  return 0;
}

