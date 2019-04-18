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
 
#include "Spot4MetadataHelper.h"
#include "GlobalDefs.h"

typedef itk::SPOT4MetadataReader                                   SPOT4MetadataReaderType;

#define TOTAL_BANDS_NO      4

template <typename PixelType, typename MasksPixelType>
Spot4MetadataHelper<PixelType, MasksPixelType>::Spot4MetadataHelper()
{
    this->m_nTotalBandsNo = TOTAL_BANDS_NO;
    this->m_bHasGlobalMeanAngles = true;
    this->m_bHasBandMeanAngles = false;
    this->m_bHasDetailedAngles = false;
    this->m_vectResolutions.push_back(20);
}

template <typename PixelType, typename MasksPixelType>
bool Spot4MetadataHelper<PixelType, MasksPixelType>::DoLoadMetadata(const std::string& file)
{
    SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();
    if (m_metadata = spot4MetadataReader->ReadMetadata(file)) {
        if(this->m_metadata->Radiometry.Bands.size() != this->m_nTotalBandsNo) {
            itkExceptionMacro("Wrong number of bands for SPOT4: " + this->m_metadata->Radiometry.Bands.size() );
            return false;
        }

        this->m_MissionShortName = "SPOT";

        this->m_strNoDataValue = std::to_string(NO_DATA_VALUE);

        // For Spot4 the bands are XS1;XS2;XS3;SWIR that correspond to GREEN, RED, NIR and SWIR
        // we have the same values for relative and absolute indexes as we have only one raster
        // with only one resolution
        this->m_nBlueBandName = "XS1";
        this->m_nGreenBandName = "XS1";
        this->m_nRedBandName = "XS2";
        this->m_nNirBandName = "XS3";
        this->m_nSwirBandName = "SWIR";

        this->m_ReflQuantifVal = 1000.0;

        this->m_Mission = this->m_metadata->Header.Mission;
        this->m_Instrument = this->m_metadata->Header.Instrument;

        // compute the AOT file name
        this->m_AotFileName = getAotFileName();

        // extract the acquisition date
        this->m_AcquisitionDate = this->m_metadata->Header.DatePdv.substr(0,4) +
                this->m_metadata->Header.DatePdv.substr(5,2) + this->m_metadata->Header.DatePdv.substr(8,2);
        this->m_AcquisitionDateTime = this->m_metadata->Header.DatePdv;
        std::string &acqDateTime = this->m_AcquisitionDateTime;
        std::string::size_type pos = acqDateTime.find('.');
        // remove the milliseconds part of the acquisition date/time
        acqDateTime = (pos != std::string::npos) ? acqDateTime.substr(0, pos) : acqDateTime;
        acqDateTime.erase(std::remove(acqDateTime.begin(), acqDateTime.end(), '-'), acqDateTime.end());
        acqDateTime.erase(std::remove(acqDateTime.begin(), acqDateTime.end(), ':'), acqDateTime.end());
        // the product date does has a space separator between date and time. We need to replace it with T
        std::replace( acqDateTime.begin(), acqDateTime.end(), ' ', 'T');

        //TODO: Add initialization for mean angles (solar and sensor)
        this->m_solarMeanAngles.zenith = this->m_metadata->Radiometry.Angles.ThetaS;
        this->m_solarMeanAngles.azimuth = this->m_metadata->Radiometry.Angles.PhiS;
        MeanAngles_Type sensorAngles;
        sensorAngles.zenith = this->m_metadata->Radiometry.Angles.ThetaV;
        sensorAngles.azimuth = this->m_metadata->Radiometry.Angles.PhiV;
        this->m_sensorBandsMeanAngles.push_back(sensorAngles);

        return true;
    }

    return false;
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer
Spot4MetadataHelper<PixelType, MasksPixelType>::GetImage(const std::vector<std::string> &bandNames, int outRes)
{
    return GetImage(bandNames, NULL, outRes);
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer
Spot4MetadataHelper<PixelType, MasksPixelType>::GetImage(const std::vector<std::string> &bandNames,
                                                         std::vector<int> *pRetRelBandIdxs, int outRes)
{

    int curRes = this->m_vectResolutions.at(0);
    if (outRes == -1) {
        outRes = curRes;
    }

    typename MetadataHelper<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(
                this->getImageFileName());
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
    typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer imageList = this->CreateImageList();
    this->m_bandsExtractor.ExtractImageBands(reader->GetOutput(), imageList, bandIdxs,
                                                Interpolator_NNeighbor, curRes, outRes);
    typename MetadataHelper<PixelType, MasksPixelType>::ListConcatenerFilterType::Pointer concat = this->CreateConcatenner();
    concat->SetInput(imageList);

    return concat->GetOutput();
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer Spot4MetadataHelper<PixelType, MasksPixelType>::GetImageList(const std::vector<std::string> &bandNames,
                                                                      typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer outImgList, int outRes)
{

    int curRes = this->m_vectResolutions.at(0);
    typename MetadataHelper<PixelType, MasksPixelType>::ImageReaderType::Pointer reader = this->CreateReader(
                this->getImageFileName());
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
std::vector<std::string> Spot4MetadataHelper<PixelType, MasksPixelType>::GetBandNamesForResolution(int)
{
    return GetAllBandNames();
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> Spot4MetadataHelper<PixelType, MasksPixelType>::GetAllBandNames()
{
    return GetPhysicalBandNames();
}

template <typename PixelType, typename MasksPixelType>
std::vector<std::string> Spot4MetadataHelper<PixelType, MasksPixelType>::GetPhysicalBandNames()
{
    return this->m_metadata->Radiometry.Bands;
}

template <typename PixelType, typename MasksPixelType>
int Spot4MetadataHelper<PixelType, MasksPixelType>::GetResolutionForBand(const std::string &bandName)
{
    for(const std::string &curBand: this->m_metadata->Radiometry.Bands) {
        if (curBand == bandName) {
            return 20;
        }
    }
    return -1;
}

template <typename PixelType, typename MasksPixelType>
float Spot4MetadataHelper<PixelType, MasksPixelType>::GetAotQuantificationValue(int)
{
    return 1000.0;
}

template <typename PixelType, typename MasksPixelType>
float Spot4MetadataHelper<PixelType, MasksPixelType>::GetAotNoDataValue(int)
{
    return 0;
}

template <typename PixelType, typename MasksPixelType>
int Spot4MetadataHelper<PixelType, MasksPixelType>::GetAotBandIndex(int)
{
    return 1;
}

template <typename PixelType, typename MasksPixelType>
std::string Spot4MetadataHelper<PixelType, MasksPixelType>::DeriveFileNameFromImageFileName(const std::string& replacement)
{
    std::string fileName;
    std::string orthoSurf = this->m_metadata->Files.OrthoSurfCorrPente;
    if(orthoSurf.empty()) {
        orthoSurf = this->m_metadata->Files.OrthoSurfCorrEnv;
        if(!orthoSurf.empty()) {
            int nPos = orthoSurf.find("ORTHO_SURF_CORR_ENV");
            orthoSurf.replace(nPos, strlen("ORTHO_SURF_CORR_ENV"), replacement);
            fileName = orthoSurf;
        }
    } else {
        int nPos = orthoSurf.find("ORTHO_SURF_CORR_PENTE");
        orthoSurf.replace(nPos, strlen("ORTHO_SURF_CORR_PENTE"), replacement);
        fileName = orthoSurf;
    }

    return fileName;
}

template <typename PixelType, typename MasksPixelType>
std::string Spot4MetadataHelper<PixelType, MasksPixelType>::getImageFileName() {

    return this->buildFullPath(this->m_metadata->Files.OrthoSurfCorrPente);
}

template <typename PixelType, typename MasksPixelType>
std::string Spot4MetadataHelper<PixelType, MasksPixelType>::getAotFileName()
{
    // Return the path to a the AOT file computed from ORTHO_SURF_CORR_PENTE or ORTHO_SURF_CORR_ENV
    // if the key is not present in the XML
    std::string fileName;
    if(this->m_metadata->Files.OrthoSurfAOT == "") {
        fileName = DeriveFileNameFromImageFileName("AOT");
    } else {
        fileName = this->m_metadata->Files.OrthoSurfAOT;
    }

    return this->buildFullPath(fileName);
}

template <typename PixelType, typename MasksPixelType>
std::string Spot4MetadataHelper<PixelType, MasksPixelType>::getCloudFileName()
{
    return this->buildFullPath(this->m_metadata->Files.MaskNua);
}

template <typename PixelType, typename MasksPixelType>
std::string Spot4MetadataHelper<PixelType, MasksPixelType>::getWaterFileName()
{
    return this->buildFullPath(this->m_metadata->Files.MaskDiv);
}

template <typename PixelType, typename MasksPixelType>
std::string Spot4MetadataHelper<PixelType, MasksPixelType>::getSnowFileName()
{
    return getWaterFileName();
}

template <typename PixelType, typename MasksPixelType>
std::string Spot4MetadataHelper<PixelType, MasksPixelType>::getSaturationFileName()
{
    return this->buildFullPath(this->m_metadata->Files.MaskSaturation);
}

template <typename PixelType, typename MasksPixelType>
typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::Pointer
        Spot4MetadataHelper<PixelType, MasksPixelType>::GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult, int) {
    // we have values for: cloud, (snow, water, valid) and saturation

    //Diverse binary masks : water, snow and no_data mask, plus (V2.0) pixels lying in terrain shadows _DIV.TIF
    //   bit 0 (1) : No data
    //   bit 1 (2) : Water
    //   bit 2 (4) : Snow
    // clouds and saturation are compared for different of 0

    std::vector< typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::Pointer> vecImgs;
    // extract the cld, div and saturation mask image bands
    if((nMaskFlags & MSK_CLOUD) != 0) {
        vecImgs.push_back(this->m_maskFlagsBandsExtractor.ExtractResampledBand(getCloudFileName(), 1, Interpolator_NNeighbor));
    }

       if((nMaskFlags & MSK_SNOW) != 0 || (nMaskFlags & MSK_WATER) != 0 || (nMaskFlags & MSK_VALID) != 0) {
        vecImgs.push_back(this->m_maskFlagsBandsExtractor.ExtractResampledBand(getWaterFileName(), 1, Interpolator_NNeighbor));
    }

    if((nMaskFlags & MSK_SAT) != 0) {
        vecImgs.push_back(this->m_maskFlagsBandsExtractor.ExtractResampledBand(getSaturationFileName(), 1, Interpolator_NNeighbor));
    }
    this->m_maskHandlerFunctor.Initialize(nMaskFlags, binarizeResult);
    this->m_maskHandlerFilter = NaryFunctorImageFilterType::New();
    this->m_maskHandlerFilter->SetFunctor(this->m_maskHandlerFunctor);
    for(size_t i = 0; i<vecImgs.size(); i++) {
        this->m_maskHandlerFilter->SetInput(i, vecImgs[i]);
    }
    return this->m_maskHandlerFilter->GetOutput();
}

// Get the id of the band. Return -1 if band not found.
template <typename PixelType, typename MasksPixelType>
int Spot4MetadataHelper<PixelType, MasksPixelType>::GetRelativeBandIdx(const std::string &bandName) {
    const std::vector<std::string> &allBands = this->GetAllBandNames();
    std::ptrdiff_t pos = std::find(allBands.begin(), allBands.end(), bandName) - allBands.begin();
    if (pos >= allBands.size()) {
        return -1;
    }
    return pos;
}
/*
void Spot4MetadataHelperBaseDummy() {
    Spot4MetadataHelper<short, short> test1;
    Spot4MetadataHelper<short, uint8_t> test11;
    Spot4MetadataHelper<float, short> test2;
    Spot4MetadataHelper<float, uint8_t> test21;
    Spot4MetadataHelper<int, short> test3;
    Spot4MetadataHelper<int, uint8_t> test31;
}
*/
