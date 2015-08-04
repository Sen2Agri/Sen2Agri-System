#include <cassert>

#include "MACCSMetadataReader.hpp"

int main()
{
    auto reader = itk::MACCSMetadataReader::New();

    auto m = reader->ReadMetadata("S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1.HDR");

    assert(m.Header.SchemaVersion == "1.00");
    assert(m.Header.SchemaLocation == "http://eop-cfi.esa.int/CFI ./SSC_PDTIMG_ImageProduct.xsd");
    assert(m.Header.Type == "PDTIMG_Header_Type");
    assert(m.Header.FileName == "S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1");
    assert(m.Header.FileDescription == "ImageProduct");
    assert(m.Header.Notes == "L2 note");
    assert(m.Header.Mission == "SENTINEL-2A");
    assert(m.Header.FileClass == "OPER");
    assert(m.Header.FileType == "SSC_PDTIMG");
    assert(m.Header.ValidityStart == "UTC=2015-04-28T00:00:00");
    assert(m.Header.ValidityStop == "UTC=2009-12-11T00:00:00");
    assert(m.Header.FileVersion == "0003");
    assert(m.Header.SourceSystem == "MACCS");
    assert(m.Header.Creator == "MACCS_L2_INIT_CHAIN");
    assert(m.Header.CreatorVersion == "0.0.0");
    assert(m.Header.CreationDate == "UTC=2015-06-30T17:26:29");

    assert(m.Consumers.size() == 0);
    assert(m.Extensions.size() == 0);

    assert(m.InstanceId.ReferenceProductSemantic == "L2VALD");
    assert(m.InstanceId.ReferenceProductInstance == "15SVD____20091211");
    assert(m.ReferenceProductHeaderId == "S2A_OPER_SSC_L2VALD_15SVD____20091211");

    assert(m.ImageInformation.ElementName == "Image_Information");
    assert(m.ImageInformation.Format == "GEOTIFF");
    assert(m.ImageInformation.BinaryEncoding == "LITTLE_ENDIAN");
    assert(m.ImageInformation.DataType == "SIGNED_SHORT");
    assert(m.ImageInformation.NumberOfSignificantBits == "16");
    assert(m.ImageInformation.NoDataValue == "-10000");
    assert(m.ImageInformation.SizeLines == "10980");
    assert(m.ImageInformation.SizeColumns == "10980");
    assert(m.ImageInformation.SizeBands == "4");
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
}
