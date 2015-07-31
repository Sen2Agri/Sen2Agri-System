#include "updatesynthesisfunctor.h"

template< class TInput, class TOutput>
UpdateSynthesisFunctor<TInput,TOutput>::UpdateSynthesisFunctor()
{
    m_fQuantificationValue = -1;
    m_bPrevL3ABandsAvailable = false;
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
    m_nNbOfL3AReflectanceBands = 0;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::Initialize(SensorType sensorType, ResolutionType resolution,
                                                        bool bPrevL3ABandsAvailable)
{
    m_sensorType = sensorType;
    m_resolution = resolution;
    m_bPrevL3ABandsAvailable = bPrevL3ABandsAvailable;
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
            m_nPrevL3AWeightBandStartIndex = S2_L2A_10M_L3A_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = S2_L2A_10M_L3A_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = S2_L2A_10M_L3A_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = S2_L2A_10M_L3A_PIXEL_STATUS_IDX;
            m_nRedBandIndex = S2_L2A_10M_RED_BAND_IDX;
            m_nBlueBandIndex = S2_L2A_10M_BLUE_BAND_IDX;

        } else if (sensorType == SENSOR_L8) {

            m_nNbL2ABands = L8_L2A_10M_BANDS_NO;
            m_nL2ABandStartIndex = L8_L2A_10M_BANDS_START_IDX;
            m_nCloudMaskBandIndex = L8_L2A_10M_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = L8_L2A_10M_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = L8_L2A_10M_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = L8_L2A_10M_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = L8_L2A_10M_L3A_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = L8_L2A_10M_L3A_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = L8_L2A_10M_L3A_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = L8_L2A_10M_L3A_PIXEL_STATUS_IDX;
            m_nRedBandIndex = L8_L2A_10M_RED_BAND_IDX;
            m_nBlueBandIndex = L8_L2A_10M_BLUE_BAND_IDX;

        } else {
            // TODO: Throw an error
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
            m_nPrevL3AWeightBandStartIndex = S2_L2A_20M_L3A_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = S2_L2A_20M_L3A_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = S2_L2A_20M_L3A_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = S2_L2A_20M_L3A_PIXEL_STATUS_IDX;
            m_nRedBandIndex = S2_L2A_20M_RED_BAND_IDX;
            m_nBlueBandIndex = S2_L2A_20M_BLUE_BAND_IDX;

        } else if (sensorType == SENSOR_L8) {

            m_nNbL2ABands = L8_L2A_20M_BANDS_NO;
            m_nL2ABandStartIndex = L8_L2A_20M_BANDS_START_IDX;
            m_nCloudMaskBandIndex = L8_L2A_20M_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = L8_L2A_20M_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = L8_L2A_20M_WATER_MASK_IDX;
            m_nCurrentL2AWeightBandIndex = L8_L2A_20M_TOTAL_WEIGHT_IDX;
            m_nPrevL3AWeightBandStartIndex = L8_L2A_20M_L3A_WEIGHT_START_IDX;
            m_nPrevL3AWeightedAvDateBandIndex = L8_L2A_20M_L3A_W_AV_DATE_IDX;
            m_nPrevL3AReflectanceBandStartIndex = L8_L2A_20M_L3A_REFL_START_IDX;
            m_nPrevL3APixelFlagBandIndex = L8_L2A_20M_L3A_PIXEL_STATUS_IDX;
            m_nRedBandIndex = L8_L2A_20M_RED_BAND_IDX;
            m_nBlueBandIndex = L8_L2A_20M_BLUE_BAND_IDX;

        } else {
            // TODO: Throw an error
        }
    } else {
        // TODO: Throw an error
    }
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetReflectanceQuantificationValue(float fQuantifVal)
{
    m_fQuantificationValue = fQuantifVal;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetCurrentDate(int nDate)
{
    m_nCurrentDate = nDate;
}

template< class TInput, class TOutput>
TOutput UpdateSynthesisFunctor<TInput,TOutput>::operator()( const TInput & A ) const
{
    ResetCurrentPixelValues();
    if(IsLandPixel(A))
    {
        HandleLandPixel(A);
    } else {
        if(IsSnowPixel(A) || IsWaterPixel(A))
        {
            // if pixel is snow or water *replace the reflectance value
            HandleSnowOrWaterPixel(A);
        } else {
            // if pixel is cloud or shadow *pixel never observed cloud snow or water free
            HandleCloudOrShadowPixel(A);
        }
    }

    TOutput var(m_nNbOfL3AReflectanceBands + 3);
    var.SetSize(m_nNbOfL3AReflectanceBands + 3);
    int i;
    int cnt = 0;
    for(i = 0; i < m_nNbOfL3AReflectanceBands; i++)
    {
        var[cnt++] = m_CurrentWeightedReflectances[i];
    }
    for(i = 0; i < m_nNbOfL3AReflectanceBands; i++)
    {
        var[cnt++] = m_CurrentPixelWeights[i];
    }
    var[cnt++] = m_fCurrentPixelFlag;
    var[cnt++] = m_fCurrentPixelWeightedDate;

    return var;
}

// private functions

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::ResetCurrentPixelValues()
{
    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        m_CurrentPixelWeights[i] = WEIGHT_NO_DATA;
        m_CurrentWeightedReflectances[i] = REFLECTANCE_NO_DATA;
    }
    m_fCurrentPixelFlag = FLAG_NO_DATA;
    m_fCurrentPixelWeightedDate = DATE_NO_DATA;
}

template< class TInput, class TOutput>
int UpdateSynthesisFunctor<TInput,TOutput>::GetAbsoluteL2ABandIndex(int index)
{
    // we know that the L2A bands are always the first bands
    if(index < m_nNbL2ABands)
        return (m_nL2ABandStartIndex + index);
    return -1;
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetL2AReflectanceForPixelVal(float fPixelVal)
{
    return (fPixelVal/m_fQuantificationValue);
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsLandPixel(const TInput & A)
{
    return (!IsSnowPixel(A) && !IsWaterPixel(A) && !IsCloudPixel(A) && !IsCloudShadowPixel(A));
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsSnowPixel(const TInput & A)
{
    // TODO: Here maybe we should get from metadata the value for no-snow (usually is 0 ... )
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

    int val = (int)static_cast<float>(A[m_nCurrentL2AWeightBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3AWeightValue(const TInput & A, int offset)
{
    if(m_nPrevL3AWeightBandStartIndex == -1)
        return WEIGHT_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevL3AWeightBandStartIndex+offset]);
    return (val != 0);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3AWeightedAvDateValue(const TInput & A)
{
    if(m_nPrevL3AWeightedAvDateBandIndex == -1)
        return DATE_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevL3AWeightedAvDateBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3AReflectanceValue(const TInput & A, int offset)
{
    if(m_nPrevL3AReflectanceBandStartIndex == -1)
        return REFLECTANCE_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevL3AReflectanceBandStartIndex + offset]);
    return (val != 0);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3APixelFlagValue(const TInput & A)
{
    if(m_nPrevL3APixelFlagBandIndex == -1)
        return FLAG_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevL3APixelFlagBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
int UpdateSynthesisFunctor<TInput,TOutput>::GetBlueBandIndex()
{
    return m_nBlueBandIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleLandPixel(const TInput & A)
{
    // we assume that the reflectance bands start from index 0
    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        m_fCurrentPixelFlag = LAND;

        // we will always have as output the number of reflectances equal or greater than
        // the number of bands in the current L2A raster for the current resolution
        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
        if(nCurrentBandIndex != -1)
        {
            // "if band is available in the case of LANDSAT 8, some bands are not available\"
            float fCurReflectance = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
            float fPrevReflect = GetPrevL3AReflectanceValue(A, i);
            float fPrevWeight = GetPrevL3AWeightValue(A, i);
            float fCurrentWeight = GetCurrentWeight(A);
            float fPrevWeightedDate = GetPrevL3AWeightedAvDateValue(A);
            m_CurrentWeightedReflectances[i] = (fPrevWeight * fPrevReflect + fCurrentWeight * fCurReflectance) /
                    (fPrevWeight + fCurrentWeight);
            m_fCurrentPixelWeightedDate = (fPrevWeight * fPrevWeightedDate + fCurrentWeight * m_nCurrentDate) /
                    (fPrevWeight + fCurrentWeight);
            m_CurrentPixelWeights[i] = (fPrevWeight + fCurrentWeight);

        } else {
            // L2A band missing - as for LANDSAT 8
            m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
            m_CurrentPixelWeights[i] = GetPrevL3AWeightValue(A, i);
            if(IsRedBand(i))
            {
                m_fCurrentPixelWeightedDate = GetPrevL3AWeightedAvDateValue(A);
            }
        }
    }
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleSnowOrWaterPixel(const TInput & A)
{
    if(IsWaterPixel(A)) {
        m_fCurrentPixelFlag = WATER;
    } else {
        m_fCurrentPixelFlag = SNOW;
    }
    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
        // band available
        if(nCurrentBandIndex != -1)
        {
            float fPrevWeight = GetPrevL3AWeightValue(A, i);
            // if pixel never observed without cloud, water or snow
            if(fPrevWeight == 0) {
                m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                m_CurrentPixelWeights[i] = 0;
                if(IsRedBand(i))
                {
                    m_fCurrentPixelWeightedDate = m_nCurrentDate;
                }
            } else {
                // pixel already observed cloud free, keep the previous weighted average
                m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                m_CurrentPixelWeights[i] = fPrevWeight;
                if(IsRedBand(i))
                {
                    m_fCurrentPixelWeightedDate = GetPrevL3AWeightedAvDateValue(A);
                    m_fCurrentPixelFlag = LAND;
                }

            }
        } else {
            // band not available, keep previous values
            m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
            m_CurrentPixelWeights[i] = GetPrevL3AWeightValue(A, i);
            // TODO: In algorithm says nothing about WDate
        }
    }
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleCloudOrShadowPixel(const TInput &A)
{
    // if flagN-1 is no-data => replace nodata with cloud
    if(m_fCurrentPixelFlag == FLAG_NO_DATA)
    {
        for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
        {
            int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
            // band available
            if(nCurrentBandIndex != -1)
            {
                m_CurrentWeightedReflectances[i] = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);;
                m_CurrentPixelWeights[i] = 0;
                if(IsRedBand(i))
                {
                    m_fCurrentPixelWeightedDate = m_nCurrentDate;
                    m_fCurrentPixelFlag = CLOUD;
                }
            } else {
                m_CurrentWeightedReflectances[i] = WEIGHT_NO_DATA;
                m_CurrentPixelWeights[i] = 0;
            }
        }
    } else {
        if((m_fCurrentPixelFlag == CLOUD) || (m_fCurrentPixelFlag == CLOUD_SHADOW))
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
                    float fCurReflectance = GetPrevL3AReflectanceValue(A, i);
                    if(fBlueReflectance < fCurReflectance)
                    {

                        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
                        // band available
                        if(nCurrentBandIndex != -1)
                        {
                            m_CurrentWeightedReflectances[i] = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
                            m_CurrentPixelWeights[i] = 0;
                            if(IsRedBand(i))
                            {
                                m_fCurrentPixelWeightedDate = m_nCurrentDate;
                                m_fCurrentPixelFlag = CLOUD;
                            }
                        } else {
                            m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                            m_CurrentPixelWeights[i] = 0;
                        }
                    }
                }
            }
        } else {
            m_fCurrentPixelFlag = GetPrevL3APixelFlagValue(A);
            for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
            {
                m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                m_fCurrentPixelWeightedDate = GetPrevL3AWeightedAvDateValue(A);
                m_CurrentPixelWeights[i] = 0;
            }

        }
    }
}
