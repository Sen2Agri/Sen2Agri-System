#include "UpdateSynthesisFunctor.h"

template< class TInput, class TOutput>
UpdateSynthesisFunctor<TInput,TOutput>::UpdateSynthesisFunctor()
{
    m_sensorType = SENSOR_LANDSAT8;
    m_resolution = RES_10M;
    m_fQuantificationValue = -1;
    m_nCurrentDate = 0;
    m_bPrevL3ABandsAvailable = false;
    m_nNbOfL3AReflectanceBands = 0;
    m_nNbL2ABands = 0;
    memset(m_arrL2ABandPresence, -1, sizeof(m_arrL2ABandPresence));
    m_nL2ABandStartIndex = 0;
    m_nCloudMaskBandIndex = -1;
    m_nSnowMaskBandIndex = -1;
    m_nWaterMaskBandIndex = -1;
    m_nCurrentL2AWeightBandIndex = -1;
    m_nPrevL3AWeightBandStartIndex = -1;
    m_nPrevL3AWeightedAvDateBandIndex = -1;
    m_nPrevL3AReflectanceBandStartIndex = -1;
    m_nPrevL3APixelFlagBandIndex = -1;
    m_nRedBandIndex = -1;
    m_nBlueBandIndex = -1;
}

template< class TInput, class TOutput>
UpdateSynthesisFunctor<TInput,TOutput>& UpdateSynthesisFunctor<TInput,TOutput>::operator =(const UpdateSynthesisFunctor& copy)
{
    m_sensorType = copy.m_sensorType;
    m_resolution = copy.m_resolution;
    m_fQuantificationValue = copy.m_fQuantificationValue;
    m_nCurrentDate = copy.m_nCurrentDate;
    m_bPrevL3ABandsAvailable = copy.m_bPrevL3ABandsAvailable;
    m_nNbOfL3AReflectanceBands = copy.m_nNbOfL3AReflectanceBands;
    m_nNbL2ABands = copy.m_nNbL2ABands;
    memcpy(m_arrL2ABandPresence, copy.m_arrL2ABandPresence, sizeof(m_arrL2ABandPresence));
    m_nL2ABandStartIndex = copy.m_nL2ABandStartIndex;
    m_nCloudMaskBandIndex = copy.m_nCloudMaskBandIndex;
    m_nSnowMaskBandIndex = copy.m_nSnowMaskBandIndex;
    m_nWaterMaskBandIndex = copy.m_nWaterMaskBandIndex;
    m_nCurrentL2AWeightBandIndex = copy.m_nCurrentL2AWeightBandIndex;
    m_nPrevL3AWeightBandStartIndex = copy.m_nPrevL3AWeightBandStartIndex;
    m_nPrevL3AWeightedAvDateBandIndex = copy.m_nPrevL3AWeightedAvDateBandIndex;
    m_nPrevL3AReflectanceBandStartIndex = copy.m_nPrevL3AReflectanceBandStartIndex;
    m_nPrevL3APixelFlagBandIndex = copy.m_nPrevL3APixelFlagBandIndex;
    m_nRedBandIndex = copy.m_nRedBandIndex;
    m_nBlueBandIndex = copy.m_nBlueBandIndex;
    return *this;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::Initialize(SensorType sensorType, ResolutionType resolution,
                                                        bool bPrevL3ABandsAvailable, int nDate, float fQuantifVal,
                                                        bool bAllInOne)
{
    m_nCurrentDate = nDate;
    m_fQuantificationValue = fQuantifVal;
    m_sensorType = sensorType;
    m_resolution = resolution;
    m_bPrevL3ABandsAvailable = bPrevL3ABandsAvailable;
    if(bAllInOne) {
        // in this case we do not care about resolution as we have the same number of bands both for 10m and 20m resolutions
        InitAllBandInResolutionInfos(sensorType);
    } else {
        InitBandInfos(sensorType, resolution);
    }
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::InitBandInfos(SensorType sensorType, ResolutionType resolution)
{
    if(resolution == RES_10M) {
        m_nNbOfL3AReflectanceBands = WEIGHTED_REFLECTANCE_10M_BANDS_NO;
        if(sensorType == SENSOR_S2)
        {
            m_nNbL2ABands = S2_L2A_10M_BANDS_NO;
            m_nL2ABandStartIndex = S2_L2A_10M_BANDS_START_IDX;
            m_nCloudMaskBandIndex = S2_L2A_10M_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = S2_L2A_10M_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = S2_L2A_10M_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = S2_L2A_10M_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = S2_L3A_10M_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = S2_L3A_10M_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = S2_L3A_10M_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = S2_L3A_10M_PIXEL_STATUS_IDX;
            m_nRedBandIndex = S2_L2A_10M_RED_BAND_IDX;
            m_nBlueBandIndex = S2_L2A_10M_BLUE_BAND_IDX;

            // For S2 at 10m all bands are available
            int arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO] = {0, 1, 2, 3, -1, -1};
            memcpy(m_arrL2ABandPresence, arrL2ABandPresence, sizeof(m_arrL2ABandPresence));

        } else if (sensorType == SENSOR_LANDSAT8) {

            m_nNbL2ABands = L8_L2A_10M_BANDS_NO;
            m_nL2ABandStartIndex = L8_L2A_10M_BANDS_START_IDX;
            m_nCloudMaskBandIndex = L8_L2A_10M_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = L8_L2A_10M_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = L8_L2A_10M_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = L8_L2A_10M_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = L8_L3A_10M_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = L8_L3A_10M_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = L8_L3A_10M_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = L8_L3A_10M_PIXEL_STATUS_IDX;
            m_nRedBandIndex = L8_L2A_10M_RED_BAND_IDX;
            m_nBlueBandIndex = L8_L2A_10M_BLUE_BAND_IDX;

            // For L8 at 10m B8 is not available
            int arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO] = {0, 1, 2, -1, -1, -1};
            memcpy(m_arrL2ABandPresence, arrL2ABandPresence, sizeof(m_arrL2ABandPresence));

        } else if (sensorType == SENSOR_SPOT4) {
            m_nNbL2ABands = SPOT4_L2A_10M_BANDS_NO;
            m_nL2ABandStartIndex = SPOT4_L2A_10M_BANDS_START_IDX;
            m_nCloudMaskBandIndex = SPOT4_L2A_10M_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = SPOT4_L2A_10M_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = SPOT4_L2A_10M_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = SPOT4_L2A_10M_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = SPOT4_L3A_10M_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = SPOT4_L3A_10M_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = SPOT4_L3A_10M_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = SPOT4_L3A_10M_PIXEL_STATUS_IDX;
            m_nRedBandIndex = SPOT4_L2A_10M_RED_BAND_IDX;
            m_nBlueBandIndex = SPOT4_L2A_10M_BLUE_BAND_IDX;

            // For L8 at 10m B2 is not available
            int arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO] = { -1, 0, 1, 2, -1, -1 };
            memcpy(m_arrL2ABandPresence, arrL2ABandPresence, sizeof(m_arrL2ABandPresence));

        } else {
            itkExceptionMacro("Invalid Sensor type " << sensorType << " for resolution 10M");
        }
    } else if(resolution == RES_20M) {
        m_nNbOfL3AReflectanceBands = WEIGHTED_REFLECTANCE_20M_BANDS_NO;
        if(sensorType == SENSOR_S2)
        {
            m_nNbL2ABands = S2_L2A_20M_BANDS_NO;
            m_nL2ABandStartIndex = S2_L2A_20M_BANDS_START_IDX;
            m_nCloudMaskBandIndex = S2_L2A_20M_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = S2_L2A_20M_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = S2_L2A_20M_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = S2_L2A_20M_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = S2_L3A_20M_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = S2_L3A_20M_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = S2_L3A_20M_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = S2_L3A_20M_PIXEL_STATUS_IDX;
            m_nRedBandIndex = S2_L2A_20M_RED_BAND_IDX;
            m_nBlueBandIndex = S2_L2A_20M_BLUE_BAND_IDX;
            // For S2 at 20m all bands are available
            int arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO] = { 0, 1, 2, 3, 4, 5, -1, -1, -1, -1 };
            memcpy(m_arrL2ABandPresence, arrL2ABandPresence, sizeof(m_arrL2ABandPresence));

        } else if (sensorType == SENSOR_LANDSAT8) {

            m_nNbL2ABands = L8_L2A_20M_BANDS_NO;
            m_nL2ABandStartIndex = L8_L2A_20M_BANDS_START_IDX;
            m_nCloudMaskBandIndex = L8_L2A_20M_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = L8_L2A_20M_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = L8_L2A_20M_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = L8_L2A_20M_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = L8_L3A_20M_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = L8_L3A_20M_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = L8_L3A_20M_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = L8_L3A_20M_PIXEL_STATUS_IDX;
            m_nRedBandIndex = L8_L2A_20M_RED_BAND_IDX;
            m_nBlueBandIndex = L8_L2A_20M_BLUE_BAND_IDX;
            // For L4 at 20m only B8A, B11 and B12 bands are available
            int arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO] = { -1, -1, -1, 0, 1, 2, -1, -1, -1, -1 };
            memcpy(m_arrL2ABandPresence, arrL2ABandPresence, sizeof(m_arrL2ABandPresence));

        } else if (sensorType == SENSOR_SPOT4) {
            m_nNbL2ABands = SPOT4_L2A_20M_BANDS_NO;
            m_nL2ABandStartIndex = SPOT4_L2A_20M_BANDS_START_IDX;
            m_nCloudMaskBandIndex = SPOT4_L2A_20M_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = SPOT4_L2A_20M_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = SPOT4_L2A_20M_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = SPOT4_L2A_20M_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = SPOT4_L3A_20M_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = SPOT4_L3A_20M_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = SPOT4_L3A_20M_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = SPOT4_L3A_20M_PIXEL_STATUS_IDX;
            m_nRedBandIndex = SPOT4_L2A_20M_RED_BAND_IDX;
            m_nBlueBandIndex = SPOT4_L2A_20M_BLUE_BAND_IDX;

            // For SPOT4 at 20m only SWIR band is available (B11)
            int arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO] = { -1, -1, -1, -1, 0, -1, -1, -1, -1, -1 };
            memcpy(m_arrL2ABandPresence, arrL2ABandPresence, sizeof(m_arrL2ABandPresence));

        } else {
            itkExceptionMacro("Invalid Sensor type " << sensorType << " for resolution 20M");
        }
    } else {
        itkExceptionMacro("Invalid resolution " << resolution );
    }
    return true;
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::InitAllBandInResolutionInfos(SensorType sensorType)
{
    // In this case we have the same number for all resolutions (10 and 20) so we don't care about this in this case
        m_nNbOfL3AReflectanceBands = WEIGHTED_REFLECTANCE_ALL_BANDS_NO;
        if(sensorType == SENSOR_S2)
        {
            m_nNbL2ABands = S2_L2A_ALL_BANDS_NO;
            m_nL2ABandStartIndex = S2_L2A_ALL_BANDS_START_IDX;
            m_nCloudMaskBandIndex = S2_L2A_ALL_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = S2_L2A_ALL_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = S2_L2A_ALL_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = S2_L2A_ALL_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = S2_L3A_ALL_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = S2_L3A_ALL_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = S2_L3A_ALL_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = S2_L3A_ALL_PIXEL_STATUS_IDX;
            m_nRedBandIndex = S2_L2A_ALL_RED_BAND_IDX;
            m_nBlueBandIndex = S2_L2A_ALL_BLUE_BAND_IDX;

            // For S2 at 10m all bands are available
            int arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            memcpy(m_arrL2ABandPresence, arrL2ABandPresence, sizeof(m_arrL2ABandPresence));

        } else if (sensorType == SENSOR_LANDSAT8) {

            m_nNbL2ABands = L8_L2A_ALL_BANDS_NO;
            m_nL2ABandStartIndex = L8_L2A_ALL_BANDS_START_IDX;
            m_nCloudMaskBandIndex = L8_L2A_ALL_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = L8_L2A_ALL_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = L8_L2A_ALL_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = L8_L2A_ALL_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = L8_L3A_ALL_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = L8_L3A_ALL_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = L8_L3A_ALL_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = L8_L3A_ALL_PIXEL_STATUS_IDX;
            m_nRedBandIndex = L8_L2A_ALL_RED_BAND_IDX;
            m_nBlueBandIndex = L8_L2A_ALL_BLUE_BAND_IDX;

            // For L8 at 10m B8 is not available
            int arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO] = {0, 1, 2, -1, -1, -1, -1, 3, 4, 5};
            memcpy(m_arrL2ABandPresence, arrL2ABandPresence, sizeof(m_arrL2ABandPresence));

        } else if (sensorType == SENSOR_SPOT4) {
            m_nNbL2ABands = SPOT4_L2A_ALL_BANDS_NO;
            m_nL2ABandStartIndex = SPOT4_L2A_ALL_BANDS_START_IDX;
            m_nCloudMaskBandIndex = SPOT4_L2A_ALL_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = SPOT4_L2A_ALL_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = SPOT4_L2A_ALL_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = SPOT4_L2A_ALL_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = SPOT4_L3A_ALL_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = SPOT4_L3A_ALL_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = SPOT4_L3A_ALL_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = SPOT4_L3A_ALL_PIXEL_STATUS_IDX;
            m_nRedBandIndex = SPOT4_L2A_ALL_RED_BAND_IDX;
            m_nBlueBandIndex = SPOT4_L2A_ALL_BLUE_BAND_IDX;

            // For L8 at 10m B2 is not available
            int arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO] = { -1, 0, 1, -1, -1, -1, 2, -1, 3, -1 };
            memcpy(m_arrL2ABandPresence, arrL2ABandPresence, sizeof(m_arrL2ABandPresence));

        } else {
            itkExceptionMacro("Invalid Sensor type " << sensorType << " for resolution 10M");
        }
        return true;

}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::operator!=( const UpdateSynthesisFunctor & other) const
{
    return true;
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::operator==( const UpdateSynthesisFunctor & other ) const
{
    return !(*this != other);
}

template< class TInput, class TOutput>
TOutput UpdateSynthesisFunctor<TInput,TOutput>::operator()( const TInput & A )
{
    OutFunctorInfos outInfos;

    ResetCurrentPixelValues(A, outInfos);
    if(IsLandPixel(A))
    {
        HandleLandPixel(A, outInfos);
    } else {
        if(IsSnowPixel(A) || IsWaterPixel(A))
        {
            // if pixel is snow or water *replace the reflectance value
            HandleSnowOrWaterPixel(A, outInfos);
        } else {
            // if pixel is cloud or shadow *pixel never observed cloud snow or water free
            HandleCloudOrShadowPixel(A, outInfos);
        }
    }

    // we will have :
    //      - The weight counter for each pixel bands -> 4 or 6 for 10m and 20m respectively
    //      - The weighted average reflectance bands -> 4 or 6 for 10m and 20m respectively
    //      - one band for weighted average date
    //      - one band for flag with the status of each pixel
    int nTotalOutBandsNo = 2*m_nNbOfL3AReflectanceBands + 2;
    TOutput var(nTotalOutBandsNo);
    var.SetSize(nTotalOutBandsNo);

    int i;
    int cnt = 0;

    // Weighted Average Reflectances
    for(i = 0; i < m_nNbOfL3AReflectanceBands; i++)
    {
        // Normalize the values
        if(outInfos.m_CurrentPixelWeights[i] < 0) {
            outInfos.m_CurrentPixelWeights[i] = WEIGHT_NO_DATA;
        }
        var[cnt++] = short(outInfos.m_CurrentPixelWeights[i] * m_fQuantificationValue);
    }
    // Weighted Average Date L3A
    var[cnt++] = short(outInfos.m_fCurrentPixelWeightedDate * m_fQuantificationValue);
    // Weight for B2 for L3A
    for(i = 0; i < m_nNbOfL3AReflectanceBands; i++)
    {
        // Normalize the values
        if(outInfos.m_CurrentWeightedReflectances[i] < 0) {
            outInfos.m_CurrentWeightedReflectances[i] = REFLECTANCE_NO_DATA;
        }
        // we save back the pixel value but as digital value and not as reflectance
        var[cnt++] = short(outInfos.m_CurrentWeightedReflectances[i] * m_fQuantificationValue);
        //var[cnt++] = outInfos.m_CurrentWeightedReflectances[i];
    }
    // Pixel status
    var[cnt++] = outInfos.m_nCurrentPixelFlag;

    return var;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::ResetCurrentPixelValues(const TInput & A, OutFunctorInfos& outInfos)
{
    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        outInfos.m_CurrentPixelWeights[i] = GetPrevL3AWeightValue(A, i);
        outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
    }
    outInfos.m_nCurrentPixelFlag = GetPrevL3APixelFlagValue(A);
    outInfos.m_fCurrentPixelWeightedDate = GetPrevL3AWeightedAvDateValue(A);
}

template< class TInput, class TOutput>
int UpdateSynthesisFunctor<TInput,TOutput>::GetAbsoluteL2ABandIndex(int index)
{
    // extract the relative index for the band in the input bands list
    // starting from the index of the band in the output L3A product
    int relIdx = m_arrL2ABandPresence[index];
    if(relIdx != -1) {
        return (m_nL2ABandStartIndex + relIdx);
    }
    return -1;
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetL2AReflectanceForPixelVal(float fPixelVal)
{
    return (fPixelVal/m_fQuantificationValue);
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleLandPixel(const TInput & A, OutFunctorInfos& outInfos)
{
    // we assume that the reflectance bands start from index 0
    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        outInfos.m_nCurrentPixelFlag = LAND;

        // we will always have as output the number of reflectances equal or greater than
        // the number of bands in the current L2A raster for the current resolution
        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
        if(nCurrentBandIndex != -1)
        {
            // "if band is available in the case of LANDSAT 8, some bands are not available\"
            float fCurReflectance = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
            float fPrevReflect = GetPrevL3AReflectanceValue(A, i);
            float fPrevWeight = GetPrevL3AWeightValue(A, i);
            float fCurrentWeight = GetCurrentL2AWeightValue(A);
            float fPrevWeightedDate = GetPrevL3AWeightedAvDateValue(A);

            bool bIsPrevReflNoData = IsNoDataValue(fPrevReflect, REFLECTANCE_NO_DATA);
            bool bIsCurReflNoData = IsNoDataValue(fCurReflectance, REFLECTANCE_NO_DATA);

            if((bIsPrevReflNoData == false) && (bIsCurReflNoData == false)
                    && !IsNoDataValue(fPrevWeight, WEIGHT_NO_DATA)
                    && !IsNoDataValue(fPrevWeightedDate, DATE_NO_DATA)
                    && !IsNoDataValue(fCurrentWeight, WEIGHT_NO_DATA)) {
                outInfos.m_CurrentWeightedReflectances[i] = (fPrevWeight * fPrevReflect + fCurrentWeight * fCurReflectance) /
                        (fPrevWeight + fCurrentWeight);
                outInfos.m_fCurrentPixelWeightedDate = (fPrevWeight * fPrevWeightedDate + fCurrentWeight * m_nCurrentDate) /
                        (fPrevWeight + fCurrentWeight);
                outInfos.m_CurrentPixelWeights[i] = (fPrevWeight + fCurrentWeight);
            } else {
                outInfos.m_CurrentWeightedReflectances[i] = fCurReflectance;
                outInfos.m_CurrentPixelWeights[i] = fCurrentWeight;
                // if we have no data so far, then set also the flag as no data
                if(bIsPrevReflNoData && bIsCurReflNoData) {
                    outInfos.m_nCurrentPixelFlag = FLAG_NO_DATA;
                    outInfos.m_fCurrentPixelWeightedDate = DATE_NO_DATA;
                } else {
                    outInfos.m_fCurrentPixelWeightedDate = m_nCurrentDate;
                }
            }
        } else {
            // TODO: This code can be removed as the initialization is made in ResetCurrentPixelValues
            // L2A band missing - as for LANDSAT 8
            outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
            outInfos.m_CurrentPixelWeights[i] = GetPrevL3AWeightValue(A, i);
            //TODO: band is missing but the algorithm checks if it is RED???
            if(IsRedBand(i))
            {
                outInfos.m_fCurrentPixelWeightedDate = GetPrevL3AWeightedAvDateValue(A);
            }
        }
    }
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleSnowOrWaterPixel(const TInput & A, OutFunctorInfos& outInfos)
{
    if(IsWaterPixel(A)) {
        outInfos.m_nCurrentPixelFlag = WATER;
    } else {
       outInfos. m_nCurrentPixelFlag = SNOW;
    }
    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
        // band available
        if(nCurrentBandIndex != -1)
        {
            float fPrevWeight = GetPrevL3AWeightValue(A, i);
            // if pixel never observed without cloud, water or snow
            if(IsNoDataValue(fPrevWeight, 0) || IsNoDataValue(fPrevWeight, WEIGHT_NO_DATA)) {
                outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                outInfos.m_CurrentPixelWeights[i] = 0;
                if(IsRedBand(i))
                {
                    outInfos.m_fCurrentPixelWeightedDate = m_nCurrentDate;
                }
            } else {
                // pixel already observed cloud free, keep the previous weighted average
                outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                outInfos.m_CurrentPixelWeights[i] = fPrevWeight;
                if(IsRedBand(i))
                {
                    outInfos.m_fCurrentPixelWeightedDate = GetPrevL3AWeightedAvDateValue(A);
                    outInfos.m_nCurrentPixelFlag = LAND;
                }

            }
        } else {
            // TODO: This code can be removed as the initialization is made in ResetCurrentPixelValues
            // band not available, keep previous values
            outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
            outInfos.m_CurrentPixelWeights[i] = GetPrevL3AWeightValue(A, i);
            // TODO: In algorithm says nothing about WDate
        }
    }
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleCloudOrShadowPixel(const TInput & A, OutFunctorInfos& outInfos)
{
    short nPrevL3AFlagVal = GetPrevL3APixelFlagValue(A);
    // if flagN-1 is no-data => replace nodata with cloud
    if(nPrevL3AFlagVal == FLAG_NO_DATA)
    {
        //outInfos.m_nCurrentPixelFlag = CLOUD;

        for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
        {
            int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
            // band available
            if(nCurrentBandIndex != -1)
            {
                outInfos.m_CurrentWeightedReflectances[i] = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
                outInfos.m_CurrentPixelWeights[i] = 0;
                // TODO: Maybe here we should check if prevRefl or current refl are valid for this pixel
                //      (a value different than no data)
                if(IsRedBand(i))
                {
                    outInfos.m_fCurrentPixelWeightedDate = m_nCurrentDate;
                    outInfos.m_nCurrentPixelFlag = CLOUD;
                }
            } else {
                outInfos.m_CurrentWeightedReflectances[i] = REFLECTANCE_NO_DATA;
                float fPrevWeight = GetPrevL3AWeightValue(A, i);
                if(IsNoDataValue(fPrevWeight, WEIGHT_NO_DATA)) {
                    outInfos.m_CurrentPixelWeights[i] = WEIGHT_NO_DATA; // TODO: This is not conform to ATBD but seems more natural
                                                                    // in order to have no data for bands that are completely missing
                } else {
                    outInfos.m_CurrentPixelWeights[i] = 0;
                }
            }
        }
    } else {
        if((nPrevL3AFlagVal == CLOUD) || (nPrevL3AFlagVal == CLOUD_SHADOW))
        {
            // get the blue band index
            int nBlueBandIdx = GetBlueBandIndex();
            // in 20m resolution we have no blue band, so we will do the following
            // only for 10m resolution
            if(nBlueBandIdx != -1)
            {
                float fBlueReflectance = GetL2AReflectanceForPixelVal(A[nBlueBandIdx]);
                for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
                {
                    float fPrevReflectance = GetPrevL3AReflectanceValue(A, i);
                    // replace value only if the new reflectance in blue is smaller
                    if(fBlueReflectance < fPrevReflectance)
                    {

                        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
                        // band available
                        if(nCurrentBandIndex != -1)
                        {
                            outInfos.m_CurrentWeightedReflectances[i] = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
                            outInfos.m_CurrentPixelWeights[i] = 0;
                            if(IsRedBand(i))
                            {
                                outInfos.m_fCurrentPixelWeightedDate = m_nCurrentDate;
                                outInfos.m_nCurrentPixelFlag = CLOUD;
                            }
                        } else {
                            outInfos.m_CurrentWeightedReflectances[i] = fPrevReflectance;
                            outInfos.m_CurrentPixelWeights[i] = 0;
                        }
                    }
                }
            }
        } else {
            for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
            {
                // TODO: The first 2 lines of code can be removed as the initialization is made in ResetCurrentPixelValues
                outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                outInfos.m_fCurrentPixelWeightedDate = GetPrevL3AWeightedAvDateValue(A);
                outInfos.m_CurrentPixelWeights[i] = 0;
            }

        }
    }
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsSnowPixel(const TInput & A)
{
    if(m_nSnowMaskBandIndex == -1)
        return false;

    int val = (int)static_cast<float>(A[m_nSnowMaskBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsWaterPixel(const TInput & A)
{
    if(m_nWaterMaskBandIndex == -1)
        return false;

    int val = (int)static_cast<float>(A[m_nWaterMaskBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsCloudPixel(const TInput & A)
{
    if(m_nCloudMaskBandIndex== -1)
        return false;

    int val = (int)static_cast<float>(A[m_nCloudMaskBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsLandPixel(const TInput & A)
{
    return (!IsSnowPixel(A) && !IsWaterPixel(A) && !IsCloudPixel(A));
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsRedBand(int index)
{
    if((m_nRedBandIndex != -1) && (index == m_nRedBandIndex)) {
        return true;
    }
    return false;
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetCurrentL2AWeightValue(const TInput & A)
{
    // TODO: Normally, this should not happen so we should log this error and maybe throw an exception
    if(m_nCurrentL2AWeightBandIndex == -1)
        return WEIGHT_NO_DATA;

    return static_cast<float>(A[m_nCurrentL2AWeightBandIndex]);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3AWeightValue(const TInput & A, int offset)
{
    if(!m_bPrevL3ABandsAvailable || m_nPrevL3AWeightBandStartIndex == -1)
        return WEIGHT_NO_DATA;

    return (static_cast<float>(A[m_nPrevL3AWeightBandStartIndex+offset]) / m_fQuantificationValue);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3AWeightedAvDateValue(const TInput & A)
{
    if(!m_bPrevL3ABandsAvailable || m_nPrevL3AWeightedAvDateBandIndex == -1)
        return DATE_NO_DATA;

    return (static_cast<float>(A[m_nPrevL3AWeightedAvDateBandIndex]) / m_fQuantificationValue);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3AReflectanceValue(const TInput & A, int offset)
{
    if(!m_bPrevL3ABandsAvailable || m_nPrevL3AReflectanceBandStartIndex == -1)
        return REFLECTANCE_NO_DATA;

    return static_cast<float>(A[m_nPrevL3AReflectanceBandStartIndex + offset])/m_fQuantificationValue;
}

template< class TInput, class TOutput>
short UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3APixelFlagValue(const TInput & A)
{
    if(!m_bPrevL3ABandsAvailable || m_nPrevL3APixelFlagBandIndex == -1)
        return FLAG_NO_DATA;

    return static_cast<short>(A[m_nPrevL3APixelFlagBandIndex]);
}

template< class TInput, class TOutput>
int UpdateSynthesisFunctor<TInput,TOutput>::GetBlueBandIndex()
{
    return m_nBlueBandIndex;
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsNoDataValue(float fValue, float fNoDataValue)
{
    return fabs(fValue - fNoDataValue) < NO_DATA_EPSILON;
}
