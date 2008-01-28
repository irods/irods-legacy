/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/**
 * @file	sdssImgCutoutMS.c
 *
 * @brief	Access to web services from NVO for  SDSS Image Cut Out service.
 *
 * This microservices handles communication with http://
 * and provides answers to queries to the SDSS Image Cut Out service. 
 * 
 *
 * @author	Arcot Rajasekar / University of California, San Diego, Jan 2008
 */

#include "rsApiHandler.h"
#include "sdssImgCutoutMS.h"
#include "sdssImgCutoutH.h" 
#include "sdssImgCutout.nsmap"


int
msiSdssImgCutout_GetJpeg(msParam_t* inRaParam, 
		 msParam_t* inDecParam, 
		 msParam_t* inScaleParam,
		 msParam_t* inWidthParam,
		 msParam_t* inHeightParam,
		 msParam_t* inOptParam,
		 msParam_t* outImgParam, 
		 ruleExecInfo_t* rei )
{

  struct soap *soap = soap_new(); 
  struct _ns1__GetJpeg inValues; 
  struct _ns1__GetJpegResponse outResponse;

  char response [1000];
  int status;
  bytesBuf_t *dataBBuf = NULL;

  RE_TEST_MACRO( "    Calling msiSdssImgCutout_GetJpeg" );
  
  inValues.ra_USCORE = atof((char *) inRaParam->inOutStruct);
  inValues.dec_USCORE = atof((char *) inDecParam->inOutStruct);
  inValues.scale_USCORE = atof((char *) inScaleParam->inOutStruct);
  inValues.width_USCORE = atoi((char *) inWidthParam->inOutStruct);
  inValues.height_USCORE = atoi((char *) inHeightParam->inOutStruct);
  inValues.opt_USCORE = (char *) inOptParam->inOutStruct;

  soap_init(soap);
  soap_set_namespaces(soap, sdssImgCutout_namespaces);  
  status = soap_call___ns2__GetJpeg(soap, NULL, NULL, &inValues, &outResponse);

  if (status == SOAP_OK) {
    dataBBuf = (bytesBuf_t *) malloc (sizeof (bytesBuf_t));
    memset (dataBBuf, 0, sizeof (bytesBuf_t));
    dataBBuf->len = outResponse.GetJpegResult->__size;
    dataBBuf->buf = outResponse.GetJpegResult->__ptr;
    fillBufLenInMsParam (outImgParam,outResponse.GetJpegResult->__size, dataBBuf);
    return(0);
  }
  else {
    snprintf(response,999, "Error in execution of msiSdssImgCutout_GetJpeg::%s\n",soap->buf);
    fillMsParam( outImgParam, NULL,
		 STR_MS_T, response, NULL );
    return(0);
  }
}

