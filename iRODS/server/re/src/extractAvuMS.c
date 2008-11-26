/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "reGlobalsExtern.h"
#include "objMetaOpr.h"
#include "miscServerFunct.h"

#if defined(solaris_platform)
#include <libgen.h>
#endif



#include <regex.h>


extern char *__loc1;


int
msiReadMDTemplateIntoTagStruct(msParam_t* bufParam, msParam_t* tagParam, ruleExecInfo_t *rei)
{
/**
 * \fn msiReadMDTemplateIntoTagStruct
 * \author  Arcot Rajasekar
 * \date   2007-02-01
 * \brief   this function parses a buffer containing a template-style file
 *  and stores the tags  in a tag structure.
 * \note  the template buffer should contain triplets be of the form 
 *  <PRETAG>re1</PRETAG>kw<POSTTAG>re2</POSTTAG>
 *  re1 identifies the pre-string and re2 ientifies the post-string and 
 *  any value between re1 and re2 in a metadata buffer can be 
 *  associated with keyword kw.
 * \param[in] tempObjBuf   is a msParam of type BUF_MS_T
 * \param[out] tagStruct is a msParam of type TagStruct_MS_T
 * \return integer
 * \retval 0 on success
 * \retval USER_PARAM_TYP_ERROR wheninput  param dont match the type
 * \retval INVALID_REGEXP if the tags are not correct
 * \retval NO_VALUES_FOUND if there are no tags identified
 * \retval from addTagStruct
 * \sa addTagStruct
 * \post
 * \pre
 * \bug  no known bugs
**/

  bytesBuf_t *tmplObjBuf;
  tagStruct_t *tagValues;

  char *t, *t1, *t2, *t3, *t4, *t5, *t6, *t7, *t8;
  /*
  int len;
  int l1, l2;
  */
  int i,j;
  /*  char *preg[4];*/
  regex_t preg[4];
  regmatch_t pm[4];
  char errbuff[100];

  RE_TEST_MACRO ("Loopback on msiReadMDTemplateIntoTagStruct");

  if (strcmp (bufParam->type,BUF_LEN_MS_T) != 0 || 
      bufParam->inpOutBuf == NULL)
    return(USER_PARAM_TYPE_ERR);
  tmplObjBuf = (bytesBuf_t *) bufParam->inpOutBuf;
  /*
  preg[0] =  regcmp("<PRETAG>", (char *)0);
  if (preg[0] == NULL) 
    return(INVALID_REGEXP);
  preg[1] =  regcmp("</PRETAG>", (char *)0);
  if (preg[1] == NULL) 
    return(INVALID_REGEXP);
  preg[2] =  regcmp("<POSTTAG>", (char *)0);
  if (preg[0] == NULL) 
    return(INVALID_REGEXP);
  preg[3] =  regcmp("</POSTTAG>", (char *)0);
  if (preg[1] == NULL) 
    return(INVALID_REGEXP);
  */
  j = regcomp (&preg[0], "<PRETAG>", REG_EXTENDED);
  if (j != 0) {
    regerror (j,&preg[0],errbuff,sizeof(errbuff)); 
    rodsLog (LOG_NOTICE,"msiReadMDTemplateIntoTagStruct: Error in regcomp: %s\n",errbuff);
    return(INVALID_REGEXP);
  }
  j = regcomp (&preg[1], "</PRETAG>", REG_EXTENDED);
  if (j != 0) {
    regerror (j,&preg[1],errbuff,sizeof(errbuff)); 
    rodsLog (LOG_NOTICE,"msiReadMDTemplateIntoTagStruct: Error in regcomp: %s\n",errbuff);
    return(INVALID_REGEXP);
  }
  j = regcomp (&preg[2], "<POSTTAG>", REG_EXTENDED);
  if (j != 0) {
    regerror (j,&preg[2],errbuff,sizeof(errbuff)); 
    rodsLog (LOG_NOTICE,"msiReadMDTemplateIntoTagStruct: Error in regcomp: %s\n",errbuff);
    return(INVALID_REGEXP);
  }
  j = regcomp (&preg[3], "</POSTTAG>", REG_EXTENDED);
  if (j != 0) {
    regerror (j,&preg[3],errbuff,sizeof(errbuff)); 
    rodsLog (LOG_NOTICE,"msiReadMDTemplateIntoTagStruct: Error in regcomp: %s\n",errbuff);
    return(INVALID_REGEXP);
  }

  t = malloc(tmplObjBuf->len + 1);
  t[tmplObjBuf->len] = '\0';
  memcpy(t,tmplObjBuf->buf,tmplObjBuf->len);
  tagValues = mallocAndZero(sizeof(tagStruct_t));
  tagValues->len = 0;
  t1 = t;
#ifdef BABABA
  /*
  while ((t2 = regex(preg[0], t1)) != NULL) { / * t2 starts preTag * /
    if ((t3 = regex(preg[1], t2)) == NULL)    / * t3 starts keyValue * /
      break;
    t6 = __loc1;                              / * t6 ends preTag * /
    *t6 = '\0';
    if ((t5 = regex(preg[2], t3)) == NULL)    / *  t5 starts postTag * /
      break;
    t4 = __loc1;                              / * t4 ends keyValue * /
    *t4 = '\0';
    if ((t7 = regex(preg[3], t5)) == NULL)    / * t7 ends the line * /
      break;
    t8 = __loc1;                              / * t8 ends postTag * /
    *t8 = '\0';
    
    i = addTagStruct (tagValues, t2, t5, t3);
    if (i != 0)
      return(i);
    t1 = t7;
    if (*t1 == '\0')
      break;
  }
  */
#endif /*  BABABA */
  while (regexec(&preg[0], t1,1,&pm[0],0) == 0) {
    t2 = t1 + pm[0].rm_eo ;                        /* t2 starts preTag */
    if  (regexec(&preg[1],t2,1,&pm[1],0) != 0)
      break;
    t3 = t2 + pm[1].rm_eo ;                        /* t3 starts keyValue */
    t6 = t2 + pm[1].rm_so;                            /* t6 ends preTag */
    *t6 = '\0';
    if  (regexec(&preg[2],t3,1,&pm[2],0) != 0)
      break;
    t5 = t3 + pm[2].rm_eo ;                        /* t5 starts postTag */
    t4 = t3 + pm[2].rm_so;                            /* t4 ends keyValue */
    *t4 = '\0';
    if  (regexec(&preg[3],t5,1,&pm[3],0) != 0)
      break;
    t7 = t5 + pm[3].rm_eo;                        /* t7 ends the line */
    t8 = t5 + pm[3].rm_so;                            /* t8 ends postTag */
    *t8 = '\0';
    /***    rodsLog(LOG_NOTICE,"msiReadMDTemplateIntoTagStruct:TAGS:%s::%s::%s::\n",t2, t5, t3);***/
    i = addTagStruct (tagValues, t2, t5, t3);
    if (i != 0)
      return(i);
    t1 = t7;
    if (*t1 == '\0')
      break;
  }

  /*
  free(preg[0]);
  free(preg[1]);
  free(preg[2]);
  free(preg[3]);
  */
  regfree(&preg[0]);
  regfree(&preg[1]);
  regfree(&preg[2]);
  regfree(&preg[3]);
  free(t);

  if (tagValues->len == 0) 
    return(NO_VALUES_FOUND);
    
  tagParam->inOutStruct = (void *) tagValues;
  tagParam->type = (char *) strdup(TagStruct_MS_T);

  return(0);
  
}


int msiGetTaggedValueFromString(msParam_t *inTagParam, msParam_t *inStrParam,
				msParam_t *outValueParam, ruleExecInfo_t *rei)
{


  int j;
  regex_t preg[2];
  regmatch_t pm[2];
  char errbuff[100];
  char *pstr[2];
  char *t1, *t2, *t3, *t4;
  char c;
  
  t1 = (char *) inStrParam->inOutStruct;
  pstr[0] = (char *) malloc(strlen(inTagParam->inOutStruct) + 6 );
  pstr[1] = (char *) malloc(strlen(inTagParam->inOutStruct) + 6 );
  sprintf(pstr[0], "<%s>", (char *) inTagParam->inOutStruct);
    j = regcomp (&preg[0], pstr[0], REG_EXTENDED);
    if (j != 0) {
      regerror (j,&preg[0],errbuff,sizeof(errbuff)); 
      rodsLog (LOG_NOTICE,"msiGetTaggedValueFromString: Error in regcomp: %s\n",errbuff);
      return(INVALID_REGEXP);
    }
    sprintf(pstr[1], "</%s>", (char *) inTagParam->inOutStruct);
    j = regcomp (&preg[1], pstr[1], REG_EXTENDED);
    if (j != 0) {
      regerror (j,&preg[1],errbuff,sizeof(errbuff)); 
      rodsLog (LOG_NOTICE,"msiGetTaggedValueFromString: Error in regcomp: %s\n",errbuff);
      return(INVALID_REGEXP);
    }
    /*    rodsLog (LOG_NOTICE,"TTTTT:%s",t1);*/
    if (regexec(&preg[0], t1,1,&pm[0],0) == 0) {
      t2 = t1 + pm[0].rm_eo ;                     /* t2 starts value */
      if (regexec(&preg[1],t2,1,&pm[1],0) != 0)
	fillMsParam( outValueParam, NULL, STR_MS_T, "", NULL );
      else {
	t4 = t2+ pm[1].rm_so;                       /* t4 ends value */
	t3 = t2+ pm[1].rm_eo;
	c = *t4;
	*t4 = '\0';
	fillMsParam( outValueParam, NULL, STR_MS_T, t2, NULL );
	*t4 = c;
      }
    }
    else
      fillMsParam( outValueParam, NULL, STR_MS_T, "", NULL );
    regfree(&preg[0]);
    regfree(&preg[1]);
    free(pstr[0]);
    free(pstr[1]);
    return(0);
}

int
msiExtractTemplateMDFromBuf(msParam_t* bufParam, msParam_t* tagParam, 
			   msParam_t *metadataParam, ruleExecInfo_t *rei)
{
/**
 * \fn msiExtractTemplateMDFromBuf
 * \author  Arcot Rajasekar
 * \date   2007-02-01
 * \brief   this function parses a buffer containing metadata 
 *  and uses the tags to identify Key-Value Pairs.
 * \note  the template structure identifies triplets 
 *  <pre-string-regexp,post-string-regexp,keyword> and the metadata buffer 
 *  is searched for the pre and post regular expressions and the string
 *  between them are associated with the keyword.
 *  A.l  <key,value> pairs found is stored in keyValPair_t structure.
 * \param[in] bufParam   is a msParam of type BUF_MS_T
 * \param[in] tagParam is a msParam of type TagStruct_MS_T
 * \param[out] metadataParam is a msParam of type KeyValPair_MS_T
 * \return integer
 * \retval 0 on success
 * \retval USER_PARAM_TYP_ERROR wheninput  param dont match the type
 * \retval INVALID_REGEXP if the tags are not correct
 * \retval from addKeyVal
 * \sa addKeyVal
 * \post
 * \pre
 * \bug  no known bugs
**/


  bytesBuf_t *metaObjBuf;
  tagStruct_t *tagValues;
  keyValPair_t *metaDataPairs;
  /*  int l1, l2; */
  int i,j;
  /*char *preg[2]; */
  regex_t preg[2];
  regmatch_t pm[2];
  char errbuff[100];
  char *t, *t1, *t2, *t3, *t4;
  char c;

  RE_TEST_MACRO ("Loopback on msiExtractTemplateMetadata");

  if (strcmp (bufParam->type,BUF_LEN_MS_T) != 0 || 
      bufParam->inpOutBuf == NULL)
    return(USER_PARAM_TYPE_ERR);
  if (strcmp (tagParam->type,TagStruct_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);
  tagValues = (tagStruct_t *) tagParam->inOutStruct;
  metaObjBuf = (bytesBuf_t *)  bufParam->inpOutBuf;
  t = malloc(metaObjBuf->len + 1);
  t[metaObjBuf->len] = '\0';
  memcpy(t,metaObjBuf->buf,metaObjBuf->len);
  metaDataPairs = mallocAndZero(sizeof(keyValPair_t));
  t1 = t;
  for (i = 0; i  < tagValues->len ; i++) {
    t1 = t;
#ifdef BABABA
    /*
    preg[0] = regcmp(tagValues->preTag[i], (char *)0);
    if (preg[0] == NULL) 
      return(INVALID_REGEXP);
    preg[1] =  regcmp(tagValues->postTag[i], (char *)0);
    if (preg[1] == NULL) 
      return(INVALID_REGEXP);
    while ((t2 = regex(preg[0], t1)) != NULL) { / * t2 starts value * /
      if ((t3 = regex(preg[1], t2)) == NULL) {
	free(preg[0]);
	free(preg[1]);
	break;
      }
      t4 = __loc1;                              / * t4 ends value * /
      c = *t4;
      *t4 = '\0';
      j = addKeyVal(metaDataPairs, tagValues->keyWord[i], t2);
      *t4 = c;
      if (j != 0)
	return(j);
      t1 = t3;
      if (*t1 == '\0')
	break;
    }
    free(preg[0]);
    free(preg[1]);
    */
#endif /*  BABABA */
    j = regcomp (&preg[0], tagValues->preTag[i], REG_EXTENDED);
    if (j != 0) {
      regerror (j,&preg[0],errbuff,sizeof(errbuff)); 
      rodsLog (LOG_NOTICE,"msiExtractTemplateMDFromBuf: Error in regcomp: %s\n",errbuff);
      return(INVALID_REGEXP);
    }
    j = regcomp (&preg[1], tagValues->postTag[i], REG_EXTENDED);
    if (j != 0) {
      regerror (j,&preg[1],errbuff,sizeof(errbuff)); 
      rodsLog (LOG_NOTICE,"msiExtractTemplateMDFromBuf: Error in regcomp: %s\n",errbuff);
      return(INVALID_REGEXP);
    }
    while (regexec(&preg[0], t1,1,&pm[0],0) == 0) {
      t2 = t1 + pm[0].rm_eo ;                     /* t2 starts value */
      if (regexec(&preg[1],t2,1,&pm[1],0) != 0)
	break;
      t4 = t2+ pm[1].rm_so;                       /* t4 ends value */
      t3 = t2+ pm[1].rm_eo;
      c = *t4;
      *t4 = '\0';
      /***      rodsLog(LOG_NOTICE,"msiExtractTemplateMDFromBuf:KVAL:%s::%s::\n",tagValues->keyWord[i], t2); ***/
      j = addKeyVal(metaDataPairs, tagValues->keyWord[i], t2);
      *t4 = c;
      if (j != 0)
	return(j);
      t1 = t3;
      if (*t1 == '\0')
	break;
    }
    regfree(&preg[0]);
    regfree(&preg[1]);
    
    continue;
  }


  metadataParam->inOutStruct = (void *) metaDataPairs;
  metadataParam->type = (char *) strdup(KeyValPair_MS_T);

  return(0);
}

int
msiAssociateKeyValuePairsToObj(msParam_t *metadataParam, msParam_t* objParam, 
			       msParam_t* typeParam, 
			       ruleExecInfo_t *rei)
{

/**
 * \fn msiAssociateKeyValuePairsToObj
 * \author  Arcot Rajasekar
 * \date   2007-02-01
 * \brief this function associateds with an object <key,value> p pairs
 *  from a given keyValPair_t structure. 
 * \note The object Type is also needed.
 * \param[in] metadataParam is a msParam of type KeyValPair_MS_T
 * \param[in] objParam   is a msParam of type STR_MS_T
 * \param[in] typeParam is a msParam of type STR_MS_T
 * \return integer
 * \retval 0 on success
 * \retval USER_PARAM_TYP_ERROR wheninput  param dont match the type
 * \retval from addAVUMetadataFromKVPairs
 * \sa addAVUMetadataFromKVPairs
 * \post
 * \pre
 * \bug  no known bugs
**/

  char *objName;
  char *objType;
  /*  int id, i;*/
  int i;

  RE_TEST_MACRO ("Loopback on msiAssociateKeyValuePairsToObj");

  if (strcmp (metadataParam->type,KeyValPair_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);
  if (strcmp (objParam->type,STR_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);
  if (strcmp (typeParam->type,STR_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);
  objName = (char *) objParam->inOutStruct;
  objType = (char *) typeParam->inOutStruct;
  i = addAVUMetadataFromKVPairs (rei->rsComm,  objName, objType,
				 (keyValPair_t *) metadataParam->inOutStruct);
  return(i);

}


int
msiGetObjType(msParam_t *objParam, msParam_t *typeParam,
	      ruleExecInfo_t *rei)
{
/**
 * \fn msiGetObjType
 * \author  Arcot Rajasekar
 * \date   2007-02-01
 * \brief this function finds from the iCat the type of a given object
 * \param[in] objParam   is a msParam of type STR_MS_T
 * \param[out] typeParam is a msParam of type STR_MS_T
 * \return integer
 * \retval 0 on success
 * \retval USER_PARAM_TYP_ERROR wheninput  param dont match the type
 * \retval  getObjType
 * \sa getObjType
 * \post
 * \pre
 * \bug  no known bugs
**/


  char *objName;
  char objType[MAX_NAME_LEN];
  int i;

  RE_TEST_MACRO ("Loopback on msiGetObjType");
  
  if (strcmp (objParam->type,STR_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);
  objName = (char *) objParam->inOutStruct;
  
  i = getObjType(rei->rsComm, objName, objType);
  if (i < 0)
    return(i);
  typeParam->inOutStruct = (char *) strdup(objType);
  typeParam->type = (char *) strdup(STR_MS_T);
  return(0);
}


int
msiRemoveKeyValuePairsFromObj(msParam_t *metadataParam, msParam_t* objParam,
                               msParam_t* typeParam, 
                               ruleExecInfo_t *rei)
{

/**
 * \fn msiRemoveKeyValuePairsFromObj
 * \author  Romain Guinot
 * \date   2008
 * \brief this function removes with an object <key,value> p pairs
 *  from a given keyValPair_t structure. 
 * \note The object Type is also needed.
 * \param[in] metadataParam is a msParam of type KeyValPair_MS_T
 * \param[in] objParam   is a msParam of type STR_MS_T
 * \param[in] typeParam is a msParam of type STR_MS_T
 * \return integer
 * \retval 0 on success
 * \retval USER_PARAM_TYP_ERROR wheninput  param dont match the type
 * \retval from removeAVUMetadataFromKVPairs
 * \sa removeAVUMetadataFromKVPairs
 * \post
 * \pre
 * \bug  no known bugs
**/

  char *objName;
  char *objType;
  /*  int id, i;*/
  int i;
  
  RE_TEST_MACRO ("Loopback on msiRemoveKeyValuePairsFromObj");
  
  if (strcmp (metadataParam->type,KeyValPair_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);
  if (strcmp (objParam->type,STR_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);
  if (strcmp (typeParam->type,STR_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);
  objName = (char *) objParam->inOutStruct;
  objType = (char *) typeParam->inOutStruct;
  i = removeAVUMetadataFromKVPairs (rei->rsComm,  objName, objType,
                                 (keyValPair_t *) metadataParam->inOutStruct);
  return(i);

}

