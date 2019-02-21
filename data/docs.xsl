<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:template match="/expr/attrs">
        <docs-config>
            <groups>
                <xsl:for-each select="attr[@name='groups']/attrs/attr">
                    <group name="{@name}"><xsl:value-of select="*/@value" /></group>
                </xsl:for-each>
            </groups>
            <fields>
                <xsl:for-each select="attr[@name='fields']/list/*">
                    <field><xsl:value-of select="@value" /></field>
                </xsl:for-each>
            </fields>
            <descriptions>
                <xsl:for-each select="attr[@name='descriptions']/attrs/attr">
                    <description name="{@name}"><xsl:value-of select="*/@value" /></description>
                </xsl:for-each>
            </descriptions>
        </docs-config>
    </xsl:template>
</xsl:stylesheet>
