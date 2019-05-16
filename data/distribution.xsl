<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/">
		<distribution>
			<xsl:for-each select="/expr/attrs/attr">
				<service name="{@name}">
					<xsl:for-each select="list/string">
						<target><xsl:value-of select="@value" /></target>
					</xsl:for-each>
				</service>
			</xsl:for-each>
		</distribution>
	</xsl:template>
</xsl:stylesheet>
