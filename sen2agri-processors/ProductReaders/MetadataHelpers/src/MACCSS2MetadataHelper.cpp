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
 
#include "MACCSS2MetadataHelper.h"
#include "ViewingAngles.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

template <typename PixelType, typename MasksPixelType>
bool MACCSS2MetadataHelper<PixelType, MasksPixelType>::is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

template <typename PixelType, typename MasksPixelType>
MACCSS2MetadataHelper<PixelType, MasksPixelType>::MACCSS2MetadataHelper()
{
    this->m_ReflQuantifVal = 1;
}

template <typename PixelType, typename MasksPixelType>
bool MACCSS2MetadataHelper<PixelType, MasksPixelType>::LoadAndUpdateMetadataValues(const std::string &file)
{
    if (!this->LoadAndCheckMetadata(file)) {
        return false;
    }

    // std::cout << "Using mission S2" << std::endl;

    this->m_MissionShortName = "SENTINEL";

    // Add the resolution of 10 and 20m but do not add 60m resolution as it is not supported
    this->m_vectResolutions.push_back(10);
    this->m_vectResolutions.push_back(20);

    this->m_nTotalBandsNo = 10;

    this->m_nBlueBandName = "B2";
    this->m_nRedBandName = "B4";
    this->m_nGreenBandName = "B3";
    this->m_nNirBandName = "B8";
    this->m_nNarrowNirBandName = "B8A";
    this->m_nSwirBandName = "B11";

    this->m_redEdgeBandNames = {"B5", "B6", "B7"};

    this->InitializeS2Angles();

    // TODO: Add here other initializations for S2 if needed
    return true;
}

template <typename PixelType, typename MasksPixelType>
bool MACCSS2MetadataHelper<PixelType, MasksPixelType>::LoadAndCheckMetadata(const std::string &file)
{
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    // just check if the file is MACCS metadata file. In this case
    // the helper will return the hardcoded values from the constructor as these are not
    // present in the metadata
    if (this->m_metadata = maccsMetadataReader->ReadMetadata(file)) {
        if (this->m_metadata->Header.FixedHeader.Mission.find(SENTINEL_MISSION_STR) != std::string::npos &&
                this->m_metadata->Header.FixedHeader.SourceSystem == "MACCS") {
            return true;
        }
    }
    return false;
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetImage(const std::vector<std::string> &bandNames, int outRes)
{
    return GetImage(bandNames, NULL, outRes);
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetImage(const std::vector<std::string> &bandNames,
                                                                                 std::vector<int> *pRetRelBandIdxs, int outRes)
{
    int minRes;
    std::map<int, std::vector<std::string>> mapResBandNames;
    std::map<int, std::vector<int>> mapRes = GroupBandsByResolution(bandNames, minRes, mapResBandNames);
    if (mapRes.size() == 0 || mapRes.size() > 2) {
        return NULL;
    }
    if (outRes == -1) {
        outRes = minRes;
    }
    if (mapRes.size() == 1) {
        std::vector<int> retRelBandIdxs(bandNames.size());
        std::fill(retRelBandIdxs.begin(), retRelBandIdxs.end(), -1);

        int curRes = mapRes.begin()->first;
        int nBandsNoForCurRes = ((curRes != 20) ? 4 : 6);
        retRelBandIdxs = mapRes.begin()->second;
        bool bHasRetRelPtr = (pRetRelBandIdxs != NULL);
        if (pRetRelBandIdxs != NULL) {
            (*pRetRelBandIdxs) = retRelBandIdxs;
        }

        typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(getImageFileName(curRes));
        // if we have uniques band IDs and they are the same as the total number of bands, we just return the raster
        if (bandNames.size() == (size_t)nBandsNoForCurRes || bHasRetRelPtr) {
            // if no resampling, just return the raster
            if (outRes == -1 || outRes == curRes) {
                return reader->GetOutput();
            }
            float fMultiplicationFactor = ((float)curRes)/outRes;
            return this->m_ImageResampler.getResampler(reader->GetOutput(), fMultiplicationFactor)->GetOutput();
        }

        typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageListType::Pointer imageList = this->CreateImageList();
        this->m_bandsExtractor.ExtractImageBands(reader->GetOutput(), imageList,
                                           retRelBandIdxs, Interpolator_BCO);

        imageList->UpdateOutputInformation();
        typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ListConcatenerFilterType::Pointer concat = this->CreateConcatenner();
        concat->SetInput(imageList);
        concat->UpdateOutputInformation();

        float fMultiplicationFactor = ((float)curRes)/outRes;
        if (fMultiplicationFactor > 1) {
            return this->m_ImageResampler.getResampler(concat->GetOutput(), fMultiplicationFactor)->GetOutput();
        }

        return concat->GetOutput();
    } else {
        // we have bands from both rasters
        typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageListType::Pointer imageList = this->CreateImageList();
        std::map<int, std::vector<int>>::iterator  containerIt = mapRes.begin();
        std::map<int, std::vector<std::string>>::iterator  containerIt2 = mapResBandNames.begin();
        std::map<std::string, int> mapBandNamesToListIdx;
        for (int curRes: this->m_vectResolutions) {
            typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(getImageFileName(curRes));
            const std::vector<int> &relBandsIdxs = containerIt->second;
            const std::vector<std::string> &resBandsNames = containerIt2->second;
            const std::vector<int> &addedRelBandIdx = this->m_bandsExtractor.ExtractImageBands(reader->GetOutput(), imageList,
                                                                                  relBandsIdxs, Interpolator_BCO, curRes, outRes);
            for (int i = 0; i<resBandsNames.size(); i++) {
                mapBandNamesToListIdx[resBandsNames[i]] = addedRelBandIdx[i];
            }
            containerIt++;
            containerIt2++;
        }

        imageList->UpdateOutputInformation();

        // create the image list containing the exact order of bands as the ones received
        typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageListType::Pointer imageListFinal = this->CreateImageList();
        typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ListConcatenerFilterType::Pointer concat = this->CreateConcatenner();
        int i = 0;
        for (const std::string &bandName: bandNames) {
            // get the band in the resolutions map
            for (int curRes: this->m_vectResolutions) {
                const std::vector<std::string> &resBandsNames = mapResBandNames[curRes];
                if (std::find(resBandsNames.begin(), resBandsNames.end(), bandName) != resBandsNames.end()) {
                    // we found the band in this resolution then we get the index
                    imageListFinal->PushBack(imageList->GetNthElement(mapBandNamesToListIdx[bandName]));
                    if (pRetRelBandIdxs != NULL) {
                        pRetRelBandIdxs->push_back(i);
                        i++;
                    }
                    break;
                }
            }
        }

        concat->SetInput(imageListFinal);
        return concat->GetOutput();
    }
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetImageList(const std::vector<std::string> &bandNames,
                                                                           typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer outImgList, int outRes)
{
    int minRes;
    std::map<int, std::vector<std::string>> mapResBandNames;
    std::map<int, std::vector<int>> mapRes = GroupBandsByResolution(bandNames, minRes, mapResBandNames);
    if (mapRes.size() == 0 || mapRes.size() > 2) {
        return NULL;
    }
    if (outRes == -1) {
        outRes = minRes;
    }

    typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageListType::Pointer imageList = outImgList;
    if (!imageList) {
        imageList = this->CreateImageList();
    }
    // we have bands from both rasters
    std::map<int, std::vector<int>>::iterator containerIt;
    for (int curRes: this->m_vectResolutions) {
        containerIt = mapRes.find(curRes);
        if (containerIt != mapRes.end()) {
            std::vector<int> relBandsIdxs = containerIt->second;
            if (relBandsIdxs.size() > 0) {
                typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(getImageFileName(curRes));
                this->m_bandsExtractor.ExtractImageBands(reader->GetOutput(), imageList, relBandsIdxs,
                                                   Interpolator_BCO, curRes, outRes);
            }
        }
    }
    return imageList;

}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetBandNamesForResolution(int res)
{
    std::vector<std::string> retVect;
    std::vector<std::string> allBands;
    for(const CommonBandResolution& maccsBandResolution: this->m_metadata->ProductInformation.BandResolutions) {
        int curRes = std::stoi(maccsBandResolution.Resolution);
        if (curRes == res) {
            retVect.push_back(maccsBandResolution.BandName);
        }
        if (curRes == 10 || curRes == 20) {
            allBands.push_back(maccsBandResolution.BandName);
        }
    }
    return retVect.size() > 0 ? retVect : allBands;
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetPhysicalBandNames()
{
    return GetBandNamesForResolution(-1);
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetAllBandNames()
{
    // We are hardcoding as in MAJA we do not have anymore the 60m resolution bands described anywhere
    return {"B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B8A", "B9", "B10", "B11", "B12"};
}

template <typename PixelType, typename MasksPixelType>
int MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetResolutionForBand(const std::string &bandName)
{
    for(const CommonBandResolution& maccsBandResolution: this->m_metadata->ProductInformation.BandResolutions) {
        if (maccsBandResolution.BandName == bandName) {
            return std::stoi(maccsBandResolution.Resolution);
        }
    }
    return -1;
}

template <typename PixelType, typename MasksPixelType>
std::string MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetAotImageFileName(int res)
{
    if(res != 20) {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_ATB_R1");
    } else {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_ATB_R2");
    }
}

template <typename PixelType, typename MasksPixelType>
float MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetAotQuantificationValue(int res)
{
    typename MetadataHelper<PixelType, MasksPixelType>::AotInfos *pInfos = ReadSpecificMACCSAotHdrFile(res);
    return pInfos->m_fAotQuantificationValue;
}

template <typename PixelType, typename MasksPixelType>
float MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetAotNoDataValue(int res)
{
    typename MetadataHelper<PixelType, MasksPixelType>::AotInfos *pInfos = ReadSpecificMACCSAotHdrFile(res);
    return pInfos->m_fAotNoDataVal;
}

template <typename PixelType, typename MasksPixelType>
int MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetAotBandIndex(int res)
{
    typename MetadataHelper<PixelType, MasksPixelType>::AotInfos *pInfos = ReadSpecificMACCSAotHdrFile(res);
    return pInfos->m_nAotBandIndex;
}

//template <typename PixelType, typename MasksPixelType>
//void MACCSS2MetadataHelper<PixelType, MasksPixelType>::ReadSpecificMACCSImgHdrFile(int res)
//{
//    std::string fileName;
//    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
//    int nBandsNoForCurRes = ((res != 20) ? 4 : 6);
//    if(res != 20) {
//        fileName = this->getMACCSImageHdrName(this->m_metadata->ProductOrganization.ImageFiles, "_FRE_R1");
//        this->m_specificImgMetadata10M = maccsMetadataReader->ReadMetadata(fileName);
//    } else {
//        fileName = this->getMACCSImageHdrName(this->m_metadata->ProductOrganization.ImageFiles, "_FRE_R2");
//        this->m_specificImgMetadata20M = maccsMetadataReader->ReadMetadata(fileName);
//    }
//    const std::unique_ptr<MACCSFileMetadata> &specificImgMetadata = ((res != 20) ? this->m_specificImgMetadata10M : this->m_specificImgMetadata20M);

//    if((size_t)nBandsNoForCurRes > specificImgMetadata->ImageInformation.Bands.size()) {
//        itkExceptionMacro("Invalid number of bands found in specific img xml: " <<
//                          specificImgMetadata->ImageInformation.Bands.size() <<
//                          ". Expected is " << this->m_nBandsNoForCurRes);
//    }
//}

//void MACCSS2MetadataHelper::ReadSpecificMACCSCldHdrFile(int res)
//{
//    std::string fileName;
//    if(res != 20) {
//        fileName = getMACCSImageHdrName(this->m_metadata->ProductOrganization.AnnexFiles, "_CLD_R1");
//    } else {
//        fileName = getMACCSImageHdrName(this->m_metadata->ProductOrganization.AnnexFiles, "_CLD_R2");
//    }

//    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
//    this->m_specificCldMetadata = maccsMetadataReader->ReadMetadata(fileName);
//}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::AotInfos *MACCSS2MetadataHelper<PixelType, MasksPixelType>::ReadSpecificMACCSAotHdrFile(int res)
{
    std::string fileName;
    typename MetadataHelper<PixelType, MasksPixelType>::AotInfos *pAotInfos;
    if(res != 20) {
        fileName = this->getMACCSImageHdrName(this->m_metadata->ProductOrganization.AnnexFiles, "_ATB_R1");
        pAotInfos = &aotInfos10M;
    } else {
        fileName = this->getMACCSImageHdrName(this->m_metadata->ProductOrganization.AnnexFiles, "_ATB_R2");
        pAotInfos = &aotInfos20M;
    }
    if (!pAotInfos->isInitialized) {
        MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
        if (std::unique_ptr<MACCSFileMetadata> specificAotMetadata = maccsMetadataReader->ReadMetadata(fileName)) {
            // add the information to the list
            pAotInfos->m_fAotQuantificationValue = atof(specificAotMetadata->ImageInformation.AOTQuantificationValue.c_str());
            pAotInfos->m_fAotNoDataVal = atof(specificAotMetadata->ImageInformation.AOTNoDataValue.c_str());
            pAotInfos->m_nAotBandIndex = this->getBandIndex(specificAotMetadata->ImageInformation.Bands, "AOT");

            if(pAotInfos->m_fAotQuantificationValue < 1) {
                pAotInfos->m_fAotQuantificationValue = (1/pAotInfos->m_fAotQuantificationValue);
            }
            pAotInfos->isInitialized = true;
        }
    }
    return pAotInfos;
}

/*
void MACCSS2MetadataHelper::ReadSpecificMACCSMskHdrFile(int res)
{
    std::string fileName;
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    if(res != 20) {
        fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_MSK_R1");
        m_specificMskMetadata10M = maccsMetadataReader->ReadMetadata(fileName);
    } else {
        fileName = getMACCSImageHdrName(m_metadata->ProductOrganization.AnnexFiles, "_MSK_R2");
        m_specificMskMetadata20M = maccsMetadataReader->ReadMetadata(fileName);
    }
}
*/

template <typename PixelType, typename MasksPixelType>
std::string MACCSS2MetadataHelper<PixelType, MasksPixelType>::getImageFileName(int res) {
    if(res != 20) {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.ImageFiles, "_FRE_R1");
    } else {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.ImageFiles, "_FRE_R2");
    }
}

template <typename PixelType, typename MasksPixelType>
std::string MACCSS2MetadataHelper<PixelType, MasksPixelType>::getCloudFileName(int res)
{
    if(res != 20) {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_CLD_R1");
    } else {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_CLD_R2");
    }
}

template <typename PixelType, typename MasksPixelType>
std::string MACCSS2MetadataHelper<PixelType, MasksPixelType>::getWaterFileName(int res)
{
    if(res != 20) {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_MSK_R1");
    } else {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_MSK_R2");
    }
}

template <typename PixelType, typename MasksPixelType>
std::string MACCSS2MetadataHelper<PixelType, MasksPixelType>::getSnowFileName(int res)
{
    // the water and snow masks are in the same file
    return getWaterFileName(res);
}

template <typename PixelType, typename MasksPixelType>
std::string MACCSS2MetadataHelper<PixelType, MasksPixelType>::getQualityFileName(int res)
{
    if(res != 20) {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_QLT_R1");
    } else {
        return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_QLT_R2");
    }
}

template <typename PixelType, typename MasksPixelType>
std::vector<MetadataHelperViewingAnglesGrid> MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetDetailedViewingAngles(int res)
{
    for(unsigned int i = 0; i<this->m_maccsBandViewingAngles.size(); i++) {
        MACCSBandViewingAnglesGrid maccsGrid = this->m_maccsBandViewingAngles[i];
        //Return only the angles of the bands for the current resolution
        if(this->BandAvailableForResolution(maccsGrid.BandId, res)) {
            MetadataHelperViewingAnglesGrid mhGrid;
            mhGrid.BandId = maccsGrid.BandId;
            //mhGrid.DetectorId = maccsGrid.DetectorId;
            mhGrid.Angles.Azimuth.ColumnStep = maccsGrid.Angles.Azimuth.ColumnStep;
            mhGrid.Angles.Azimuth.ColumnUnit = maccsGrid.Angles.Azimuth.ColumnUnit;
            mhGrid.Angles.Azimuth.RowStep = maccsGrid.Angles.Azimuth.RowStep;
            mhGrid.Angles.Azimuth.RowUnit = maccsGrid.Angles.Azimuth.RowUnit;
            mhGrid.Angles.Azimuth.Values = maccsGrid.Angles.Azimuth.Values;

            mhGrid.Angles.Zenith.ColumnStep = maccsGrid.Angles.Zenith.ColumnStep;
            mhGrid.Angles.Zenith.ColumnUnit = maccsGrid.Angles.Zenith.ColumnUnit;
            mhGrid.Angles.Zenith.RowStep = maccsGrid.Angles.Zenith.RowStep;
            mhGrid.Angles.Zenith.RowUnit = maccsGrid.Angles.Zenith.RowUnit;
            mhGrid.Angles.Zenith.Values = maccsGrid.Angles.Zenith.Values;

            this->m_detailedViewingAngles.push_back(mhGrid);
        }
    }
    return this->m_detailedViewingAngles;
}

template <typename PixelType, typename MasksPixelType>
const CommonResolution& MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetMACCSResolutionInfo(int nResolution) {
    for(const CommonResolution& maccsRes: this->m_metadata->ImageInformation.Resolutions) {
        if(std::atoi(maccsRes.Id.c_str()) == nResolution) {
            return maccsRes;
        }
    }
    itkExceptionMacro("No resolution structure was found in the main xml for S2 resolution : " << nResolution);
}

template <typename PixelType, typename MasksPixelType>
std::vector<CommonBand> MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetAllMACCSBandsInfos() {
    // Sentinel 2
    std::vector<CommonBand> maccsBands;
    int bandIdx = 1;
    for(const CommonBandWavelength& maccsBandWavelength: this->m_metadata->ProductInformation.BandWavelengths) {
        CommonBand band;
        band.Id = std::to_string(bandIdx);
        band.Name = maccsBandWavelength.BandName;
        maccsBands.push_back(band);
        bandIdx++;
    }
    return maccsBands;
}

template <typename PixelType, typename MasksPixelType>
void MACCSS2MetadataHelper<PixelType, MasksPixelType>::InitializeS2Angles() {
    this->m_bHasGlobalMeanAngles = true;
    this->m_bHasBandMeanAngles = true;
    this->m_bHasDetailedAngles = true;
    this->m_detailedAnglesGridSize = 23;

    // update the solar mean angle
    this->m_solarMeanAngles.azimuth = this->m_metadata->ProductInformation.MeanSunAngle.AzimuthValue;
    this->m_solarMeanAngles.zenith = this->m_metadata->ProductInformation.MeanSunAngle.ZenithValue;

    // first compute the total number of bands to add into this->m_sensorBandsMeanAngles
    unsigned int nMaxBandId = 0;
    std::vector<CommonMeanViewingIncidenceAngle> angles = this->m_metadata->ProductInformation.MeanViewingIncidenceAngles;
    bool bHasBandIds = true;
    for(unsigned int i = 0; i<angles.size(); i++) {
        if (!is_number(angles[i].BandId)) {
            bHasBandIds = false;
            break;
        }
        unsigned int anglesBandId = std::atoi(angles[i].BandId.c_str());
        if(nMaxBandId < anglesBandId) {
            nMaxBandId = anglesBandId;
        }
    }
    if(bHasBandIds && nMaxBandId+1 != angles.size()) {
        std::cout << "ATTENTION: Number of mean viewing angles different than the maximum band + 1 " << std::endl;
    }
    // compute the array size
    unsigned int nArrSize = (nMaxBandId > angles.size() ? nMaxBandId+1 : angles.size());
    // update the viewing mean angle
    this->m_sensorBandsMeanAngles.resize(nArrSize);
    for(unsigned int i = 0; i<angles.size(); i++) {
        this->m_sensorBandsMeanAngles[i].azimuth = angles[i].Angles.AzimuthValue;
        this->m_sensorBandsMeanAngles[i].zenith = angles[i].Angles.ZenithValue;
    }

    for (const auto &grid : this->m_metadata->ProductInformation.ViewingAngles) {
        MetadataHelperViewingAnglesGrid mhGrid;
        mhGrid.DetectorId = grid.DetectorId;
        mhGrid.BandId = grid.BandId;

        mhGrid.Angles.Azimuth.ColumnStep = grid.Angles.Azimuth.ColumnStep;
        mhGrid.Angles.Azimuth.ColumnUnit = grid.Angles.Azimuth.ColumnUnit;
        mhGrid.Angles.Azimuth.RowStep = grid.Angles.Azimuth.RowStep;
        mhGrid.Angles.Azimuth.RowUnit = grid.Angles.Azimuth.RowUnit;
        mhGrid.Angles.Azimuth.Values = grid.Angles.Azimuth.Values;

        mhGrid.Angles.Zenith.ColumnStep = grid.Angles.Zenith.ColumnStep;
        mhGrid.Angles.Zenith.ColumnUnit = grid.Angles.Zenith.ColumnUnit;
        mhGrid.Angles.Zenith.RowStep = grid.Angles.Zenith.RowStep;
        mhGrid.Angles.Zenith.RowUnit = grid.Angles.Zenith.RowUnit;
        mhGrid.Angles.Zenith.Values = grid.Angles.Zenith.Values;

        this->m_allDetectorsDetailedViewingAngles.push_back(mhGrid);
    }
    // extract the detailed viewing and solar angles
    this->m_maccsBandViewingAngles = ComputeViewingAngles(this->m_metadata->ProductInformation.ViewingAngles);

    this->m_detailedSolarAngles.Azimuth.ColumnStep = this->m_metadata->ProductInformation.SolarAngles.Azimuth.ColumnStep;
    this->m_detailedSolarAngles.Azimuth.ColumnUnit = this->m_metadata->ProductInformation.SolarAngles.Azimuth.ColumnUnit;
    this->m_detailedSolarAngles.Azimuth.RowStep = this->m_metadata->ProductInformation.SolarAngles.Azimuth.RowStep;
    this->m_detailedSolarAngles.Azimuth.RowUnit = this->m_metadata->ProductInformation.SolarAngles.Azimuth.RowUnit;
    this->m_detailedSolarAngles.Azimuth.Values = this->m_metadata->ProductInformation.SolarAngles.Azimuth.Values;

    this->m_detailedSolarAngles.Zenith.ColumnStep = this->m_metadata->ProductInformation.SolarAngles.Zenith.ColumnStep;
    this->m_detailedSolarAngles.Zenith.ColumnUnit = this->m_metadata->ProductInformation.SolarAngles.Zenith.ColumnUnit;
    this->m_detailedSolarAngles.Zenith.RowStep = this->m_metadata->ProductInformation.SolarAngles.Zenith.RowStep;
    this->m_detailedSolarAngles.Zenith.RowUnit = this->m_metadata->ProductInformation.SolarAngles.Zenith.RowUnit;
    this->m_detailedSolarAngles.Zenith.Values = this->m_metadata->ProductInformation.SolarAngles.Zenith.Values;
}

template <typename PixelType, typename MasksPixelType>
bool MACCSS2MetadataHelper<PixelType, MasksPixelType>::BandAvailableForResolution(const std::string &bandId, int nRes) {
    // Sentinel 2
    int nBand = std::atoi(bandId.c_str());
    if(nBand < this->m_metadata->ProductInformation.BandResolutions.size()) {
        const CommonBandResolution& maccsBandResolution = this->m_metadata->ProductInformation.BandResolutions[nBand];
        int nBandRes = std::atoi(maccsBandResolution.Resolution.c_str());
        if(nBandRes == nRes) {
            return true;
        }
    }
    return false;
}

template <typename PixelType, typename MasksPixelType>
std::map<int, std::vector<int>> MACCSS2MetadataHelper<PixelType, MasksPixelType>::GroupBandsByResolution(const std::vector<std::string> &bandNames, int &minRes,
                                                                  std::map<int, std::vector<std::string>> &mapBandNames)
{
    std::map<int, std::vector<int>> mapRes;
    std::map<int, std::vector<int>>::iterator containerIt;
    std::map<int, std::vector<std::string>>::iterator containerIt2;

    minRes = -1;
    for (const std::string &bandName : bandNames) {
        int bandIdx = this->GetRelativeBandIdx(bandName) -1;
        int curRes = this->GetResolutionForBand(bandName);
        if (minRes == -1 || curRes < minRes) {
            minRes = curRes;
        }
        if (std::find(this->m_vectResolutions.begin(), this->m_vectResolutions.end(), curRes) == this->m_vectResolutions.end()) {
            // ignore the 60m resolution or invalid resolutions
            continue;
        }
        containerIt = mapRes.find(curRes);
        containerIt2 = mapBandNames.find(curRes);
        if(containerIt != mapRes.end()) {
            containerIt->second.push_back(bandIdx);
            containerIt2->second.push_back(bandName);
        } else {
            // add it into the container
            mapRes[curRes] = {bandIdx};
            mapBandNames[curRes] = {bandName};
        }
    }

    return mapRes;
}

// Get the id of the band. Return -1 if band not found.
template <typename PixelType, typename MasksPixelType>
int MACCSS2MetadataHelper<PixelType, MasksPixelType>::GetRelativeBandIdx(const std::string &bandName) {
    for(const CommonResolution &resInfo: this->m_metadata->ImageInformation.Resolutions) {
        int bandIdx = this->getBandIndex(resInfo.Bands, bandName);
        if (bandIdx != -1) {
            return bandIdx;
        }
    }
    return -1;
}

/*
void MACCSS2MetadataHelperDummy() {
    MACCSS2MetadataHelper<short, short> test1;
    MACCSS2MetadataHelper<short, uint8_t> test11;
    MACCSS2MetadataHelper<float, short> test2;
    MACCSS2MetadataHelper<float, uint8_t> test21;
    MACCSS2MetadataHelper<int, short> test3;
    MACCSS2MetadataHelper<int, uint8_t> test31;
}
*/
