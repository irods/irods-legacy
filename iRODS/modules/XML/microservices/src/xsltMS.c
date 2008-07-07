/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "xsltMS.h"

#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

extern int xmlLoadExtDtdDefaultValue;



/*
 * msiXsltApply()
 *
 */
int
msiXsltApply(msParam_t *xsltObj, msParam_t *xmlObj, msParam_t *msParamOut, ruleExecInfo_t *rei)
{
	/* for parsing msParams and to open iRods objects */
	dataObjInp_t xmlDataObjInp, *myXmlDataObjInp;
	dataObjInp_t xsltDataObjInp, *myXsltDataObjInp;
	int xmlObjID, xsltObjID;

	/* for getting size of objects to read from */
	rodsObjStat_t *rodsObjStatOut = NULL;

	/* for reading from iRods objects */
	dataObjReadInp_t dataObjReadInp;
	dataObjCloseInp_t dataObjCloseInp;
	bytesBuf_t xmlBuf, xsltBuf;

	/* misc. to avoid repeating rei->rsComm */
	rsComm_t *rsComm;

	/* for xml parsing */
	char *outStr;
	int outLen;
	xsltStylesheetPtr style = NULL;
	xmlDocPtr xslSheet, doc, res;



	/*********************************  USUAL INIT PROCEDURE **********************************/
	
	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiXsltApply")


	/* Sanity checks */
	if (rei == NULL || rei->rsComm == NULL)
	{
		rodsLog (LOG_ERROR, "msiXsltApply: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;


	
	/********************************** RETRIEVE INPUT PARAMS **********************************/

	/* Get xsltObj: the XSLT stylesheet */
	rei->status = parseMspForDataObjInp (xsltObj, &xsltDataObjInp, &myXsltDataObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiXsltApply: input xsltObj error. status = %d", rei->status);
		return (rei->status);
	}


	/* Get xmlObj: the XML document */
	rei->status = parseMspForDataObjInp (xmlObj, &xmlDataObjInp, &myXmlDataObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiXsltApply: input xmlObj error. status = %d", rei->status);
		return (rei->status);
	}



	/******************************** GET CONTENTS OF IRODS OBJS ********************************/

	/* Open XSLT file */
	if ((xsltObjID = rsDataObjOpen(rsComm, &xsltDataObjInp)) < 0) 
	{
		rodsLog (LOG_ERROR, "msiXsltApply: Cannot open XSLT data object. status = %d", xsltObjID);
		return (xsltObjID);
	}


	/* Get size of XSLT file */
	rei->status = rsObjStat (rsComm, &xsltDataObjInp, &rodsObjStatOut);


	/* Read XSLT file */
	memset (&dataObjReadInp, 0, sizeof (dataObjReadInp));
	dataObjReadInp.l1descInx = xsltObjID;
	dataObjReadInp.len = (int)rodsObjStatOut->objSize;

	rei->status = rsDataObjRead (rsComm, &dataObjReadInp, &xsltBuf);


	/* Close XSLT file */
	memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
	dataObjCloseInp.l1descInx = xsltObjID;
	
	rei->status = rsDataObjClose (rsComm, &dataObjCloseInp);


	/* Cleanup. Needed before using rodsObjStatOut for a new rsObjStat() call */
	freeRodsObjStat (rodsObjStatOut);


	/* Open XML file */
	if ((xmlObjID = rsDataObjOpen(rsComm, &xmlDataObjInp)) < 0) 
	{
		rodsLog (LOG_ERROR, "msiXsltApply: Cannot open XML data object. status = %d", xmlObjID);
		return (xmlObjID);
	}


	/* Get size of XML file */
	rei->status = rsObjStat (rsComm, &xmlDataObjInp, &rodsObjStatOut);


	/* Read XML file */
	memset (&dataObjReadInp, 0, sizeof (dataObjReadInp));
	dataObjReadInp.l1descInx = xmlObjID;
	dataObjReadInp.len = (int)rodsObjStatOut->objSize;

	rei->status = rsDataObjRead (rsComm, &dataObjReadInp, &xmlBuf);


	/* Close XML file */
	memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
	dataObjCloseInp.l1descInx = xmlObjID;
	
	rei->status = rsDataObjClose (rsComm, &dataObjCloseInp);

	/* cleanup */
	freeRodsObjStat (rodsObjStatOut);



	/******************************** PARSE XML DOCS AND APPLY XSL ********************************/

	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;


	/* Parse xsltBuf.buf into an xmlDocPtr, and the xmlDocPtr into an xsltStylesheetPtr */
	xslSheet = xmlParseDoc((const xmlChar*)xsltBuf.buf);
	style = xsltParseStylesheetDoc(xslSheet);

	/* Parse xmlBuf.buf into an xmlDocPtr */
	doc = xmlParseDoc((const xmlChar*)xmlBuf.buf);

	/* And the magic happens */
	res = xsltApplyStylesheet(style, doc, NULL);

	/* Save result XML document to a string */
	rei->status = xsltSaveResultToString((xmlChar**)&outStr, &outLen, res, style);


	/* cleanup of all xml parsing stuff */
	xsltFreeStylesheet(style);
	xmlFreeDoc(res);
	xmlFreeDoc(doc);

        xsltCleanupGlobals();
        xmlCleanupParser();



	/************************************** WE'RE DONE **************************************/

	/* send results out to msParamOut */
	fillStrInMsParam (msParamOut, outStr);

	return (rei->status);
}




