<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <ids>
      <xsl:for-each select="attr">
        <resource name="{@name}">
          <xsl:for-each select="attrs/attr">
            <assignment name="{@name}"><xsl:value-of select="*/@value" /></assignment>
          </xsl:for-each>
        </resource>
      </xsl:for-each>
    </ids>
  </xsl:template>
</xsl:stylesheet>
