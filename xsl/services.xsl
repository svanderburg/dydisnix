<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <!-- Transformation templates -->

  <xsl:template match="string | int | float | bool">
    <xsl:attribute name="type">
      <xsl:value-of select="local-name()" />
    </xsl:attribute>

    <xsl:value-of select="@value" />
  </xsl:template>

  <xsl:template match="list">
    <xsl:attribute name="type">list</xsl:attribute>

    <xsl:for-each select="*">
      <elem>
        <xsl:apply-templates select="." />
      </elem>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="convert_attrs_verbose">
    <xsl:for-each select="attr">
      <property name="{@name}">
        <xsl:apply-templates select="*" />
      </property>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="attrs">
    <xsl:attribute name="type">attrs</xsl:attribute>
    <xsl:call-template name="convert_attrs_verbose" />
  </xsl:template>

  <!-- Transformation procedure -->

  <xsl:template match="/expr/attrs">
    <services>
      <xsl:for-each select="attr">
        <service name="{@name}">
          <property name="dependsOn">
            <xsl:for-each select="attrs/attr[@name='dependsOn']">
              <xsl:for-each select="list/*">
                <dependency>
                  <xsl:value-of select="@value"/>
                </dependency>
              </xsl:for-each>
            </xsl:for-each>
          </property>
          <property name="connectsTo">
            <xsl:for-each select="attrs/attr[@name='connectsTo']">
              <xsl:for-each select="list/*">
                <dependency>
                  <xsl:value-of select="@value"/>
                </dependency>
              </xsl:for-each>
            </xsl:for-each>
          </property>
          <xsl:for-each select="attrs/attr[not(@name='dependsOn' or @name='connectsTo')]">
            <property name="{@name}">
              <xsl:apply-templates select="*" />
            </property>
          </xsl:for-each>
        </service>
      </xsl:for-each>
    </services>
  </xsl:template>
</xsl:stylesheet>
