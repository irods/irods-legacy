/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/**
 * @file	propertiesMS.c
 *
 * @brief	Manage a list of keyword-value pair properties.
 *
 * Properties can be used to store metadata extracted from a file or
 * database, or to build up a list of options to control how a
 * microservice prcesses data.
 *
 * @author	David R. Nadeau / University of California, San Diego
 */

#include "rsApiHandler.h"
#include "propertiesMS.h"





/**
 * Create a new empty property list.
 *
 * @param[out]		listParam	the returned new property list
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 */
int
msiPropertiesNew( msParam_t *listParam,
	ruleExecInfo_t *rei )
{
	RE_TEST_MACRO( "    Calling msiPropertiesNew" );

	// Create empty list
	fillMsParam( listParam, NULL,
		KeyValPair_MS_T, mallocAndZero( sizeof(keyValPair_t) ), NULL );
	return 0;
}

/**
 * Clear a property list.
 *
 * @param[in,out]	listParam	the property list to clear
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 */
int
msiPropertiesClear( msParam_t *listParam,
	ruleExecInfo_t *rei )
{
	RE_TEST_MACRO( "    Calling msiPropertiesClear" );

	// Check parameters
	if ( strcmp( listParam->type, KeyValPair_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;

	// Clear list
	clearKeyVal( (keyValPair_t*)listParam->inOutStruct );
	return 0;
}

/**
 * Clone a property list, returning a new property list.
 *
 * @param[in]		listParam	the property list to clone
 * @param[out]		cloneParam	the returned property list clone
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 */
int
msiPropertiesClone( msParam_t *listParam, msParam_t *cloneParam, ruleExecInfo_t *rei )
{
	RE_TEST_MACRO( "    Calling msiPropertiesClone" );

	// Check parameters
	if ( strcmp( listParam->type, KeyValPair_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;

	fillMsParam( cloneParam, NULL,
		KeyValPair_MS_T, mallocAndZero( sizeof(keyValPair_t) ), NULL );
	replKeyVal( (keyValPair_t*)listParam->inOutStruct, (keyValPair_t*)cloneParam->inOutStruct );
	return 0;
}






/**
 * Add a property and value to a property list.  If the property is already
 * in the list, its value is changed.  Otherwise the property is added.
 *
 * @param[in,out]	listParam	the property list to be added to
 * @param[in]		keywordParam	a keyword to add
 * @param[in]		valueParam	a value to add
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 * @see	#msiPropertiesSet
 */
int
msiPropertiesAdd( msParam_t *listParam, msParam_t* keywordParam, msParam_t* valueParam,
	ruleExecInfo_t *rei )
{
	RE_TEST_MACRO( "    Calling msiPropertiesAdd" );

	// Check parameters
	if ( strcmp( listParam->type, KeyValPair_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;
	if ( strcmp( keywordParam->type, STR_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;
	if ( strcmp( valueParam->type, STR_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;

	// Add entry
	addKeyVal( (keyValPair_t*)listParam->inOutStruct,
		(char*)keywordParam->inOutStruct,
		(char*)valueParam->inOutStruct );
	return 0;
}

/**
 * Remove a property from the list.
 *
 * @param[in,out]	listParam	the property list to be removed from
 * @param[in]		keywordParam	a keyword to remove
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 */
int
msiPropertiesRemove( msParam_t *listParam, msParam_t* keywordParam,
	ruleExecInfo_t *rei )
{
	RE_TEST_MACRO( "    Calling msiPropertiesRemove" );

	// Check parameters
	if ( strcmp( listParam->type, KeyValPair_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;
	if ( strcmp( keywordParam->type, STR_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;

	// Remove entry
	rmKeyVal( (keyValPair_t*)listParam->inOutStruct,
		(char*)keywordParam->inOutStruct );
	return 0;
}

/**
 * Get the value of a property in a property list.  The property list is
 * left unmodified.
 *
 * @param[in]		listParam	the property list to get from
 * @param[in]		keywordParam	a keyword to get
 * @param[out]		valueParam	a value
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 */
int
msiPropertiesGet( msParam_t *listParam, msParam_t* keywordParam, msParam_t* valueParam,
	ruleExecInfo_t *rei )
{
	char* value = NULL;

	RE_TEST_MACRO( "    Calling msiPropertiesGet" );

	// Check parameters
	if ( strcmp( listParam->type, KeyValPair_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;
	if ( strcmp( keywordParam->type, STR_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;

	// Get value and return it
	value = getValByKey( (keyValPair_t*)listParam->inOutStruct,
		(char*)keywordParam->inOutStruct );
	fillMsParam( valueParam, NULL, STR_MS_T, value, NULL );
	return 0;
}

/**
 * Set the value of a property in a property list.  If the property is already
 * in the list, its value is changed.  Otherwise the property is added.
 * Same as {@link #msiPropertiesAdd}.
 *
 * @param[in,out]	listParam	the property list to set within
 * @param[in]		keywordParam	a keyword to set
 * @param[in]		valueParam	a value
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 * @see	#msiPropertiesAdd
 */
int
msiPropertiesSet( msParam_t *listParam, msParam_t* keywordParam, msParam_t* valueParam,
	ruleExecInfo_t *rei )
{
	return msiPropertiesAdd( listParam, keywordParam, valueParam, rei );
}

/**
 * Return true (integer 1) if the keyword has a property value in the property list,
 * and false (integer 0) otherwise.  The property list is unmodified.
 *
 * @param[in]		listParam	the property list to look in
 * @param[in]		keywordParam	a keyword to get
 * @param[out]		trueFalseParam	true if set
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 */
int
msiPropertiesExists( msParam_t *listParam, msParam_t* keywordParam, msParam_t* trueFalseParam,
	ruleExecInfo_t *rei )
{
	char* value = NULL;

	RE_TEST_MACRO( "    Calling msiPropertiesExists" );

	// Check parameters
	if ( strcmp( listParam->type, KeyValPair_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;
	if ( strcmp( keywordParam->type, STR_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;

	// Get value and return true if exists
	value = getValByKey( (keyValPair_t*)listParam->inOutStruct,
		(char*)keywordParam->inOutStruct );
	fillIntInMsParam( trueFalseParam, (value==NULL) ? 0 : 1 );
	free( (char*)value );
	return 0;
}





/**
 * Convert a property list into a string buffer.  The property list is
 * left unmodified.
 *
 * @param[in]		listParam	the property list
 * @param[out]		stringParam	a string buffer
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 */
int
msiPropertiesToString( msParam_t *listParam, msParam_t* stringParam,
	ruleExecInfo_t *rei )
{
	char* string = NULL;

	RE_TEST_MACRO( "    Calling msiPropertiesToString" );

	// Check parameters
	if ( strcmp( listParam->type, KeyValPair_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;

	// Create and return string
	keyValToString( (keyValPair_t*)listParam->inOutStruct, &string );
	fillStrInMsParam( stringParam, string );
	return 0;
}

/**
 * Parse a string into a new property list.  The existing property list,
 * if any, is deleted.
 *
 * @param[in]		stringParam	a string buffer
 * @param[out]		listParam	the property list with the strings added
 * @param[in,out]	rei		the rule execution information
 * @return				the status code, 0 on success
 */
int
msiPropertiesFromString( msParam_t *stringParam, msParam_t* listParam,
	ruleExecInfo_t *rei )
{
	keyValPair_t* list = NULL;

	RE_TEST_MACRO( "    Calling msiPropertiesToString" );

	// Check parameters
	if ( strcmp( stringParam->type, STR_MS_T ) != 0 )
		return USER_PARAM_TYPE_ERR;

	// Parse string and return list
	if ( keyValFromString( (char*)stringParam->inOutStruct, &list ) == 0 )
		fillMsParam( listParam, NULL, KeyValPair_MS_T, list, NULL );
	return 0;
}

