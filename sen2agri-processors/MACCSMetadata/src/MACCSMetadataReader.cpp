#include "otbMacro.h"

#include "MACCSMetadataReader.hpp"

static std::string GetAttribute(const TiXmlElement *element, const char *attributeName);
static std::string GetText(const TiXmlElement *element);
static std::string GetChildText(const TiXmlElement *element, const char *childName);
static std::string
GetChildAttribute(const TiXmlElement *element, const char *childName, const char *attributeName);

namespace itk
{
MACCSFileMetadata MACCSMetadataReader::ReadMetadata(const std::string &path)
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
            try
            {
                count = std::stoi(consumerCountStr);
                countValid = true;
            }
            catch (const std::runtime_error &e)
            {
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
            try
            {
                count = std::stoi(extensionCountStr);
                countValid = true;
            }
            catch (const std::runtime_error &e)
            {
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
        try
        {
            count = std::stoi(countStr);
            countValid = true;
        }
        catch (const std::runtime_error &e)
        {
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
        try
        {
            count = std::stoi(countStr);
            countValid = true;
        }
        catch (const std::runtime_error &e)
        {
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
        try
        {
            count = std::stoi(countStr);
            countValid = true;
        }
        catch (const std::runtime_error &e)
        {
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

MACCSFileMetadata MACCSMetadataReader::ReadMetadataXml(const TiXmlDocument &doc)
{
    MACCSFileMetadata file;
    TiXmlHandle hDoc(const_cast<TiXmlDocument *>(&doc));

    // BUG: TinyXML can't properly read stylesheet declarations, see
    // http://sourceforge.net/p/tinyxml/patches/37/
    // Our files start with one, but we can't read it in.
    TiXmlHandle root = hDoc.FirstChildElement();
    auto rootElement = root.ToElement();

    if (!rootElement) {
        return file;
    }

    auto &header = file.Header;

    header.SchemaVersion = GetAttribute(rootElement, "schema_version");
    header.SchemaLocation = GetAttribute(rootElement, "xsi:schemaLocation");
    header.Type = GetAttribute(rootElement, "xsi:type");

    header.FixedHeader = ReadFixedHeader(root.FirstChildElement("Fixed_Header").ToElement());

    if (auto element = root.FirstChildElement("Variable_Header").ToElement()) {
        file.MainProductHeader =
            ReadMainProductHeader(element->FirstChildElement("Main_Product_Header"));

        if (auto specHdrEl = element->FirstChildElement("Specific_Product_Header")) {
            file.InstanceId = ReadInstanceId(specHdrEl->FirstChildElement("Instance_Id"));

            file.ReferenceProductHeaderId = GetChildText(specHdrEl, "Reference_Product_Header_Id");
            file.AnnexCompleteName = GetChildText(specHdrEl, "Annex_Complete_Name");

            auto imgInfEl = specHdrEl->FirstChildElement("Image_Information");
            if (!imgInfEl) {
                imgInfEl = specHdrEl->FirstChildElement("Annex_Information");
            }
            if (!imgInfEl) {
                imgInfEl = specHdrEl->FirstChildElement("Quick_Look_Information");
            }
            if (imgInfEl) {
                file.ImageInformation.ElementName = imgInfEl->Value();

                file.ImageInformation.Format = GetChildText(imgInfEl, "Format");
                file.ImageInformation.BinaryEncoding = GetChildText(imgInfEl, "Binary_Encoding");
                file.ImageInformation.DataType = GetChildText(imgInfEl, "Data_Type");
                file.ImageInformation.NumberOfSignificantBits =
                    GetChildText(imgInfEl, "Number_of_Significant_Bits");
                file.ImageInformation.NoDataValue = GetChildText(imgInfEl, "Nodata_Value");

                file.ImageInformation.Resolutions =
                    ReadResolutions(imgInfEl->FirstChildElement("List_of_Resolutions"));

                file.ImageInformation.VAPNoDataValue = GetChildText(imgInfEl, "VAP_Nodata_Value");
                file.ImageInformation.VAPQuantificationValue =
                    GetChildText(imgInfEl, "VAP_Quantification_Value");
                file.ImageInformation.AOTNoDataValue = GetChildText(imgInfEl, "AOT_Nodata_Value");
                file.ImageInformation.AOTQuantificationValue =
                    GetChildText(imgInfEl, "AOT_Quantification_Value");

                file.ImageInformation.Size = ReadSize(imgInfEl->FirstChildElement("Size"));

                file.ImageInformation.ImageCompactingTool =
                    GetChildText(imgInfEl, "Image_Compacting_Tool");

                file.ImageInformation.Bands =
                    ReadBands(imgInfEl->FirstChildElement("List_of_Bands"));

                if (auto subsampEl = imgInfEl->FirstChildElement("Subsampling_Factor")) {
                    file.ImageInformation.SubSamplingFactorLine =
                        GetChildText(subsampEl, "By_Line");
                    file.ImageInformation.SubSamplingFactorColumn =
                        GetChildText(subsampEl, "By_Column");

                    if (file.ImageInformation.SubSamplingFactorLine.empty() &&
                        file.ImageInformation.SubSamplingFactorColumn.empty()) {
                        if (auto factor = subsampEl->GetText()) {
                            file.ImageInformation.SubSamplingFactor = factor;
                        }
                    }
                }

                file.ImageInformation.ValuesUnit = GetChildText(imgInfEl, "Values_Unit");
                file.ImageInformation.QuantificationBitValue =
                    GetChildText(imgInfEl, "Quantification_Bit_Value");
                file.ImageInformation.ColorSpace = GetChildText(imgInfEl, "Colorspace");
                file.ImageInformation.BandsOrder = GetChildText(imgInfEl, "Bands_Order");
            }

            file.ProductOrganization = ReadProductOrganization(specHdrEl->FirstChildElement("Product_Organization"));
        }
    }

    return file;
}
}

static std::string GetAttribute(const TiXmlElement *element, const char *attributeName)
{
    if (const char *at = element->Attribute(attributeName)) {
        return at;
    }

    return std::string();
}

static std::string GetText(const TiXmlElement *element)
{
    if (const char *text = element->GetText()) {
        return text;
    }

    return std::string();
}

static std::string GetChildText(const TiXmlElement *element, const char *childName)
{
    if (auto el = element->FirstChildElement(childName)) {
        if (const char *text = el->GetText())
            return text;
    }

    return std::string();
}

static std::string
GetChildAttribute(const TiXmlElement *element, const char *childName, const char *attributeName)
{
    if (auto el = element->FirstChildElement(childName)) {
        if (const char *at = el->Attribute(attributeName)) {
            return at;
        }
    }

    return std::string();
}
