#include <limits>

#include "otbMacro.h"

#include "MACCSMetadataReader.hpp"
#include "tinyxml_utils.hpp"

namespace itk
{
std::unique_ptr<MACCSFileMetadata> MACCSMetadataReader::ReadMetadata(const std::string &path)
{
    TiXmlDocument doc(path);
    if (!doc.LoadFile()) {
        itkExceptionMacro(<< "Can't open metadata file");
    }

    return ReadMetadataXml(doc);
}

MACCSFixedHeader ReadFixedHeader(const TiXmlElement *el)
{
    MACCSFixedHeader result;

    if (!el) {
        return result;
    }

    result.FileName = GetChildText(el, "File_Name");
    result.FileDescription = GetChildText(el, "File_Description");
    result.Notes = GetChildText(el, "Notes");
    result.Mission = GetChildText(el, "Mission");
    result.FileClass = GetChildText(el, "File_Class");
    result.FileType = GetChildText(el, "File_Type");

    if (auto validityEl = el->FirstChildElement("Validity_Period")) {
        result.ValidityStart = GetChildText(validityEl, "Validity_Start");
        result.ValidityStop = GetChildText(validityEl, "Validity_Stop");
    }

    result.FileVersion = GetChildText(el, "File_Version");

    if (auto srcEl = el->FirstChildElement("Source")) {
        result.SourceSystem = GetChildText(srcEl, "System");
        result.Creator = GetChildText(srcEl, "Creator");
        result.CreatorVersion = GetChildText(srcEl, "Creator_Version");
        result.CreationDate = GetChildText(srcEl, "Creation_Date");
    }

    return result;
}

MACCSMainProductHeader ReadMainProductHeader(const TiXmlElement *el)
{
    MACCSMainProductHeader result;

    if (!el) {
        return result;
    }

    if (auto consumersEl = el->FirstChildElement("List_of_Consumers")) {
        if (auto consumerCountStr = consumersEl->Attribute("count")) {
            size_t count;
            bool countValid;
            try {
                count = std::stoi(consumerCountStr);
                countValid = true;
            } catch (const std::runtime_error &e) {
                countValid = false;

                otbMsgDevMacro("Invalid consumer count value: " << consumerCountStr);
                otbMsgDevMacro(<< e.what());
            }
            if (countValid) {
                result.Consumers.reserve(count);

                // NOTE: We're not sure about the schema, so consumers have no fields
                // right now
                result.Consumers.resize(count);
            }
        }
    }

    if (auto extensionsEl = el->FirstChildElement("List_of_Extensions")) {
        if (auto extensionCountStr = extensionsEl->Attribute("count")) {
            size_t count;
            bool countValid;
            try {
                count = std::stoi(extensionCountStr);
                countValid = true;
            } catch (const std::runtime_error &e) {
                countValid = false;

                otbMsgDevMacro("Invalid extension count value: " << extensionCountStr);
                otbMsgDevMacro(<< e.what());
            }
            if (countValid) {
                result.Extensions.reserve(count);

                // NOTE: We're not sure about the schema, so extensions have no fields
                // right now
                result.Extensions.resize(count);
            }
        }
    }

    return result;
}

MACCSInstanceId ReadInstanceId(const TiXmlElement *el)
{
    MACCSInstanceId result;

    if (!el) {
        return result;
    }

    result.ReferenceProductSemantic = GetChildText(el, "Reference_Product_Semantic");
    result.ReferenceProductInstance = GetChildText(el, "Reference_Product_Instance");
    result.AnnexCode = GetChildText(el, "Annex_Code");
    result.NickName = GetChildText(el, "Nick_Name");
    result.AcquisitionDate = GetChildText(el, "Acquisition_Date");

    return result;
}

double ReadDouble(const std::string &s)
{
    try {
        return std::stod(s);
    } catch (const std::exception &e) {
        otbMsgDevMacro("Invalid double value " << s << ": " << e.what());

        return std::numeric_limits<double>::quiet_NaN();
    }
}

std::vector<double> ReadDoubleList(const std::string &s)
{
    std::vector<double> result;

    std::istringstream is(s);
    std::string value;
    while (is >> value) {
        result.emplace_back(ReadDouble(value));
    }

    return result;
}

MACCSAngleList ReadAngleList(const TiXmlElement *el)
{
    MACCSAngleList result;

    if (!el) {
        return result;
    }

    if (auto colStepEl = el->FirstChildElement("COL_STEP")) {
        result.ColumnUnit = GetAttribute(colStepEl, "unit");
        result.ColumnStep = GetText(colStepEl);
    }

    if (auto rowStepEl = el->FirstChildElement("ROW_STEP")) {
        result.RowUnit = GetAttribute(rowStepEl, "unit");
        result.RowStep = GetText(rowStepEl);
    }

    if (auto valuesListEl = el->FirstChildElement("Values_List")) {
        for (auto valuesEl = valuesListEl->FirstChildElement("VALUES"); valuesEl;
             valuesEl = valuesEl->NextSiblingElement("VALUES")) {

            result.Values.emplace_back(ReadDoubleList(GetText(valuesEl)));
        }
    }

    return result;
}

MACCSAnglePair ReadAnglePair(const TiXmlElement *el)
{
    MACCSAnglePair result;
    result.ZenithValue = std::numeric_limits<double>::quiet_NaN();
    result.AzimuthValue = std::numeric_limits<double>::quiet_NaN();

    if (!el) {
        return result;
    }

    if (auto zenithEl = el->FirstChildElement("ZENITH_ANGLE")) {
        result.ZenithUnit = GetAttribute(zenithEl, "unit");
        result.ZenithValue = ReadDouble(GetText(zenithEl));
    }

    if (auto azimuthEl = el->FirstChildElement("AZIMUTH_ANGLE")) {
        result.AzimuthUnit = GetAttribute(azimuthEl, "unit");
        result.AzimuthValue = ReadDouble(GetText(azimuthEl));
    }

    return result;
}

MACCSAngles ReadSolarAngles(const TiXmlElement *el)
{
    MACCSAngles result;

    if (!el) {
        return result;
    }

    if (auto zenithEl = el->FirstChildElement("Zenith")) {
        result.Zenith = ReadAngleList(zenithEl);
    }

    if (auto azimuthEl = el->FirstChildElement("Azimuth")) {
        result.Azimuth = ReadAngleList(azimuthEl);
    }

    return result;
}

MACCSMeanViewingIncidenceAngle ReadMeanViewingIncidenceAngle(const TiXmlElement *el)
{
    MACCSMeanViewingIncidenceAngle result;

    if (!el) {
        return result;
    }

    result.BandId = GetAttribute(el, "bandId");
    result.Angles = ReadAnglePair(el);

    return result;
}

std::vector<MACCSMeanViewingIncidenceAngle> ReadMeanViewingIncidenceAngles(const TiXmlElement *el)
{
    std::vector<MACCSMeanViewingIncidenceAngle> result;

    if (!el) {
        return result;
    }

    for (auto angleEl = el->FirstChildElement("Mean_Viewing_Incidence_Angle"); angleEl;
         angleEl = angleEl->NextSiblingElement("Mean_Viewing_Incidence_Angle")) {
        result.emplace_back(ReadMeanViewingIncidenceAngle(angleEl));
    }

    return result;
}

MACCSViewingAnglesGrid ReadViewingAnglesGrid(const TiXmlElement *el)
{
    MACCSViewingAnglesGrid result;

    if (!el) {
        return result;
    }

    result.BandId = GetAttribute(el, "bandId");
    result.DetectorId = GetAttribute(el, "detectorId");
    result.Angles = ReadSolarAngles(el);

    return result;
}

std::vector<MACCSViewingAnglesGrid> ReadViewingAnglesGridList(const TiXmlElement *el)
{
    std::vector<MACCSViewingAnglesGrid> result;

    if (!el) {
        return result;
    }

    for (auto gridsEl = el->FirstChildElement("Viewing_Incidence_Angles_Grids"); gridsEl;
         gridsEl = gridsEl->NextSiblingElement("Viewing_Incidence_Angles_Grids")) {
        result.emplace_back(ReadViewingAnglesGrid(gridsEl));
    }

    return result;
}

MACCSProductInformation ReadProductInformation(const TiXmlElement *el)
{
    MACCSProductInformation result;

    if (!el) {
        return result;
    }

    result.MeanSunAngle = ReadAnglePair(el->FirstChildElement("Mean_Sun_Angle"));
    result.SolarAngles = ReadSolarAngles(el->FirstChildElement("Solar_Angles"));
    result.MeanViewingIncidenceAngles =
        ReadMeanViewingIncidenceAngles(el->FirstChildElement("Mean_Viewing_Incidence_Angle_List"));
    result.ViewingAngles =
        ReadViewingAnglesGridList(el->FirstChildElement("List_of_Viewing_Angles"));
    result.ReflectanceQuantificationValue = GetChildText(el, "Reflectance_Quantification_Value");

    return result;
}

MACCSSize ReadSize(const TiXmlElement *el)
{
    MACCSSize result;

    if (!el) {
        return result;
    }

    result.Lines = GetChildText(el, "Lines");
    result.Columns = GetChildText(el, "Columns");
    result.Bands = GetChildText(el, "Bands");

    return result;
}

MACCSBand ReadBand(const TiXmlElement *el)
{
    MACCSBand result;

    if (!el) {
        return result;
    }

    result.Id = GetAttribute(el, "sn");
    result.Name = GetText(el);

    return result;
}

MACCSGeoPosition ReadGeoPosition(const TiXmlElement *el)
{
    MACCSGeoPosition result;

    if (!el) {
        return result;
    }

    result.UnitLengthX = GetChildText(el, "ULX");
    result.UnitLengthY = GetChildText(el, "ULY");
    result.DimensionX = GetChildText(el, "XDIM");
    result.DimensionY = GetChildText(el, "YDIM");

    return result;
}

MACCSProductSampling ReadProductSampling(const TiXmlElement *el)
{
    MACCSProductSampling result;

    if (!el) {
        return result;
    }

    if (auto byLineEl = el->FirstChildElement("By_Line")) {
        result.ByLineUnit = GetAttribute(byLineEl, "unit");
        result.ByLineValue = GetText(byLineEl);
    }

    if (auto byColumnEl = el->FirstChildElement("By_Column")) {
        result.ByColumnUnit = GetAttribute(byColumnEl, "unit");
        result.ByColumnValue = GetText(byColumnEl);
    }

    return result;
}

std::vector<MACCSBand> ReadBands(const TiXmlElement *el)
{
    std::vector<MACCSBand> result;

    if (!el) {
        return result;
    }

    if (auto countStr = el->Attribute("count")) {
        size_t count;
        bool countValid;
        try {
            count = std::stoi(countStr);
            countValid = true;
        } catch (const std::runtime_error &e) {
            countValid = false;

            otbMsgDevMacro("Invalid band count value: " << countStr);
            otbMsgDevMacro(<< e.what());
        }
        if (countValid) {
            result.reserve(count);
        }

        for (auto bandEl = el->FirstChildElement("Band"); bandEl;
             bandEl = bandEl->NextSiblingElement("Band")) {

            result.emplace_back(ReadBand(bandEl));
        }
    }

    return result;
}

MACCSResolution ReadResolution(const TiXmlElement *el)
{
    MACCSResolution result;

    if (!el) {
        return result;
    }

    result.Id = GetAttribute(el, "r");
    result.Size = ReadSize(el->FirstChildElement("Size"));
    result.GeoPosition = ReadGeoPosition(el->FirstChildElement("Geoposition"));
    result.ProductSampling = ReadProductSampling(el->FirstChildElement("Product_Sampling"));
    result.Bands = ReadBands(el->FirstChildElement("List_of_Bands"));

    return result;
}

std::vector<MACCSResolution> ReadResolutions(const TiXmlElement *el)
{
    std::vector<MACCSResolution> result;

    if (!el) {
        return result;
    }

    if (auto countStr = el->Attribute("count")) {
        size_t count;
        bool countValid;
        try {
            count = std::stoi(countStr);
            countValid = true;
        } catch (const std::runtime_error &e) {
            countValid = false;

            otbMsgDevMacro("Invalid resolution count value: " << countStr);
            otbMsgDevMacro(<< e.what());
        }
        if (countValid) {
            result.reserve(count);
        }

        for (auto resolutionEl = el->FirstChildElement("Resolution"); resolutionEl;
             resolutionEl = resolutionEl->NextSiblingElement("Resolution")) {

            result.emplace_back(ReadResolution(resolutionEl));
        }
    }

    return result;
}

MACCSImageInformation ReadImageInformation(const TiXmlElement *el)
{
    MACCSImageInformation result;

    if (!el) {
        return result;
    }

    result.ElementName = el->Value();

    result.Format = GetChildText(el, "Format");
    result.BinaryEncoding = GetChildText(el, "Binary_Encoding");
    result.DataType = GetChildText(el, "Data_Type");
    result.NumberOfSignificantBits = GetChildText(el, "Number_of_Significant_Bits");
    result.NoDataValue = GetChildText(el, "Nodata_Value");

    result.Resolutions = ReadResolutions(el->FirstChildElement("List_of_Resolutions"));

    result.VAPNoDataValue = GetChildText(el, "VAP_Nodata_Value");
    result.VAPQuantificationValue = GetChildText(el, "VAP_Quantification_Value");
    result.AOTNoDataValue = GetChildText(el, "AOT_Nodata_Value");
    result.AOTQuantificationValue = GetChildText(el, "AOT_Quantification_Value");

    result.Size = ReadSize(el->FirstChildElement("Size"));

    result.ImageCompactingTool = GetChildText(el, "Image_Compacting_Tool");

    result.Bands = ReadBands(el->FirstChildElement("List_of_Bands"));

    if (auto subsampEl = el->FirstChildElement("Subsampling_Factor")) {
        result.SubSamplingFactorLine = GetChildText(subsampEl, "By_Line");
        result.SubSamplingFactorColumn = GetChildText(subsampEl, "By_Column");

        if (result.SubSamplingFactorLine.empty() && result.SubSamplingFactorColumn.empty()) {
            if (auto factor = subsampEl->GetText()) {
                result.SubSamplingFactor = factor;
            }
        }
    }

    result.ValuesUnit = GetChildText(el, "Values_Unit");
    result.QuantificationBitValue = GetChildText(el, "Quantification_Bit_Value");
    result.ColorSpace = GetChildText(el, "Colorspace");
    result.BandsOrder = GetChildText(el, "Bands_Order");

    return result;
}

MACCSFileInformation ReadFileInformation(const TiXmlElement *el)
{
    MACCSFileInformation result;

    if (!el) {
        return result;
    }

    result.Nature = GetChildText(el, "Nature");
    result.FileLocation = GetChildText(el, "File_Location");
    result.LogicalName = GetChildText(el, "Logical_Name");

    return result;
}

MACCSAnnexInformation ReadAnnexInformation(const TiXmlElement *el)
{
    MACCSAnnexInformation result;

    if (!el) {
        return result;
    }

    result.Id = GetAttribute(el, "sk");
    result.File = ReadFileInformation(el);

    return result;
}

std::vector<MACCSAnnexInformation> ReadAnnexes(const TiXmlElement *el)
{
    std::vector<MACCSAnnexInformation> result;

    if (!el) {
        return result;
    }

    if (auto countStr = el->Attribute("count")) {
        size_t count;
        bool countValid;
        try {
            count = std::stoi(countStr);
            countValid = true;
        } catch (const std::runtime_error &e) {
            countValid = false;

            otbMsgDevMacro("Invalid annex count value: " << countStr);
            otbMsgDevMacro(<< e.what());
        }
        if (countValid) {
            result.reserve(count);
        }

        for (auto annexEl = el->FirstChildElement("Annex_File"); annexEl;
             annexEl = annexEl->NextSiblingElement("Annex_File")) {

            result.emplace_back(ReadAnnexInformation(annexEl));
        }
    }

    return result;
}

MACCSProductOrganization ReadProductOrganization(const TiXmlElement *el)
{
    MACCSProductOrganization result;

    if (!el) {
        return result;
    }

    for (auto fileEl = el->FirstChildElement("Image_File"); fileEl;
         fileEl = fileEl->NextSiblingElement("Image_File")) {

        result.ImageFiles.emplace_back(ReadFileInformation(fileEl));
    }

    for (auto fileEl = el->FirstChildElement("Quicklook_File"); fileEl;
         fileEl = fileEl->NextSiblingElement("Quicklook_File")) {

        result.QuickLookFiles.emplace_back(ReadFileInformation(fileEl));
    }

    result.AnnexFiles = ReadAnnexes(el->FirstChildElement("List_of_Annex_Files"));

    return result;
}

std::unique_ptr<MACCSFileMetadata> MACCSMetadataReader::ReadMetadataXml(const TiXmlDocument &doc)
{
    TiXmlHandle hDoc(const_cast<TiXmlDocument *>(&doc));

    // BUG: TinyXML can't properly read stylesheet declarations, see
    // http://sourceforge.net/p/tinyxml/patches/37/
    // Our files start with one, but we can't read it in.
    auto rootElement = hDoc.FirstChildElement("Earth_Explorer_Header").ToElement();

    if (!rootElement) {
        return nullptr;
    }

    auto file = std::unique_ptr<MACCSFileMetadata>(new MACCSFileMetadata);

    file->Header.SchemaVersion = GetAttribute(rootElement, "schema_version");
    file->Header.SchemaLocation = GetAttribute(rootElement, "xsi:schemaLocation");
    file->Header.Type = GetAttribute(rootElement, "xsi:type");

    file->Header.FixedHeader = ReadFixedHeader(rootElement->FirstChildElement("Fixed_Header"));

    if (auto element = rootElement->FirstChildElement("Variable_Header")) {
        file->MainProductHeader =
            ReadMainProductHeader(element->FirstChildElement("Main_Product_Header"));

        if (auto specHdrEl = element->FirstChildElement("Specific_Product_Header")) {
            file->InstanceId = ReadInstanceId(specHdrEl->FirstChildElement("Instance_Id"));

            file->ReferenceProductHeaderId = GetChildText(specHdrEl, "Reference_Product_Header_Id");
            file->AnnexCompleteName = GetChildText(specHdrEl, "Annex_Complete_Name");

            file->ProductInformation =
                ReadProductInformation(specHdrEl->FirstChildElement("Product_Information"));

            auto imgInfEl = specHdrEl->FirstChildElement("Image_Information");
            if (!imgInfEl) {
                imgInfEl = specHdrEl->FirstChildElement("Annex_Information");
            }
            if (!imgInfEl) {
                imgInfEl = specHdrEl->FirstChildElement("Quick_Look_Information");
            }

            file->ImageInformation = ReadImageInformation(imgInfEl);

            file->ProductOrganization =
                ReadProductOrganization(specHdrEl->FirstChildElement("Product_Organization"));
        }
    }

    return file;
}
}
