<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output omit-xml-declaration="yes" />

	<xsl:template match="/">
	{
		<xsl:for-each select="/manifest/services/service[not(name=preceding-sibling::service/name)]">
			<xsl:variable name="key" select="@name" />
			<xsl:value-of select="name" /> = [
				<xsl:for-each select="/manifest/serviceMappings/mapping[service=$key]">
					"<xsl:value-of select="target" />"
				</xsl:for-each>
			];
		</xsl:for-each>
	}
	</xsl:template>
</xsl:stylesheet>
