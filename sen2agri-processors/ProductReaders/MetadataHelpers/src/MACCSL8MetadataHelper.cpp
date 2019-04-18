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
 
#include "MACCSL8MetadataHelper.h"
#include "ViewingAngles.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

template <typename PixelType, typename MasksPixelType>
MACCSL8MetadataHelper<PixelType, MasksPixelType>::MACCSL8MetadataHelper()
{
    this->m_ReflQuantifVal = 1;
}

template <typename PixelType, typename MasksPixelType>
bool MACCSL8MetadataHelper<PixelType, MasksPixelType>::LoadAndUpdateMetadataValues(const std::string &file)
{
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    // just check if the file is Spot4 metadata file. In this case
    // the helper will return the hardcoded values from the constructor as these are not
    // present in the metadata
    if (this->m_metadata = maccsMetadataReader->ReadMetadata(file)) {
        if (this->m_metadata->Header.FixedHeader.Mission.find(LANDSAT_MISSION_STR) == std::string::npos) {
            return false;
        }

        //std::cout << "Using mission L8" << std::endl;

        this->m_MissionShortName = "LANDSAT";
        this->m_nTotalBandsNo = this->m_metadata->ImageInformation.Bands.size();
        // Add the resolution of 30 m
        this->m_vectResolutions.push_back(30);
        // we have the same values for relative and absolute bands indexes as we have only one raster
        this->m_nBlueBandName = "B2";
        this->m_nGreenBandName = "B3";
        this->m_nRedBandName = "B4";
        this->m_nNirBandName = "B5";
        this->m_nSwirBandName = "B6";

        this->m_bHasGlobalMeanAngles = true;
        this->m_bHasBandMeanAngles = false;

        m_aotFileName = this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_ATB");

        // update the solar mean angle
        this->m_solarMeanAngles.azimuth = this->m_metadata->ProductInformation.MeanSunAngle.AzimuthValue;
        this->m_solarMeanAngles.zenith = this->m_metadata->ProductInformation.MeanSunAngle.ZenithValue;

        // update the solar mean angle
        MeanAngles_Type sensorAngle = {0,0};
        // TODO: Here we should get this from the Mean_Viewing_Angle. Most probably if we get here it will crash
        // if the MACCS Metadata Reader will not be updated to read this element for LANDSAT8
        if(this->m_metadata->ProductInformation.MeanViewingIncidenceAngles.size() > 0) {
            sensorAngle.azimuth = this->m_metadata->ProductInformation.MeanViewingIncidenceAngles.at(0).Angles.AzimuthValue;
            sensorAngle.zenith = this->m_metadata->ProductInformation.MeanViewingIncidenceAngles.at(0).Angles.ZenithValue;
        }
        this->m_sensorBandsMeanAngles.push_back(sensorAngle);

        // TODO: Add here other initializations for LANDSAT if needed

        return true;
    }
    return false;
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetImage(const std::vector<std::string> &bandNames, int outRes)
{
    return GetImage(bandNames, NULL, outRes);
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetImage(const std::vector<std::string> &bandNames,
                                                                                 std::vector<int> *pRetRelBandIdxs, int outRes)
{
    int curRes = this->m_vectResolutions.at(0);
    if (outRes == -1) {
        outRes = curRes;
    }

    typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(
                this->getMACCSImageFileName(this->m_metadata->ProductOrganization.ImageFiles, "_FRE"));
    // if we have uniques band IDs and they are the same as the total number of bands, we just return the raster
    if (bandNames.size() == (size_t)this->m_nTotalBandsNo || pRetRelBandIdxs != NULL) {
        // update the ret indexes
        if (pRetRelBandIdxs) {
            for (const std::string &bandName : bandNames) {
                pRetRelBandIdxs->push_back(this->GetRelativeBandIdx(bandName));
            }
        }
        if (outRes == curRes) {
            return reader->GetOutput();
        }
        float fMultiplicationFactor = ((float)curRes)/outRes;
        return this->m_ImageResampler.getResampler(reader->GetOutput(), fMultiplicationFactor)->GetOutput();
    }

    // extract the band indexes
    std::vector<int> bandIdxs;
    for (const std::string &bandName: bandNames) {
        bandIdxs.push_back(this->GetRelativeBandIdx(bandName));
    }
    typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageListType::Pointer imageList = this->CreateImageList();
    this->m_bandsExtractor.ExtractImageBands(reader->GetOutput(), imageList, bandIdxs,
                                                Interpolator_NNeighbor, curRes, outRes);
    typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ListConcatenerFilterType::Pointer concat = this->CreateConcatenner();
    concat->SetInput(imageList);

    return concat->GetOutput();
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetImageList(const std::vector<std::string> &bandNames,
                                                                           typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer outImgList, int outRes)
{
    int curRes = this->m_vectResolutions.at(0);
    typename MACCSMetadataHelperBase<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(
                this->getMACCSImageFileName(this->m_metadata->ProductOrganization.ImageFiles, "_FRE"));
    // extract the band indexes
    std::vector<int> bandIdxs;
    for (const std::string &bandName: bandNames) {
        bandIdxs.push_back(this->GetRelativeBandIdx(bandName));
    }
    typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer imageList = outImgList;
    if (!imageList) {
        imageList = this->CreateImageList();
    }
    this->m_bandsExtractor.ExtractImageBands(reader->GetOutput(), imageList, bandIdxs, Interpolator_NNeighbor, curRes, outRes);
    return imageList;
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetBandNamesForResolution(int)
{
    return GetAllBandNames();
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetPhysicalBandNames()
{
    std::vector<std::string> retVect;
    for(const CommonBand& maccsBand: this->m_metadata->ImageInformation.Bands) {
        retVect.push_back(maccsBand.Name);
    }
    return retVect;
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetAllBandNames()
{
    return GetPhysicalBandNames();
}

template <typename PixelType, typename MasksPixelType>
int MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetResolutionForBand(const std::string &bandName)
{
    for(const CommonBandResolution& maccsBandResolution: this->m_metadata->ProductInformation.BandResolutions) {
        if (maccsBandResolution.BandName == bandName) {
            return std::stoi(maccsBandResolution.Resolution);
        }
    }
    return -1;
}

template <typename PixelType, typename MasksPixelType>
float MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetAotQuantificationValue(int)
{
    if(!this->m_specificAotMetadata) {
        ReadSpecificMACCSAotHdrFile();
    }
    if(this->m_fAotQuantificationValue < 1) {
        this->m_fAotQuantificationValue = (1/this->m_fAotQuantificationValue);
    }
    return this->m_fAotQuantificationValue;
}

template <typename PixelType, typename MasksPixelType>
float MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetAotNoDataValue(int)
{
    if(!this->m_specificAotMetadata) {
        ReadSpecificMACCSAotHdrFile();
    }
    return this->m_fAotNoDataVal;
}

template <typename PixelType, typename MasksPixelType>
int MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetAotBandIndex(int)
{
    if(!this->m_specificAotMetadata) {
        ReadSpecificMACCSAotHdrFile();
    }
    return this->m_nAotBandIndex;
}

//template <typename PixelType, typename MasksPixelType>
//void MACCSL8MetadataHelper<PixelType, MasksPixelType>::ReadSpecificMACCSImgHdrFile()
//{
//    const std::string &fileName = this->getMACCSImageHdrName(this->m_metadata->ProductOrganization.ImageFiles, "_FRE");

//    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
//    this->m_specificImgMetadata = maccsMetadataReader->ReadMetadata(fileName);
//    if(this->m_nBandsNoForCurRes > this->m_specificImgMetadata->ImageInformation.Bands.size()) {
//        itkExceptionMacro("Invalid number of bands found in specific img xml: " <<
//                          this->m_specificImgMetadata->ImageInformation.Bands.size() <<
//                          ". Expected is " << this->m_nBandsNoForCurRes);
//    }
//}

//void MACCSL8MetadataHelper::ReadSpecificMACCSCldHdrFile()
//{
//    const std::string &fileName = this->getMACCSImageHdrName(this->m_metadata->ProductOrganization.AnnexFiles, "_CLD");

//    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
//    this->m_specificCldMetadata = maccsMetadataReader->ReadMetadata(fileName);
//}

template <typename PixelType, typename MasksPixelType>
void MACCSL8MetadataHelper<PixelType, MasksPixelType>::ReadSpecificMACCSAotHdrFile()
{
    const std::string &fileName = this->getMACCSImageHdrName(this->m_metadata->ProductOrganization.AnnexFiles, "_ATB");

    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    if (this->m_specificAotMetadata = maccsMetadataReader->ReadMetadata(fileName)) {
        // add the information to the list
        this->m_fAotQuantificationValue = atof(this->m_specificAotMetadata->ImageInformation.AOTQuantificationValue.c_str());
        this->m_fAotNoDataVal = atof(this->m_specificAotMetadata->ImageInformation.AOTNoDataValue.c_str());
        this->m_nAotBandIndex = this->getBandIndex(this->m_specificAotMetadata->ImageInformation.Bands, "AOT");
    }
}

template <typename PixelType, typename MasksPixelType>
void MACCSL8MetadataHelper<PixelType, MasksPixelType>::ReadSpecificMACCSMskHdrFile()
{
    const std::string &fileName = this->getMACCSImageHdrName(this->m_metadata->ProductOrganization.AnnexFiles, "_MSK");
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    this->m_specificMskMetadata = maccsMetadataReader->ReadMetadata(fileName);
}

template <typename PixelType, typename MasksPixelType>
std::string MACCSL8MetadataHelper<PixelType, MasksPixelType>::getCloudFileName(int)
{
    return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_CLD");
}

template <typename PixelType, typename MasksPixelType>
std::string MACCSL8MetadataHelper<PixelType, MasksPixelType>::getWaterFileName(int)
{
    return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_MSK");
}

template <typename PixelType, typename MasksPixelType>
std::string MACCSL8MetadataHelper<PixelType, MasksPixelType>::getSnowFileName(int res)
{
    // the water and snow masks are in the same file
    return getWaterFileName(res);
}

template <typename PixelType, typename MasksPixelType>
std::string MACCSL8MetadataHelper<PixelType, MasksPixelType>::getQualityFileName(int)
{
    return this->getMACCSImageFileName(this->m_metadata->ProductOrganization.AnnexFiles, "_QLT");
}

template <typename PixelType, typename MasksPixelType>
std::vector<CommonBand> MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetAllMACCSBandsInfos() {
    return this->m_metadata->ImageInformation.Bands;
}

template <typename PixelType, typename MasksPixelType>
bool MACCSL8MetadataHelper<PixelType, MasksPixelType>::BandAvailableForCurrentResolution(unsigned int nBand) {
    // Landsat
    if(nBand < this->m_metadata->ProductInformation.BandWavelengths.size())
        return true;
    return false;
}

// Get the id of the band. Return -1 if band not found.
template <typename PixelType, typename MasksPixelType>
int MACCSL8MetadataHelper<PixelType, MasksPixelType>::GetRelativeBandIdx(const std::string &bandName) {
    int i = 0;
    for(const CommonBandWavelength &bandInfo: this->m_metadata->ProductInformation.BandWavelengths) {
        if (bandInfo.BandName == bandName) {
            return i;
        }
        i++;
    }
    return -1;
}

/*
void MACCSL8MetadataHelperDummy() {
    MACCSL8MetadataHelper<short, short> test1;
    MACCSL8MetadataHelper<short, uint8_t> test11;
    MACCSL8MetadataHelper<float, short> test2;
    MACCSL8MetadataHelper<float, uint8_t> test21;
    MACCSL8MetadataHelper<int, short> test3;
    MACCSL8MetadataHelper<int, uint8_t> test31;
}
*/
