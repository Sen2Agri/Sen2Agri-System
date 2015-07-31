#include "updatesynthesisfunctor.h"

#define FLAG_NO_DATA -1.0f
#define DATE_NO_DATA -1.0f
#define REFLECTANCE_NO_DATA -1.0f
#define WEIGHT_NO_DATA -1.0f


#define S2_L2A_10M_BANDS_NO     4
#define L8_L2A_10M_BANDS_NO     3
#define WEIGHTED_REFLECTANCE_10M_BANDS_NO   S2_L2A_10M_BANDS_NO

#define S2_L2A_10M_BANDS_START_IDX      0
#define S2_L2A_10M_CLD_MASK_IDX         4
#define S2_L2A_10M_WATER_MASK_IDX       5
#define S2_L2A_10M_SNOW_MASK_IDX        6
#define S2_L2A_10M_TOTAL_WEIGHT_IDX     7
#define S2_L2A_10M_L3A_WEIGHT_START_IDX 8
#define S2_L2A_10M_L3A_W_AV_DATE_IDX    12
#define S2_L2A_10M_L3A_REFL_START_IDX   13
#define S2_L2A_10M_L3A_PIXEL_STATUS_IDX 17

#define L8_L2A_10M_BANDS_START_IDX      0
#define L8_L2A_10M_CLD_MASK_IDX         3
#define L8_L2A_10M_WATER_MASK_IDX       4
#define L8_L2A_10M_SNOW_MASK_IDX        5
#define L8_L2A_10M_TOTAL_WEIGHT_IDX     6
#define L8_L2A_10M_L3A_WEIGHT_START_IDX 7
#define L8_L2A_10M_L3A_W_AV_DATE_IDX    11
#define L8_L2A_10M_L3A_REFL_START_IDX   12
#define L8_L2A_10M_L3A_PIXEL_STATUS_IDX 16

// 20M Positions Definition
#define S2_L2A_20M_BANDS_NO     6
#define L8_L2A_20M_BANDS_NO     3
#define WEIGHTED_REFLECTANCE_20M_BANDS_NO   S2_L2A_20M_BANDS_NO

#define S2_L2A_20M_BANDS_START_IDX      0
#define S2_L2A_20M_CLD_MASK_IDX         6
#define S2_L2A_20M_WATER_MASK_IDX       7
#define S2_L2A_20M_SNOW_MASK_IDX        8
#define S2_L2A_20M_TOTAL_WEIGHT_IDX     9
#define S2_L2A_20M_L3A_WEIGHT_START_IDX 10
#define S2_L2A_20M_L3A_W_AV_DATE_IDX    16
#define S2_L2A_20M_L3A_REFL_START_IDX   17
#define S2_L2A_20M_L3A_PIXEL_STATUS_IDX 23

#define L8_L2A_20M_BANDS_START_IDX      0
#define L8_L2A_20M_CLD_MASK_IDX         3
#define L8_L2A_20M_WATER_MASK_IDX       4
#define L8_L2A_20M_SNOW_MASK_IDX        5
#define L8_L2A_20M_TOTAL_WEIGHT_IDX     6
#define L8_L2A_20M_L3A_WEIGHT_START_IDX 7
#define L8_L2A_20M_L3A_W_AV_DATE_IDX    13
#define L8_L2A_20M_L3A_REFL_START_IDX   14
#define L8_L2A_20M_L3A_PIXEL_STATUS_IDX 20


template< class TInput, class TOutput>
UpdateSynthesisFunctor<TInput,TOutput>::UpdateSynthesisFunctor()
{
    //TODO: initialize m_CurrentWeightedReflectances

    m_fQuantificationValue = -1;
    m_bPrevL3ABandsAvailable = false;
    m_nCloudMaskBandIndex = -1;
    m_nSnowMaskBandIndex = -1;
    m_nWaterMaskBandIndex = -1;
    m_nPrevWeightBandIndex = -1;
    m_nPrevWeightedAvDateBandIndex = -1;
    m_nPrevReflectanceBandIndex = -1;
    m_nPrevPixelFlagBandIndex = -1;
    m_nRedBandIndex = -1;
    m_nNbOfReflectanceBands = 0;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::Initialize(SensorType sensorType, ResolutionType resolution,
                                                        bool bPrevL3ABandsAvailable)
{
    m_sensorType = sensorType;
    m_resolution = resolution;
    m_bPrevL3ABandsAvailable = bPrevL3ABandsAvailable;
    if(resolution == RES_10M) {
        m_nNbOfReflectanceBands = WEIGHTED_REFLECTANCE_10M_BANDS_NO;
        if(sensorType == SENSOR_S2)
        {
#define S2_L2A_10M_BANDS_START_IDX      0
#define S2_L2A_10M_CLD_MASK_IDX         4
#define S2_L2A_10M_WATER_MASK_IDX       5
#define S2_L2A_10M_SNOW_MASK_IDX        6
#define S2_L2A_10M_TOTAL_WEIGHT_IDX     7
#define S2_L2A_10M_L3A_WEIGHT_START_IDX 8
#define S2_L2A_10M_L3A_W_AV_DATE_IDX    12
#define S2_L2A_10M_L3A_REFL_START_IDX   13
#define S2_L2A_10M_L3A_PIXEL_STATUS_IDX 17

            m_nNbL2ABands = S2_L2A_10M_BANDS_NO;
            m_nCloudMaskBandIndex = S2_L2A_10M_CLD_MASK_IDX;
            m_nSnowMaskBandIndex = S2_L2A_10M_SNOW_MASK_IDX;
            m_nWaterMaskBandIndex = S2_L2A_10M_WATER_MASK_IDX;
            m_nPrevWeightBandIndex = -1;
            m_nPrevWeightedAvDateBandIndex = -1;
            m_nPrevReflectanceBandIndex = -1;
            m_nPrevPixelFlagBandIndex = -1;
            m_nRedBandIndex = -1;
        } else if (sensorType == SENSOR_L8) {

        } else {
            // TODO: Throw an error
        }
    } else if(resolution == RES_20M) {
        if(sensorType == SENSOR_S2)
        {
        } else if (sensorType == SENSOR_L8) {

        } else {
            // TODO: Throw an error
        }
    } else {
        // TODO: Throw an error
    }

    if(sensorType == SENSOR_S2)
    {
        if(resolution == RES_10M) {
            m_nNbOfReflectanceBands = WEIGHTED_REFLECTANCE_10M_BANDS_NO;
            m_nNbL2ABands = S2_L2A_10M_BANDS_NO;
            m_nCloudMaskBandIndex = -1;
            m_nSnowMaskBandIndex = -1;
            m_nWaterMaskBandIndex = -1;
            m_nPrevWeightBandIndex = -1;
            m_nPrevWeightedAvDateBandIndex = -1;
            m_nPrevReflectanceBandIndex = -1;
            m_nPrevPixelFlagBandIndex = -1;
            m_nRedBandIndex = -1;
        } else if(resolution == RES_20M) {
            m_nNbOfReflectanceBands = WEIGHTED_REFLECTANCE_20M_BANDS_NO;
            m_nNbL2ABands = S2_L2A_20M_BANDS_NO;

            m_nCloudMaskBandIndex = -1;
            m_nSnowMaskBandIndex = -1;
            m_nWaterMaskBandIndex = -1;
            m_nPrevWeightBandIndex = -1;
            m_nPrevWeightedAvDateBandIndex = -1;
            m_nPrevReflectanceBandIndex = -1;
            m_nPrevPixelFlagBandIndex = -1;
            m_nRedBandIndex = -1;
        } else {
            // TODO: Throw an error
        }
    } else if (sensorType == SENSOR_L8) {
        if(resolution == RES_10M) {
            m_nNbOfReflectanceBands = 0;
            m_nNbL2ABands = 4;

            m_nCloudMaskBandIndex = -1;
            m_nSnowMaskBandIndex = -1;
            m_nWaterMaskBandIndex = -1;
            m_nPrevWeightBandIndex = -1;
            m_nPrevWeightedAvDateBandIndex = -1;
            m_nPrevReflectanceBandIndex = -1;
            m_nPrevPixelFlagBandIndex = -1;
            m_nRedBandIndex = -1;
        } else if(resolution == RES_20M) {
            m_nNbOfReflectanceBands = 0;
            m_nNbL2ABands = 4;

            m_nCloudMaskBandIndex = -1;
            m_nSnowMaskBandIndex = -1;
            m_nWaterMaskBandIndex = -1;
            m_nPrevWeightBandIndex = -1;
            m_nPrevWeightedAvDateBandIndex = -1;
            m_nPrevReflectanceBandIndex = -1;
            m_nPrevPixelFlagBandIndex = -1;
            m_nRedBandIndex = -1;
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

    TOutput var(m_nNbOfReflectanceBands + 3);
    var.SetSize(m_nNbOfReflectanceBands + 3);
    int i;
    int cnt = 0;
    for(i = 0; i < m_nNbOfReflectanceBands; i++)
    {
        var[cnt++] = m_CurrentWeightedReflectances[i];
    }
    for(i = 0; i < m_nNbOfReflectanceBands; i++)
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
    for(int i = 0; i<m_nNbOfReflectanceBands; i++)
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
        return index;
    return -1;
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetReflectanceForPixelVal(float fPixelVal)
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
    if((m_resolution == RES_10M) && (index == 2)) {
        return true;
    }
    return false;
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevWeightValue(const TInput & A)
{
    if(m_nPrevWeightBandIndex == -1)
        return WEIGHT_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevWeightBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevWeightedAvDateValue(const TInput & A)
{
    if(m_nPrevWeightedAvDateBandIndex == -1)
        return DATE_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevWeightedAvDateBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevReflectanceValue(const TInput & A)
{
    if(m_nPrevReflectanceBandIndex == -1)
        return REFLECTANCE_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevReflectanceBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevPixelFlagValue(const TInput & A)
{
    if(m_nPrevPixelFlagBandIndex == -1)
        return FLAG_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevPixelFlagBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
int UpdateSynthesisFunctor<TInput,TOutput>::GetBlueBandIndex()
{
    if(m_resolution == RES_10M) {
        return 0;
    }
    return -1;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleLandPixel(const TInput & A)
{
    // we assume that the reflectance bands start from index 0
    for(int i = 0; i<m_nNbOfReflectanceBands; i++)
    {
        m_fCurrentPixelFlag = LAND;

        // we will always have as output the number of reflectances equal or greater than
        // the number of bands in the current L2A raster for the current resolution
        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
        if(nCurrentBandIndex != -1)
        {
            // "if band is available in the case of LANDSAT 8, some bands are not available\"
            float fCurReflectance = GetReflectanceForPixelVal(A[nCurrentBandIndex]);
            float fPrevReflect = GetPrevReflectance(A);
            float fPrevWeight = GetPrevWeightValue(A);
            float fCurrentWeight = GetCurrentWeight(A);
            float fPrevWeightedDate = GetPrevWeightedAvDateValue(A);
            m_CurrentWeightedReflectances[i] = (fPrevWeight * fPrevReflect + fCurrentWeight * fCurReflectance) /
                    (fPrevWeight + fCurrentWeight);
            m_fCurrentPixelWeightedDate = (fPrevWeight * fPrevWeightedDate + fCurrentWeight * m_nCurrentDate) /
                    (fPrevWeight + fCurrentWeight);
            m_CurrentPixelWeights[i] = (fPrevWeight + fCurrentWeight);

        } else {
            // L2A band missing - as for LANDSAT 8
            m_CurrentWeightedReflectances[i] = GetPrevReflectance(A);
            m_CurrentPixelWeights[i] = GetPrevWeightValue(A);
            if(IsRedBand(i))
            {
                m_fCurrentPixelWeightedDate = GetPrevWeightedAvDateValue(A);
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
    for(int i = 0; i<m_nNbOfReflectanceBands; i++)
    {
        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
        // band available
        if(nCurrentBandIndex != -1)
        {
            float fPrevWeight = GetPrevWeightValue(A);
            // if pixel never observed without cloud, water or snow
            if(fPrevWeight == 0) {
                m_CurrentWeightedReflectances[i] = GetPrevReflectance(A);
                m_CurrentPixelWeights[i] = 0;
                if(IsRedBand(i))
                {
                    m_fCurrentPixelWeightedDate = m_nCurrentDate;
                }
            } else {
                // pixel already observed cloud free, keep the previous weighted average
                m_CurrentWeightedReflectances[i] = GetPrevReflectance(A);
                m_CurrentPixelWeights[i] = fPrevWeight;
                if(IsRedBand(i))
                {
                    m_fCurrentPixelWeightedDate = GetPrevWeightedAvDateValue(A);
                    m_fCurrentPixelFlag = LAND;
                }

            }
        } else {
            // band not available, keep previous values
            m_CurrentWeightedReflectances[i] = GetPrevReflectance(A);
            m_CurrentPixelWeights[i] = GetPrevWeightValue(A);
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
        for(int i = 0; i<m_nNbOfReflectanceBands; i++)
        {
            int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
            // band available
            if(nCurrentBandIndex != -1)
            {
                m_CurrentWeightedReflectances[i] = GetReflectanceForPixelVal(A[nCurrentBandIndex]);;
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
                float fBlueReflectance = GetReflectanceForPixelVal(A[nBlueBandIdx]);
                float fCurReflectance = GetPrevReflectance(A);
                if(fBlueReflectance < fCurReflectance)
                {
                    for(int i = 0; i<m_nNbOfReflectanceBands; i++)
                    {
                        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
                        // band available
                        if(nCurrentBandIndex != -1)
                        {
                            m_CurrentWeightedReflectances[i] = GetReflectanceForPixelVal(A[nCurrentBandIndex]);
                            m_CurrentPixelWeights[i] = 0;
                            if(IsRedBand(i))
                            {
                                m_fCurrentPixelWeightedDate = m_nCurrentDate;
                                m_fCurrentPixelFlag = CLOUD;
                            }
                        } else {
                            m_CurrentWeightedReflectances[i] = GetPrevReflectance(A);
                            m_CurrentPixelWeights[i] = 0;
                        }
                    }
                }
            }
        } else {
            m_fCurrentPixelFlag = GetPrevPixelFlagValue(A);
            for(int i = 0; i<m_nNbOfReflectanceBands; i++)
            {
                m_CurrentWeightedReflectances[i] = GetPrevReflectance(A);
                m_fCurrentPixelWeightedDate = GetPrevWeightedAvDateValue(A);
                m_CurrentPixelWeights[i] = 0;
            }

        }
    }
}
