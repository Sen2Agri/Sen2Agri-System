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
 
#include "MetadataHelper.h"
#include <time.h>
#include <ctime>
#include <cmath>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

template<typename PixelType, typename MasksPixelType>
MetadataHelper<PixelType, MasksPixelType>::MetadataHelper()
{
    m_detailedAnglesGridSize = 0;
}

template<typename PixelType, typename MasksPixelType>
MetadataHelper<PixelType, MasksPixelType>::~MetadataHelper()
{
}

template<typename PixelType, typename MasksPixelType>
bool MetadataHelper<PixelType, MasksPixelType>::LoadMetadataFile(const std::string& file)
{
    Reset();
    m_inputMetadataFileName = file;

    boost::filesystem::path p(m_inputMetadataFileName);
    p.remove_filename();
    m_DirName = p.native();

    return DoLoadMetadata(file);
}

template<typename PixelType, typename MasksPixelType>
void MetadataHelper<PixelType, MasksPixelType>::Reset()
{
    m_Mission = "";
    m_AcquisitionDate = "";

    m_ReflQuantifVal = 1;

    m_solarMeanAngles.azimuth = m_solarMeanAngles.zenith = 0.0;
    m_bHasDetailedAngles = false;
}

template<typename PixelType, typename MasksPixelType>
int MetadataHelper<PixelType, MasksPixelType>::GetAcquisitionDateAsDoy()
{
    struct tm tmDate = {};
    if (strptime(m_AcquisitionDate.c_str(), "%Y%m%d", &tmDate) == NULL) {
        return -1;
    }
    auto curTime = std::mktime(&tmDate);

    std::tm tmYearStart = {};
    tmYearStart.tm_year = tmDate.tm_year;
    tmYearStart.tm_mon = 0;
    tmYearStart.tm_mday = 1;

    auto yearStart = std::mktime(&tmYearStart);
    auto diff = curTime - yearStart;

    return lrintf(diff / 86400 /* 60*60*24*/);
}

template<typename PixelType, typename MasksPixelType>
MeanAngles_Type MetadataHelper<PixelType, MasksPixelType>::GetSensorMeanAngles() {
    MeanAngles_Type angles = {0,0};

    if(HasBandMeanAngles()) {
        size_t nBandsCnt = m_sensorBandsMeanAngles.size();
        int nValidAzimuths = 0;
        int nValidZeniths = 0;
        for(size_t i = 0; i < nBandsCnt; i++) {
            MeanAngles_Type bandAngles = m_sensorBandsMeanAngles[i];
            if(!std::isnan(bandAngles.azimuth)) {
                angles.azimuth += bandAngles.azimuth;
                nValidAzimuths++;
            }
            if(!std::isnan(bandAngles.zenith)) {
                angles.zenith += bandAngles.zenith;
                nValidZeniths++;
            }
        }
        if(nValidAzimuths > 0) {
            angles.azimuth = angles.azimuth / nValidAzimuths;
        } else {
            angles.azimuth = 0;
        }
        if(nValidZeniths > 0) {
            angles.zenith = angles.zenith / nValidZeniths;
        } else {
            angles.zenith = 0;
        }
    } else {
        angles = GetSensorMeanAngles(0);
    }

    return angles;
}

template<typename PixelType, typename MasksPixelType>
MeanAngles_Type MetadataHelper<PixelType, MasksPixelType>::GetSensorMeanAngles(int nBand) {
    MeanAngles_Type angles = {0,0};
    if(nBand >= 0 && nBand < (int)m_sensorBandsMeanAngles.size()) {
        angles = m_sensorBandsMeanAngles[nBand];
    } else if(m_sensorBandsMeanAngles.size() > 0) {
        angles = m_sensorBandsMeanAngles[0];
    }
    return angles;
}

template<typename PixelType, typename MasksPixelType>
double MetadataHelper<PixelType, MasksPixelType>::GetRelativeAzimuthAngle()
{
    MeanAngles_Type solarAngle = GetSolarMeanAngles();
    MeanAngles_Type sensorAngle = GetSensorMeanAngles();

    double relAzimuth = solarAngle.azimuth - sensorAngle.azimuth/* - 180.0 */;
/*
    if (relAzimuth < -180.0) {
        relAzimuth = relAzimuth + 360.0;
    }
    if (relAzimuth > 180.0) {
        relAzimuth = relAzimuth - 360.0;
    }
*/
    return relAzimuth;
}

template<typename PixelType, typename MasksPixelType>
bool MetadataHelper<PixelType, MasksPixelType>::GetTrueColourBandNames(std::string &redBandName, std::string &greenBandName, std::string &blueBandName)
{
    redBandName = m_nRedBandName;
    greenBandName = m_nGreenBandName;
    blueBandName = m_nBlueBandName;
    // we might not have blue and is green (as for spot, in this case we use NIR)
    // TODO: We should override this function for SPOT
    if(blueBandName == greenBandName) {
        blueBandName = m_nNirBandName;
    }
    if(redBandName.size() > 0 && greenBandName.size() > 0 && blueBandName.size() > 0) {
        return true;
    }
    return false;
}

template<typename PixelType, typename MasksPixelType>
std::string MetadataHelper<PixelType, MasksPixelType>::buildFullPath(const std::string& fileName)
{
    boost::filesystem::path p(m_DirName);
    p /= fileName;
    return p.string();
}



