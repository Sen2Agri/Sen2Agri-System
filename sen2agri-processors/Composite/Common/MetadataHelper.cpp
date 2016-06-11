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
#include "libgen.h"

MetadataHelper::MetadataHelper()
{
    m_nResolution = -1;
    m_detailedAnglesGridSize = 0;
}

MetadataHelper::~MetadataHelper()
{

}

bool MetadataHelper::LoadMetadataFile(const std::string& file, int nResolution)
{
    Reset();
    m_inputMetadataFileName = file;
    m_nResolution = nResolution;
    //std::vector<char> buf(m_inputMetadataFileName.begin(), m_inputMetadataFileName.end());
    //m_DirName = std::string(dirname(buf.data()));
    m_DirName = extractFolder(m_inputMetadataFileName);

    return DoLoadMetadata();
}

void MetadataHelper::Reset()
{
    m_Mission = "";

    m_AotFileName = "";
    m_CloudFileName = "";
    m_WaterFileName = "";
    m_SnowFileName = "";
    m_ImageFileName = "";
    m_AcquisitionDate = "";

    m_ReflQuantifVal = 1;

    m_fAotQuantificationValue = 0.0;
    m_fAotNoDataVal = 0;
    m_nAotBandIndex = -1;

    m_nAbsRedBandIndex = -1;
    m_nAbsGreenBandIndex = -1;
    m_nAbsBlueBandIndex = -1;
    m_nAbsNirBandIndex = -1;

    m_nRelRedBandIndex = -1;
    m_nRelGreenBandIndex = -1;
    m_nRelBlueBandIndex = -1;
    m_nRelNirBandIndex = -1;


    m_solarMeanAngles.azimuth = m_solarMeanAngles.zenith = 0.0;
    m_bHasDetailedAngles = false;
}

int MetadataHelper::GetAcquisitionDateAsDoy()
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

MeanAngles_Type MetadataHelper::GetSensorMeanAngles() {
    MeanAngles_Type angles;

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

MeanAngles_Type MetadataHelper::GetSensorMeanAngles(int nBand) {
    MeanAngles_Type angles;
    angles.azimuth = 0;
    angles.zenith = 0;
    if(nBand >= 0 && nBand < (int)m_sensorBandsMeanAngles.size()) {
        angles = m_sensorBandsMeanAngles[nBand];
    } else if(m_sensorBandsMeanAngles.size() > 0) {
        angles = m_sensorBandsMeanAngles[0];
    }
    return angles;
}

double MetadataHelper::GetRelativeAzimuthAngle()
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

bool MetadataHelper::GetTrueColourBandIndexes(int &redIdx, int &greenIdx, int &blueIdx)
{
    redIdx = m_nAbsRedBandIndex;
    greenIdx = m_nAbsGreenBandIndex;
    blueIdx = m_nAbsBlueBandIndex;
    // we might not have blue and is green (as for spot, in this case we use NIR)
    // TODO: We should override this function for SPOT
    if(blueIdx == greenIdx) {
        blueIdx = m_nAbsNirBandIndex;
    }
    if(redIdx > 0 && greenIdx > 0 && blueIdx > 0) {
        return true;
    }
    return false;
}

// Extract the folder from a given path.
std::string MetadataHelper::extractFolder(const std::string& filename) {
    size_t pos = filename.find_last_of("/\\");
    if (pos == std::string::npos) {
        return "";
    }

    return filename.substr(0, pos) + "/";
}

std::string MetadataHelper::buildFullPath(const std::string& fileName)
{
    return m_DirName + "/" + fileName;
}


