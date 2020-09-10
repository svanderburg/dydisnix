<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <resources>
      <xsl:for-each select="attr">
        <resource name="{@name}">
          <min><xsl:value-of select="attrs/attr[@name='min']/*/@value" /></min>
          <max><xsl:value-of select="attrs/attr[@name='max']/*/@value" /></max>
          <scope><xsl:value-of select="attrs/attr[@name='scope']/*/@value" /></scope>
        </resource>
      </xsl:for-each>
    </resources>
  </xsl:template>
</xsl:stylesheet>
