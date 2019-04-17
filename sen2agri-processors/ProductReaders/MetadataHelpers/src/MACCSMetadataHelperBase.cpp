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
 
#include "MACCSMetadataHelperBase.h"
#include "ViewingAngles.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

template <typename PixelType, typename MasksPixelType>
MACCSMetadataHelperBase<PixelType, MasksPixelType>::MACCSMetadataHelperBase()
{
    this->m_ReflQuantifVal = 1;
}

template <typename PixelType, typename MasksPixelType>
bool MACCSMetadataHelperBase<PixelType, MasksPixelType>::DoLoadMetadata(const std::string &file)
{
    if (LoadAndUpdateMetadataValues(file))
    {
        this->m_Mission = this->m_metadata->Header.FixedHeader.Mission;
        // Transform names like SENTINEL-2A to SENTINEL2A
        this->m_Mission.erase(std::remove(this->m_Mission.begin(), this->m_Mission.end(), '-'), this->m_Mission.end());
        this->m_ReflQuantifVal = std::stod(this->m_metadata->ProductInformation.ReflectanceQuantificationValue);
        if(this->m_ReflQuantifVal < 1) {
            this->m_ReflQuantifVal = (1 / this->m_ReflQuantifVal);
        }

        // set the acquisition date
        this->m_AcquisitionDate = this->m_metadata->InstanceId.AcquisitionDate;
        this->m_AcquisitionDateTime = this->m_metadata->ProductInformation.AcquisitionDateTime;

        // Remove the eventual UTC= from the prefix of the date
        std::string &acqDateTime = this->m_AcquisitionDateTime;
        std::string::size_type pos = acqDateTime.find('=');
        acqDateTime = (pos!= std::string::npos) ? acqDateTime.substr(pos+1) : acqDateTime;
        pos = acqDateTime.find_last_of('.');
        acqDateTime = (pos!= std::string::npos) ? acqDateTime.substr(0, pos) : acqDateTime;
        // normally, the product date time respects already the standard ISO 8601
        // we need just to replace - and : from it
        acqDateTime.erase(std::remove(acqDateTime.begin(), acqDateTime.end(), '-'), acqDateTime.end());
        acqDateTime.erase(std::remove(acqDateTime.begin(), acqDateTime.end(), ':'), acqDateTime.end());

        this->m_strNoDataValue = this->m_metadata->ImageInformation.NoDataValue;

        //TODO: Add initialization for mean angles (solar and sensor)

        return true;
    }

    return false;
}

// Return the path to a file for which the name end in the ending
template <typename PixelType, typename MasksPixelType>
std::string MACCSMetadataHelperBase<PixelType, MasksPixelType>::getMACCSImageFileName(const std::vector<CommonFileInformation>& imageFiles,
                                                       const std::string& ending) {
    std::string retStr;
    for (const CommonFileInformation& fileInfo : imageFiles) {
        if(getMACCSImageFileName(fileInfo, ending, retStr))
            return retStr;
    }
    return "";
}

// Return the path to a file for which the name end in the ending
template <typename PixelType, typename MasksPixelType>
std::string MACCSMetadataHelperBase<PixelType, MasksPixelType>::getMACCSImageFileName(const std::vector<CommonAnnexInformation>& maskFiles,
                                                       const std::string& ending) {
    std::string retStr;
    for (const CommonAnnexInformation& fileInfo : maskFiles) {
        if(getMACCSImageFileName(fileInfo.File, ending, retStr))
            return retStr;
    }
    return "";
}

template <typename PixelType, typename MasksPixelType>
bool MACCSMetadataHelperBase<PixelType, MasksPixelType>::getMACCSImageFileName(const CommonFileInformation& fileInfo,
                                                       const std::string& ending, std::string& retStr) {
    if (fileInfo.LogicalName.length() >= ending.length() &&
            0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
        boost::filesystem::path rootFolder(this->m_DirName);
        // Get the extension of file (default for MACCS is ".DBL.TIF" and for MAJA is ".TIF")
        const std::string &ext = this->GetMaccsImageExtension();
        retStr = (rootFolder / (fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ext)).string();
        if(!CheckFileExistence(retStr)) {
            itkWarningMacro("Cannot find the file (even with lowercase extension): " << retStr);
        }
        return true;
    }
    return false;
}

// Return the path to a file for which the name end in the ending
template <typename PixelType, typename MasksPixelType>
std::string MACCSMetadataHelperBase<PixelType, MasksPixelType>::getMACCSImageHdrName(const std::vector<CommonAnnexInformation>& maskFiles,
                                                      const std::string& ending) {
    std::string retStr;
    for (const CommonAnnexInformation& fileInfo : maskFiles) {
        if(getMACCSImageHdrName(fileInfo.File, ending, retStr))
            return retStr;
    }
    return "";
}

// Return the path to a file for which the name end in the ending
template <typename PixelType, typename MasksPixelType>
std::string MACCSMetadataHelperBase<PixelType, MasksPixelType>::getMACCSImageHdrName(const std::vector<CommonFileInformation>& imageFiles,
                                                      const std::string& ending) {
    std::string retStr;
    for (const CommonFileInformation& fileInfo : imageFiles) {
        if(getMACCSImageHdrName(fileInfo, ending, retStr))
            return retStr;
    }
    return "";
}

template <typename PixelType, typename MasksPixelType>
bool MACCSMetadataHelperBase<PixelType, MasksPixelType>::getMACCSImageHdrName(const CommonFileInformation& fileInfo,
                                                      const std::string& ending, std::string &retStr) {
    if (fileInfo.LogicalName.length() >= ending.length() &&
            0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
        retStr = this->buildFullPath(fileInfo.FileLocation);
        if(!CheckFileExistence(retStr)) {
            itkWarningMacro("Cannot find the file (even with lowercase extension): " << retStr);
        }
        return true;
    }
    return false;
}

/**
 * @brief MACCSMetadataHelperBase::CheckFileExistence
 * Checks if the file exists and if not, it tries to change the extension to small capitals letters.
 * If the file with the changed extension exists, it is returned instead otherwise it is not modified
 * @param fileName - in/out parameter
 * @return
 */
template <typename PixelType, typename MasksPixelType>
bool MACCSMetadataHelperBase<PixelType, MasksPixelType>::CheckFileExistence(std::string &fileName) {
    bool ret = true;
    boost::system::error_code ec;
    if (!boost::filesystem::exists(fileName, ec)) {
        size_t lastindex = fileName.find_last_of(".");
        if((lastindex != std::string::npos) && (lastindex != (fileName.length()-1))) {
            std::string rawname = fileName.substr(0, lastindex);
            std::string ext = fileName.substr(lastindex+1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            std::string recomputedName = rawname + "." + ext;
            if (boost::filesystem::exists(recomputedName, ec)) {
                fileName = recomputedName;
            } else {
                ret = false;
            }

        }
    }
    return ret;
}


// Get the id of the band. Return -1 if band not found.
template <typename PixelType, typename MasksPixelType>
int MACCSMetadataHelperBase<PixelType, MasksPixelType>::getBandIndex(const std::vector<CommonBand>& bands, const std::string& name) {
    for (const CommonBand& band : bands) {
        if (band.Name == name) {
            return std::stoi(band.Id);
        }
    }
    return -1;
}

template <typename PixelType, typename MasksPixelType>
const CommonResolution& MACCSMetadataHelperBase<PixelType, MasksPixelType>::GetMACCSResolutionInfo(int nResolution) {
    for(const CommonResolution& maccsRes: m_metadata->ImageInformation.Resolutions) {
        if(std::atoi(maccsRes.Id.c_str()) == nResolution) {
            return maccsRes;
        }
    }
    itkExceptionMacro("No resolution structure was found in the main xml for S2 resolution : " << nResolution);
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::Pointer
         MACCSMetadataHelperBase<PixelType, MasksPixelType>::GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult, int resolution) {
    // Saturation is the first band from the Quality Image
    // Validity is the 3rd band from the Quality Image
    // Water is the bit 3 in the MSK image
    // Snow is the bit 5 in the MSK image
    // Cloud should be 0 in the CLD image

    std::vector< typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::Pointer> vecImgs;
    // extract the cld, div and saturation mask image bands
    if((nMaskFlags & MSK_CLOUD) != 0) {
        vecImgs.push_back(this->m_maskFlagsBandsExtractor.ExtractResampledBand(getCloudFileName(resolution), 1, Interpolator_NNeighbor));
    }

    if((nMaskFlags & MSK_SAT) != 0) {
        vecImgs.push_back(this->m_maskFlagsBandsExtractor.ExtractResampledBand(getQualityFileName(resolution), 1, Interpolator_NNeighbor));
    }
    if((nMaskFlags & MSK_VALID) != 0) {
        vecImgs.push_back(this->m_maskFlagsBandsExtractor.ExtractResampledBand(getQualityFileName(resolution), 3, Interpolator_NNeighbor));
    }

    if((nMaskFlags & MSK_SNOW) != 0 || (nMaskFlags & MSK_WATER) != 0 ) {
        vecImgs.push_back(this->m_maskFlagsBandsExtractor.ExtractResampledBand(getWaterFileName(resolution), 1, Interpolator_NNeighbor));
    }

    this->m_maccsMaskHandlerFunctor.Initialize(nMaskFlags, binarizeResult);
    this->m_maccsMaskHandlerFilter = NaryFunctorImageFilterType::New();
    this->m_maccsMaskHandlerFilter->SetFunctor(this->m_maccsMaskHandlerFunctor);
    for(size_t i = 0; i<vecImgs.size(); i++) {
        this->m_maccsMaskHandlerFilter->SetInput(i, vecImgs[i]);
    }
    return this->m_maccsMaskHandlerFilter->GetOutput();
}
