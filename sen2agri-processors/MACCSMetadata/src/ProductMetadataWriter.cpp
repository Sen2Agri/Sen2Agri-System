/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
#include "ProductMetadataWriter.hpp"

namespace itk
{

XElement Format(const Granule &granuleContent)
{
    XElement granulesEl("Granules", XAttribute("granuleIdentifier", granuleContent.GranuleIdentifier),
                                               XAttribute("imageFormat", granuleContent.ImageFormat));
    for (const auto &imageID : granuleContent.ImageIDList) {
        granulesEl.Append(XElement("IMAGE_ID", imageID));
    }

    return granulesEl;
}


XElement Format(const std::vector<Granule> &granules)
{
    XElement granulesEl("Granule_List");

    for (const auto &granule : granules) {
            granulesEl.Append(Format(granule));
     }
    return granulesEl;
}

XElement Format(const std::vector<Band> &bands)
{
    XElement bandsEl("Band_List");

    for (const auto &band : bands) {
        bandsEl.Append(XElement("BAND_NAME", XAttribute("resolution", std::to_string(band.Resolution)), XText(band.BandName)));
    }

    return bandsEl;
}

XElement Format(const AuxList &AuxListContent)
{
    XElement AuxListEl("Aux_List", XAttribute("productLevel", AuxListContent.ProductLevel),
                      XElement("aux",
                              XElement("GIPP", AuxListContent.GIPP),
                              XElement("ISD", AuxListContent.ISD)));

    return AuxListEl;
}


XElement Format(const QueryOptionsContent &ProductQueryOptions)
{
    std::string strLowerCorner = std::to_string(ProductQueryOptions.AreaOfInterest.LowerCornerLon) + " " +  std::to_string(ProductQueryOptions.AreaOfInterest.LowerCornerLat);
    std::string strUpperCorner = std::to_string(ProductQueryOptions.AreaOfInterest.UpperCornerLon) + " " + std::to_string(ProductQueryOptions.AreaOfInterest.UpperCornerLat);
    std::string previewImg = "true";
    std::string aggregationFlag = "true";

    if(ProductQueryOptions.PreviewImage == false)
    {
        previewImg = "false";
    }

    if(ProductQueryOptions.AggregationFlag == false)
    {
        aggregationFlag = "false";
    }

    XElement ProductQueryInfoEl("Query_Options",
                   XElement("Area_Of_Interest",
                                   XElement("Bbox",
                                            XElement("LOWER_CORNER", strLowerCorner),
                                            XElement("UPPER_CORNER", strUpperCorner))),
                    XElement("PREVIEW_IMAGE", previewImg),
                    Format(ProductQueryOptions.BandList),
                    XElement("METADATA_LEVEL", ProductQueryOptions.MetadataLevel),
                    Format(ProductQueryOptions.AuxListContent),
                    XElement("PRODUCT_FORMAT", ProductQueryOptions.ProductFormat),
                    XElement("AGGREGATION_FLAG", aggregationFlag));

    return ProductQueryInfoEl;
}


XElement Format(const ProductInfoMetadata &ProductInfoContent)
{

    XElement ProductInfoEl("Product_Info",
                   XElement("PRODUCT_URI", ProductInfoContent.ProductURI),
                   XElement("PROCESSING_LEVEL", ProductInfoContent.ProcessingLevel),
                   XElement("PRODUCT_TYPE", ProductInfoContent.ProductType),
                   XElement("PROCESSING_BASELINE", ProductInfoContent.ProcessingBaseline),
                   XElement("GENERATION_TIME", ProductInfoContent.GenerationTime),
                   XElement("PREVIEW_IMAGE_URL", ProductInfoContent.PreviewImageURL),
                   XElement("SPACECRAFT_NAME", ProductInfoContent.SpacecraftName),
                   XElement("Query_Options"), Format(ProductInfoContent.QueryOptions),
                   XElement("Product_Organisation"), Format(ProductInfoContent.ProductOrganisation));

    return ProductInfoEl;
}

XElement Format(const ProductImageCharacteristicsMetadata &ProductImageCharacteristics)
{

    XElement ProductImageCharEl("Product_Image_Characteristics");
    for (const auto &specialValues : ProductImageCharacteristics.SpecialValuesList) {
        ProductImageCharEl.Append(XElement("Special_Values",
                                           XElement("SPECIAL_VALUE_TEXT", specialValues.SpecialValueText),
                                           XElement("SPECIAL_VALUE_INDEX", specialValues.SpecialValueIndex)));
    }

    if((ProductImageCharacteristics.ImageDisplayOrder.RedChannel == 0) &&
       (ProductImageCharacteristics.ImageDisplayOrder.GreenChannel == 0) &&
       (ProductImageCharacteristics.ImageDisplayOrder.BlueChannel == 0))
    {
        //crop mask and crop type products
        ProductImageCharEl.Append(XElement("Image_Display_Order",
                                       XElement("GREY_CHANNEL", std::to_string(ProductImageCharacteristics.ImageDisplayOrder.RedChannel))));

    }
    else
    {
        ProductImageCharEl.Append(XElement("Image_Display_Order",
                                       XElement("RED_CHANNEL", std::to_string(ProductImageCharacteristics.ImageDisplayOrder.RedChannel)),
                                       XElement("GREEN_CHANNEL", std::to_string(ProductImageCharacteristics.ImageDisplayOrder.GreenChannel)),
                                       XElement("BLUE_CHANNEL", std::to_string(ProductImageCharacteristics.ImageDisplayOrder.BlueChannel))));

    }

    ProductImageCharEl.Append(XElement("QUANTIFICATION_VALUE", XAttribute("unit", ProductImageCharacteristics.QuantificationUnit),
                                       XText(std::to_string(ProductImageCharacteristics.QuantificationValue))));
    return ProductImageCharEl;
}

XElement Format(const ProductFootprintMetadata &ProductFootprint)
{
    std::string strExtPosList = "";

    for (const auto &footprint : ProductFootprint.ExtPosList) {
        strExtPosList += std::to_string(footprint) + " ";
    }

    XElement ProductFootprintEl("Product_Footprint");
    ProductFootprintEl.Append(XElement("Global_Footprint",
                                     XElement("EXT_POS_LIST", strExtPosList)));

    ProductFootprintEl.Append(XElement("RASTER_CS_TYPE", ProductFootprint.RatserCSType));
    ProductFootprintEl.Append(XElement("PIXEL_ORIGIN", std::to_string(ProductFootprint.PixelOrigin)));

    return ProductFootprintEl;
}

XElement Format(const CoordReferenceSystemMetadata &CoordRefSystem)
{

    XElement ProductCoordRefSystemEl("Coordinate_Reference_System");

    std::cout << "CoordRefSystem.HorizCSName =" << CoordRefSystem.HorizCSName << "  CoordRefSystem.HorizCSName.empty() =" << CoordRefSystem.HorizCSName.empty()  << std::endl;

    if(!CoordRefSystem.HorizCSName.empty())
    {
        ProductCoordRefSystemEl.Append(XElement("HORIZONTAL_CS_NAME", CoordRefSystem.HorizCSName));
    }

    if(!CoordRefSystem.HorizCSCode.empty())
    {
        ProductCoordRefSystemEl.Append(XElement("HORIZONTAL_CS_CODE", CoordRefSystem.HorizCSCode));
    }

    if(!CoordRefSystem.HorizCSType.empty())
    {
        ProductCoordRefSystemEl.Append(XElement("HORIZONTAL_CS_TYPE", CoordRefSystem.HorizCSType));
    }

    if(!CoordRefSystem.GeoTables.empty())
    {
        ProductCoordRefSystemEl.Append(XElement("GEO_TABLES", XAttribute("version", std::to_string(CoordRefSystem.nGeoTablesVersion)), XText(CoordRefSystem.HorizCSType)));
    }
    return ProductCoordRefSystemEl;
}

XElement Format(const std::vector<GIPPInfo> &GIPPInfoList)
{
    XElement addEl("GIPP_List");

    for (const auto &GIPPInfoEl : GIPPInfoList) {
        addEl.Append(XElement("GIPP_FILENAME",
                              XAttribute("type", GIPPInfoEl.GIPPType),
                              XAttribute("version", GIPPInfoEl.GIPPVersion), XText(GIPPInfoEl.GIPPFileName)));
    }

    return addEl;
}

XElement Format(const std::vector<ISDInfo> &ISDInfoList)
{
    XElement addEl("ISD_List");

    for (const auto &ISDInfoEl : ISDInfoList) {
        addEl.Append(XElement("ISD_FILENAME",
                              XText(ISDInfoEl.ISDFileName)));
    }

    return addEl;
}

XElement Format(const TechnicalQualityAssessmentMetadata &TechnicalQualityAss)
{

    XElement addEl("Technical_Quality_Assessment");

    addEl.Append(XElement("DEGRADED_ANC_DATA_PERCENTAGE", TechnicalQualityAss.DegratedANCDataPercentage));
    addEl.Append(XElement("DEGRADED_MSI_DATA_PERCENTAGE", TechnicalQualityAss.DegratedMSIDataPercentage));

    return addEl;
}

XElement Format(const std::vector<GranuleReport> &GranuleReportList)
{
    XElement addEl("Failed_Inspections");

    for (const auto &GranuleReportEl : GranuleReportList) {
        addEl.Append(XElement("Granule_Report",
                              XAttribute("granuleId", GranuleReportEl.GranuleReportId),
                              XElement("REPORT_FILENAME", GranuleReportEl.GranuleReportFileName)));
    }

    return addEl;
}

XElement Format(const QualityControlChecksMetadata &QualityControlChecks)
{

    XElement addEl("Quality_Control_Checks");

    addEl.Append(XElement("Quality_Inspections",
                          XElement("SENSOR_QUALITY_FLAG", QualityControlChecks.QualityInspections.SensorQualityFlag),
                          XElement("GEOMETRIC_QUALITY_FLAG", QualityControlChecks.QualityInspections.GeneralQualityFlag),
                          XElement("GENERAL_QUALITY_FLAG", QualityControlChecks.QualityInspections.GeneralQualityFlag),
                          XElement("FORMAT_CORRECTNESS_FLAG", QualityControlChecks.QualityInspections.FormatCorectnessFlag),
                          XElement("RADIOMETRIC_QUALITY_FLAG", QualityControlChecks.QualityInspections.RadiometricQualityFlag)));

    addEl.Append(Format(QualityControlChecks.FailedInspections));

    return addEl;
}

XDocument ProductMetadataWriter::CreateProductMetadataXml(const ProductFileMetadata &metadata)
{

    XDocument doc(XDeclaration("1.0", "UTF-8", ""));

    XElement root(metadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.ProductLevel + "_User_Product",
                      XAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance"));

    root.Append(
        XElement(
           "General_Info",
            Format(metadata.GeneralInfo.ProductInfo),
            Format(metadata.GeneralInfo.ProductImageCharacteristics)));

    root.Append(
        XElement(
           "Geometric_Info",
            Format(metadata.GeometricInfo.ProductFootprint),
            Format(metadata.GeometricInfo.CoordReferenceSystem)));

    root.Append(
        XElement(
           "Auxiliary_Data_Info",
            Format(metadata.AuxiliaryDataInfo.GIPPList),
            Format(metadata.AuxiliaryDataInfo.ISDList)));

    root.Append(
        XElement(
           "Quality_Indicators_Info",
            XElement("Cloud_Coverage_Assessment", metadata.QualityIndicatorsInfo.CloudCoverage),
                    Format(metadata.QualityIndicatorsInfo.TechnicalQualityAssessment),
                    Format(metadata.QualityIndicatorsInfo.QualityControlChecks)));
    doc.Append(root);


    return doc;
}

void ProductMetadataWriter::WriteProductMetadata(const ProductFileMetadata &metadata, const std::string &path)
{
    CreateProductMetadataXml(metadata).Save(path);
}
}
