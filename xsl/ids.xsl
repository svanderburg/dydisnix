<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/expr/attrs">
    <ids>
      <ids>
        <xsl:for-each select="attr[@name='ids']/attrs/attr">
          <resource name="{@name}">
            <xsl:for-each select="attrs/attr">
              <assignment name="{@name}"><xsl:value-of select="*/@value" /></assignment>
            </xsl:for-each>
          </resource>
        </xsl:for-each>
      </ids>
      <lastAssignments>
        <xsl:for-each select="attr[@name='lastAssignments']/attrs/attr">
          <assignment name="{@name}"><xsl:value-of select="*/@value" /></assignment>
        </xsl:for-each>
      </lastAssignments>
      <lastAssignmentsPerTarget>
        <xsl:for-each select="attr[@name='lastAssignmentsPerTarget']/attrs/attr">
          <target name="{@name}">
            <xsl:for-each select="attrs/attr">
              <assignment name="{@name}"><xsl:value-of select="*/@value" /></assignment>
            </xsl:for-each>
          </target>
        </xsl:for-each>
      </lastAssignmentsPerTarget>
    </ids>
  </xsl:template>
</xsl:stylesheet>
