#include <cassert>

#include "MACCSMetadataReader.hpp"

int main()
{
    auto reader = itk::MACCSMetadataReader::New();

    auto m = reader->ReadMetadata("S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1.HDR");

    assert(m.Header.SchemaVersion == "1.00");
    assert(m.Header.SchemaLocation == "http://eop-cfi.esa.int/CFI ./SSC_PDTIMG_ImageProduct.xsd");
    assert(m.Header.Type == "PDTIMG_Header_Type");
    assert(m.Header.FixedHeader.FileName == "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1");
    assert(m.Header.FixedHeader.FileDescription == "ImageProduct");
    assert(m.Header.FixedHeader.Notes == "L2 note");
    assert(m.Header.FixedHeader.Mission == "SENTINEL-2A");
    assert(m.Header.FixedHeader.FileClass == "OPER");
    assert(m.Header.FixedHeader.FileType == "SSC_PDTIMG");
    assert(m.Header.FixedHeader.ValidityStart == "UTC=2015-04-28T00:00:00");
    assert(m.Header.FixedHeader.ValidityStop == "UTC=2009-12-11T00:00:00");
    assert(m.Header.FixedHeader.FileVersion == "0003");
    assert(m.Header.FixedHeader.SourceSystem == "MACCS");
    assert(m.Header.FixedHeader.Creator == "MACCS_L2_INIT_CHAIN");
    assert(m.Header.FixedHeader.CreatorVersion == "0.0.0");
    assert(m.Header.FixedHeader.CreationDate == "UTC=2015-06-30T17:26:29");

    assert(m.MainProductHeader.Consumers.size() == 0);
    assert(m.MainProductHeader.Extensions.size() == 0);

    assert(m.InstanceId.ReferenceProductSemantic == "L2VALD");
    assert(m.InstanceId.ReferenceProductInstance == "15SVD____20091211");
    assert(m.ReferenceProductHeaderId == "S2A_OPER_SSC_L2VALD_15SVD____20091211");

    assert(m.ImageInformation.ElementName == "Image_Information");
    assert(m.ImageInformation.Format == "GEOTIFF");
    assert(m.ImageInformation.BinaryEncoding == "LITTLE_ENDIAN");
    assert(m.ImageInformation.DataType == "SIGNED_SHORT");
    assert(m.ImageInformation.NumberOfSignificantBits == "16");
    assert(m.ImageInformation.NoDataValue == "-10000");
    assert(m.ImageInformation.Size.Lines == "10980");
    assert(m.ImageInformation.Size.Columns == "10980");
    assert(m.ImageInformation.Size.Bands == "4");
    assert(m.ImageInformation.ImageCompactingTool == "NO");
    assert(m.ImageInformation.Bands.size() == 4);
    assert(m.ImageInformation.Bands[0].Id == "1");
    assert(m.ImageInformation.Bands[0].Name == "B2");
    assert(m.ImageInformation.Bands[1].Id == "2");
    assert(m.ImageInformation.Bands[1].Name == "B3");
    assert(m.ImageInformation.Bands[2].Id == "3");
    assert(m.ImageInformation.Bands[2].Name == "B4");
    assert(m.ImageInformation.Bands[3].Id == "4");
    assert(m.ImageInformation.Bands[3].Name == "B8");

    m = reader->ReadMetadata("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_ATB_R1.HDR");
    assert(m.InstanceId.AnnexCode == "ATB");
    assert(m.AnnexCompleteName == "Other");

    assert(m.ImageInformation.ElementName == "Annex_Information");
    assert(m.ImageInformation.VAPNoDataValue == "0");
    assert(m.ImageInformation.VAPQuantificationValue == "0.1");
    assert(m.ImageInformation.AOTNoDataValue == "0");
    assert(m.ImageInformation.AOTQuantificationValue == "0.005");
    assert(m.ImageInformation.SubSamplingFactorLine == "1");
    assert(m.ImageInformation.SubSamplingFactorColumn == "1");
    assert(m.ImageInformation.ValuesUnit == "nil");

    m = reader->ReadMetadata("S2A_OPER_SSC_PDTQLK_L2VALD_15SVD____20091211.HDR");
    assert(m.InstanceId.AnnexCode == "");
    assert(m.AnnexCompleteName == "");

    assert(m.ImageInformation.ElementName == "Quick_Look_Information");
    assert(m.ImageInformation.VAPNoDataValue == "");
    assert(m.ImageInformation.VAPQuantificationValue == "");
    assert(m.ImageInformation.AOTNoDataValue == "");
    assert(m.ImageInformation.AOTQuantificationValue == "");
    assert(m.ImageInformation.SubSamplingFactor == "24");
    assert(m.ImageInformation.SubSamplingFactorLine == "");
    assert(m.ImageInformation.SubSamplingFactorColumn == "");
    assert(m.ImageInformation.ColorSpace == "RGB");
    assert(m.ImageInformation.BandsOrder == "RGB");

    m = reader->ReadMetadata("S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_QLT_R1.HDR");

    assert(m.ImageInformation.QuantificationBitValue == "1");

    m = reader->ReadMetadata("S2A_OPER_SSC_L2VALD_15SVD____20091211.HDR");

    assert(m.ImageInformation.Resolutions.size() == 2);
    assert(m.ImageInformation.Resolutions[0].Id == "10");
    assert(m.ImageInformation.Resolutions[0].Size.Lines == "10980");
    assert(m.ImageInformation.Resolutions[0].Size.Columns == "10980");
    assert(m.ImageInformation.Resolutions[0].Size.Bands == "4");
    assert(m.ImageInformation.Resolutions[0].GeoPosition.UnitLengthX == "399960");
    assert(m.ImageInformation.Resolutions[0].GeoPosition.UnitLengthY == "4400040");
    assert(m.ImageInformation.Resolutions[0].GeoPosition.DimensionX == "10");
    assert(m.ImageInformation.Resolutions[0].GeoPosition.DimensionY == "-10");
    assert(m.ImageInformation.Resolutions[0].ProductSampling.ByLineUnit == "m");
    assert(m.ImageInformation.Resolutions[0].ProductSampling.ByLineValue == "10");
    assert(m.ImageInformation.Resolutions[0].ProductSampling.ByColumnUnit == "m");
    assert(m.ImageInformation.Resolutions[0].ProductSampling.ByColumnValue == "10");
    assert(m.ImageInformation.Resolutions[0].Bands.size() == 4);
    assert(m.ImageInformation.Resolutions[0].Bands[0].Id == "1");
    assert(m.ImageInformation.Resolutions[0].Bands[0].Name == "B2");
    assert(m.ImageInformation.Resolutions[0].Bands[1].Id == "2");
    assert(m.ImageInformation.Resolutions[0].Bands[1].Name == "B3");
    assert(m.ImageInformation.Resolutions[0].Bands[2].Id == "3");
    assert(m.ImageInformation.Resolutions[0].Bands[2].Name == "B4");
    assert(m.ImageInformation.Resolutions[0].Bands[3].Id == "4");
    assert(m.ImageInformation.Resolutions[0].Bands[3].Name == "B8");

    assert(m.ImageInformation.Resolutions[1].Id == "20");
    assert(m.ImageInformation.Resolutions[1].Size.Lines == "5490");
    assert(m.ImageInformation.Resolutions[1].Size.Columns == "5490");
    assert(m.ImageInformation.Resolutions[1].Size.Bands == "6");
    assert(m.ImageInformation.Resolutions[1].GeoPosition.UnitLengthX == "399960");
    assert(m.ImageInformation.Resolutions[1].GeoPosition.UnitLengthY == "4400040");
    assert(m.ImageInformation.Resolutions[1].GeoPosition.DimensionX == "20");
    assert(m.ImageInformation.Resolutions[1].GeoPosition.DimensionY == "-20");
    assert(m.ImageInformation.Resolutions[1].ProductSampling.ByLineUnit == "m");
    assert(m.ImageInformation.Resolutions[1].ProductSampling.ByLineValue == "20");
    assert(m.ImageInformation.Resolutions[1].ProductSampling.ByColumnUnit == "m");
    assert(m.ImageInformation.Resolutions[1].ProductSampling.ByColumnValue == "20");
    assert(m.ImageInformation.Resolutions[1].Bands.size() == 6);
    assert(m.ImageInformation.Resolutions[1].Bands[0].Id == "1");
    assert(m.ImageInformation.Resolutions[1].Bands[0].Name == "B5");
    assert(m.ImageInformation.Resolutions[1].Bands[1].Id == "2");
    assert(m.ImageInformation.Resolutions[1].Bands[1].Name == "B6");
    assert(m.ImageInformation.Resolutions[1].Bands[2].Id == "3");
    assert(m.ImageInformation.Resolutions[1].Bands[2].Name == "B7");
    assert(m.ImageInformation.Resolutions[1].Bands[3].Id == "4");
    assert(m.ImageInformation.Resolutions[1].Bands[3].Name == "B8A");
    assert(m.ImageInformation.Resolutions[1].Bands[4].Id == "5");
    assert(m.ImageInformation.Resolutions[1].Bands[4].Name == "B11");
    assert(m.ImageInformation.Resolutions[1].Bands[5].Id == "6");
    assert(m.ImageInformation.Resolutions[1].Bands[5].Name == "B12");
}
