<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/expr">
		<distribution>
			<xsl:for-each select="attrs/attr">
				<service name="{@name}">
					<xsl:for-each select="list/attrs">
						<mapping>
							 <xsl:for-each select="attr">
								<xsl:element name="{@name}"><xsl:value-of select="*/@value" /></xsl:element>
							</xsl:for-each>
						</mapping>
					</xsl:for-each>
					<xsl:for-each select="list/string">
						<mapping>
							<target><xsl:value-of select="@value" /></target>
						</mapping>
					</xsl:for-each>
				</service>
			</xsl:for-each>
		</distribution>
	</xsl:template>
</xsl:stylesheet>
