<?xml version="1.0" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" />
<xsl:strip-space elements="*" />

<xsl:template match="/configuration/map">
    <xsl:for-each select="entry">
        <xsl:value-of select="key/*" />
        <xsl:for-each select="value/list/*">
            <xsl:text> </xsl:text>
            <xsl:value-of select="text()" />
        </xsl:for-each>
        <xsl:text>&#xa;</xsl:text>
    </xsl:for-each>
</xsl:template>

</xsl:stylesheet>
