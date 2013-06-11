<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:variable name="list" 
select="' RuleExecInfo RsComm DataObjInfo DataObjInp DataOprInp RescInfo RescGrpInfo UserInfo CollInfo KeyValPair Version AuthInfo UserOtherInfo FileOpenInp RodsHostAddr '"/>

<xsl:variable name="upper"
select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'"/>

<xsl:variable name="lower"
select="'abcdefghijklmnopqrstuvwxyz'"/>

<xsl:template name="mtype-template">
		<xsl:variable name="mtype">
			<xsl:choose>
				<xsl:when test="pointer_type/struct_type">
					<xsl:value-of select="pointer_type/struct_type/@name"/>
				</xsl:when>
				<xsl:when test="pointer_type/base_type and substring(pointer_type/base_type/@name, string-length(pointer_type/base_type/@name) -1) = '_t'">
					<xsl:value-of select="concat(
						translate(substring(pointer_type/base_type/@name,1,1), $lower, $upper), 
						substring(pointer_type/base_type/@name,2,string-length(pointer_type/base_type/@name) - 3))"/>
				</xsl:when>
				<xsl:otherwise>#</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="mtest">
			<xsl:call-template name="include">
				<xsl:with-param name="name" select="$mtype"/>
			</xsl:call-template>
		</xsl:variable>
		<xsl:choose>
			<xsl:when test="contains($mtest, 'true')">
				<xsl:value-of select="$mtype"/>
			</xsl:when>
			<xsl:otherwise>#</xsl:otherwise>
		</xsl:choose>
</xsl:template>

<xsl:template name="to-base-type">
	<xsl:param name="name"/>
	<xsl:value-of select="
		concat(
			translate(substring($name,1,1), $upper, $lower), 
			substring($name,2,string-length($name)), 
			'_t'
		)"/>
</xsl:template>

<xsl:template name="include">
<xsl:param name="name"/>
<xsl:value-of select="
contains(
	$list,
	concat(' ', $name, ' ')
)"/>
</xsl:template>


</xsl:stylesheet>
