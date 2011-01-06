<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output omit-xml-declaration="yes" />

	<xsl:template match="/">
	{
		<xsl:for-each select="/manifest/activation/mapping[not(name=preceding-sibling::mapping/name)]">
			<xsl:variable name="name" select="name" />
			<xsl:value-of select="$name" /> = [
				<xsl:for-each select="/manifest/activation/mapping[name=$name]/target">
					"<xsl:value-of select="hostname" />"
				</xsl:for-each>
			];
		</xsl:for-each>
	}
	</xsl:template>
</xsl:stylesheet>
