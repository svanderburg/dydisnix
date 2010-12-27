<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/expr/attrs">
		<services>
			<xsl:for-each select="attr">
				<service name="{@name}">
					<xsl:for-each select="attrs/attr">
						<xsl:element name="{@name}">
							<xsl:value-of select="*/@value" />
							<xsl:for-each select="list/*">
								<xsl:value-of select="@value" /><xsl:text>&#x20;</xsl:text>
							</xsl:for-each>
						</xsl:element>
					</xsl:for-each>
				</service>
			</xsl:for-each>
		</services>
	</xsl:template>
</xsl:stylesheet>
