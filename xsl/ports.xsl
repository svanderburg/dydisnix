<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/expr/attrs/attr[@name='portConfiguration']/attrs">
		<portConfiguration>
			<xsl:for-each select="attr[@name='globalConfig']">
				<globalConfig>
					<lastPort><xsl:value-of select="attrs/attr[@name='lastPort']/int/@value" /></lastPort>
					<minPort><xsl:value-of select="attrs/attr[@name='minPort']/int/@value" /></minPort>
					<maxPort><xsl:value-of select="attrs/attr[@name='maxPort']/int/@value" /></maxPort>
					<servicesToPorts>
						<xsl:for-each select="attrs/attr[@name='servicesToPorts']/attrs/attr">
						    <xsl:element name="{@name}"><xsl:value-of select="int/@value" /></xsl:element>
						</xsl:for-each>
					</servicesToPorts>
				</globalConfig>
			</xsl:for-each>
			<targetConfigs>
				<xsl:for-each select="attr[@name='targetConfigs']/attrs/attr">
					<xsl:element name="{@name}">
						<lastPort><xsl:value-of select="attrs/attr[@name='lastPort']/int/@value" /></lastPort>
						<minPort><xsl:value-of select="attrs/attr[@name='minPort']/int/@value" /></minPort>
						<maxPort><xsl:value-of select="attrs/attr[@name='maxPort']/int/@value" /></maxPort>
						<servicesToPorts>
							<xsl:for-each select="attrs/attr[@name='servicesToPorts']/attrs/attr">
							    <xsl:element name="{@name}"><xsl:value-of select="int/@value" /></xsl:element>
							</xsl:for-each>
						</servicesToPorts>
					</xsl:element>
				</xsl:for-each>
			</targetConfigs>
		</portConfiguration>
	</xsl:template>
</xsl:stylesheet>
