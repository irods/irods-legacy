/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/**
 * @file	nedMS.c
 *
 * @brief	Access to web services from NVO for NASA/IPAC Extragalactic Database (NED)
 *
 * This microservices handles communication with http://voservices.net/NED/ws_v2_0/NED.asmx
 * and provides answers to queries to the NED database. 
 * 
 *
 * @author	Arcot Rajasekar / University of California, San Diego, Jan 2008
 */

#include "rsApiHandler.h"
#include "nedMS.h"
#include "nedH.h" 
#include "ned.nsmap"


int
msiObjByName(msParam_t* inObjByNameParam, 
	     msParam_t* outRaParam, 
	     msParam_t* outDecParam, 
	     msParam_t* outTypParam, 
	     ruleExecInfo_t* rei )
{

  struct soap *soap = soap_new(); 
  struct _ns1__ObjByName objByName; 
  struct _ns1__ObjByNameResponse objByNameResponse;
  char response [1000];


  
  RE_TEST_MACRO( "    Calling msiObjByName" );
  
  objByName.objname = (char *) inObjByNameParam->inOutStruct;

  soap_init(soap);
  soap_set_namespaces(soap, ned_namespaces);  

  if (soap_call___ns2__ObjByName(soap, NULL, NULL, &objByName, &objByNameResponse) == SOAP_OK) {
    sprintf(response,"%f",objByNameResponse.ObjByNameResult->ra);
    fillMsParam( outRaParam, NULL, STR_MS_T, response, NULL );
    sprintf(response,"%f",objByNameResponse.ObjByNameResult->dec);
    fillMsParam( outDecParam, NULL, STR_MS_T, response, NULL );
    fillMsParam( outTypParam, NULL,
		 STR_MS_T, objByNameResponse.ObjByNameResult->objtype, NULL );
    return(0);
  }
  else {
    snprintf(response,999, "Error in execution of msiObjByName::%s\n",soap->buf);
    fillMsParam( outTypParam, NULL,
		 STR_MS_T, response, NULL );
    return(0);
  }
}

