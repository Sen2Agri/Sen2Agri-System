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


#include "MAJAMetadataReader.hpp"
#include "MetadataUtil.hpp"
#include "tinyxml_utils.hpp"
#include "string_utils.hpp"

namespace itk
{

std::unique_ptr<MACCSFileMetadata> MAJAMetadataReader::ReadMetadata(const std::string &path)
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

MAJAMetadataIdentification ReadMAJAMetadataIdentification(const TiXmlElement *el)
{
    MAJAMetadataIdentification result;
    if (!el) {
        return result;
    }
    for (auto metadataFormatEl = el->FirstChildElement("METADATA_FORMAT"); metadataFormatEl;
         metadataFormatEl = metadataFormatEl->NextSiblingElement("METADATA_FORMAT")) {
        result.MetdataFormatVersion = GetAttribute(metadataFormatEl, "version");
        if(result.MetdataFormatVersion.size() > 0) {
            result.MetadataFormat = GetText(metadataFormatEl);
        }
    }
    result.MetadataFormat = GetChildText(el, "METADATA_FORMAT");
    result.MetadataProfile = GetChildText(el, "METADATA_PROFILE");
    result.MetadataInformation = GetChildText(el, "METADATA_INFORMATION");
    return result;
}

MAJADatasetIdentification ReadMAJADatasetIdentification(const TiXmlElement *el)
{
    MAJADatasetIdentification result;
    if (!el) {
        return result;
    }
    result.Identifier = GetChildText(el, "IDENTIFIER");
    result.Authority = GetChildText(el, "AUTHORITY");
    result.Producer = GetChildText(el, "PRODUCER");
    result.Project = GetChildText(el, "PROJECT");
    for (auto geographicalZoneEl = el->FirstChildElement("GEOGRAPHICAL_ZONE"); geographicalZoneEl;
         geographicalZoneEl = geographicalZoneEl->NextSiblingElement("GEOGRAPHICAL_ZONE")) {
        auto tileAttributeVal = GetAttribute(geographicalZoneEl, "type");
        if(tileAttributeVal.size() > 0 && tileAttributeVal.compare("Tile") == 0) {
            result.GeographicalZoneTile = GetText(geographicalZoneEl);
        }
    }
    return result;
}

CommonSize ReadMAJASize(const TiXmlElement *el)
{
    CommonSize result;
    if (!el) {
        return result;
    }

    result.Lines = GetChildText(el, "NROWS");
    result.Columns = GetChildText(el, "NCOLS");
    return result;
}

MAJAProductCharacteristics ReadMAJAProductCharacteristics(const TiXmlElement *prodCharacteristicsEl, const TiXmlElement *geopositionInfoEl)
{
    MAJAProductCharacteristics result;
    if (!prodCharacteristicsEl || !geopositionInfoEl) {
        return result;
    }
    auto geopositioningEl = geopositionInfoEl->FirstChildElement("Geopositioning");
    if (!geopositioningEl) {
        return result;
    }
    auto groupGeopositioningListEl = geopositioningEl->FirstChildElement("Group_Geopositioning_List");
    if (!groupGeopositioningListEl) {
        return result;
    }
    result.AcquisitionDateTime = GetChildText(prodCharacteristicsEl, "ACQUISITION_DATE");
    result.Mission = GetChildText(prodCharacteristicsEl, "PLATFORM");
    auto bandGroupList = prodCharacteristicsEl->FirstChildElement("Band_Group_List");
    if (bandGroupList) {
        unsigned short idx = 0;
        for (auto groupEl = bandGroupList->FirstChildElement("Group"); groupEl;
             groupEl = groupEl->NextSiblingElement("Group")) {
            CommonResolution groupRes;
            groupRes.Id = GetAttribute(groupEl, "group_id");
            if (groupRes.Id.compare("R1") != 0 && groupRes.Id.compare("R2")) {
                continue;
            }
            if (auto bandList = groupEl->FirstChildElement("Band_List")) {
                 const TiXmlElement *bandEl = nullptr;
                for (bandEl = bandList->FirstChildElement("BAND_ID"); bandEl;
                     bandEl = bandEl->NextSiblingElement("BAND_ID"), idx++) {
                    CommonBand band;
                    band.Id = std::to_string(idx);
                    band.Name = GetText(bandEl);
                    groupRes.Bands.push_back(band);
                }
            }
            bool geopositioningFound = false;
            for (auto groupGeopositiongEl = groupGeopositioningListEl->FirstChildElement("Group_Geopositioning"); groupGeopositiongEl;
                 groupGeopositiongEl = groupGeopositiongEl->NextSiblingElement("Group_Geopositioning")) {
                if(groupRes.Id.compare(GetAttribute(groupGeopositiongEl, "group_id")) == 0) {
                    groupRes.GeoPosition = ReadGeoPosition(groupGeopositiongEl);
                    //can't find product sampling for MAJA processor, so hardcode one:
                    groupRes.ProductSampling.ByLineUnit = "m";
                    groupRes.ProductSampling.ByLineValue = (groupRes.Id.compare("R1") == 0 ? "10" : "20");
                    groupRes.ProductSampling.ByColumnUnit = "m";
                    groupRes.ProductSampling.ByColumnValue = (groupRes.Id.compare("R1") == 0 ? "10" : "20");
                    groupRes.Size = ReadMAJASize(groupGeopositiongEl);
                    groupRes.Size.Bands = std::to_string(groupRes.Bands.size());
                    geopositioningFound = true;
                }
            }
            if (geopositioningFound)
                result.GroupResolutions.push_back(groupRes);
        }
    }
    return result;
}

CommonFileInformation ReadMAJAImageFile (const TiXmlElement *el, const std::string nature) {
    CommonFileInformation result;
    if (!el) {
        return result;
    }

    result.BandNumber = -1;
    result.BitNumber = -1;
    result.Nature = nature;
    result.FileLocation = GetText(el);
    if (result.FileLocation.size() >= 2) {
        if (result.FileLocation.substr(0, 2).compare("./") != 0) {
            result.FileLocation.insert(0, "./");
        }
    }
    result.LogicalName = GetLogicalFileName(result.FileLocation, false);
    result.BandId = GetAttribute(el, "band_id");
    if (auto bandNumberStr = el->Attribute("band_number")) {
        try
        {
            result.BandNumber = std::stoi(bandNumberStr);
        }
        catch (const std::runtime_error &e)
        {
            otbMsgDevMacro("Invalid band number : " << bandNumberStr);
            otbMsgDevMacro(<< e.what());
        }
    }
    if (auto bitNumberStr = el->Attribute("bit_number")) {
        try
        {
            result.BitNumber = std::stoi(bitNumberStr);
        }
        catch (const std::runtime_error &e)
        {
            otbMsgDevMacro("Invalid bit number : " << bitNumberStr);
            otbMsgDevMacro(<< e.what());
        }
    }
    result.GroupId = GetAttribute(el, "group_id");
    return result;
}

std::vector<CommonFileInformation> ReadGeneralImageList(const TiXmlElement *el, const std::string imgListTag, const std::string imgTag,
                          const std::string imgPropTag, const std::string imgFileListTag,
                          const std::string imgFileTag)
{
    std::vector<CommonFileInformation> result;
    if (!el) {
        return result;
    }

    auto imageListEl = el->FirstChildElement(imgListTag);
    for (auto imageEl = imageListEl->FirstChildElement(imgTag); imageEl;
         imageEl = imageEl->NextSiblingElement(imgTag)) {
        std::string nature;
        auto imagePropEl = imageEl->FirstChildElement(imgPropTag);
        if (imagePropEl) {
            nature = GetChildText(imagePropEl, "NATURE");
        }
        auto imageFileListEl = imageEl->FirstChildElement(imgFileListTag);
        if (imageFileListEl) {
            for(auto imageFileEl = imageFileListEl->FirstChildElement(imgFileTag); imageFileEl;
                imageFileEl = imageFileEl->NextSiblingElement(imgFileTag)) {
                result.push_back(ReadMAJAImageFile(imageFileEl, nature));
            }
        }
    }
    return result;
}

std::vector<CommonFileInformation> ReadQuickLookFiles(const TiXmlElement *el) {
    std::vector<CommonFileInformation> result;
    if (!el) {
        return result;
    }

    CommonFileInformation quickLookFile;
    quickLookFile.BandNumber = -1;
    quickLookFile.BitNumber = -1;
    quickLookFile.Nature = "QCK";
    quickLookFile.FileLocation = GetChildText(el, "QUICKLOOK");
    if (quickLookFile.FileLocation.size() >= 2) {
        if (quickLookFile.FileLocation.substr(0, 2).compare("./") != 0) {
            quickLookFile.FileLocation.insert(0, "./");
        }
    }
    quickLookFile.LogicalName = GetLogicalFileName(quickLookFile.FileLocation, false);

    result.push_back(quickLookFile);
    return result;
}

CommonProductOrganization ReadMAJAProductOrganization(const TiXmlElement *el)
{
    CommonProductOrganization result;
    if (!el) {
        return result;
    }
    auto muscateProductEl = el->FirstChildElement("Muscate_Product");
    if(!muscateProductEl) {
        return result;
    }
    CommonFileInformation quickLookFile;

    result.QuickLookFiles = ReadQuickLookFiles(muscateProductEl);
    result.ImageFiles = ReadGeneralImageList(muscateProductEl, "Image_List", "Image",
                         "Image_Properties", "Image_File_List", "IMAGE_FILE");
    std::vector<CommonFileInformation> tmpFiles =  ReadGeneralImageList(muscateProductEl, "Mask_List", "Mask",
                         "Mask_Properties", "Mask_File_List", "MASK_FILE");
    for (auto tmpFile : tmpFiles) {
        CommonAnnexInformation maskFile;
        maskFile.File = std::move(tmpFile);
        result.AnnexFiles.push_back(maskFile);
    }
    return result;
}

std::vector<CommonViewingAnglesGrid> ReadMAJAViewingAnglesGridList(const TiXmlElement *el)
{
    std::vector<CommonViewingAnglesGrid> result;

    if (!el) {
        return result;
    }
    std::string bandId = "";
    for (auto bandEl = el->FirstChildElement("Band_Viewing_Incidence_Angles_Grids_List"); bandEl;
         bandEl = bandEl->NextSiblingElement("Band_Viewing_Incidence_Angles_Grids_List")) {
        bandId = GetAttribute(bandEl, "band_id");
        if(bandId.empty())
            bandId = GetAttribute(bandEl, "bandId");
        for (auto detectorEl = bandEl->FirstChildElement("Viewing_Incidence_Angles_Grids"); detectorEl;
             detectorEl = detectorEl->NextSiblingElement("Viewing_Incidence_Angles_Grids")) {
            CommonViewingAnglesGrid angleGrid;
            angleGrid.BandId = bandId;
            angleGrid.DetectorId = GetAttribute(detectorEl, "detector_id");
            if(angleGrid.DetectorId.empty())
                angleGrid.DetectorId = GetAttribute(detectorEl, "detectorId");

            angleGrid.Angles = ReadSolarAngles(detectorEl);
            result.emplace_back(angleGrid);
        }
    }

    return result;
}

MAJAGeometricInformation ReadMAJAGeometricInformation(const TiXmlElement *el)
{
    MAJAGeometricInformation result;

    if (!el) {
        return result;
    }
    auto meanValueListEl = el->FirstChildElement("Mean_Value_List");
    auto anglesGridListEl = el->FirstChildElement("Angles_Grids_List");
    if(!meanValueListEl || !anglesGridListEl) {
        return result;
    }
    result.MeanSunAngle = ReadAnglePair(meanValueListEl->FirstChildElement("Sun_Angles"),
                                        "ZENITH_ANGLE", "AZIMUTH_ANGLE");

    result.SolarAngles = ReadSolarAngles(anglesGridListEl->FirstChildElement("Sun_Angles_Grids"));

    result.MeanViewingIncidenceAngles =
        ReadMeanViewingIncidenceAngles(meanValueListEl->FirstChildElement("Mean_Viewing_Incidence_Angle_List"));

    result.ViewingAngles =
        ReadMAJAViewingAnglesGridList(anglesGridListEl->FirstChildElement("Viewing_Incidence_Angles_Grids_List"));

    return result;
}

CommonBandResolution ReadMAJABandResolution(const TiXmlElement *el)
{
    CommonBandResolution result;

    if (!el) {
        return result;
    }

    result.BandName = GetAttribute(el, "band_id");
    if (auto resolutionEl = el->FirstChildElement("SPATIAL_RESOLUTION")) {
        result.Unit = GetAttribute(resolutionEl, "unit");
        result.Resolution = GetText(resolutionEl);
    }

    return result;
}

std::vector<CommonBandResolution> ReadMAJABandResolutions(const TiXmlElement *el)
{
    std::vector<CommonBandResolution> result;

    if (!el) {
        return result;
    }

    for (auto resEl = el->FirstChildElement("Spectral_Band_Informations"); resEl;
        resEl = resEl->NextSiblingElement("Spectral_Band_Informations")) {
        result.emplace_back(ReadMAJABandResolution(resEl));
    }

    return result;
}

CommonBandWavelength ReadMAJABandWavelength(const TiXmlElement *el)
{
    CommonBandWavelength result;

    if (!el) {
        return result;
    }

    result.BandName = GetAttribute(el, "band_id");
    if (auto waveLenEl = el->FirstChildElement("Wavelength")) {
        if(auto centralEl = waveLenEl->FirstChildElement("CENTRAL")) {
            result.Unit = GetAttribute(centralEl, "unit");
            result.WaveLength = GetText(centralEl);
        }
        if(auto maxEl = waveLenEl->FirstChildElement("MAX")) {
            result.MaxUnit = GetAttribute(maxEl, "unit");
            result.MaxWaveLength = GetText(maxEl);
        }
        if(auto minEl = waveLenEl->FirstChildElement("MIN")) {
            result.MinUnit = GetAttribute(minEl, "unit");
            result.MinWaveLength = GetText(minEl);
        }
    }

    return result;
}

std::vector<CommonBandWavelength> ReadMAJABandWavelengths(const TiXmlElement *el)
{
    std::vector<CommonBandWavelength> result;

    if (!el) {
        return result;
    }

    for (auto resEl = el->FirstChildElement("Spectral_Band_Informations"); resEl;
        resEl = resEl->NextSiblingElement("Spectral_Band_Informations")) {
        result.emplace_back(ReadMAJABandWavelength(resEl));
    }

    return result;
}

MAJARadiometricInformation ReadMAJARadiometricInformation(const TiXmlElement *el)
{
    MAJARadiometricInformation result;

    if (!el) {
        return result;
    }
    result.ReflectanceQuantificationValue = GetChildText(el, "REFLECTANCE_QUANTIFICATION_VALUE");
    result.AOTQuantificationValue = GetChildText(el, "AEROSOL_OPTICAL_THICKNESS_QUANTIFICATION_VALUE");
    result.VAPQuantificationValue = GetChildText(el, "WATER_VAPOR_CONTENT_QUANTIFICATION_VALUE");
    if (auto specialValueListEl = el->FirstChild("Special_Values_List")) {
        for (auto specialValueEl = specialValueListEl->FirstChildElement("SPECIAL_VALUE"); specialValueEl;
             specialValueEl = specialValueEl->NextSiblingElement("SPECIAL_VALUE")) {
            std::string nameAttribute = GetAttribute(specialValueEl, "name");
            if (nameAttribute.compare("nodata") == 0) {
                result.NoDataValue = GetText(specialValueEl);
            }
            if (nameAttribute.compare("water_vapor_content_nodata") == 0) {
                result.AOTNoDataValue = GetText(specialValueEl);
            }
            if (nameAttribute.compare("aerosol_optical_thickness_nodata") == 0) {
                result.VAPNoDataValue = GetText(specialValueEl);
            }
        }
    }
    result.BandResolutions =
        ReadMAJABandResolutions(el->FirstChildElement("Spectral_Band_Informations_List"));
    result.BandWavelengths =
        ReadMAJABandWavelengths(el->FirstChildElement("Spectral_Band_Informations_List"));
    return result;
}

std::unique_ptr<MACCSFileMetadata> ConvertToMACCSStructure(std::unique_ptr<MAJAFileMetadata> &fileMaja) {
    if (!fileMaja) {
        return nullptr;
    }

    std::unique_ptr<MACCSFileMetadata> fileRetMACCS = std::unique_ptr<MACCSFileMetadata>(new MACCSFileMetadata);
    fileRetMACCS->Header.FixedHeader.Mission = std::move(fileMaja->ProductCharacteristics.Mission);
    fileRetMACCS->Header.FixedHeader.SourceSystem = std::move(fileMaja->DatasetIdentification.Producer);
    fileRetMACCS->InstanceId.AcquisitionDate = ExtractDateFromDateTime(fileMaja->ProductCharacteristics.AcquisitionDateTime);
    fileRetMACCS->ProductInformation.AcquisitionDateTime = std::move(fileMaja->ProductCharacteristics.AcquisitionDateTime);
    for (unsigned short resolutionIdx = 0; resolutionIdx < fileMaja->ProductCharacteristics.GroupResolutions.size();
         resolutionIdx++) {
        for (unsigned short bandIdx = 0; bandIdx < fileMaja->ProductCharacteristics.GroupResolutions[resolutionIdx].Bands.size();
             bandIdx++) {

            fileRetMACCS->ImageInformation.Bands.push_back(fileMaja->ProductCharacteristics.GroupResolutions[resolutionIdx].Bands[bandIdx]);
        }
    }
    fileRetMACCS->ImageInformation.Resolutions = std::move(fileMaja->ProductCharacteristics.GroupResolutions);
    fileRetMACCS->ImageInformation.NoDataValue = std::move(fileMaja->RadiometricInformation.NoDataValue);
    fileRetMACCS->ImageInformation.AOTQuantificationValue = std::move(fileMaja->RadiometricInformation.AOTQuantificationValue);
    fileRetMACCS->ImageInformation.AOTNoDataValue = std::move(fileMaja->RadiometricInformation.AOTNoDataValue);
    fileRetMACCS->ImageInformation.VAPQuantificationValue = std::move(fileMaja->RadiometricInformation.VAPQuantificationValue);
    fileRetMACCS->ImageInformation.VAPNoDataValue = std::move(fileMaja->RadiometricInformation.VAPNoDataValue);
    fileRetMACCS->ProductOrganization = std::move(fileMaja->ProductOrganization);

    fileRetMACCS->ProductInformation.ReflectanceQuantificationValue = fileMaja->RadiometricInformation.ReflectanceQuantificationValue;
    fileRetMACCS->ProductInformation.BandResolutions = std::move(fileMaja->RadiometricInformation.BandResolutions);
    fileRetMACCS->ProductInformation.BandWavelengths = std::move(fileMaja->RadiometricInformation.BandWavelengths);
    fileRetMACCS->ProductInformation.MeanViewingIncidenceAngles = std::move(fileMaja->GeometricInformation.MeanViewingIncidenceAngles);
    fileRetMACCS->ProductInformation.MeanSunAngle = std::move(fileMaja->GeometricInformation.MeanSunAngle);
    fileRetMACCS->ProductInformation.ViewingAngles = std::move(fileMaja->GeometricInformation.ViewingAngles);
    fileRetMACCS->ProductInformation.SolarAngles = std::move(fileMaja->GeometricInformation.SolarAngles);
    return fileRetMACCS;
}

std::unique_ptr<MACCSFileMetadata> MAJAMetadataReader::ReadMetadataXml(const TiXmlDocument &doc)
{
    TiXmlHandle hDoc(const_cast<TiXmlDocument *>(&doc));

    // BUG: TinyXML can't properly read stylesheet declarations, see
    // http://sourceforge.net/p/tinyxml/patches/37/
    // Our files start with one, but we can't read it in.
    auto rootElement = hDoc.FirstChildElement("Muscate_Metadata_Document").ToElement();

    if (!rootElement) {
        return nullptr;
    }

    std::unique_ptr<MAJAFileMetadata> file = std::unique_ptr<MAJAFileMetadata>(new MAJAFileMetadata);

    file->MetadataIdentification = ReadMAJAMetadataIdentification(rootElement->FirstChildElement("Metadata_Identification"));
    file->DatasetIdentification = ReadMAJADatasetIdentification(rootElement->FirstChildElement("Dataset_Identification"));
    file->ProductCharacteristics = ReadMAJAProductCharacteristics(rootElement->FirstChildElement("Product_Characteristics"), rootElement->FirstChildElement("Geoposition_Informations"));
    file->ProductOrganization = ReadMAJAProductOrganization(rootElement->FirstChildElement("Product_Organisation"));
    file->GeometricInformation = ReadMAJAGeometricInformation(rootElement->FirstChildElement("Geometric_Informations"));
    file->RadiometricInformation = ReadMAJARadiometricInformation(rootElement->FirstChildElement("Radiometric_Informations"));

    return ConvertToMACCSStructure(file);
}
}
