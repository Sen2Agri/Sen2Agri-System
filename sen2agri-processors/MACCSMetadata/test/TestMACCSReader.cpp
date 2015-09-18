#include <cmath>

#include "itkTestingMacros.h"

#include "MACCSMetadataReader.hpp"

int main()
{
    auto reader = itk::MACCSMetadataReader::New();

    auto m = reader->ReadMetadata("S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1.HDR");

    TEST_EXPECT_TRUE(m);

    TEST_EXPECT_EQUAL(m->Header.SchemaVersion, "1.00");
    TEST_EXPECT_EQUAL(m->Header.SchemaLocation,
                      "http://eop-cfi.esa.int/CFI ./SSC_PDTIMG_ImageProduct.xsd");
    TEST_EXPECT_EQUAL(m->Header.Type, "PDTIMG_Header_Type");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.FileName,
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.FileDescription, "ImageProduct");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.Notes, "L2 note");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.Mission, "SENTINEL-2A");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.FileClass, "OPER");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.FileType, "SSC_PDTIMG");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.ValidityStart, "UTC=2015-04-28T00:00:00");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.ValidityStop, "UTC=2009-12-11T00:00:00");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.FileVersion, "0003");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.SourceSystem, "MACCS");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.Creator, "MACCS_L2_INIT_CHAIN");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.CreatorVersion, "0.0.0");
    TEST_EXPECT_EQUAL(m->Header.FixedHeader.CreationDate, "UTC=2015-06-30T17:26:29");

    TEST_EXPECT_EQUAL(m->MainProductHeader.Consumers.size(), 0);
    TEST_EXPECT_EQUAL(m->MainProductHeader.Extensions.size(), 0);

    TEST_EXPECT_EQUAL(m->InstanceId.ReferenceProductSemantic, "L2VALD");
    TEST_EXPECT_EQUAL(m->InstanceId.ReferenceProductInstance, "15SVD____20091211");
    TEST_EXPECT_EQUAL(m->ReferenceProductHeaderId, "S2A_OPER_SSC_L2VALD_15SVD____20091211");

    TEST_EXPECT_EQUAL(m->ImageInformation.ElementName, "Image_Information");
    TEST_EXPECT_EQUAL(m->ImageInformation.Format, "GEOTIFF");
    TEST_EXPECT_EQUAL(m->ImageInformation.BinaryEncoding, "LITTLE_ENDIAN");
    TEST_EXPECT_EQUAL(m->ImageInformation.DataType, "SIGNED_SHORT");
    TEST_EXPECT_EQUAL(m->ImageInformation.NumberOfSignificantBits, "16");
    TEST_EXPECT_EQUAL(m->ImageInformation.NoDataValue, "-10000");
    TEST_EXPECT_EQUAL(m->ImageInformation.Size.Lines, "10980");
    TEST_EXPECT_EQUAL(m->ImageInformation.Size.Columns, "10980");
    TEST_EXPECT_EQUAL(m->ImageInformation.Size.Bands, "4");
    TEST_EXPECT_EQUAL(m->ImageInformation.ImageCompactingTool, "NO");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands.size(), 4);
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[0].Id, "1");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[0].Name, "B2");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[1].Id, "2");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[1].Name, "B3");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[2].Id, "3");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[2].Name, "B4");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[3].Id, "4");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[3].Name, "B8");

    m = reader->ReadMetadata("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_ATB_R1.HDR");

    TEST_EXPECT_TRUE(m);

    TEST_EXPECT_EQUAL(m->InstanceId.AnnexCode, "ATB");
    TEST_EXPECT_EQUAL(m->AnnexCompleteName, "Other");

    TEST_EXPECT_EQUAL(m->ImageInformation.ElementName, "Annex_Information");
    TEST_EXPECT_EQUAL(m->ImageInformation.VAPNoDataValue, "0");
    TEST_EXPECT_EQUAL(m->ImageInformation.VAPQuantificationValue, "0.1");
    TEST_EXPECT_EQUAL(m->ImageInformation.AOTNoDataValue, "0");
    TEST_EXPECT_EQUAL(m->ImageInformation.AOTQuantificationValue, "0.005");
    TEST_EXPECT_EQUAL(m->ImageInformation.SubSamplingFactorLine, "1");
    TEST_EXPECT_EQUAL(m->ImageInformation.SubSamplingFactorColumn, "1");
    TEST_EXPECT_EQUAL(m->ImageInformation.ValuesUnit, "nil");

    m = reader->ReadMetadata("S2A_OPER_SSC_PDTQLK_L2VALD_15SVD____20091211.HDR");

    TEST_EXPECT_TRUE(m);

    TEST_EXPECT_EQUAL(m->InstanceId.AnnexCode, "");
    TEST_EXPECT_EQUAL(m->AnnexCompleteName, "");

    TEST_EXPECT_EQUAL(m->ImageInformation.ElementName, "Quick_Look_Information");
    TEST_EXPECT_EQUAL(m->ImageInformation.VAPNoDataValue, "");
    TEST_EXPECT_EQUAL(m->ImageInformation.VAPQuantificationValue, "");
    TEST_EXPECT_EQUAL(m->ImageInformation.AOTNoDataValue, "");
    TEST_EXPECT_EQUAL(m->ImageInformation.AOTQuantificationValue, "");
    TEST_EXPECT_EQUAL(m->ImageInformation.SubSamplingFactor, "24");
    TEST_EXPECT_EQUAL(m->ImageInformation.SubSamplingFactorLine, "");
    TEST_EXPECT_EQUAL(m->ImageInformation.SubSamplingFactorColumn, "");
    TEST_EXPECT_EQUAL(m->ImageInformation.ColorSpace, "RGB");
    TEST_EXPECT_EQUAL(m->ImageInformation.BandsOrder, "RGB");

    m = reader->ReadMetadata("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_QLT_R1.HDR");

    TEST_EXPECT_TRUE(m);

    TEST_EXPECT_EQUAL(m->ImageInformation.QuantificationBitValue, "1");

    m = reader->ReadMetadata("S2A_OPER_SSC_L2VALD_15SVD____20091211.HDR");

    TEST_EXPECT_TRUE(m);

    TEST_EXPECT_EQUAL(m->InstanceId.NickName, "15SVD___");
    TEST_EXPECT_EQUAL(m->InstanceId.AcquisitionDate, "20091211");

    TEST_EXPECT_EQUAL(m->ProductInformation.MeanSunAngle.ZenithUnit, "deg");
    TEST_EXPECT_EQUAL(m->ProductInformation.MeanSunAngle.ZenithValue, 64.155021);

    TEST_EXPECT_EQUAL(m->ProductInformation.MeanSunAngle.AzimuthUnit, "deg");
    TEST_EXPECT_EQUAL(m->ProductInformation.MeanSunAngle.AzimuthValue, 162.454864);

    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Zenith.ColumnUnit, "m");
    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Zenith.ColumnStep, "5000");
    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Zenith.RowUnit, "m");
    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Zenith.RowStep, "5000");

    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values.size(), 23);
    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[0].size(), 23);
    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Zenith.Values[0][0], 64.777785);

    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Azimuth.ColumnUnit, "m");
    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Azimuth.ColumnStep, "5000");
    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Azimuth.RowUnit, "m");
    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Azimuth.RowStep, "5000");

    TEST_EXPECT_EQUAL(m->ProductInformation.SolarAngles.Azimuth.Values[0][0],
                      161.865766); // HACK fp precision

    TEST_EXPECT_EQUAL(m->ProductInformation.ViewingAngles.size(), 91);
    TEST_EXPECT_EQUAL(m->ProductInformation.ViewingAngles[0].BandId, "0");
    TEST_EXPECT_EQUAL(m->ProductInformation.ViewingAngles[0].DetectorId, "2");
    TEST_EXPECT_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values.size(), 23);
    TEST_EXPECT_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0].size(), 23);
    TEST_EXPECT_EQUAL(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0][0],
                      8.612239); // HACK fp precision
    TEST_EXPECT_TRUE(std::isnan(m->ProductInformation.ViewingAngles[0].Angles.Zenith.Values[0][5]));

    TEST_EXPECT_EQUAL(m->ProductInformation.ReflectanceQuantificationValue, "0.000683994528");

    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions.size(), 2);
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Id, "10");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Size.Lines, "10980");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Size.Columns, "10980");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Size.Bands, "4");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.UnitLengthX, "399960");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.UnitLengthY, "4400040");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.DimensionX, "10");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].GeoPosition.DimensionY, "-10");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByLineUnit, "m");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByLineValue, "10");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByColumnUnit, "m");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].ProductSampling.ByColumnValue, "10");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Bands.size(), 4);
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Bands[0].Id, "1");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Bands[0].Name, "B2");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Bands[1].Id, "2");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Bands[1].Name, "B3");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Bands[2].Id, "3");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Bands[2].Name, "B4");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Bands[3].Id, "4");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[0].Bands[3].Name, "B8");

    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Id, "20");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Size.Lines, "5490");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Size.Columns, "5490");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Size.Bands, "6");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.UnitLengthX, "399960");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.UnitLengthY, "4400040");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.DimensionX, "20");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].GeoPosition.DimensionY, "-20");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByLineUnit, "m");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByLineValue, "20");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByColumnUnit, "m");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].ProductSampling.ByColumnValue, "20");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands.size(), 6);
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[0].Id, "1");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[0].Name, "B5");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[1].Id, "2");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[1].Name, "B6");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[2].Id, "3");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[2].Name, "B7");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[3].Id, "4");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[3].Name, "B8A");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[4].Id, "5");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[4].Name, "B11");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[5].Id, "6");
    TEST_EXPECT_EQUAL(m->ImageInformation.Resolutions[1].Bands[5].Name, "B12");

    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles.size(), 3);
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[0].Nature, "SSC_PDTIMG");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[0].FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_FRE_R2.HDR");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[0].LogicalName,
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_FRE_R2");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[1].Nature, "SSC_PDTIMG");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[1].FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R2.HDR");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[1].LogicalName,
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R2");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[2].Nature, "SSC_PDTIMG");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[2].FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1.HDR");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[2].LogicalName,
                      "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1");

    TEST_EXPECT_EQUAL(m->ProductOrganization.QuickLookFiles.size(), 1);
    TEST_EXPECT_EQUAL(m->ProductOrganization.QuickLookFiles[0].Nature, "SSC_PDTQLK");
    TEST_EXPECT_EQUAL(m->ProductOrganization.QuickLookFiles[0].FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTQLK_L2VALD_15SVD____20091211.HDR");
    TEST_EXPECT_EQUAL(m->ProductOrganization.QuickLookFiles[0].LogicalName,
                      "S2A_OPER_SSC_PDTQLK_L2VALD_15SVD____20091211");

    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles.size(), 8);
    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles[0].Id, "MSK");
    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles[0].File.Nature, "SSC_PDTANX");
    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles[0].File.FileLocation,
                      "./S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/"
                      "S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_MSK_R1.HDR");
    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles[0].File.LogicalName,
                      "S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_MSK_R1");

    m = reader->ReadMetadata("L8_TEST_L8C_L2VALD_198030_20130626.HDR");

    TEST_EXPECT_TRUE(m);

    TEST_EXPECT_EQUAL(m->ImageInformation.Bands.size(), 7);
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[0].Id, "1");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[0].Name, "B1");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[1].Id, "2");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[1].Name, "B2");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[6].Id, "7");
    TEST_EXPECT_EQUAL(m->ImageInformation.Bands[6].Name, "B7");

    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles.size(), 2);
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[0].Nature, "L8C_PDTIMG");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[0].FileLocation,
                      "./L8_TEST_L8C_L2VALD_198030_20130626.DBL.DIR/"
                      "L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_FRE.HDR");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[0].LogicalName,
                      "L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_FRE");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[1].Nature, "L8C_PDTIMG");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[1].FileLocation,
                      "./L8_TEST_L8C_L2VALD_198030_20130626.DBL.DIR/"
                      "L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_SRE.HDR");
    TEST_EXPECT_EQUAL(m->ProductOrganization.ImageFiles[1].LogicalName,
                      "L8_TEST_L8C_PDTIMG_L2VALD_198030_20130626_SRE");

    TEST_EXPECT_EQUAL(m->ProductOrganization.QuickLookFiles.size(), 1);
    TEST_EXPECT_EQUAL(m->ProductOrganization.QuickLookFiles[0].Nature, "L8C_PDTQLK");
    TEST_EXPECT_EQUAL(m->ProductOrganization.QuickLookFiles[0].FileLocation,
                      "./L8_TEST_L8C_L2VALD_198030_20130626.DBL.DIR/"
                      "L8_TEST_L8C_PDTQLK_L2VALD_198030_20130626.HDR");
    TEST_EXPECT_EQUAL(m->ProductOrganization.QuickLookFiles[0].LogicalName,
                      "L8_TEST_L8C_PDTQLK_L2VALD_198030_20130626");

    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles.size(), 4);
    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles[0].Id, "MSK");
    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles[0].File.Nature, "L8C_PDTANX");
    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles[0].File.FileLocation,
                      "./L8_TEST_L8C_L2VALD_198030_20130626.DBL.DIR/"
                      "L8_TEST_L8C_PDTANX_L2VALD_198030_20130626_MSK.HDR");
    TEST_EXPECT_EQUAL(m->ProductOrganization.AnnexFiles[0].File.LogicalName,
                      "L8_TEST_L8C_PDTANX_L2VALD_198030_20130626_MSK");
}
