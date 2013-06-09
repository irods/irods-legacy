<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<xsl:template match="/">
/* this is automatically generated code */

#include "reVariableMap.gen.h"

#include "reVariableMap.h"

<xsl:apply-templates/>
</xsl:template>

<xsl:template match="def_set/struct">
<xsl:if test="@name='RuleExecInfo' or 
              @name='RsComm' or
              @name='DataObjInfo' or
              @name='DataObjInp' or
              @name='DataOprInp' or
              @name='RescInfo' or
              @name='RescGrpInfo' or
              @name='UserInfo' or
              @name='CollInfo' or
              @name='KeyValPair' or
              @name='Version' or
              @name='AuthInfo' or
              @name='UserOtherInfo' or
              @name='FileOpenInp' or
              @name='RodsHostAddr'">
<xsl:variable name="type"><xsl:value-of select="concat(translate(substring(@name,1,1), 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz'), substring(@name,2,string-length(@name)), '_t')"/></xsl:variable>
int getValFrom<xsl:value-of select="@name"/>(char *varMap, <xsl:copy-of select="$type"/> *rei, Res **varValue, Region *r)
{
  char varName[NAME_LEN];
  char *varMapCPtr;
  int i;
  
  if (varMap == NULL) {
      i = getPtrLeafValue(varValue, (void *) rei, NULL, <xsl:value-of select="@name"/>_MS_T, r);
      return(i);
  }
  if (rei == NULL)
    return(NULL_VALUE_ERR);

  i = getVarNameFromVarMap(varMap, varName, &#38;varMapCPtr);
  if (i != 0)
    return(i);

	<xsl:for-each select="struct_member">
	  if (strcmp(varName, "<xsl:value-of select="@name"/>") == 0) {
		<xsl:choose>
			<xsl:when test="array_type/base_type[@name='char']">
			    i = getStrLeafValue(varValue, rei-><xsl:value-of select="@name"/>, r);
			</xsl:when>
			<xsl:when test="base_type[@name='int']">
			    i = getIntLeafValue(varValue, rei-><xsl:value-of select="@name"/>, r);
			</xsl:when>
			<xsl:when test="base_type[@name='rodsLong_t']">
			    i = getLongLeafValue(varValue, rei-><xsl:value-of select="@name"/>, r);
			</xsl:when>
			<xsl:when test="pointer_type/base_type[@name='char']">
			    i = getStrLeafValue(varValue, rei-><xsl:value-of select="@name"/>, r);
			</xsl:when>
			<xsl:when test="pointer_type/struct_type[@name='RuleExecInfo' or 
			      @name='RsComm' or
			      @name='DataObjInfo' or
			      @name='DataObjInp' or
			      @name='DataOprInp' or
			      @name='RescInfo' or
			      @name='RescGrpInfo' or
			      @name='UserInfo' or
			      @name='CollInfo' or
			      @name='KeyValPair' or
			      @name='Version' or
			      @name='AuthInfo' or
			      @name='UserOtherInfo' or
			      @name='FileOpenInp' or
			      @name='RodsHostAddr']">
			    i = getValFrom<xsl:value-of select="pointer_type/struct_type/@name"/>(varMapCPtr, rei-><xsl:value-of select="@name"/>, varValue, r);

			</xsl:when>
			<xsl:when test="pointer_type/base_type[
			      @name='ruleExecInfo_t' or 
			      @name='rsComm_t' or
			      @name='dataObjInfo_t' or
			      @name='dataObjInp_t' or
			      @name='dataOprInp_t' or
			      @name='rescInfo_t' or
			      @name='rescGrpInfo_t' or
			      @name='userInfo_t' or
			      @name='collInfo_t' or
			      @name='keyValPair_t' or
			      @name='version_t' or
			      @name='authInfo_t' or
			      @name='userOtherInfo_t' or
			      @name='fileOpenInp_t' or
			      @name='rodsHostAddr_t']">
			    <xsl:variable name="name"><xsl:value-of select="concat(translate(substring(pointer_type/base_type/@name,1,1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring(pointer_type/base_type/@name,2,string-length(pointer_type/base_type/@name) - 3))"/></xsl:variable>
			    i = getValFrom<xsl:copy-of select="$name"/>(varMapCPtr, rei-><xsl:value-of select="@name"/>, varValue, r);
			</xsl:when>
			<xsl:otherwise>
			    i = UNDEFINED_VARIABLE_MAP_ERR;
			</xsl:otherwise>
		</xsl:choose>
		return i;
	  }
	</xsl:for-each>

    return(UNDEFINED_VARIABLE_MAP_ERR);
}
int setValFrom<xsl:value-of select="@name"/>(char *varMap, <xsl:copy-of select="$type"/> **inrei, Res *newVarValue)
{
  char varName[NAME_LEN];
  char *varMapCPtr;
  int i;
  <xsl:value-of select="$type"/> *rei;

  rei = *inrei;

  if (varMap == NULL) {
      i = setStructPtrLeafValue((void**)inrei, newVarValue);
      return(i);
  }
  if (rei == NULL)
    return(NULL_VALUE_ERR);

  i = getVarNameFromVarMap(varMap, varName, &#38;varMapCPtr);
  if (i != 0)
    return(i);

	<xsl:for-each select="struct_member">
	  if (strcmp(varName, "<xsl:value-of select="@name"/>") == 0) {
		<xsl:choose>
			<xsl:when test="array_type/base_type[@name='char']">
			    i = setStrLeafValue(rei-><xsl:value-of select="@name"/>, <xsl:value-of select="array_type/array_dim/@name"/>, newVarValue);
			</xsl:when>
			<xsl:when test="base_type[@name='int']">
			    i = setIntLeafValue(&#38;(rei-><xsl:value-of select="@name"/>), newVarValue);
			</xsl:when>
			<xsl:when test="base_type[@name='rodsLong_t']">
			    i = setLongLeafValue(&#38;(rei-><xsl:value-of select="@name"/>), newVarValue);
			</xsl:when>
			<xsl:when test="pointer_type/base_type[@name='char']">
			    i = setStrDupLeafValue(&#38;(rei-><xsl:value-of select="@name"/>), newVarValue);
			</xsl:when>
			<xsl:when test="pointer_type/struct_type[@name='RuleExecInfo' or 
			      @name='RsComm' or
			      @name='DataObjInfo' or
			      @name='DataObjInp' or
			      @name='DataOprInp' or
			      @name='RescInfo' or
			      @name='RescGrpInfo' or
			      @name='UserInfo' or
			      @name='CollInfo' or
			      @name='KeyValPair' or
			      @name='Version' or
			      @name='AuthInfo' or
			      @name='UserOtherInfo' or
			      @name='FileOpenInp' or
			      @name='RodsHostAddr']">
			    i = setValFrom<xsl:value-of select="pointer_type/struct_type/@name"/>(varMapCPtr, &#38;(rei-><xsl:value-of select="@name"/>), newVarValue);

			</xsl:when>
			<xsl:when test="pointer_type/base_type[
			      @name='ruleExecInfo_t' or 
			      @name='rsComm_t' or
			      @name='dataObjInfo_t' or
			      @name='dataObjInp_t' or
			      @name='dataOprInp_t' or
			      @name='rescInfo_t' or
			      @name='rescGrpInfo_t' or
			      @name='userInfo_t' or
			      @name='collInfo_t' or
			      @name='keyValPair_t' or
			      @name='version_t' or
			      @name='authInfo_t' or
			      @name='userOtherInfo_t' or
			      @name='fileOpenInp_t' or
			      @name='rodsHostAddr_t']">
			    <xsl:variable name="name"><xsl:value-of select="concat(translate(substring(pointer_type/base_type/@name,1,1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring(pointer_type/base_type/@name,2,string-length(pointer_type/base_type/@name) - 3))"/></xsl:variable>
			    i = setValFrom<xsl:copy-of select="$name"/>(varMapCPtr, &#38;(rei-><xsl:value-of select="@name"/>), newVarValue);
			</xsl:when>
			<xsl:otherwise>
			    i = UNDEFINED_VARIABLE_MAP_ERR;
			</xsl:otherwise>
		</xsl:choose>
		return i;
	  }
	</xsl:for-each>

    return(UNDEFINED_VARIABLE_MAP_ERR);
}
ExprType *getVarTypeFrom<xsl:value-of select="@name"/>(char *varMap, Region *r)
{
  char varName[NAME_LEN];
  char *varMapCPtr;
  int i;

  if (varMap == NULL) {
      return newIRODSType(<xsl:value-of select="@name"/>_MS_T, r);
  }

  i = getVarNameFromVarMap(varMap, varName, &#38;varMapCPtr);
  if (i != 0)
    return newErrorType(i, r);

	<xsl:for-each select="struct_member">
	  if (strcmp(varName, "<xsl:value-of select="@name"/>") == 0) {
		<xsl:choose>
			<xsl:when test="array_type/base_type[@name='char']">
			    return newSimpType(T_STRING, r);
			</xsl:when>
			<xsl:when test="base_type[@name='int']">
			    return newSimpType(T_INT, r);
			</xsl:when>
			<xsl:when test="base_type[@name='rodsLong_t']">
			    return newSimpType(T_DOUBLE, r);
			</xsl:when>
			<xsl:when test="pointer_type/base_type[@name='char']">
			    return newSimpType(T_STRING, r);
			</xsl:when>
			<xsl:when test="pointer_type/struct_type[@name='RuleExecInfo' or 
			      @name='RsComm' or
			      @name='DataObjInfo' or
			      @name='DataObjInp' or
			      @name='DataOprInp' or
			      @name='RescInfo' or
			      @name='RescGrpInfo' or
			      @name='UserInfo' or
			      @name='CollInfo' or
			      @name='KeyValPair' or
			      @name='Version' or
			      @name='AuthInfo' or
			      @name='UserOtherInfo' or
			      @name='FileOpenInp' or
			      @name='RodsHostAddr']">
			    return getVarTypeFrom<xsl:value-of select="pointer_type/struct_type/@name"/>(varMapCPtr, r);

			</xsl:when>
			<xsl:when test="pointer_type/base_type[
			      @name='ruleExecInfo_t' or 
			      @name='rsComm_t' or
			      @name='dataObjInfo_t' or
			      @name='dataObjInp_t' or
			      @name='dataOprInp_t' or
			      @name='rescInfo_t' or
			      @name='rescGrpInfo_t' or
			      @name='userInfo_t' or
			      @name='collInfo_t' or
			      @name='keyValPair_t' or
			      @name='version_t' or
			      @name='authInfo_t' or
			      @name='userOtherInfo_t' or
			      @name='fileOpenInp_t' or
			      @name='rodsHostAddr_t']">
			    <xsl:variable name="name"><xsl:value-of select="concat(translate(substring(pointer_type/base_type/@name,1,1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring(pointer_type/base_type/@name,2,string-length(pointer_type/base_type/@name) - 3))"/></xsl:variable>
			    return getVarTypeFrom<xsl:copy-of select="$name"/>(varMapCPtr, r);
			</xsl:when>
			<xsl:otherwise>
			    return newErrorType( UNDEFINED_VARIABLE_MAP_ERR, r);
			</xsl:otherwise>
		</xsl:choose>
	  }
	</xsl:for-each>

    return newErrorType(UNDEFINED_VARIABLE_MAP_ERR, r);
}
</xsl:if>
</xsl:template>
</xsl:stylesheet>
