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

#define BOOST_TEST_MODULE MACCSReader
#include <boost/test/unit_test.hpp>

#include "MACCSMetadataReader.hpp"
#include "ViewingAngles.hpp"

BOOST_AUTO_TEST_CASE(MACCSReader)
{
    auto reader = itk::MACCSMetadataReader::New();

    auto m = reader->ReadMetadata("S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1.HDR");

    BOOST_REQUIRE(m);

    BOOST_CHECK_EQUAL(m->Header.SchemaVersion, "1.00");
    BOOST_CHECK_EQUAL(m->Header.SchemaLocation,
                      "http://eop-cfi.esa.int/CFI ./SSC_PDTIMG_ImageProduct.xsd");
    BOOST_CHECK_EQUAL(m->Header.Type, "PDTIMG_Header_Type");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.FileName,
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.FileDescription, "ImageProduct");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.Notes, "L2 note");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.Mission, "SENTINEL-2A");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.FileClass, "OPER");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.FileType, "SSC_PDTIMG");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.ValidityStart, "UTC=2015-04-28T00:00:00");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.ValidityStop, "UTC=2009-12-11T00:00:00");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.FileVersion, "0003");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.SourceSystem, "MACCS");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.Creator, "MACCS_L2_INIT_CHAIN");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.CreatorVersion, "0.0.0");
    BOOST_CHECK_EQUAL(m->Header.FixedHeader.CreationDate, "UTC=2015-06-30T17:26:29");

    BOOST_CHECK_EQUAL(m->MainProductHeader.Consumers.size(), 0);
    BOOST_CHECK_EQUAL(m->MainProductHeader.Extensions.size(), 0);

    BOOST_CHECK_EQUAL(m->InstanceId.ReferenceProductSemantic, "L2VALD");
    BOOST_CHECK_EQUAL(m->InstanceId.ReferenceProductInstance, "15SVD____20091211");
    BOOST_CHECK_EQUAL(m->ReferenceProductHeaderId, "S2A_OPER_SSC_L2VALD_15SVD____20091211");

    BOOST_CHECK_EQUAL(m->ImageInformation.ElementName, "Image_Information");
    BOOST_CHECK_EQUAL(m->ImageInformation.Format, "GEOTIFF");
    BOOST_CHECK_EQUAL(m->ImageInformation.BinaryEncoding, "LITTLE_ENDIAN");
    BOOST_CHECK_EQUAL(m->ImageInformation.DataType, "SIGNED_SHORT");
    BOOST_CHECK_EQUAL(m->ImageInformation.NumberOfSignificantBits, "16");
    BOOST_CHECK_EQUAL(m->ImageInformation.NoDataValue, "-10000");
    BOOST_CHECK_EQUAL(m->ImageInformation.Size.Lines, "10980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Size.Columns, "10980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Size.Bands, "4");
    BOOST_CHECK_EQUAL(m->ImageInformation.ImageCompactingTool, "NO");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands.size(), 4);
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[0].Id, "1");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[0].Name, "B2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[1].Id, "2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[1].Name, "B3");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[2].Id, "3");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[2].Name, "B4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[3].Id, "4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[3].Name, "B8");

    m = reader->ReadMetadata("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_ATB_R1.HDR");

    BOOST_REQUIRE(m);

    BOOST_CHECK_EQUAL(m->InstanceId.AnnexCode, "ATB");
    BOOST_CHECK_EQUAL(m->AnnexCompleteName, "Other");

    BOOST_CHECK_EQUAL(m->ImageInformation.ElementName, "Annex_Information");
    BOOST_CHECK_EQUAL(m->ImageInformation.VAPNoDataValue, "0");
    BOOST_CHECK_EQUAL(m->ImageInformation.VAPQuantificationValue, "0.1");
    BOOST_CHECK_EQUAL(m->ImageInformation.AOTNoDataValue, "0");
    BOOST_CHECK_EQUAL(m->ImageInformation.AOTQuantificationValue, "0.005");
    BOOST_CHECK_EQUAL(m->ImageInformation.SubSamplingFactorLine, "1");
    BOOST_CHECK_EQUAL(m->ImageInformation.SubSamplingFactorColumn, "1");
    BOOST_CHECK_EQUAL(m->ImageInformation.ValuesUnit, "nil");

    m = reader->ReadMetadata("S2A_OPER_SSC_PDTQLK_L2VALD_15SVD____20091211.HDR");

    BOOST_REQUIRE(m);

    BOOST_CHECK_EQUAL(m->InstanceId.AnnexCode, "");
    BOOST_CHECK_EQUAL(m->AnnexCompleteName, "");

    BOOST_CHECK_EQUAL(m->ImageInformation.ElementName, "Quick_Look_Information");
    BOOST_CHECK_EQUAL(m->ImageInformation.VAPNoDataValue, "");
    BOOST_CHECK_EQUAL(m->ImageInformation.VAPQuantificationValue, "");
    BOOST_CHECK_EQUAL(m->ImageInformation.AOTNoDataValue, "");
    BOOST_CHECK_EQUAL(m->ImageInformation.AOTQuantificationValue, "");
    BOOST_CHECK_EQUAL(m->ImageInformation.SubSamplingFactor, "24");
    BOOST_CHECK_EQUAL(m->ImageInformation.SubSamplingFactorLine, "");
    BOOST_CHECK_EQUAL(m->ImageInformation.SubSamplingFactorColumn, "");
    BOOST_CHECK_EQUAL(m->ImageInformation.ColorSpace, "RGB");
    BOOST_CHECK_EQUAL(m->ImageInformation.BandsOrder, "RGB");

    m = reader->ReadMetadata("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_QLT_R1.HDR");

    BOOST_REQUIRE(m);

    BOOST_CHECK_EQUAL(m->ImageInformation.QuantificationBitValue, "1");

    m = reader->ReadMetadata("S2A_OPER_SSC_L2VALD_15SVD____20091211.HDR");

    BOOST_REQUIRE(m);

    BOOST_CHECK_EQUAL(m->InstanceId.NickName, "15SVD___");
    BOOST_CHECK_EQUAL(m->InstanceId.AcquisitionDate, "20091211");

    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.ZenithValue, 64.155021);

    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.AzimuthUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanSunAngle.AzimuthValue, 162.454864);

    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.RowUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.RowStep, "5000");

    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values.size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[0].size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[0][0], 64.777785);

    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.RowUnit, "m");
    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.RowStep, "5000");

    BOOST_CHECK_EQUAL(m->ProductInformation.SolarAngles.Azimuth.Values[0][0],
                      161.865766); // HACK fp precision

    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles.size(), 13);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].BandId, "0");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.ZenithUnit, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.ZenithValue,
                      3.300947);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.AzimuthUnit,
                      "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[0].Angles.AzimuthValue,
                      119.551215);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[12].BandId, "12");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[12].Angles.ZenithUnit,
                      "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[12].Angles.ZenithValue,
                      3.304303);
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[12].Angles.AzimuthUnit,
                      "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.MeanViewingIncidenceAngles[12].Angles.AzimuthValue,
                      119.529656);

    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles.size(), 91);
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].BandId, "0");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].DetectorId, "2");
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values.size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0].size(), 23);
    BOOST_CHECK_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0][0],
                      8.612239); // HACK fp precision
    BOOST_CHECK(std::isnan(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0][5]));

    const auto &viewingAngles = ComputeViewingAngles(m->ProductInformation.ViewingAngles);
    BOOST_CHECK_EQUAL(viewingAngles.size(), 13);
    BOOST_CHECK_EQUAL(viewingAngles[0].BandId, "0");
    BOOST_CHECK_EQUAL(viewingAngles[12].BandId, "12");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.RowUnit, "m");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.RowStep, "5000");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.ColumnUnit, "m");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.ColumnStep, "5000");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.RowUnit, "m");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.RowStep, "5000");
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Azimuth.Values[0][0], 119.857036);
    BOOST_CHECK_EQUAL(viewingAngles[0].Angles.Zenith.Values[0][0], 8.900153);

    BOOST_CHECK_EQUAL(m->ProductInformation.ReflectanceQuantificationValue, "0.000683994528");

    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions.size(), 2);
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Id, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Size.Lines, "10980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Size.Columns, "10980");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Size.Bands, "4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.UnitLengthX, "399960");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.UnitLengthY, "4400040");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.DimensionX, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.DimensionY, "-10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByLineUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByLineValue, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByColumnValue, "10");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands.size(), 4);
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[0].Id, "1");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[0].Name, "B2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[1].Id, "2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[1].Name, "B3");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[2].Id, "3");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[2].Name, "B4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[3].Id, "4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[0].Bands[3].Name, "B8");

    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Id, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Size.Lines, "5490");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Size.Columns, "5490");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Size.Bands, "6");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.UnitLengthX, "399960");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.UnitLengthY, "4400040");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.DimensionX, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.DimensionY, "-20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByLineUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByLineValue, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByColumnUnit, "m");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByColumnValue, "20");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands.size(), 6);
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[0].Id, "1");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[0].Name, "B5");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[1].Id, "2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[1].Name, "B6");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[2].Id, "3");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[2].Name, "B7");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[3].Id, "4");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[3].Name, "B8A");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[4].Id, "5");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[4].Name, "B11");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[5].Id, "6");
    BOOST_CHECK_EQUAL(m->ImageInformation.Resolutions[1].Bands[5].Name, "B12");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles.size(), 4);
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].Nature, "SSC_PDTIMG");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_FRE_R2.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].LogicalName,
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_FRE_R2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].Nature, "SSC_PDTIMG");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R2.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].LogicalName,
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R2");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[2].Nature, "SSC_PDTIMG");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[2].FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[2].LogicalName,
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[3].Nature, "SSC_PDTIMG");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[3].FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_FRE_R1.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[3].LogicalName,
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_FRE_R1");

    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles.size(), 1);
    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles[0].Nature, "SSC_PDTQLK");
    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles[0].FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTQLK_L2VALD_15SVD____20091211.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles[0].LogicalName,
                      "S2A_OPER_SSC_PDTQLK_L2VALD_15SVD____20091211");

    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles.size(), 8);
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].Id, "MSK");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.Nature, "SSC_PDTANX");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_MSK_R1.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.LogicalName,
                      "S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_MSK_R1");

    m = reader->ReadMetadata("L8_TEST_L8C_L2VALD_198030_20130626.HDR");

    BOOST_REQUIRE(m);

    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.UpperLeftCorner.UnitLong, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.UpperLeftCorner.UnitLat, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.UpperLeftCorner.Long, 0.38468);
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.UpperLeftCorner.Lat, 44.0979);
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.UpperRightCorner.UnitLong, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.UpperRightCorner.UnitLat, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.UpperRightCorner.Long, 3.1258);
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.UpperRightCorner.Lat, 44.0979);
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.LowerLeftCorner.UnitLong, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.LowerLeftCorner.UnitLat, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.LowerLeftCorner.Long, 0.38468);
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.LowerLeftCorner.Lat, 42.2751);
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.LowerRightCorner.UnitLong, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.LowerRightCorner.UnitLat, "deg");
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.LowerRightCorner.Long, 3.1258);
    BOOST_CHECK_EQUAL(m->ProductInformation.GeoCoverage.LowerRightCorner.Lat, 42.2751);

    BOOST_CHECK_EQUAL(m->ImageInformation.Bands.size(), 7);
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[0].Id, "1");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[0].Name, "B1");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[1].Id, "2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[1].Name, "B2");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[6].Id, "7");
    BOOST_CHECK_EQUAL(m->ImageInformation.Bands[6].Name, "B7");

    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles.size(), 2);
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].Nature, "L8C_PDTIMG");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].FileLocation,
                      "./L8_TEST_L8C_L2VALD_198030_20130626.DBL.DIR/"
                      "L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_FRE.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[0].LogicalName,
                      "L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_FRE");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].Nature, "L8C_PDTIMG");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].FileLocation,
                      "./L8_TEST_L8C_L2VALD_198030_20130626.DBL.DIR/"
                      "L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_SRE.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.ImageFiles[1].LogicalName,
                      "L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_SRE");

    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles.size(), 1);
    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles[0].Nature, "L8C_PDTQLK");
    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles[0].FileLocation,
                      "./L8_TEST_L8C_L2VALD_198030_20130626.DBL.DIR/"
                      "L8_TEST_L8C_PDTQLK_L2VALD_198030_20130626.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.QuickLookFiles[0].LogicalName,
                      "L8_TEST_L8C_PDTQLK_L2VALD_198030_20130626");

    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles.size(), 4);
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].Id, "MSK");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.Nature, "L8C_PDTANX");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.FileLocation,
                      "./L8_TEST_L8C_L2VALD_198030_20130626.DBL.DIR/"
                      "L8_TEST_L8C_PDTANX_L2VALD_198030_20130626_MSK.HDR");
    BOOST_CHECK_EQUAL(m->ProductOrganization.AnnexFiles[0].File.LogicalName,
                      "L8_TEST_L8C_PDTANX_L2VALD_198030_20130626_MSK");
}
