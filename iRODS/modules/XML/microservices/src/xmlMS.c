/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "xmlMS.h"

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlschemas.h>






/* get the first child node with a given name */
static xmlNodePtr
getChildNodeByName(xmlNodePtr cur, char *name)
{
        if (cur == NULL || name == NULL)
	{
		return NULL;
	}

        for(cur=cur->children; cur != NULL; cur=cur->next)
	{
                if(cur->name && (strcmp((char*)cur->name, name) == 0))
		{
                        return cur;
                }
        }

	return NULL;
}



/* custom handler to catch XML validation errors and print them to a buffer */
static int 
myErrorCallback(bytesBuf_t *errBuf, const char* errMsg, ...)
{
	va_list ap;
	char tmpStr[MAX_NAME_LEN];
	int written;

	va_start(ap, errMsg);
	written = vsnprintf(tmpStr, MAX_NAME_LEN, errMsg, ap);
	va_end(ap);

	appendToByteBuf(errBuf, tmpStr);
	return (written);
}




/*
 * msiLoadMetadataFromXml() - Prototype
 *
 * Requires XPAth support
 *
 */
#if defined(LIBXML_XPATH_ENABLED) && \
    defined(LIBXML_SAX1_ENABLED) && \
    defined(LIBXML_OUTPUT_ENABLED)

int 
msiLoadMetadataFromXml(msParam_t *targetObj, msParam_t *xmlObj, ruleExecInfo_t *rei) 
{
	/* for parsing msParams and to open iRods objects */
	dataObjInp_t xmlDataObjInp, *myXmlDataObjInp;
	dataObjInp_t targetObjInp, *myTargetObjInp;
	int xmlObjID;

	/* for getting size of objects to read from */
	rodsObjStat_t *rodsObjStatOut = NULL;

	/* for reading from iRods objects */
	dataObjReadInp_t dataObjReadInp;
	dataObjCloseInp_t dataObjCloseInp;
	bytesBuf_t xmlBuf;

	/* misc. to avoid repeating rei->rsComm */
	rsComm_t *rsComm;

	/* for xml parsing */
	xmlDocPtr doc;

	/* for XPath evaluation */
	xmlXPathContextPtr xpathCtx;
	xmlXPathObjectPtr xpathObj;
	xmlChar xpathExpr[] = "//AVU";
	xmlNodeSetPtr nodes;
	int avuNbr, i;
	
	/* for new AVU creation */
	modAVUMetadataInp_t modAVUMetadataInp;
	char attrStr[MAX_NAME_LEN];



	/*********************************  USUAL INIT PROCEDURE **********************************/
	
	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiLoadMetadataFromXml")


	/* Sanity checks */
	if (rei == NULL || rei->rsComm == NULL)
	{
		rodsLog (LOG_ERROR, "msiLoadMetadataFromXml: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;


	
	/********************************** RETRIEVE INPUT PARAMS **************************************/

	/* Get path of target object */
	rei->status = parseMspForDataObjInp (targetObj, &targetObjInp, &myTargetObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiLoadMetadataFromXml: input targetObj error. status = %d", rei->status);
		return (rei->status);
	}


	/* Get path of XML document */
	rei->status = parseMspForDataObjInp (xmlObj, &xmlDataObjInp, &myXmlDataObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiLoadMetadataFromXml: input xmlObj error. status = %d", rei->status);
		return (rei->status);
	}



	/******************************** OPEN AND READ FROM XML OBJECT ********************************/

	/* Open XML file */
	if ((xmlObjID = rsDataObjOpen(rsComm, &xmlDataObjInp)) < 0) 
	{
		rodsLog (LOG_ERROR, "msiLoadMetadataFromXml: Cannot open XML data object. status = %d", xmlObjID);
		return (xmlObjID);
	}


	/* Get size of XML file */
	rei->status = rsObjStat (rsComm, &xmlDataObjInp, &rodsObjStatOut);


	/* Read XML file */
	memset (&dataObjReadInp, 0, sizeof (dataObjReadInp));
	dataObjReadInp.l1descInx = xmlObjID;
	dataObjReadInp.len = (int)rodsObjStatOut->objSize;

	rei->status = rsDataObjRead (rsComm, &dataObjReadInp, &xmlBuf);
	
	/* Make sure that the result is null terminated */
	((char*)xmlBuf.buf)[dataObjReadInp.len]='\0';


	/* Close XML file */
	memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
	dataObjCloseInp.l1descInx = xmlObjID;
	
	rei->status = rsDataObjClose (rsComm, &dataObjCloseInp);

	/* cleanup */
	freeRodsObjStat (rodsObjStatOut);



	/******************************** PARSE XML DOCUMENT ********************************/

	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;


	/* Parse xmlBuf.buf into an xmlDocPtr */
	doc = xmlParseDoc((const xmlChar*)xmlBuf.buf);


	/* Create xpath evaluation context */
	xpathCtx = xmlXPathNewContext(doc);
	if(xpathCtx == NULL) 
	{
		rodsLog (LOG_ERROR, "msiLoadMetadataFromXml: Unable to create new XPath context.");
		xmlFreeDoc(doc); 
		return(-1);
	}


	/* Evaluate xpath expression */
	xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
	if(xpathObj == NULL) 
	{
		rodsLog (LOG_ERROR, "msiLoadMetadataFromXml: Unable to evaluate XPath expression \"%s\".", xpathExpr);
		xmlXPathFreeContext(xpathCtx); 
		xmlFreeDoc(doc); 
		return(-1);
	}

	/* How many AVU nodes did we get? */
	if ((nodes = xpathObj->nodesetval) != NULL)
	{
		avuNbr = nodes->nodeNr;
	}
	else 
	{
		avuNbr = 0;
	}



	/******************************** CREATE AVU TRIPLETS ********************************/

	/* Add a new AVU for each node. It's ok to process the nodes in forward order since we're not modifying them */
	for(i = 0; i < avuNbr; i++) 
	{
		if (nodes->nodeTab[i])
		{
			/* temporary: Add index number to avoid duplicating attribute names */
			memset(attrStr, '\0', MAX_NAME_LEN);
			snprintf(attrStr, MAX_NAME_LEN - 1, "%04d: %s", i+1, (char*)xmlNodeGetContent(getChildNodeByName(nodes->nodeTab[i], "Attribute")) );

			/* init modAVU input */
			memset (&modAVUMetadataInp, 0, sizeof(modAVUMetadataInp_t));
			modAVUMetadataInp.arg0 = "add";
			modAVUMetadataInp.arg1 = "-d";
			modAVUMetadataInp.arg2 = myTargetObjInp->objPath;
			modAVUMetadataInp.arg3 = attrStr;
			modAVUMetadataInp.arg4 = (char*)xmlNodeGetContent(getChildNodeByName(nodes->nodeTab[i], "Value"));
			modAVUMetadataInp.arg5 = (char*)xmlNodeGetContent(getChildNodeByName(nodes->nodeTab[i], "Unit"));
			
			/* invoke rsModAVUMetadata() */
			rei->status = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
		}
	}



	/************************************** WE'RE DONE **************************************/

	/* cleanup of all xml parsing stuff */
	xmlFreeDoc(doc);
        xmlCleanupParser();

	return (rei->status);
}


/* Default stub if no XPath support */
#else
int 
msiLoadMetadataFromXml(msParam_t *targetObj, msParam_t *xmlObj, ruleExecInfo_t *rei) 
{
	rodsLog (LOG_ERROR, "msiLoadMetadataFromXml: XPath support not compiled in.");
	return (SYS_NOT_SUPPORTED);
}
#endif



/*
 * msiXmlDocSchemaValidate()
 *
 */
int 
msiXmlDocSchemaValidate(msParam_t *xmlObj, msParam_t *xsdObj, msParam_t *status, ruleExecInfo_t *rei) 
{
	/* for parsing msParams and to open iRods objects */
	dataObjInp_t xmlObjInp, *myXmlObjInp;
	dataObjInp_t xsdObjInp, *myXsdObjInp;
	int xmlObjID, xsdObjID;

	/* for getting size of objects to read from */
	rodsObjStat_t *rodsObjStatOut = NULL;

	/* for reading from iRods objects */
	dataObjReadInp_t dataObjReadInp;
	dataObjCloseInp_t dataObjCloseInp;
	bytesBuf_t xmlBuf;
	char *tail;

	/* for xml parsing and validating */
	xmlDocPtr doc, xsd_doc;
	xmlSchemaParserCtxtPtr parser_ctxt;
	xmlSchemaPtr schema;
	xmlSchemaValidCtxtPtr valid_ctxt;
	bytesBuf_t *errBuf;

	/* misc. to avoid repeating rei->rsComm */
	rsComm_t *rsComm;



	/*************************************  USUAL INIT PROCEDURE **********************************/
	
	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiXmlDocSchemaValidate")


	/* Sanity checks */
	if (rei == NULL || rei->rsComm == NULL)
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;



	/************************************ ADDITIONAL INIT SETTINGS *********************************/

	/* XML constants */
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;


	/* allocate memory for output error buffer */
	errBuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	errBuf->buf = strdup("");
	errBuf->len = strlen(errBuf->buf);

	/* Default status is failure, overwrite if success */
	fillBufLenInMsParam (status, -1, NULL);

	
	/********************************** RETRIEVE INPUT PARAMS **************************************/

	/* Get path of XML document */
	rei->status = parseMspForDataObjInp (xmlObj, &xmlObjInp, &myXmlObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: input xmlObj error. status = %d", rei->status);
		return (rei->status);
	}


	/* Get path of schema */
	rei->status = parseMspForDataObjInp (xsdObj, &xsdObjInp, &myXsdObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: input xsdObj error. status = %d", rei->status);
		return (rei->status);
	}


	/******************************** OPEN AND READ FROM XML OBJECT ********************************/

	/* Open XML file */
	if ((xmlObjID = rsDataObjOpen(rsComm, &xmlObjInp)) < 0) 
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: Cannot open XML data object. status = %d", xmlObjID);
		return (xmlObjID);
	}


	/* Get size of XML file */
	rei->status = rsObjStat (rsComm, &xmlObjInp, &rodsObjStatOut);


	/* Read content of XML file */
	memset (&dataObjReadInp, 0, sizeof (dataObjReadInp));
	dataObjReadInp.l1descInx = xmlObjID;
	dataObjReadInp.len = (int)rodsObjStatOut->objSize + 1;	/* extra byte to add a null char */

	rei->status = rsDataObjRead (rsComm, &dataObjReadInp, &xmlBuf);

	/* add terminating null character */
	tail = xmlBuf.buf;
	tail[dataObjReadInp.len - 1] = '\0';


	/* Close XML file */
	memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
	dataObjCloseInp.l1descInx = xmlObjID;
	
	rei->status = rsDataObjClose (rsComm, &dataObjCloseInp);

	/* cleanup */
	freeRodsObjStat (rodsObjStatOut);



	/*************************************** PARSE XML DOCUMENT **************************************/

	/* Parse xmlBuf.buf into an xmlDocPtr */
	doc = xmlParseDoc((const xmlChar*)xmlBuf.buf);
	clearBBuf(&xmlBuf);

	if (doc == NULL)
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: XML document cannot be loaded or is not well-formed.");

	        xmlCleanupParser();

	        return (USER_INPUT_FORMAT_ERR);
	}



	/******************************** OPEN AND READ FROM XSD OBJECT ********************************/

	/* Open schema file */
	if ((xsdObjID = rsDataObjOpen(rsComm, &xsdObjInp)) < 0) 
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: Cannot open XSD data object. status = %d", xsdObjID);

		xmlFreeDoc(doc);
	        xmlCleanupParser();

		return (xsdObjID);
	}


	/* Get size of schema file */
	rei->status = rsObjStat (rsComm, &xsdObjInp, &rodsObjStatOut);


	/* Read entire schema file */
	memset (&dataObjReadInp, 0, sizeof (dataObjReadInp));
	dataObjReadInp.l1descInx = xsdObjID;
	dataObjReadInp.len = (int)rodsObjStatOut->objSize + 1;	/* to add null char */

	rei->status = rsDataObjRead (rsComm, &dataObjReadInp, &xmlBuf);

	/* add terminating null character */
	tail = xmlBuf.buf;
	tail[dataObjReadInp.len - 1] = '\0';


	/* Close schema file */
	memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
	dataObjCloseInp.l1descInx = xsdObjID;
	
	rei->status = rsDataObjClose (rsComm, &dataObjCloseInp);

	/* cleanup */
	freeRodsObjStat (rodsObjStatOut);



	/*************************************** PARSE XSD DOCUMENT **************************************/

	/* Parse xmlBuf.buf into an xmlDocPtr */
	xsd_doc = xmlParseDoc((const xmlChar*)xmlBuf.buf);
	clearBBuf(&xmlBuf);

	if (xsd_doc == NULL)
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: XML Schema cannot be loaded or is not well-formed.");

		xmlFreeDoc(doc);
		xmlCleanupParser();

	        return (USER_INPUT_FORMAT_ERR);
	}



	/**************************************** VALIDATE DOCUMENT **************************************/

	/* Create a parser context */
	parser_ctxt = xmlSchemaNewDocParserCtxt(xsd_doc);
	if (parser_ctxt == NULL)
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: Unable to create a parser context for the schema.");

		xmlFreeDoc(xsd_doc);
		xmlFreeDoc(doc);
	        xmlCleanupParser();

	        return (USER_INPUT_FORMAT_ERR);
	}


	/* Parse the XML schema */
	schema = xmlSchemaParse(parser_ctxt);
	if (schema == NULL) 
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: Invalid schema.");

		xmlSchemaFreeParserCtxt(parser_ctxt);
		xmlFreeDoc(doc);
		xmlFreeDoc(xsd_doc);
        	xmlCleanupParser();

	        return (USER_INPUT_FORMAT_ERR);
	}


	/* Create a validation context */
	valid_ctxt = xmlSchemaNewValidCtxt(schema);
	if (valid_ctxt == NULL) 
	{
		rodsLog (LOG_ERROR, "msiXmlDocSchemaValidate: Unable to create a validation context for the schema.");

		xmlSchemaFree(schema);
		xmlSchemaFreeParserCtxt(parser_ctxt);
		xmlFreeDoc(xsd_doc);
		xmlFreeDoc(doc);
	        xmlCleanupParser();

	        return (USER_INPUT_FORMAT_ERR);
	}


	/* Set myErrorCallback() as the default handler for error messages and warnings */
	xmlSchemaSetValidErrors(valid_ctxt, (xmlSchemaValidityErrorFunc)myErrorCallback, (xmlSchemaValidityWarningFunc)myErrorCallback, errBuf);


	/* Validate XML doc */
	rei->status = xmlSchemaValidateDoc(valid_ctxt, doc);



	/******************************************* WE'RE DONE ******************************************/

	/* return both error code and messages through status */
	resetMsParam (status);
	fillBufLenInMsParam (status, rei->status, errBuf);


	/* cleanup of all xml parsing stuff */
	xmlSchemaFreeValidCtxt(valid_ctxt);
	xmlSchemaFree(schema);
	xmlSchemaFreeParserCtxt(parser_ctxt);
	xmlFreeDoc(doc);
	xmlFreeDoc(xsd_doc);
        xmlCleanupParser();

	return (rei->status);
}









