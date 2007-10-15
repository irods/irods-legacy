/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/**
 * @file	stockQuoteMS.c
 *
 * @brief	Acces to stock quotation web services
 *
 * This microservices handles comminication with http://www.webserviceX.NET
 * and provides stock quotation - delayed by the web server. 
 * 
 *
 * @author	Arcot Rajasekar / University of California, San Diego
 */

#include "rsApiHandler.h"
#include "stockQuoteMS.h"
#include "stockQuoteH.h" 
#include "stockQuote.nsmap"


int
msiGetQuote(msParam_t* inSymbolParam, msParam_t* outQuoteParam, ruleExecInfo_t* rei )
{

  struct soap *soap = soap_new(); 
  struct _ns1__GetQuote sym;
  struct _ns1__GetQuoteResponse quote;
  char response[10000];
  
  RE_TEST_MACRO( "    Calling msiGetQuote" );
  
  sym.symbol = (char *) inSymbolParam->inOutStruct;

  soap_init(soap);
  soap_set_namespaces(soap, stockQuote_namespaces);  
  if (soap_call___ns1__GetQuote(soap, NULL, NULL, &sym, &quote) == SOAP_OK) {
    fillMsParam( outQuoteParam, NULL,
		 STR_MS_T, quote.GetQuoteResult, NULL );
    free (quote.GetQuoteResult);
    return(0);
  }
  else {
    sprintf(response,"Error in execution of msiIp2location::%s\n",soap->buf);
    fillMsParam( outQuoteParam, NULL,
		 STR_MS_T, response, NULL );
    return(0);
  }
}

