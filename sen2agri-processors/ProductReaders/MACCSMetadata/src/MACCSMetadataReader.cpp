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
 
#include <limits>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include "otbMacro.h"

#include "MACCSMetadataReader.hpp"
#include "MetadataUtil.hpp"
#include "tinyxml_utils.hpp"
#include "string_utils.hpp"

namespace itk
{
std::unique_ptr<MACCSFileMetadata> MACCSMetadataReader::ReadMetadata(const std::string &path)
{
    TiXmlDocument doc(path);
    if (!doc.LoadFile()) {
        return nullptr;
    }

    auto metadata = ReadMetadataXml(doc);
    if (metadata) {
        metadata->ProductPath = path;
    }
    return metadata;
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

MACCSGeoPoint ReadGeoPoint(const TiXmlElement *el)
{
    MACCSGeoPoint result;
    result.Long = std::numeric_limits<double>::quiet_NaN();
    result.Lat = std::numeric_limits<double>::quiet_NaN();

    if (!el) {
        return result;
    }

    result.UnitLong = GetChildAttribute(el, "Long", "unit");
    result.UnitLat = GetChildAttribute(el, "Lat", "unit");
    result.Long = ReadDouble(GetChildText(el, "Long"));
    result.Lat = ReadDouble(GetChildText(el, "Lat"));

    return result;
}

MACCSGeoCoverage ReadGeoCoverage(const TiXmlElement *el)
{
    MACCSGeoCoverage result;

    if (!el) {
        return result;
    }

    result.UpperLeftCorner = ReadGeoPoint(el->FirstChildElement("Upper_Left_Corner"));
    result.UpperRightCorner = ReadGeoPoint(el->FirstChildElement("Upper_Right_Corner"));
    result.LowerLeftCorner = ReadGeoPoint(el->FirstChildElement("Lower_Left_Corner"));
    result.LowerRightCorner = ReadGeoPoint(el->FirstChildElement("Lower_Right_Corner"));

    return result;
}

CommonBandWavelength ReadBandWavelength(const TiXmlElement *el)
{
    CommonBandWavelength result;

    if (!el) {
        return result;
    }

    result.BandName = GetAttribute(el, "sk");
    result.Unit = GetAttribute(el, "unit");
    result.WaveLength = GetText(el);
    result.MaxUnit = result.Unit;
    result.MaxWaveLength = result.WaveLength;
    result.MinUnit = result.Unit;
    result.MinWaveLength = result.WaveLength;
    return result;
}

std::vector<CommonBandWavelength> ReadBandWavelengths(const TiXmlElement *el)
{
    std::vector<CommonBandWavelength> result;

    if (!el) {
        return result;
    }

    for (auto wavelenEl = el->FirstChildElement("Band_Central_Wavelength"); wavelenEl;
        wavelenEl = wavelenEl->NextSiblingElement("Band_Central_Wavelength")) {
        result.emplace_back(ReadBandWavelength(wavelenEl));
    }

    return result;
}

CommonBandResolution ReadBandResolution(const TiXmlElement *el)
{
    CommonBandResolution result;

    if (!el) {
        return result;
    }

    result.BandName = GetAttribute(el, "sk");
    result.Unit = GetAttribute(el, "unit");
    result.Resolution = GetText(el);

    return result;
}

std::vector<CommonBandResolution> ReadBandResolutions(const TiXmlElement *el)
{
    std::vector<CommonBandResolution> result;

    if (!el) {
        return result;
    }

    for (auto resEl = el->FirstChildElement("Band_Central_Resolution"); resEl;
        resEl = resEl->NextSiblingElement("Band_Central_Resolution")) {
        result.emplace_back(ReadBandResolution(resEl));
    }

    return result;
}

MACCSProductInformation ReadProductInformation(const TiXmlElement *el)
{
    MACCSProductInformation result;

    if (!el) {
        return result;
    }

    result.AcquisitionDateTime = GetChildText(el, "Acquisition_Date_Time");
    result.GeoCoverage = ReadGeoCoverage(el->FirstChildElement("Image_Geo_Coverage"));

    result.MeanSunAngle = ReadAnglePair(el->FirstChildElement("Mean_Sun_Angle"),
                                        "ZENITH_ANGLE", "AZIMUTH_ANGLE");
    if(std::isnan(result.MeanSunAngle.AzimuthValue) || std::isnan(result.MeanSunAngle.ZenithValue)) {
        result.MeanSunAngle = ReadAnglePair(el->FirstChildElement("Mean_Solar_Angles"),
                                            "Zenith", "Azimuth");
    }
    result.SolarAngles = ReadSolarAngles(el->FirstChildElement("Solar_Angles"));
    result.MeanViewingIncidenceAngles =
        ReadMeanViewingIncidenceAngles(el->FirstChildElement("Mean_Viewing_Incidence_Angle_List"));
    if(result.MeanViewingIncidenceAngles.empty()) {
        CommonMeanViewingIncidenceAngle meanVIncAng;
        meanVIncAng.Angles = ReadAnglePair(el->FirstChildElement("Mean_Viewing_Angles"),
                                                  "Zenith", "Azimuth");
        if(!std::isnan(meanVIncAng.Angles.AzimuthValue) &&
           !std::isnan(meanVIncAng.Angles.ZenithValue)) {
            meanVIncAng.BandId = 1;
            result.MeanViewingIncidenceAngles.push_back(meanVIncAng);
        }
    }
    result.ViewingAngles =
        ReadViewingAnglesGridList(el->FirstChildElement("List_of_Viewing_Angles"));
    result.ReflectanceQuantificationValue = GetChildText(el, "Reflectance_Quantification_Value");
    result.BandWavelengths =
        ReadBandWavelengths(el->FirstChildElement("List_of_Band_Central_Wavelength"));
    result.BandResolutions =
        ReadBandResolutions(el->FirstChildElement("List_of_Band_Resolution"));

    return result;
}

CommonSize ReadSize(const TiXmlElement *el)
{
    CommonSize result;

    if (!el) {
        return result;
    }

    result.Lines = GetChildText(el, "Lines");
    result.Columns = GetChildText(el, "Columns");
    result.Bands = GetChildText(el, "Bands");

    return result;
}

CommonBand ReadBand(const TiXmlElement *el)
{
    CommonBand result;

    if (!el) {
        return result;
    }

    result.Id = GetAttribute(el, "sn");
    result.Name = GetText(el);

    return result;
}

CommonProductSampling ReadProductSampling(const TiXmlElement *el)
{
    CommonProductSampling result;

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

std::vector<CommonBand> ReadBands(const TiXmlElement *el)
{
    std::vector<CommonBand> result;

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

CommonResolution ReadResolution(const TiXmlElement *el)
{
    CommonResolution result;

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

std::vector<CommonResolution> ReadResolutions(const TiXmlElement *el)
{
    std::vector<CommonResolution> result;

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

CommonFileInformation ReadFileInformation(const TiXmlElement *el)
{
    CommonFileInformation result;

    if (!el) {
        return result;
    }
    // no BandNumber, BitNumber or GroupId in MACCS
    result.BandNumber = -1;
    result.BitNumber = -1;
    result.GroupId = "";

    result.Nature = GetChildText(el, "Nature");
    result.FileLocation = GetChildText(el, "File_Location");
    result.LogicalName = GetChildText(el, "Logical_Name");

    return result;
}

CommonAnnexInformation ReadAnnexInformation(const TiXmlElement *el)
{
    CommonAnnexInformation result;

    if (!el) {
        return result;
    }

    result.Id = GetAttribute(el, "sk");
    result.File = ReadFileInformation(el);

    return result;
}

std::vector<CommonAnnexInformation> ReadAnnexes(const TiXmlElement *el)
{
    std::vector<CommonAnnexInformation> result;

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

static void FixProductOrganization(CommonProductOrganization &po)
{
    auto foundSRE = false;
    auto foundSRE_R1 = false;
    auto foundSRE_R2 = false;
    auto foundFRE = false;
    auto foundFRE_R1 = false;
    auto foundFRE_R2 = false;

    std::string dir;
    std::string name;
    const auto &compareSuffix = [](
        const std::string &name, const std::string &suffix, bool &found, std::string &rest) {
        if (boost::algorithm::ends_with(name, suffix)) {
            found = true;
            if (rest.empty()) {
                rest = name.substr(0, name.length() - suffix.length());
            }
            return true;
        }

        return false;
    };

    for (const auto &file : po.ImageFiles) {
        if (compareSuffix(file.LogicalName, "_SRE", foundSRE, name) ||
            compareSuffix(file.LogicalName, "_SRE_R1", foundSRE_R1, name) ||
            compareSuffix(file.LogicalName, "_SRE_R2", foundSRE_R2, name) ||
            compareSuffix(file.LogicalName, "_FRE", foundFRE, name) ||
            compareSuffix(file.LogicalName, "_FRE_R1", foundFRE_R1, name) ||
            compareSuffix(file.LogicalName, "_FRE_R2", foundFRE_R2, name)) {
            if (dir.empty()) {
                boost::filesystem::path p(file.FileLocation);
                p.remove_filename();
                dir = p.native();
            }
        }
    }

    const auto addImageFile = [&po, &dir, &name](const std::string &nature,
                                                 const std::string &suffix) {
        const auto &logicalName = name + suffix;
        po.ImageFiles.emplace_back(
            CommonFileInformation{ nature, dir + "/" + logicalName + ".HDR", logicalName });
    };

    if (!foundSRE) {
        if (!foundSRE_R1) {
            addImageFile("SSC_PDTIMG", "_SRE_R1");
            if (!foundSRE_R2) {
                addImageFile("SSC_PDTIMG", "_SRE_R2");
            }
        }
    }
    if (!foundFRE) {
        if (!foundFRE_R1) {
            addImageFile("SSC_PDTIMG", "_FRE_R1");
        }
        if (!foundFRE_R2) {
            addImageFile("SSC_PDTIMG", "_FRE_R2");
        }
    }
}

CommonProductOrganization ReadProductOrganization(const TiXmlElement *el)
{
    CommonProductOrganization result;

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

    FixProductOrganization(result);

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
