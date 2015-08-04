#include "otbMacro.h"

#include "MACCSMetadataReader.hpp"

static std::string GetAttribute(TiXmlElement *element, const char *attributeName);
static std::string GetChildText(TiXmlElement *element, const char *childName);
static std::string
GetChildAttribute(TiXmlElement *element, const char *childName, const char *attributeName);

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

    if (auto fixHdrEl = root.FirstChildElement("Fixed_Header").ToElement()) {
        header.FileName = GetChildText(fixHdrEl, "File_Name");
        header.FileDescription = GetChildText(fixHdrEl, "File_Description");
        header.Notes = GetChildText(fixHdrEl, "Notes");
        header.Mission = GetChildText(fixHdrEl, "Mission");
        header.FileClass = GetChildText(fixHdrEl, "File_Class");
        header.FileType = GetChildText(fixHdrEl, "File_Type");

        if (auto validityEl = fixHdrEl->FirstChildElement("Validity_Period")) {
            header.ValidityStart = GetChildText(validityEl, "Validity_Start");
            header.ValidityStop = GetChildText(validityEl, "Validity_Stop");
        }

        header.FileVersion = GetChildText(fixHdrEl, "File_Version");

        if (auto srcEl = fixHdrEl->FirstChildElement("Source")) {
            header.SourceSystem = GetChildText(srcEl, "System");
            header.Creator = GetChildText(srcEl, "Creator");
            header.CreatorVersion = GetChildText(srcEl, "Creator_Version");
            header.CreationDate = GetChildText(srcEl, "Creation_Date");
        }
    }
    if (auto element = root.FirstChildElement("Variable_Header").ToElement()) {
        if (auto mainHdrEl = element->FirstChildElement("Main_Product_Header")) {
            if (auto consumersEl = mainHdrEl->FirstChildElement("List_of_Consumers")) {
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
                        file.Consumers.reserve(count);

                        // NOTE: We're not sure about the schema, so consumers have no fields
                        // right now
                        file.Consumers.resize(count);
                    }
                }
            }

            if (auto extensionsEl = mainHdrEl->FirstChildElement("List_of_Extensions")) {
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
                        file.Extensions.reserve(count);

                        // NOTE: We're not sure about the schema, so extensions have no fields
                        // right now
                        file.Extensions.resize(count);
                    }
                }
            }
        }

        if (auto specHdrEl = element->FirstChildElement("Specific_Product_Header")) {
            if (auto instEl = specHdrEl->FirstChildElement("Instance_Id")) {
                file.InstanceId.ReferenceProductSemantic =
                    GetChildText(instEl, "Reference_Product_Semantic");
                file.InstanceId.ReferenceProductInstance =
                    GetChildText(instEl, "Reference_Product_Instance");
                file.InstanceId.AnnexCode = GetChildText(instEl, "Annex_Code");
            }

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

                file.ImageInformation.VAPNoDataValue = GetChildText(imgInfEl, "VAP_Nodata_Value");
                file.ImageInformation.VAPQuantificationValue = GetChildText(imgInfEl, "VAP_Quantification_Value");
                file.ImageInformation.AOTNoDataValue = GetChildText(imgInfEl, "AOT_Nodata_Value");
                file.ImageInformation.AOTQuantificationValue = GetChildText(imgInfEl, "AOT_Quantification_Value");

                if (auto sizeEl = imgInfEl->FirstChildElement("Size")) {
                    file.ImageInformation.SizeLines = GetChildText(sizeEl, "Lines");
                    file.ImageInformation.SizeColumns = GetChildText(sizeEl, "Columns");
                    file.ImageInformation.SizeBands = GetChildText(sizeEl, "Bands");
                }

                file.ImageInformation.ImageCompactingTool =
                    GetChildText(imgInfEl, "Image_Compacting_Tool");

                if (auto bandsEl = imgInfEl->FirstChildElement("List_of_Bands")) {
                    if (auto bandCountStr = bandsEl->Attribute("count")) {
                        size_t count;
                        bool countValid;
                        try
                        {
                            count = std::stoi(bandCountStr);
                            countValid = true;
                        }
                        catch (const std::runtime_error &e)
                        {
                            countValid = false;

                            otbMsgDevMacro("Invalid band count value: " << bandCountStr);
                            otbMsgDevMacro(<< e.what());
                        }
                        if (countValid) {
                            file.Extensions.reserve(count);
                        }

                        for (auto bandEl = bandsEl->FirstChildElement("Band"); bandEl;
                             bandEl = bandEl->NextSiblingElement("Band")) {
                            MACCSBand band;
                            if (auto sn = bandEl->Attribute("sn")) {
                                band.Id = sn;
                            }
                            if (auto name = bandEl->GetText()) {
                                band.Name = name;
                            }

                            file.ImageInformation.Bands.emplace_back(std::move(band));
                        }
                    }
                }

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

                file.ImageInformation.ValuesUnit =
                    GetChildText(imgInfEl, "Values_Unit");
                file.ImageInformation.QuantificationBitValue =
                    GetChildText(imgInfEl, "Quantification_Bit_Value");
                file.ImageInformation.ColorSpace = GetChildText(imgInfEl, "Colorspace");
                file.ImageInformation.BandsOrder = GetChildText(imgInfEl, "Bands_Order");
            }
        }
    }

    return file;
}
}

static std::string GetAttribute(TiXmlElement *element, const char *attributeName)
{
    if (const char *at = element->Attribute(attributeName)) {
        return at;
    }

    return std::string();
}

static std::string GetChildText(TiXmlElement *element, const char *childName)
{
    if (auto el = element->FirstChildElement(childName)) {
        if (const char *text = el->GetText())
            return text;
    }

    return std::string();
}

static std::string
GetChildAttribute(TiXmlElement *element, const char *childName, const char *attributeName)
{
    if (auto el = element->FirstChildElement(childName)) {
        if (const char *at = el->Attribute(attributeName)) {
            return at;
        }
    }

    return std::string();
}
