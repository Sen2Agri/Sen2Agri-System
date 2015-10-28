<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
   <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes" omit-xml-declaration="no" />
   <xsl:template match="Refining_Scenario">
      <Refining_Scenario xmlns="http://eop-cfi.esa.int/CFI">NONE</Refining_Scenario>
   </xsl:template>
   <xsl:template match="*">
      <xsl:choose>
         <xsl:when test="local-name(.)='Use_Default_Constant_Ozone_Amount'">
            <xsl:copy>
               <xsl:copy-of select="@*" />
               <xsl:value-of select="'true'" />
            </xsl:copy>
         </xsl:when>
         <xsl:when test="local-name(.)='CheckXMLFilesWithSchema'">
            <xsl:copy>
               <xsl:copy-of select="@*" />
               <xsl:value-of select="'false'" />
            </xsl:copy>
         </xsl:when>
         <xsl:otherwise>
            <xsl:copy>
              <xsl:for-each select="@*">
                <xsl:copy><xsl:value-of select="."/></xsl:copy>
              </xsl:for-each>
              <xsl:apply-templates select="node()"/>
            </xsl:copy>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <!-- Copy text, comments and PIs (xml-stylesheet, etc.) -->
   <xsl:template match="comment() | processing-instruction() | text()">
       <xsl:copy>
           <xsl:apply-templates />
       </xsl:copy>
   </xsl:template>  
</xsl:stylesheet>

