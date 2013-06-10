<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<xsl:template match="/">
/* this is automatically generated code */

#include "reGlobalsExtern.h"
#include "reVariables.h"
#include "rcMisc.h"
#include "restructs.h"

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
#define <xsl:value-of select="@name"/>_MS_T "<xsl:value-of select="@name"/>_PI"
<xsl:variable name="type"><xsl:value-of select="concat(translate(substring(@name,1,1), 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz'), substring(@name,2,string-length(@name)), '_t')"/></xsl:variable>
int setValFrom<xsl:value-of select="@name"/>(char *varMap, <xsl:copy-of select="$type"/> **inrei, Res *newVarValue);
int getValFrom<xsl:value-of select="@name"/>(char *varMap, <xsl:copy-of select="$type"/> *inrei, Res **varValue, Region *r);
ExprType *getVarTypeFrom<xsl:value-of select="@name"/>(char *varMap, Region *r);
</xsl:if>
</xsl:template>
</xsl:stylesheet>
