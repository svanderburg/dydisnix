<?xml version="1.0"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/expr/attrs">
		<services>
			<xsl:for-each select="attr">
				<service name="{@name}">
					<property name="name"><xsl:value-of select="@name" /></property>
					<property name="dependsOn">
						<xsl:for-each select="attrs/attr[@name='dependsOn']">
							<xsl:for-each select="list/*">
								<dependency><xsl:value-of select="@value" /></dependency>
							</xsl:for-each>
						</xsl:for-each>
					</property>
					<property name="connectsTo">
						<xsl:for-each select="attrs/attr[@name='connectsTo']">
							<xsl:for-each select="list/*">
								<dependency><xsl:value-of select="@value" /></dependency>
							</xsl:for-each>
						</xsl:for-each>
					</property>
					<xsl:for-each select="attrs/attr[not(@name='dependsOn' or @name='connectsTo')]">
						<property name="{@name}">
							<xsl:value-of select="*/@value" />
							<xsl:for-each select="list/*">
								<xsl:value-of select="@value" /><xsl:text>&#x20;</xsl:text>
							</xsl:for-each>
						</property>
					</xsl:for-each>
				</service>
			</xsl:for-each>
		</services>
	</xsl:template>
</xsl:stylesheet>
