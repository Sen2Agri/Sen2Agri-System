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

#include "SEN2CORMetadataReader.hpp"
#include "MetadataUtil.hpp"
#include "tinyxml_utils.hpp"
#include "string_utils.hpp"

namespace itk
{
std::unique_ptr<MACCSFileMetadata> SEN2CORMetadataReader::ReadMetadata(const std::string &path)
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

MACCSFixedHeader ReadSEN2CORGeneralInfo(const TiXmlElement *el)
{
    MACCSFixedHeader result;

    if (!el) {
        return result;
    }
    result.CreationDate = GetChildText(el, "PRODUCT_START_TIME");
    if (auto dataTakeEl = el->FirstChildElement("Datatake")) {
        result.Mission = GetChildText(dataTakeEl, "SPACECRAFT_NAME");
    }

    return result;
}

MACCSImageInformation ReadSEN2CORProductImageCharacteristics(const TiXmlElement *el)
{    
    //el should be Spectral_Information_List node in case of MTD_MSIl2A.xml
    MACCSImageInformation result;

    if (!el) {
        return result;
    }

    // find the No Data value
    bool noDataValueFound = false;
    for (auto specialValuesEl = el->FirstChildElement("Special_Values"); specialValuesEl && !noDataValueFound;
        specialValuesEl = specialValuesEl->NextSiblingElement("Special_Values")) {
        if (const TiXmlElement *specialValueTextEl = specialValuesEl->FirstChildElement("SPECIAL_VALUE_TEXT")) {
            if (GetText(specialValueTextEl).compare("NODATA") == 0) {
                noDataValueFound = true;
                result.NoDataValue = GetChildText(specialValuesEl, "SPECIAL_VALUE_INDEX");
                // ?? I can't find AOT No Data Value within the xml files, shall NoDataValue be used as a general "No Data" value for the others? (aot, boa, vap?)
                result.AOTNoDataValue = result.NoDataValue;
                result.VAPNoDataValue = result.NoDataValue;
            }
        }
    }
    //get the quantification values
    if (auto quantificationValueListEl = el->FirstChildElement("QUANTIFICATION_VALUES_LIST")) {
        result.AOTQuantificationValue = GetChildText(quantificationValueListEl, "AOT_QUANTIFICATION_VALUE");
        result.VAPQuantificationValue = GetChildText(quantificationValueListEl, "WVP_QUANTIFICATION_VALUE");
    }
    // get the bands
    if (auto spectralInformationListEl = el->FirstChildElement("Spectral_Information_List")) {
        for (auto spectralInfoEl = spectralInformationListEl->FirstChildElement("Spectral_Information"); spectralInfoEl;
             spectralInfoEl = spectralInfoEl->NextSiblingElement("Spectral_Information")) {
            CommonBand band;
            band.Id = GetAttribute(spectralInfoEl, "bandId");
            band.Name = GetAttribute(spectralInfoEl, "physicalBand");
            result.Bands.push_back(band);
        }
    }
    return result;
}

MACCSProductInformation ReadSEN2CORProductInformation(const TiXmlElement *el) {
    //el should be Spectral_Information_List node in case of MTD_MSIl2A.xml
    MACCSProductInformation result;

    if (!el) {
        return result;
    }
    if (auto productInfoEl = el->FirstChildElement("Product_Info")) {
        result.AcquisitionDateTime = GetChildText(productInfoEl, "PRODUCT_START_TIME");
    }
    if(auto productImageCharaceristicsEl = el->FirstChildElement("Product_Image_Characteristics")) {
        if (auto spectralInformationListEl = productImageCharaceristicsEl->FirstChildElement("Spectral_Information_List")) {
            for (auto spectralInfoEl = spectralInformationListEl->FirstChildElement("Spectral_Information"); spectralInfoEl;
                 spectralInfoEl = spectralInfoEl->NextSiblingElement("Spectral_Information")) {
                if (auto waveLengthEl = spectralInfoEl->FirstChildElement("Wavelength")) {
                    CommonBandWavelength bandWaveLength;
                    bandWaveLength.BandName = GetAttribute(spectralInfoEl, "physicalBand");
                    bandWaveLength.MinUnit = GetChildAttribute(waveLengthEl, "MIN", "unit");
                    bandWaveLength.MinWaveLength = GetChildText(waveLengthEl, "MIN");
                    bandWaveLength.MaxUnit = GetChildAttribute(waveLengthEl, "MAX", "unit");
                    bandWaveLength.MaxWaveLength = GetChildText(waveLengthEl, "MAX");
                    bandWaveLength.Unit = GetChildAttribute(waveLengthEl, "CENTRAL", "unit");
                    bandWaveLength.WaveLength = GetChildText(waveLengthEl, "CENTRAL");
                    result.BandWavelengths.push_back(bandWaveLength);
                }
                CommonBandResolution resolution;
                resolution.BandName = GetAttribute(spectralInfoEl, "physicalBand");
                resolution.Resolution = GetChildText(spectralInfoEl,"RESOLUTION");
                // no unit information within the xml file, so hardcode it
                resolution.Unit = "m";
                if(resolution.BandName.size() > 0 && resolution.Resolution.size() > 0) {
                    result.BandResolutions.push_back(resolution);
                }
            }
        }
        //get the reflectance quantification value (BOA)
        if (auto quantificationValueListEl = productImageCharaceristicsEl->FirstChildElement("QUANTIFICATION_VALUES_LIST")) {
            result.ReflectanceQuantificationValue = GetChildText(quantificationValueListEl, "BOA_QUANTIFICATION_VALUE");
        }
    }

    // get the angles, these are to be found within ./GRANULE/"tile_id.SAFE"/MTD_TL.xml file
    if (auto sunAnglesGridEl = el->FirstChildElement("Sun_Angles_Grid")) {
        result.SolarAngles = ReadSolarAngles(sunAnglesGridEl);
    }
    if (auto meanSunAngleEl = el->FirstChildElement("Mean_Sun_Angle")) {
        result.MeanSunAngle = ReadAnglePair(meanSunAngleEl,
                                        "ZENITH_ANGLE", "AZIMUTH_ANGLE");
    }
    if (auto meanViewingIncidenceAngleEl = el->FirstChildElement("Mean_Viewing_Incidence_Angle_List")) {
        result.MeanViewingIncidenceAngles = ReadMeanViewingIncidenceAngles(meanViewingIncidenceAngleEl);
    }
    if (el->FirstChildElement("Viewing_Incidence_Angles_Grids")) {
        result.ViewingAngles = ReadViewingAnglesGridList(el);
    }
    return result;
}

MACCSImageInformation ReadSEN2CORTile_Geocoding(const TiXmlElement *el)
{
    //el should be Tile_Geocoding node in case of  ./GRANULE/"tile_id.SAFE"/MTD_TL.xml file
    MACCSImageInformation result;

    if (!el) {
        return result;
    }
    // resolutions, they will be read from ./GRANULE/"tile_id.SAFE"/MTD_TL.xml file

    for (auto sizeEl = el->FirstChildElement("Size"); sizeEl;
         sizeEl = sizeEl->NextSiblingElement("Size")) {
        CommonResolution resolution;
        resolution.Id = GetAttribute(sizeEl, "resolution");
        resolution.Size.Lines = GetChildText(sizeEl, "NROWS");
        resolution.Size.Columns = GetChildText(sizeEl, "NCOLS");
        //find the geoposition for this resolution
        for (auto geopostionEl = el->FirstChildElement("Geoposition"); geopostionEl;
             geopostionEl = geopostionEl->NextSiblingElement("Geoposition")) {
            if (GetAttribute(geopostionEl, "resolution").compare(resolution.Id) == 0) {
                resolution.GeoPosition.UnitLengthX = GetChildText(geopostionEl, "ULX");
                resolution.GeoPosition.UnitLengthY = GetChildText(geopostionEl, "ULY");
                resolution.GeoPosition.DimensionX = GetChildText(geopostionEl, "XDIM");
                resolution.GeoPosition.DimensionY = GetChildText(geopostionEl, "YDIM");
                //can't find product sampling for SEN2COR processor, so hardcode one:
                resolution.ProductSampling.ByLineUnit = "m";
                resolution.ProductSampling.ByLineValue = resolution.Id;
                resolution.ProductSampling.ByColumnUnit = "m";
                resolution.ProductSampling.ByColumnValue = resolution.Id;
                result.Resolutions.push_back(resolution);
                break;
            }
        }
    }
    return result;
}

CommonAnnexInformation ReadSEN2CORAnnexInformation(const TiXmlElement *el)
{
    CommonAnnexInformation result;

    if (!el) {
        return result;
    }
    result.Id = GetAttribute(el, "bandId");
    // no BandNumber, BitNumber or GroupId in SEN2COR
    result.File.BandNumber = -1;
    result.File.BitNumber = -1;
    result.File.GroupId = "";

    result.File.Nature = GetAttribute(el, "type");
    result.File.FileLocation = GetText(el);
    if (result.File.FileLocation.size() > 2) {
        if (result.File.FileLocation.substr(0, 2).compare("./") != 0) {
            result.File.FileLocation.insert(0, "./");
        }
        result.File.LogicalName = GetLogicalFileName(result.File.FileLocation, false);
        /*
        size_t lastSlashPos = result.File.FileLocation.find_last_of("/");

        if (lastSlashPos != std::string::npos && lastSlashPos + 1 < result.File.FileLocation.size()) {
            unsigned short extLen = 0;
            if (result.File.FileLocation.find_last_of(".jp2") != std::string::npos ||
                    result.File.FileLocation.find_last_of(".gml") != std::string::npos) {
                extLen = 4;
            }
            result.File.LogicalName = result.File.FileLocation.substr(lastSlashPos + 1, result.File.FileLocation.size() - lastSlashPos - 1 - extLen);
        } else {
            result.File.LogicalName = result.File.FileLocation;
        }
        */
    }
    return result;
}

std::vector<CommonAnnexInformation> ReadSEN2CORAnnexFileInformation(const TiXmlElement *el)
{
    std::vector<CommonAnnexInformation> result;

    if (!el) {
        return result;
    }

    for (auto annexEl = el->FirstChildElement("MASK_FILENAME"); annexEl;
         annexEl = annexEl->NextSiblingElement("MASK_FILENAME")) {
        result.emplace_back(ReadSEN2CORAnnexInformation(annexEl));
    }
    return result;
}

std::vector<CommonFileInformation> ReadSEN2CORImageFileInformation(const TiXmlElement *el)
{
    std::vector<CommonFileInformation> result;

    if (!el) {
        return result;
    }
    const TiXmlElement *productOrganizationEl = el->FirstChildElement("Product_Organisation");
    if (!productOrganizationEl) {
        return result;
    }
    const TiXmlElement *granuleListEl = productOrganizationEl->FirstChildElement("Granule_List");
    if (!granuleListEl) {
        return result;
    }
    // the structure of the xml files sugests that there may be more than 1 tile pre product. This isn't handled here,
    // so if there are more than 1 tile per product, only the first one will be handled.
    const TiXmlElement *granuleEl = granuleListEl->FirstChildElement("Granule");
    if (!granuleEl) {
        return result;
    }

    for (auto fileEl = granuleEl->FirstChildElement("IMAGE_FILE"); fileEl;
         fileEl = fileEl->NextSiblingElement("IMAGE_FILE")) {
        CommonFileInformation imageFile;
        //no nature for the image files, so set one by default
        imageFile.Nature = "PIC";
        // no BandNumber, BitNumber or GroupId in SEN2COR
        imageFile.BandNumber = -1;
        imageFile.BitNumber = -1;
        imageFile.GroupId = "";

        imageFile.FileLocation = GetText(fileEl);
        if (imageFile.FileLocation.size() >= 2) {
            if (imageFile.FileLocation.substr(0, 2).compare("./") != 0) {
                imageFile.FileLocation.insert(0, "./");
            }
            imageFile.FileLocation.append(".jp2");
            imageFile.LogicalName = GetLogicalFileName(imageFile.FileLocation, false);
            /*size_t lastSlashPos = imageFile.FileLocation.find_last_of("/");
            if (lastSlashPos != std::string::npos && lastSlashPos + 1 < imageFile.FileLocation.size()) {
                imageFile.LogicalName = imageFile.FileLocation.substr(lastSlashPos + 1, imageFile.FileLocation.size() - lastSlashPos);
            } else {
                imageFile.LogicalName = imageFile.FileLocation;
            }
            */
        }
        result.emplace_back(imageFile);
    }

    return result;
}

std::unique_ptr<MACCSFileMetadata> SEN2CORMetadataReader::ReadMetadataXml(const TiXmlDocument &doc)
{
    TiXmlHandle hDoc(const_cast<TiXmlDocument *>(&doc));

    // BUG: TinyXML can't properly read stylesheet declarations, see
    // http://sourceforge.net/p/tinyxml/patches/37/
    // Our files start with one, but we can't read it in.
    const TiXmlElement* rootProductGeneralInfoElement = hDoc.FirstChildElement("n1:Level-2A_User_Product").ToElement();
    const TiXmlElement* generalInfoElement = nullptr;
    auto file = std::unique_ptr<MACCSFileMetadata>(new MACCSFileMetadata);

    if ( rootProductGeneralInfoElement && (generalInfoElement = rootProductGeneralInfoElement->FirstChildElement("n1:General_Info")) ) {

        file->Header.SchemaLocation = GetAttribute(rootProductGeneralInfoElement, "xsi:schemaLocation");
        file->Header.FixedHeader = ReadSEN2CORGeneralInfo(generalInfoElement->FirstChildElement("Product_Info"));
        file->ImageInformation = ReadSEN2CORProductImageCharacteristics(generalInfoElement->FirstChildElement("Product_Image_Characteristics"));
        file->ProductOrganization.ImageFiles = ReadSEN2CORImageFileInformation(generalInfoElement->FirstChildElement("Product_Info"));
        file->ProductInformation = ReadSEN2CORProductInformation(generalInfoElement);
        file->InstanceId.AcquisitionDate = ExtractDateFromDateTime(file->ProductInformation.AcquisitionDateTime);
        return file;
    }
    if (auto rootTileInfoElement = hDoc.FirstChildElement("n1:Level-2A_Tile_ID").ToElement()) {

        if (auto geometricInfoElement = rootTileInfoElement->FirstChildElement("n1:Geometric_Info")) {
            file->ImageInformation = ReadSEN2CORTile_Geocoding(geometricInfoElement->FirstChildElement("Tile_Geocoding"));
            file->ProductInformation = ReadSEN2CORProductInformation(geometricInfoElement->FirstChildElement("Tile_Angles"));
        }
        if (auto qualityIndicatorElement = rootTileInfoElement->FirstChildElement("n1:Quality_Indicators_Info")) {
            file->ProductOrganization.AnnexFiles = ReadSEN2CORAnnexFileInformation(qualityIndicatorElement->FirstChildElement("Pixel_Level_QI"));
        }

        return file;
    }
    return nullptr;
}
}
