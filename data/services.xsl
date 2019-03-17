<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/expr/attrs">
		<services>
			<xsl:for-each select="attr">
				<service>
					<name><xsl:value-of select="@name" /></name>
					<dependsOn>
						<xsl:for-each select="attrs/attr[@name='dependsOn']">
							<xsl:for-each select="list/*">
								<dependency><xsl:value-of select="@value" /></dependency>
							</xsl:for-each>
						</xsl:for-each>
					</dependsOn>
					<connectsTo>
						<xsl:for-each select="attrs/attr[@name='connectsTo']">
							<xsl:for-each select="list/*">
								<dependency><xsl:value-of select="@value" /></dependency>
							</xsl:for-each>
						</xsl:for-each>
					</connectsTo>
					<properties>
						<xsl:for-each select="attrs/attr[not(@name='dependsOn' or @name='connectsTo')]">
							<xsl:element name="{@name}">
								<xsl:value-of select="*/@value" />
								<xsl:for-each select="list/*">
									<xsl:value-of select="@value" /><xsl:text>&#x20;</xsl:text>
								</xsl:for-each>
							</xsl:element>
						</xsl:for-each>
					</properties>
				</service>
			</xsl:for-each>
		</services>
	</xsl:template>
</xsl:stylesheet>
