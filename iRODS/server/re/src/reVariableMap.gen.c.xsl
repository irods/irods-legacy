<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<xsl:include href="reVariableMap.gen.g.xsl"/>
<xsl:template match="/">
/* this is automatically generated C code */

#include "reVariableMap.gen.h"

#include "reVariableMap.h"

<xsl:apply-templates/>
</xsl:template>

<xsl:template match="def_set/struct">

<xsl:variable name="test">
	<xsl:call-template name="include">
		<xsl:with-param name="name" select="@name"/>
	</xsl:call-template>
</xsl:variable>

<xsl:if test="contains($test,'true')">

<xsl:variable name="type">
	<xsl:call-template name="to-base-type">
		<xsl:with-param name="name" select="@name"/>
	</xsl:call-template>
</xsl:variable>

int getValFrom<xsl:value-of select="@name"/>(char *varMap, <xsl:value-of select="$type"/> *rei, Res **varValue, Region *r)
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
		<xsl:variable name="mtype">
			<xsl:call-template name="mtype-template"/>
		</xsl:variable>

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
			<xsl:when test="$mtype != '#'">
			    i = getValFrom<xsl:value-of select="$mtype"/>(varMapCPtr, rei-><xsl:value-of select="@name"/>, varValue, r);
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
	  	<xsl:variable name="mtype">
	  		<xsl:call-template name="mtype-template"/>
	  	</xsl:variable>
	  	
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
			<xsl:when test="$mtype != '#'">
			    i = setValFrom<xsl:value-of select="$mtype"/>(varMapCPtr, &#38;(rei-><xsl:value-of select="@name"/>), newVarValue);
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
	  	<xsl:variable name="mtype">
	  		<xsl:call-template name="mtype-template"/>
	  	</xsl:variable>
	  	
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
			<xsl:when test="$mtype != '#'">
			    return getVarTypeFrom<xsl:value-of select="$mtype"/>(varMapCPtr, r);
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
