<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" indent="yes" encoding="UTF-8"/>
  <xsl:template match="/">
<metadata>
                 <xsl:for-each select="plist/dict/dict/dict">
        <xsl:sort select="key[. = 'Track ID']/following-sibling::*/text()"/>
           <xsl:sort select="key[. = 'Artist']/following-sibling::*/text()"/>
              <xsl:sort select="key[. = 'Album']/following-sibling::*/text()"/>
                 <xsl:sort select="key[. = 'Name']/following-sibling::*/text()"/>
                    <xsl:if test="contains(key[. = 'Track Type']/following-sibling::*/text(), 'File')">
          <AVU>
               <Target><xsl:apply-templates select="key[. = 'Location']" mode="pathname"/></Target>
               <Attribute>Artist</Attribute>
               <Value><xsl:apply-templates select="key[. = 'Artist']" mode="content"/></Value>
          </AVU>
          <AVU>
               <Target><xsl:apply-templates select="key[. = 'Location']" mode="pathname"/></Target>
               <Attribute>Album</Attribute>
               <Value><xsl:apply-templates select="key[. = 'Album']" mode="content"/></Value>
          </AVU>
          <AVU>
               <Target><xsl:apply-templates select="key[. = 'Location']" mode="pathname"/></Target>
               <Attribute>Name</Attribute>
               <Value><xsl:apply-templates select="key[. = 'Name']" mode="content"/></Value>
          </AVU>
          <AVU>
               <Target><xsl:apply-templates select="key[. = 'Location']" mode="pathname"/></Target>
               <Attribute>Year</Attribute>
               <Value><xsl:apply-templates select="key[. = 'Year']" mode="content"/></Value>
          </AVU>
          <AVU>
               <Target><xsl:apply-templates select="key[. = 'Location']" mode="pathname"/></Target>
               <Attribute>Genre</Attribute>
               <Value><xsl:apply-templates select="key[. = 'Genre']" mode="content"/></Value>
          </AVU>
          <AVU>
               <Target><xsl:apply-templates select="key[. = 'Location']" mode="pathname"/></Target>
               <Attribute>Total Time</Attribute>
               <Value><xsl:apply-templates select="key[. = 'Total Time']" mode="time"/></Value>
          </AVU>
               </xsl:if>
            </xsl:for-each>
    </metadata>
	
  </xsl:template>
  <xsl:template match="key" mode="pathname">
	<xsl:variable name="s_out">
		<xsl:call-template name="replace-substring">
			<xsl:with-param name="mystring" select="following-sibling::*/text()"/>
			<xsl:with-param name="from" select="'file://localhost/Users/antoine/Music/iTunes'"/>
			<xsl:with-param name="to" select="'/lifelibZone/home/adetorcy'"/>
		</xsl:call-template>
	</xsl:variable>
	<xsl:value-of select="$s_out"/>
  </xsl:template>
  
  <xsl:template match="key" mode="content">
    <xsl:value-of select="following-sibling::*/text()"/>
  </xsl:template>
  <xsl:template match="key" mode="time">
    <!-- since time is in milliseconds we have to recalculate it -->
    <xsl:variable name="mytime" select="following-sibling::*/text()"/>
	<xsl:value-of select="concat(floor ($mytime div 60000),'m ', floor((($mytime div 60000) - floor ($mytime div 60000)) * 60), 's')" />
  </xsl:template>
  
<!-- Replaces a substring with something else -->
<xsl:template name="replace-substring">
	<xsl:param name="mystring"/>
	<xsl:param name="from"/>
	<xsl:param name="to"/>
	<xsl:choose>
		<xsl:when test="contains($mystring,$from)">
		<xsl:value-of select="substring-before($mystring,$from)"/>
		<xsl:value-of select="$to"/>
		<xsl:call-template name="replace-substring">
			<xsl:with-param name="mystring" select="substring-after($mystring,$from)"/>
			<xsl:with-param name="from" select="$from"/>
			<xsl:with-param name="to" select="$to"/>
		</xsl:call-template>
		</xsl:when>
		<xsl:otherwise>
			<xsl:value-of select="$mystring"/>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>
  
</xsl:stylesheet>