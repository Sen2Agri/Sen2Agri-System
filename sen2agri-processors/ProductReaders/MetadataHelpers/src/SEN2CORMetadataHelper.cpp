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
 
#include "SEN2CORMetadataHelper.h"
#include "ViewingAngles.hpp"
#include "GlobalDefs.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

template <typename PixelType, typename MasksPixelType>
bool SEN2CORMetadataHelper<PixelType, MasksPixelType>::is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

template <typename PixelType, typename MasksPixelType>
SEN2CORMetadataHelper<PixelType, MasksPixelType>::SEN2CORMetadataHelper()
{
    this->m_AotQuantifVal = 1;
    this->m_AotNoDataVal = 0;
    this->m_ReflQuantifVal = 1;
    m_bGranuleMetadataUpdated = false;
}

template <typename PixelType, typename MasksPixelType>
bool SEN2CORMetadataHelper<PixelType, MasksPixelType>::DoLoadMetadata(const std::string& file)
{
    SEN2CORMetadataReaderType::Pointer sen2CorMetadataReader = SEN2CORMetadataReaderType::New();
    if (m_metadata = sen2CorMetadataReader->ReadMetadata(file)) {
        if (!boost::ifind_first(this->m_metadata->Header.FixedHeader.Mission, SENTINEL_MISSION_STR).empty()) {
            m_bGranuleMetadataUpdated = false;
            this->m_MissionShortName = "SENTINEL";

            this->m_strNoDataValue = this->m_metadata->ImageInformation.NoDataValue;

            this->m_AotQuantifVal = std::stod(this->m_metadata->ImageInformation.AOTQuantificationValue);
            this->m_AotNoDataVal = std::stod(this->m_metadata->ImageInformation.AOTNoDataValue);

            this->m_nBlueBandName = "B2";
            this->m_nRedBandName = "B4";
            this->m_nGreenBandName = "B3";
            this->m_nNirBandName = "B8";
            this->m_nNarrowNirBandName = "B8A";
            this->m_nSwirBandName = "B11";

            this->m_redEdgeBandNames = {"B5", "B6", "B7"};

            this->m_ReflQuantifVal = std::stod(this->m_metadata->ProductInformation.ReflectanceQuantificationValue);

            this->m_Mission = this->m_metadata->Header.FixedHeader.Mission;
            boost::algorithm::to_upper(this->m_Mission);
            // we remove the - to have something similar to MAJA/MACCS like SENTINEL2A, etc.
            boost::algorithm::erase_all(this->m_Mission, "-");
            // this->m_Instrument = this->m_metadata->Header.Instrument;

            // extract the acquisition date
            this->m_AcquisitionDate = this->m_metadata->InstanceId.AcquisitionDate;
            this->m_AcquisitionDateTime = this->m_metadata->ProductInformation.AcquisitionDateTime;

            std::string &acqDateTime = this->m_AcquisitionDateTime;
            std::string::size_type pos = acqDateTime.find('.');
            // remove the milliseconds part of the acquisition date/time
            acqDateTime = (pos != std::string::npos) ? acqDateTime.substr(0, pos) : acqDateTime;
            acqDateTime.erase(std::remove(acqDateTime.begin(), acqDateTime.end(), '-'), acqDateTime.end());
            acqDateTime.erase(std::remove(acqDateTime.begin(), acqDateTime.end(), ':'), acqDateTime.end());
            // the product date does has a space separator between date and time. We need to replace it with T
            std::replace( acqDateTime.begin(), acqDateTime.end(), ' ', 'T');

            this->m_vectResolutions.push_back(10);
            this->m_vectResolutions.push_back(20);
            this->m_vectResolutions.push_back(60);

            this->m_nTotalBandsNo = 10;
            this->InitializeS2Angles();

            return true;
        }
    }

    return false;
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer
SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetImage(const std::vector<std::string> &bandNames, int outRes)
{
    return GetImage(bandNames, NULL, outRes);
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer
SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetImage(const std::vector<std::string> &bandNames,
                                                         std::vector<int> *pRetRelBandIdxs, int outRes)
{
    std::vector<std::string> validBandNames;
    std::vector<int> retBandIdxs(bandNames.size());
    this->GetValidBandNames(bandNames, validBandNames, retBandIdxs, outRes);

    if (pRetRelBandIdxs != NULL) {
        (*pRetRelBandIdxs) = retBandIdxs;
    }

    // if we have only one band requested, then return only that band
    if (validBandNames.size() == 1) {
        typename MetadataHelper<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(GetImageFileName(validBandNames[0], outRes));
        typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer img = reader->GetOutput();
        img->UpdateOutputInformation();
        int curRes = img->GetSpacing()[0];
        // if no resampling, just return the raster
        if (outRes == curRes) {
            return reader->GetOutput();
        }
        float fMultiplicationFactor = ((float)curRes)/outRes;
        return this->m_ImageResampler.getResampler(reader->GetOutput(), fMultiplicationFactor)->GetOutput();
    }

    // if we have several bands
    typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer imageList = this->CreateImageList();
    for (const std::string &bandName: validBandNames) {
        typename MetadataHelper<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(GetImageFileName(bandName, outRes));
        typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer img = reader->GetOutput();
        img->UpdateOutputInformation();
        int curRes = img->GetSpacing()[0];
        // for MAJA we have only one band per reflectance raster
        this->m_bandsExtractor.ExtractImageBands(img, imageList, {0}, Interpolator_NNeighbor, curRes, outRes);
    }

    imageList->UpdateOutputInformation();

    typename MetadataHelper<PixelType, MasksPixelType>::ListConcatenerFilterType::Pointer concat = this->CreateConcatenner();
    concat->SetInput(imageList);
    return concat->GetOutput();
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetImageList(const std::vector<std::string> &bandNames,
                                                                      typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer outImgList, int outRes)
{
    std::vector<std::string> validBandNames;
    std::vector<int> retBandIdxs(bandNames.size());
    this->GetValidBandNames(bandNames, validBandNames, retBandIdxs, outRes);

    typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer imageList = outImgList;
    if (!imageList) {
        imageList = this->CreateImageList();
    }

    // if we have several bands
    for (const std::string &bandName: validBandNames) {
        typename MetadataHelper<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(GetImageFileName(bandName, outRes));
        typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer img = reader->GetOutput();
        img->UpdateOutputInformation();
        int curRes = img->GetSpacing()[0];
        // for MAJA we have only one band per reflectance raster
        this->m_bandsExtractor.ExtractImageBands(img, imageList, {0}, Interpolator_NNeighbor, curRes, outRes);
    }

    imageList->UpdateOutputInformation();
    return imageList;
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetBandNamesForResolution(int res)
{
    std::vector<std::string> retVect;
    std::vector<std::string> allBands;
    for(const CommonBandResolution& maccsBandResolution: this->m_metadata->ProductInformation.BandResolutions) {
        int curRes = std::stoi(maccsBandResolution.Resolution);
        if (curRes == res) {
            retVect.push_back(maccsBandResolution.BandName);
        }
        if (curRes == 10 || curRes == 20 || curRes == 60) {
            if (std::find(allBands.begin(), allBands.end(), maccsBandResolution.BandName) == allBands.end()) {
                allBands.push_back(maccsBandResolution.BandName);
            }
        }
    }
    return retVect.size() > 0 ? retVect : allBands;
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetAllBandNames()
{
    // We are hardcoding as in MAJA we do not have anymore the 60m resolution bands described anywhere
    return {"B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B8A", "B9", "B10", "B11", "B12"};
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetPhysicalBandNames()
{
    return GetBandNamesForResolution(-1);
}

template <typename PixelType, typename MasksPixelType>
int SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetResolutionForBand(const std::string &bandName)
{
    for(const CommonBandResolution& maccsBandResolution: this->m_metadata->ProductInformation.BandResolutions) {
        if (maccsBandResolution.BandName == bandName) {
            return std::stoi(maccsBandResolution.Resolution);
        }
    }
    return -1;
}

template <typename PixelType, typename MasksPixelType>
float SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetAotQuantificationValue(int)
{
    return this->m_AotQuantifVal;
}

template <typename PixelType, typename MasksPixelType>
float SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetAotNoDataValue(int)
{
    return this->m_AotNoDataVal;
}

template <typename PixelType, typename MasksPixelType>
int SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetAotBandIndex(int)
{
    return 1;
}

template <typename PixelType, typename MasksPixelType>
void SEN2CORMetadataHelper<PixelType, MasksPixelType>::InitializeS2Angles() {
    this->m_bHasGlobalMeanAngles = true;
    this->m_bHasBandMeanAngles = true;
    this->m_bHasDetailedAngles = true;
    this->m_detailedAnglesGridSize = 23;

    // Update the angles and other information from granule metadata
    UpdateFromGranuleMetadata();

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
    this->m_bandViewingAngles = ComputeViewingAngles(this->m_metadata->ProductInformation.ViewingAngles);

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
std::vector<MetadataHelperViewingAnglesGrid> SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetDetailedViewingAngles(int res)
{
    for(unsigned int i = 0; i<this->m_bandViewingAngles.size(); i++) {
        MACCSBandViewingAnglesGrid maccsGrid = this->m_bandViewingAngles[i];
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
bool SEN2CORMetadataHelper<PixelType, MasksPixelType>::BandAvailableForResolution(const std::string &bandId, int nRes) {
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
std::string SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetImageFileName(const std::string &bandName, int prefferedRes) {
    std::string suffix = ("_" + NormalizeBandName(bandName) + "_" + std::to_string(prefferedRes) + "m");
    const std::string &fileName = this->GetSen2CorImageFileName(this->m_metadata->ProductOrganization.ImageFiles, suffix);
    if (fileName.length() != 0) {
        return fileName;
    }
    // if not found in the preferred resolution, search in the others
    std::vector<int> vectResolutions = this->m_vectResolutions;
    vectResolutions.erase(std::remove(vectResolutions.begin(), vectResolutions.end(), prefferedRes), vectResolutions.end());
    for (auto res: vectResolutions) {
        suffix = ("_" + NormalizeBandName(bandName) + "_" + std::to_string(res) + "m");
        const std::string &fileName2 = this->GetSen2CorImageFileName(this->m_metadata->ProductOrganization.ImageFiles, suffix);
        if (fileName2.length() != 0) {
            return fileName2;
        }
    }
    return "";
}

template <typename PixelType, typename MasksPixelType>
std::string SEN2CORMetadataHelper<PixelType, MasksPixelType>::NormalizeBandName(const std::string &bandName) {
    if (bandName.length() == 0 || bandName[0] != 'B' || bandName.length() == 1 || bandName.length() > 3) {
        return bandName;
    }
    char buff[4];
    const std::string &bandNr = bandName.substr(1, bandName.length() - 1);
    snprintf(buff, sizeof(buff), "B%02d", std::atoi(bandNr.c_str()));
    return buff;
}

template <typename PixelType, typename MasksPixelType>
bool SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetValidBandNames(const std::vector<std::string> &bandNames,
                                                                         std::vector<std::string> &validBandNames,
                                                                         std::vector<int> &relBandIndexes, int &outRes)
{
    relBandIndexes.resize(bandNames.size());
    std::fill(relBandIndexes.begin(), relBandIndexes.end(), -1);
    int valdBandIdx = 0;
    int i = 0;
    std::map<int, int> resolutions;
    bool bHasBandName, bHas10mBandName;
    const std::vector<std::string> &bands10m = this->GetBandNamesForResolution(10);
    const std::vector<std::string> &bands20m = this->GetBandNamesForResolution(20);
    const std::vector<std::string> &bands60m = this->GetBandNamesForResolution(60);
    for(const std::string &bandName: bandNames) {
        bHasBandName = false;
        bHas10mBandName = false;
        if (this->HasBandName(bands10m, bandName)) {
            if (outRes == -1 || outRes == 10) {
                resolutions[10] = 10;
                bHasBandName = true;
            }
            bHas10mBandName = true;
        }
        // if the band is found for 20m but not found for 10m and the resolution is not 10m
        if (!bHasBandName && (outRes == -1 || outRes == 10 || outRes == 20) && this->HasBandName(bands20m, bandName)) {
            resolutions[20] = 20;
            bHasBandName = true;
        }
        // if the band was not found in all other resolutions, try to get it from 60m res (for ex. B01)
        if (!bHasBandName && this->HasBandName(bands60m, bandName)) {
            resolutions[60] = 60;
            bHasBandName = true;
        }
        // This is because band B08 apears only in the 10m resolution and not in 20m and 60m resolutions
        if (!bHasBandName && bHas10mBandName) {
            resolutions[10] = 10;
            bHasBandName = true;
        }
        if (bHasBandName) {
            relBandIndexes[i] = valdBandIdx++;
            validBandNames.push_back(bandName);
        }
        i++;
    }
    if (validBandNames.size() == 0) {
        itkExceptionMacro("No valid band was provided for extracting the image " << bandNames[0] << " ... ");
        return false;
    }

    // set the out resolution to a valid value
    if (outRes == -1) {
        if (resolutions.size() == 1) {
            outRes = resolutions.begin()->first;
        } else {
            outRes = 10;
        }
    }
    return true;
}

template <typename PixelType, typename MasksPixelType>
bool SEN2CORMetadataHelper<PixelType, MasksPixelType>::HasBandName(const std::vector<std::string> &bandNames, const std::string &bandName) {
    if (std::find(bandNames.begin(), bandNames.end(), bandName) != bandNames.end()) {
        return true;
    }
    return false;
}

template <typename PixelType, typename MasksPixelType>
std::string SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetAotImageFileName(int res)
{
    switch (res) {
        case 20:
            return this->GetSen2CorImageFileName(this->m_metadata->ProductOrganization.ImageFiles, "_AOT_20m");
        case 60:
            return this->GetSen2CorImageFileName(this->m_metadata->ProductOrganization.ImageFiles, "_AOT_60m");
        default:
            return this->GetSen2CorImageFileName(this->m_metadata->ProductOrganization.ImageFiles, "_AOT_10m");
    }
}

template <typename PixelType, typename MasksPixelType>
std::string SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetSCLFileName(int res)
{
    switch (res) {
        case 60:
            return this->GetSen2CorImageFileName(this->m_metadata->ProductOrganization.ImageFiles, "_SCL_60m");
        default:
            return this->GetSen2CorImageFileName(this->m_metadata->ProductOrganization.ImageFiles, "_SCL_20m");
    }
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::Pointer
        SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult, int resolution) {
    this->InitializeS2Angles();

    // We use SCL file for all flags
    typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::Pointer img;
    // force resolution to 10 m if not specified
    img = this->m_maskFlagsBandsExtractor.ExtractResampledBand(GetSCLFileName(resolution), 1, Interpolator_NNeighbor, -1,
                                                               (resolution == -1 ? 10 : resolution));

    this->m_maskHandlerFunctor.Initialize(nMaskFlags, binarizeResult);
    this->m_maskHandlerFilter = Sen2CorUnaryFunctorImageFilterType::New();
    this->m_maskHandlerFilter->SetFunctor(this->m_maskHandlerFunctor);
    this->m_maskHandlerFilter->SetInput(img);
    return this->m_maskHandlerFilter->GetOutput();
}

// Get the id of the band. Return -1 if band not found.
template <typename PixelType, typename MasksPixelType>
int SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetRelativeBandIdx(const std::string &bandName) {
    const std::vector<std::string> &allBands = this->GetAllBandNames();
    std::ptrdiff_t pos = std::find(allBands.begin(), allBands.end(), bandName) - allBands.begin();
    if (pos >= allBands.size()) {
        return -1;
    }
    return pos;
}

// Return the path to a file for which the name end in the ending
template <typename PixelType, typename MasksPixelType>
std::string SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetSen2CorImageFileName(const std::vector<CommonFileInformation>& imageFiles,
                                                       const std::string& ending) {
    std::string retStr;
    for (const CommonFileInformation& fileInfo : imageFiles) {
        if(GetSen2CorImageFileName(fileInfo, ending, retStr))
            return retStr;
    }
    return "";
}

// Return the path to a file for which the name end in the ending
template <typename PixelType, typename MasksPixelType>
std::string SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetSen2CorImageFileName(const std::vector<CommonAnnexInformation>& maskFiles,
                                                       const std::string& ending) {
    std::string retStr;
    for (const CommonAnnexInformation& fileInfo : maskFiles) {
        if(GetSen2CorImageFileName(fileInfo.File, ending, retStr))
            return retStr;
    }
    return "";
}

template <typename PixelType, typename MasksPixelType>
bool SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetSen2CorImageFileName(const CommonFileInformation& fileInfo,
                                                       const std::string& ending, std::string& retStr) {
    if (fileInfo.LogicalName.length() >= ending.length() &&
            0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
        boost::filesystem::path rootFolder(this->m_DirName);
        // Get the extension of file (default for Sen2Cor is ".xml")
        const std::string &ext = this->GetRasterFileExtension();
        retStr = (rootFolder / (fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ext)).string();
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
bool SEN2CORMetadataHelper<PixelType, MasksPixelType>::CheckFileExistence(std::string &fileName) {
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

template <typename PixelType, typename MasksPixelType>
std::string SEN2CORMetadataHelper<PixelType, MasksPixelType>::GetGranuleXmlPath(const std::unique_ptr<MACCSFileMetadata> &metadata)
{
    boost::filesystem::path path(metadata->ProductPath);
    path = path.parent_path();
    std::string relFileLocation = metadata->ProductOrganization.ImageFiles[0].FileLocation;
    relFileLocation = relFileLocation.substr(0, relFileLocation.find("/IMG_DATA/"));
    //std::replace( relFileLocation.begin(), relFileLocation.end(), './', '');

    path /= relFileLocation;
    path /= "MTD_TL.xml";

    return path.string();
}

template <typename PixelType, typename MasksPixelType>
void SEN2CORMetadataHelper<PixelType, MasksPixelType>::UpdateFromGranuleMetadata()
{
    if (!m_bGranuleMetadataUpdated) {
        SEN2CORMetadataReaderType::Pointer sen2CorMetadataReader = SEN2CORMetadataReaderType::New();
        std::unique_ptr<MACCSFileMetadata> granuleMetadata;
        const std::string &granuleFile = GetGranuleXmlPath(m_metadata);
        if (granuleMetadata = sen2CorMetadataReader->ReadMetadata(granuleFile)) {
            // now fill the global metadata
            m_metadata->ProductOrganization.AnnexFiles = granuleMetadata->ProductOrganization.AnnexFiles;
            m_metadata->ImageInformation.Resolutions = granuleMetadata->ImageInformation.Resolutions;

            m_metadata->ProductInformation.SolarAngles = granuleMetadata->ProductInformation.SolarAngles;
            m_metadata->ProductInformation.MeanSunAngle = granuleMetadata->ProductInformation.MeanSunAngle;
            m_metadata->ProductInformation.MeanViewingIncidenceAngles = granuleMetadata->ProductInformation.MeanViewingIncidenceAngles;
            m_metadata->ProductInformation.ViewingAngles = granuleMetadata->ProductInformation.ViewingAngles;
        }
    }
}

