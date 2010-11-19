<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/">
		<distribution>
			<xsl:for-each select="/expr/attrs/attr">
				<distributionitem>
					<service><xsl:value-of select="@name" /></service>
					<targets>
						<xsl:for-each select="list/string">
							<target><xsl:value-of select="@value" /></target>
						</xsl:for-each>
					</targets>
				</distributionitem>
			</xsl:for-each>
		</distribution>
	</xsl:template>
</xsl:stylesheet>
