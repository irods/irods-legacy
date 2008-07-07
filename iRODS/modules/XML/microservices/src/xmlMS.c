/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "xmlMS.h"

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>



xmlNodePtr getChildNodeByName(xmlNodePtr cur, char *name){

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



#if defined(LIBXML_XPATH_ENABLED) && \
    defined(LIBXML_SAX1_ENABLED) && \
    defined(LIBXML_OUTPUT_ENABLED)


/*
 * msiLoadMetadataFromXml() - Prototype
 *
 */
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








