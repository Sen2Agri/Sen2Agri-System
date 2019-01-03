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
#include <boost/algorithm/string.hpp>

#include "MetadataUtil.hpp"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"

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

CommonAngleList ReadAngleList(const TiXmlElement *el)
{
    CommonAngleList result;

    if (!el) {
        return result;
    }

    if (auto colStepEl = el->FirstChildElement("COL_STEP")) {
        //maccs case
        result.ColumnUnit = GetAttribute(colStepEl, "unit");
        if(result.ColumnUnit.size() == 0) {
            //maja case
            result.ColumnUnit = GetAttribute(el, "step_unit");
        }
        result.ColumnStep = GetText(colStepEl);
    }

    if (auto rowStepEl = el->FirstChildElement("ROW_STEP")) {
        //maccs case
        result.RowUnit = GetAttribute(rowStepEl, "unit");
        if(result.RowUnit.size() == 0) {
            //maja case
            result.RowUnit = GetAttribute(el, "step_unit");
        }
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

CommonAngles ReadSolarAngles(const TiXmlElement *el)
{
    CommonAngles result;

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

CommonViewingAnglesGrid ReadViewingAnglesGrid(const TiXmlElement *el)
{
    CommonViewingAnglesGrid result;

    if (!el) {
        return result;
    }

    result.BandId = GetAttribute(el, "bandId");
    if(result.BandId.empty())
        result.BandId = GetAttribute(el, "band_id");
    result.DetectorId = GetAttribute(el, "detectorId");
    if(result.DetectorId.empty())
        result.DetectorId = GetAttribute(el, "detector_id");
    result.Angles = ReadSolarAngles(el);

    return result;
}

std::vector<CommonViewingAnglesGrid> ReadViewingAnglesGridList(const TiXmlElement *el)
{
    std::vector<CommonViewingAnglesGrid> result;

    if (!el) {
        return result;
    }

    for (auto gridsEl = el->FirstChildElement("Viewing_Incidence_Angles_Grids"); gridsEl;
         gridsEl = gridsEl->NextSiblingElement("Viewing_Incidence_Angles_Grids")) {
        result.emplace_back(ReadViewingAnglesGrid(gridsEl));
    }

    return result;
}

CommonGeoPosition ReadGeoPosition(const TiXmlElement *el)
{
    CommonGeoPosition result;

    if (!el) {
        return result;
    }

    result.UnitLengthX = GetChildText(el, "ULX");
    result.UnitLengthY = GetChildText(el, "ULY");
    result.DimensionX = GetChildText(el, "XDIM");
    result.DimensionY = GetChildText(el, "YDIM");

    return result;
}

std::string ExtractDateFromDateTime(std::string dateTime) {
    if(dateTime.size() == 0)
        return dateTime;
    std::string date;

    size_t tPos = dateTime.find_first_of('T');
    if (tPos != std::string::npos) {
        //set date from date time
        date = dateTime.substr(0, tPos);
        boost::erase_all(date, "-");
    }
    return date;
}

/* Get File Name from a Path with or without extension
*/
std::string GetLogicalFileName(std::string filePath, bool withExtension)
{
   // Create a Path object from File Path
   boost::filesystem::path pathObj(filePath);

   // Check if file name is required without extension
   if(withExtension == false)
   {
       // Check if file has stem i.e. filename without extension
       if(pathObj.has_stem())
       {
           // return the stem (file name without extension) from path object
           return pathObj.stem().string();
       }
       return "";
   }
   else
   {
       // return the file name with extension from path object
       return pathObj.filename().string();
   }

}
