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
 
#include <cmath>

#define BOOST_TEST_MODULE TileMetadataWriter
#include <boost/test/unit_test.hpp>

#include "ProductMetadataWriter.hpp"

BOOST_AUTO_TEST_CASE(ProductMetadataWriter)
{
    auto writer = itk::ProductMetadataWriter::New();

    ProductFileMetadata metadata;

    metadata.GeneralInfo.ProductInfo.ProductURI = "ala bala portoclala";
    metadata.GeneralInfo.ProductInfo.ProcessingLevel = "Level-3A";
    metadata.GeneralInfo.ProductInfo.ProductType = "Composite";
    metadata.GeneralInfo.ProductInfo.ProcessingBaseline = "01.00";
    metadata.GeneralInfo.ProductInfo.GenerationTime = "2015-07-04T10:12:29.000413Z";
    metadata.GeneralInfo.ProductInfo.PreviewImageURL = "preview.jpg";
    metadata.GeneralInfo.ProductInfo.SpacecraftName = "Sentinel-2A";

    metadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLon = 29.8715;
    metadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLat = 2.22;
    metadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLon = 31.365;
    metadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLat = 2.755;

    metadata.GeneralInfo.ProductInfo.QueryOptions.PreviewImage = true;

    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.reserve(13);
    Band bandEl;

    bandEl.Resolution = 10;
    bandEl.BandName = "B1";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 20;
    bandEl.BandName = "B2";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 30;
    bandEl.BandName = "B3";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 40;
    bandEl.BandName = "B4";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 50;
    bandEl.BandName = "B5";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 60;
    bandEl.BandName = "B6";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 70;
    bandEl.BandName = "B7";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 80;
    bandEl.BandName = "B8";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 90;
    bandEl.BandName = "B9";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 100;
    bandEl.BandName = "B10";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 110;
    bandEl.BandName = "B11";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 120;
    bandEl.BandName = "B12";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 120;
    bandEl.BandName = "B12";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    bandEl.Resolution = 130;
    bandEl.BandName = "B8A";
    metadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);

    metadata.GeneralInfo.ProductInfo.QueryOptions.MetadataLevel = "SuperBrief";
    metadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.ProductLevel = "Level-3A";
    metadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.GIPP = "YES";
    metadata.GeneralInfo.ProductInfo.QueryOptions.ProductFormat = "SAFE";
    metadata.GeneralInfo.ProductInfo.QueryOptions.AggregationFlag = true;

    metadata.GeneralInfo.ProductInfo.ProductOrganisation.reserve(2);
    Granule granuleEl;

    granuleEl.GranuleIdentifier ="S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDP_N01.00";
    granuleEl.ImageFormat = "GEOTIFF";
    granuleEl.ImageIDList.reserve(1);
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDP_B02");
    metadata.GeneralInfo.ProductInfo.ProductOrganisation.emplace_back(granuleEl);

    granuleEl.GranuleIdentifier = "S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_N01.00.00";
    granuleEl.ImageFormat = "GEOTIFF";
    granuleEl.ImageIDList.reserve(13);
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B05");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B01");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B08");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B8A");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B07");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B09");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B06");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B11");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B04");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B12");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B03");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B02");
    granuleEl.ImageIDList.emplace_back("S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_B10");
    metadata.GeneralInfo.ProductInfo.ProductOrganisation.emplace_back(granuleEl);

    metadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.RedChannel = 3;
    metadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.GreenChannel = 2;
    metadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.BlueChannel = 1;

    metadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.reserve(2);

    SpecialValues specialValue;
    specialValue.SpecialValueIndex = "0";
    specialValue.SpecialValueText = "NODATA";
    metadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.emplace_back(specialValue);

    specialValue.SpecialValueIndex = "1";
    specialValue.SpecialValueText = "NOTVALID";
    metadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.emplace_back(specialValue);

    metadata.GeneralInfo.ProductImageCharacteristics.QuantificationUnit = "none";
    metadata.GeneralInfo.ProductImageCharacteristics.QuantificationValue = 1000;

    metadata.GeometricInfo.CoordReferenceSystem.HorizCSCode = "epsg:27582";
    metadata.GeometricInfo.CoordReferenceSystem.HorizCSName = "WGS84 / UTM zone 31N";

    metadata.GeometricInfo.ProductFootprint.ExtPosList.reserve(14);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(31.63118914570015);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(1.9451030140255115);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(30.68414571835096);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(1.9555317441539943);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(29.738160367940328);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(1.965487583280152);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(29.742165274932102);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(3.100934830469728);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(30.688304693419948);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(3.1019064092865207);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(31.635505628734723);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(3.102924143664107);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(31.63118914570015);
    metadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(1.945103014025511);

    metadata.GeometricInfo.ProductFootprint.RatserCSType = "POINT";
    metadata.GeometricInfo.ProductFootprint.PixelOrigin = 1;

    metadata.QualityIndicatorsInfo.CloudCoverage = "0.0";
    metadata.QualityIndicatorsInfo.TechnicalQualityAssessment.DegratedANCDataPercentage = "0";
    metadata.QualityIndicatorsInfo.TechnicalQualityAssessment.DegratedMSIDataPercentage = "0";

    metadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.FormatCorectnessFlag = "PASS";
    metadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.GeometricQualityFlag = "PASS";
    metadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.GeneralQualityFlag = "PASS";
    metadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.SensorQualityFlag = "PASS";
    metadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.RadiometricQualityFlag = "PASS";

    metadata.QualityIndicatorsInfo.QualityControlChecks.FailedInspections.reserve(2);
    GranuleReport granuleReport;
    granuleReport.GranuleReportId = "S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_N01.00";
    granuleReport.GranuleReportFileName = "S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDQ_FORMAT_CORRECTNESS_report.xml";
    metadata.QualityIndicatorsInfo.QualityControlChecks.FailedInspections.emplace_back(granuleReport);

    granuleReport.GranuleReportId = "S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDP_N01.00";
    granuleReport.GranuleReportFileName = "S2A_OPER_MSI_L1C_TL_MTI__20150627T180307_A000062_T31RDP_FORMAT_CORRECTNESS_report.xml";
    metadata.QualityIndicatorsInfo.QualityControlChecks.FailedInspections.emplace_back(granuleReport);
    writer->WriteProductMetadata(metadata, "test_productWriter.xml");

}
