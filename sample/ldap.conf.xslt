<?xml version="1.0" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" />
<xsl:strip-space elements="*" />

<xsl:template match="/configuration/map">
    <xsl:for-each select="entry">
        <xsl:choose>
            <xsl:when test="name(value/*) = 'string'">
                <xsl:value-of select="key/*" />
                <xsl:text> </xsl:text>
                <xsl:value-of select="value/string" />
                <xsl:text>&#xa;</xsl:text>
            </xsl:when>
            <xsl:when test="name(value/*) = 'list'">
                <xsl:value-of select="key/*" />
                <xsl:for-each select="value/list/*">
                    <xsl:text> </xsl:text>
                    <xsl:value-of select="text()" />
                    <xsl:if test="not(position() = last())">,</xsl:if>
                </xsl:for-each>
                <xsl:text>&#xa;</xsl:text>
            </xsl:when>
            <xsl:when test="name(value/*) = 'map'">
                <xsl:for-each select="value/map/entry">
                    <xsl:value-of select="key/*" />
                    <xsl:text> </xsl:text>
                    <xsl:value-of select="key/*" />
                    <xsl:text> </xsl:text>
                    <xsl:value-of select="value/*" />
                    <xsl:text>&#xa;</xsl:text>
                </xsl:for-each>
            </xsl:when>
        </xsl:choose>
    </xsl:for-each>
</xsl:template>

</xsl:stylesheet>
