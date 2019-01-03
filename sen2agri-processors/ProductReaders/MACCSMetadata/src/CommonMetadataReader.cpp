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
 
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include "CommonMetadata.hpp"


std::string getRasterFile(const MACCSFileMetadata &metadata, const char *suffix)
{
    std::string file;

    for (const auto &fileInfo : metadata.ProductOrganization.ImageFiles) {
        if (boost::algorithm::ends_with(fileInfo.LogicalName, suffix)) {
            boost::filesystem::path p(metadata.ProductPath);
            p.remove_filename();
            p /= fileInfo.FileLocation;
            p.replace_extension(".DBL.TIF");
            file = p.string();
            break;
        }

    }

    return file;
}

std::string getRasterFile(const SPOT4Metadata &metadata, const std::string &file)
{
    boost::filesystem::path p(metadata.ProductPath);
    p.remove_filename();
    p /= file;
    return p.string();
}

std::string getMainRasterFile(const MACCSFileMetadata &metadata)
{
    auto raster = getRasterFile(metadata, "_FRE_R1");
    if (raster.empty()) {
        raster = getRasterFile(metadata, "_FRE");
    }
    if (raster.empty()) {
        raster = getRasterFile(metadata, "_SRE_R1");
    }
    if (raster.empty()) {
        raster = getRasterFile(metadata, "_SRE");
    }
    return raster;
}

std::string getMainRasterFile(const SPOT4Metadata &metadata)
{
    auto raster = getRasterFile(metadata, metadata.Files.OrthoSurfCorrPente);
    if (raster.empty()) {
        raster = getRasterFile(metadata, metadata.Files.OrthoSurfCorrEnv);
    }
    return raster;
}

std::string getMainRasterFile(const std::string &path)
{
    if (auto metadata = itk::MACCSMetadataReader::New()->ReadMetadata(path)) {
        return getMainRasterFile(*metadata);
    } else if (auto metadata = itk::SPOT4MetadataReader::New()->ReadMetadata(path)) {
        return getMainRasterFile(*metadata);
    } else {
        return path;
    }
}

CommonAnglePair ReadAnglePair(const TiXmlElement *el, const std::string &zenithElName,
                             const std::string &azimuthElName)
{
    CommonAnglePair result;
    result.ZenithValue = std::numeric_limits<double>::quiet_NaN();
    result.AzimuthValue = std::numeric_limits<double>::quiet_NaN();

    if (!el) {
        return result;
    }

    if (auto zenithEl = el->FirstChildElement(zenithElName)) {
        result.ZenithUnit = GetAttribute(zenithEl, "unit");
        result.ZenithValue = ReadDouble(GetText(zenithEl));
    }

    if (auto azimuthEl = el->FirstChildElement(azimuthElName)) {
        result.AzimuthUnit = GetAttribute(azimuthEl, "unit");
        result.AzimuthValue = ReadDouble(GetText(azimuthEl));
    }

    return result;
}

CommonMeanViewingIncidenceAngle ReadMeanViewingIncidenceAngle(const TiXmlElement *el)
{
    CommonMeanViewingIncidenceAngle result;

    if (!el) {
        return result;
    }

    result.BandId = GetAttribute(el, "bandId");
    if(result.BandId.empty())
        result.BandId = GetAttribute(el, "band_id");
    result.Angles = ReadAnglePair(el, "ZENITH_ANGLE", "AZIMUTH_ANGLE");

    return result;
}

std::vector<CommonMeanViewingIncidenceAngle> ReadMeanViewingIncidenceAngles(const TiXmlElement *el)
{
    std::vector<CommonMeanViewingIncidenceAngle> result;

    if (!el) {
        return result;
    }

    for (auto angleEl = el->FirstChildElement("Mean_Viewing_Incidence_Angle"); angleEl;
         angleEl = angleEl->NextSiblingElement("Mean_Viewing_Incidence_Angle")) {
        result.emplace_back(ReadMeanViewingIncidenceAngle(angleEl));
    }

    return result;
}
