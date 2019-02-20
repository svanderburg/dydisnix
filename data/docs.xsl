<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:template match="/expr/attrs">
        <docs-config>
            <groups>
                <xsl:for-each select="attr[@name='groups']/attrs/attr">
                    <group name="{@name}"><xsl:value-of select="*/@value" /></group>
                </xsl:for-each>
            </groups>
        </docs-config>
    </xsl:template>
</xsl:stylesheet>
