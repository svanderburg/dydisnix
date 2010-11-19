<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output omit-xml-declaration="yes" />

	<xsl:template match="/">
		{infrastructure}:

		{
			<xsl:for-each select="/expr/attrs/attr">
				<xsl:value-of select="@name" /> =
				[
					<xsl:for-each select="list/string">
						infrastructure.<xsl:value-of select="@value" />
					</xsl:for-each>
				];
			</xsl:for-each>
		}
	</xsl:template>
</xsl:stylesheet>
