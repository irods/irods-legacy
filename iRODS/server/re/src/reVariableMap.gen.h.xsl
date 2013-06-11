<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<xsl:include href="reVariableMap.gen.g.xsl"/>

<xsl:template match="/">
/* this is automatically generated C code */

#ifndef RE_VARIABLE_MAP_GEN
#define RE_VARIABLE_MAP_GEN
#include "reGlobalsExtern.h"
#include "reVariables.h"
#include "rcMisc.h"
#include "restructs.h"

<xsl:apply-templates/>
#endif
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
#define <xsl:value-of select="@name"/>_MS_T "<xsl:value-of select="@name"/>_PI"
int setValFrom<xsl:value-of select="@name"/>(char *varMap, <xsl:copy-of select="$type"/> **inrei, Res *newVarValue);
int getValFrom<xsl:value-of select="@name"/>(char *varMap, <xsl:copy-of select="$type"/> *inrei, Res **varValue, Region *r);
ExprType *getVarTypeFrom<xsl:value-of select="@name"/>(char *varMap, Region *r);
</xsl:if>
</xsl:template>
</xsl:stylesheet>
