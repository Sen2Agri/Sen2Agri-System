#include "Spot4MetadataHelper.h"
#include "GlobalDefs.h"

#define TOTAL_BANDS_NO      4

Spot4MetadataHelper::Spot4MetadataHelper()
{
    m_fAotQuantificationValue = 1000.0;
    m_fAotNoDataVal = 0;
    m_nAotBandIndex = 1;
    m_nTotalBandsNo = TOTAL_BANDS_NO;
    m_nBandsNoForCurRes = m_nTotalBandsNo;
    m_bHasGlobalMeanAngles = true;
    m_bHasBandMeanAngles = false;
    m_bHasDetailedAngles = false;
}

std::string Spot4MetadataHelper::GetBandName(unsigned int nIdx, bool bRelativeIdx)
{
    UNUSED(bRelativeIdx);

    if(nIdx >= m_nBandsNoForCurRes) {
        itkExceptionMacro("Invalid band index requested: " << nIdx << ". Maximum is " << m_nBandsNoForCurRes);
    }
    return m_metadata->Radiometry.Bands[nIdx];
}

bool Spot4MetadataHelper::DoLoadMetadata()
{
    SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();
    if (m_metadata = spot4MetadataReader->ReadMetadata(m_inputMetadataFileName)) {
        if(m_metadata->Radiometry.Bands.size() != m_nBandsNoForCurRes) {
            itkExceptionMacro("Wrong number of bands for SPOT4: " + m_metadata->Radiometry.Bands.size() );
            return false;
        }

        // the helper will return the hardcoded values from the constructor as these are not
        // present in the metadata
        m_fAotQuantificationValue = 1000.0;
        m_fAotNoDataVal = 0;
        m_nAotBandIndex = 1;

        m_strNoDataValue = std::to_string(NO_DATA_VALUE);

        // For Spot4 the bands are XS1;XS2;XS3;SWIR that correspond to RED, GREEN, NIR and SWIR
        // we have the same values for relative and absolute indexes as we have only one raster
        // with only one resolution
        m_nAbsGreenBandIndex = m_nRelGreenBandIndex = 1;
        m_nAbsRedBandIndex = m_nRelRedBandIndex = 2;
        m_nAbsBlueBandIndex = m_nRelBlueBandIndex = m_nAbsGreenBandIndex;
        m_nAbsNirBandIndex = m_nRelNirBandIndex = 3;

        m_ReflQuantifVal = 1000.0;

        m_Mission = "SPOT4";
        // compute the Image file name
        m_ImageFileName = getImageFileName();

        // compute the AOT file name
        m_AotFileName = getAotFileName();
        // compute the Cloud file name
        m_CloudFileName = getCloudFileName();
        // compute the Water file name
        m_WaterFileName = getWaterFileName();
        // compute the Snow file name
        m_SnowFileName = getSnowFileName();
        // compute the Saturation file name
        m_SaturationFileName = getSaturationFileName();

        // extract the acquisition date
        m_AcquisitionDate = m_metadata->Header.DatePdv.substr(0,4) +
                m_metadata->Header.DatePdv.substr(5,2) + m_metadata->Header.DatePdv.substr(8,2);

        //TODO: Add initialization for mean angles (solar and sensor)
        m_solarMeanAngles.zenith = m_metadata->Radiometry.Angles.ThetaS;
        m_solarMeanAngles.azimuth = m_metadata->Radiometry.Angles.PhiS;
        MeanAngles_Type sensorAngles;
        sensorAngles.zenith = m_metadata->Radiometry.Angles.ThetaV;
        sensorAngles.azimuth = m_metadata->Radiometry.Angles.PhiV;
        m_sensorBandsMeanAngles.push_back(sensorAngles);

        return true;
    }

    return false;
}

std::string Spot4MetadataHelper::DeriveFileNameFromImageFileName(const std::string& replacement)
{
    std::string fileName;
    std::string orthoSurf = m_metadata->Files.OrthoSurfCorrPente;
    if(orthoSurf.empty()) {
        orthoSurf = m_metadata->Files.OrthoSurfCorrEnv;
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

std::string Spot4MetadataHelper::getImageFileName() {

    return buildFullPath(m_metadata->Files.OrthoSurfCorrPente);
}

std::string Spot4MetadataHelper::getAotFileName()
{
    // Return the path to a the AOT file computed from ORTHO_SURF_CORR_PENTE or ORTHO_SURF_CORR_ENV
    // if the key is not present in the XML
    std::string fileName;
    if(m_metadata->Files.OrthoSurfAOT == "") {
        fileName = DeriveFileNameFromImageFileName("AOT");
    } else {
        fileName = m_metadata->Files.OrthoSurfAOT;
    }

    return buildFullPath(fileName);
}

std::string Spot4MetadataHelper::getCloudFileName()
{
    return buildFullPath(m_metadata->Files.MaskNua);
}

std::string Spot4MetadataHelper::getWaterFileName()
{
    return buildFullPath(m_metadata->Files.MaskDiv);
}

std::string Spot4MetadataHelper::getSnowFileName()
{
    return getWaterFileName();
}

std::string Spot4MetadataHelper::getSaturationFileName()
{
    return buildFullPath(m_metadata->Files.MaskSaturation);
}

MetadataHelper::SingleBandShortImageType::Pointer Spot4MetadataHelper::GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult) {
    // we have values for: cloud, (snow, water, valid) and saturation

    //Diverse binary masks : water, snow and no_data mask, plus (V2.0) pixels lying in terrain shadows _DIV.TIF
    //   bit 0 (1) : No data
    //   bit 1 (2) : Water
    //   bit 2 (4) : Snow
    // clouds and saturation are compared for different of 0

    std::vector< MetadataHelper::SingleBandShortImageType::Pointer> vecImgs;
    // extract the cld, div and saturation mask image bands
    if((nMaskFlags & MSK_CLOUD) != 0) {
        vecImgs.push_back(m_bandsExtractor.ExtractResampledBand(getCloudFileName(), 1));
    }

       if((nMaskFlags & MSK_SNOW) != 0 || (nMaskFlags & MSK_WATER) != 0 || (nMaskFlags & MSK_VALID) != 0) {
        vecImgs.push_back(m_bandsExtractor.ExtractResampledBand(getWaterFileName(), 1));
    }

    if((nMaskFlags & MSK_SAT) != 0) {
        vecImgs.push_back(m_bandsExtractor.ExtractResampledBand(getSaturationFileName(), 1));
    }
    m_maskHandlerFunctor.Initialize(nMaskFlags, binarizeResult);
    m_maskHandlerFilter = NaryFunctorImageFilterType::New();
    m_maskHandlerFilter->SetFunctor(m_maskHandlerFunctor);
    for(size_t i = 0; i<vecImgs.size(); i++) {
        m_maskHandlerFilter->SetInput(i, vecImgs[i]);
    }
    return m_maskHandlerFilter->GetOutput();
}
