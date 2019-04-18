#include "UpdateSynthesisFunctor_2.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

template< class TInput, class TOutput>
UpdateSynthesisFunctor<TInput,TOutput>::UpdateSynthesisFunctor()
{
    m_fReflQuantifValue = -1;
    m_nCurrentDate = 0;
    m_bPrevL3ABandsAvailable = false;
    m_nNbOfL3AReflectanceBands = 0;
    m_nNbL2ABands = 0;
    m_nL2ABandStartIndex = 0;
    m_nCloudMaskBandIndex = -1;
    m_nSnowMaskBandIndex = -1;
    m_nWaterMaskBandIndex = -1;
    m_nCurrentL2AWeightBandIndex = -1;
    m_nPrevL3AWeightBandStartIndex = -1;
    m_nPrevL3AWeightedAvDateBandIndex = -1;
    m_nPrevL3AReflectanceBandStartIndex = -1;
    m_nPrevL3APixelFlagBandIndex = -1;
    m_nL2ABlueBandIndex = -1;
    m_nL3ABlueBandIndex = -1;
}

template< class TInput, class TOutput>
UpdateSynthesisFunctor<TInput,TOutput>& UpdateSynthesisFunctor<TInput,TOutput>::operator =(const UpdateSynthesisFunctor& copy)
{
    m_fReflQuantifValue = copy.m_fReflQuantifValue;
    m_nCurrentDate = copy.m_nCurrentDate;
    m_bPrevL3ABandsAvailable = copy.m_bPrevL3ABandsAvailable;
    m_nNbOfL3AReflectanceBands = copy.m_nNbOfL3AReflectanceBands;
    m_nNbL2ABands = copy.m_nNbL2ABands;
    m_arrL2ABandPresence = copy.m_arrL2ABandPresence;
    m_nL2ABandStartIndex = copy.m_nL2ABandStartIndex;
    m_nCloudMaskBandIndex = copy.m_nCloudMaskBandIndex;
    m_nSnowMaskBandIndex = copy.m_nSnowMaskBandIndex;
    m_nWaterMaskBandIndex = copy.m_nWaterMaskBandIndex;
    m_nCurrentL2AWeightBandIndex = copy.m_nCurrentL2AWeightBandIndex;
    m_nPrevL3AWeightBandStartIndex = copy.m_nPrevL3AWeightBandStartIndex;
    m_nPrevL3AWeightedAvDateBandIndex = copy.m_nPrevL3AWeightedAvDateBandIndex;
    m_nPrevL3AReflectanceBandStartIndex = copy.m_nPrevL3AReflectanceBandStartIndex;
    m_nPrevL3APixelFlagBandIndex = copy.m_nPrevL3APixelFlagBandIndex;
    m_nL2ABlueBandIndex = copy.m_nL2ABlueBandIndex;
    m_nL3ABlueBandIndex = copy.m_nL3ABlueBandIndex;

    //m_fReflNoDataValue = copy.m_fReflNoDataValue;

    return *this;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::Initialize(const std::vector<int> presenceVect, int nExtractedL2ABandsNo, int nBlueBandIdx,
                                                        bool bHasAppendedPrevL2ABlueBand, bool bPrevL3ABandsAvailable,
                                                        int nDate, float fReflQuantifVal)
{
    m_nCurrentDate = nDate;
    m_fReflQuantifValue = fReflQuantifVal;
    m_bPrevL3ABandsAvailable = bPrevL3ABandsAvailable;
    m_arrL2ABandPresence = presenceVect;

    m_nNbOfL3AReflectanceBands = presenceVect.size();
    m_nNbL2ABands = nExtractedL2ABandsNo;
    // these indexes are 0 based
    m_nL2ABlueBandIndex = nBlueBandIdx;
    m_bHasAppendedPrevL2ABlueBand = bHasAppendedPrevL2ABlueBand;

    // the L2A Reflectance bands start at index 0
    m_nL2ABandStartIndex = 0;
    // the next band after L2A reflectances is the L2A cloud mask
    m_nCloudMaskBandIndex = m_nL2ABandStartIndex + m_nNbL2ABands;
    // the next band after L2A cloud mask is the L2A water mask
    m_nWaterMaskBandIndex = m_nCloudMaskBandIndex + 1;
    // the next band after L2A water mask is the L2A snow mask
    m_nSnowMaskBandIndex = m_nWaterMaskBandIndex + 1;
    // the next band after L2A snow mask is the L2A total weight
    m_nCurrentL2AWeightBandIndex = m_nSnowMaskBandIndex + 1;
    // the next bands after L2A total weight are the L3A Weights
    m_nPrevL3AWeightBandStartIndex = m_nCurrentL2AWeightBandIndex + 1;
    // the next band after L3A weights is the L3A average Weighted date band
    m_nPrevL3AWeightedAvDateBandIndex = m_nPrevL3AWeightBandStartIndex + m_nNbOfL3AReflectanceBands;
    // the next bands after L3A average weighted band are the L3A reflectances bands
    m_nPrevL3AReflectanceBandStartIndex = m_nPrevL3AWeightedAvDateBandIndex + m_nNbOfL3AReflectanceBands;
    // the last band after the L3A reflectances bands is the flags band
    m_nPrevL3APixelFlagBandIndex = m_nPrevL3AReflectanceBandStartIndex + m_nNbOfL3AReflectanceBands;
    // by default, the L3A blue band index follows the l2a blue band index
    // if we have appended an additional band for L3A blue band, update the indexes
    m_nL3ABlueBandIndex = m_bHasAppendedPrevL2ABlueBand ?
                (m_nPrevL3AReflectanceBandStartIndex + m_nNbOfL3AReflectanceBands - 1):
                m_nL2ABlueBandIndex;
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::operator!=( const UpdateSynthesisFunctor & other) const
{
    UNUSED(other);
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
    OutFunctorInfos outInfos(m_nNbOfL3AReflectanceBands);

    ResetCurrentPixelValues(A, outInfos);
    if(IsLandPixel(A)) {
        HandleLandPixel(A, outInfos);
    } else {
        if(IsCloudPixel(A)) {
            // if pixel is cloud or shadow *pixel never observed cloud snow or water free
            HandleCloudOrShadowPixel(A, outInfos);
        } else if(IsSnowPixel(A) || IsWaterPixel(A)) {
            // if pixel is snow or water *replace the reflectance value
            HandleSnowOrWaterPixel(A, outInfos);
        }
    }

    // we will have :
    //      - The weight counter for each pixel bands -> 4 or 6 for 10m and 20m respectively
    //      - The weighted average reflectance bands -> 4 or 6 for 10m and 20m respectively
    //      - one band for weighted average date
    //      - one band for flag with the status of each pixel
    int nTotalOutBandsNo = GetNbOfOutputComponents();
    TOutput var(nTotalOutBandsNo);
    var.SetSize(nTotalOutBandsNo);

    int i;
    int cnt = 0;

    // Weight for B2 for L3A
    for(i = 0; i < m_nNbOfL3AReflectanceBands; i++)
    {
        // Normalize the values
        if(outInfos.m_CurrentPixelWeights[i] < 0) {
            outInfos.m_CurrentPixelWeights[i] = WEIGHT_NO_DATA;
        }
        var[cnt++] = short(outInfos.m_CurrentPixelWeights[i] * WEIGHT_QUANTIF_VALUE);
    }

    // Weighted Average Date L3A
    for(i = 0; i < m_nNbOfL3AReflectanceBands; i++)
    {
        var[cnt++] = outInfos.m_nCurrentPixelWeightedDate[i];
    }

    // Weighted Average Reflectances
    for(i = 0; i < m_nNbOfL3AReflectanceBands; i++)
    {
        // Normalize the values
        if(outInfos.m_CurrentWeightedReflectances[i] < 0) {
            //outInfos.m_CurrentWeightedReflectances[i] = NO_DATA_VALUE;
            var[cnt++] = NO_DATA_VALUE;
        } else {
            // we save back the pixel value but as digital value and not as reflectance
            var[cnt++] = short(outInfos.m_CurrentWeightedReflectances[i] * DEFAULT_COMPOSITION_QUANTIF_VALUE);
            //var[cnt++] = outInfos.m_CurrentWeightedReflectances[i];
        }
    }

    // Pixel status
    for(i = 0; i < m_nNbOfL3AReflectanceBands; i++)
    {
        var[cnt++] = outInfos.m_nCurrentPixelFlag[i];
    }

    return var;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::ResetCurrentPixelValues(const TInput & A, OutFunctorInfos& outInfos)
{
    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        outInfos.m_CurrentPixelWeights[i] = GetPrevL3AWeightValue(A, i);
        outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
        outInfos.m_nCurrentPixelFlag[i] = GetPrevL3APixelFlagValue(A, i);
        outInfos.m_nCurrentPixelWeightedDate[i] = GetPrevL3AWeightedAvDateValue(A, i);
    }
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
     if(fPixelVal < 0) {
        fPixelVal = NO_DATA_VALUE;
    }
    return (fPixelVal/m_fReflQuantifValue);
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleLandPixel(const TInput & A, OutFunctorInfos& outInfos)
{
    bool bAllReflsAreNoData = true;

    // we assume that the reflectance bands start from index 0
    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        // we will always have as output the number of reflectances equal or greater than
        // the number of bands in the current L2A raster for the current resolution
        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
        if(nCurrentBandIndex != -1)
        {
            outInfos.m_nCurrentPixelFlag[i] = IMG_FLG_LAND;
            // "if band is available in the case of LANDSAT 8, some bands are not available\"
            float fCurReflectance = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
            float fPrevReflect = GetPrevL3AReflectanceValue(A, i);
            float fPrevWeight = GetPrevL3AWeightValue(A, i);
            float fCurrentWeight = GetCurrentL2AWeightValue(A);
            short fPrevWeightedDate = GetPrevL3AWeightedAvDateValue(A, i);

            bool bIsPrevReflNoData = IsNoDataValue(fPrevReflect, NO_DATA_VALUE);
            bool bIsCurReflNoData = IsNoDataValue(fCurReflectance, NO_DATA_VALUE);
            if(!bIsCurReflNoData) {
                bAllReflsAreNoData = false;
            }

            if((bIsPrevReflNoData == false) && (bIsCurReflNoData == false)) {
                if(IsNoDataValue(fPrevWeight, 0)) {
                    fPrevWeight = 0;
                }
                if(IsNoDataValue(fPrevWeightedDate, 0)) {
                    fPrevWeightedDate = 0;
                }
                if(IsNoDataValue(fCurrentWeight, 0)) {
                    fCurrentWeight = 0;
                }

                outInfos.m_CurrentWeightedReflectances[i] = (fPrevWeight * fPrevReflect + fCurrentWeight * fCurReflectance) /
                        (fPrevWeight + fCurrentWeight);
                outInfos.m_nCurrentPixelWeightedDate[i] = (short)((fPrevWeight * fPrevWeightedDate + fCurrentWeight * m_nCurrentDate) /
                        (fPrevWeight + fCurrentWeight));
                outInfos.m_CurrentPixelWeights[i] = (fPrevWeight + fCurrentWeight);
            } else {
                if(bIsCurReflNoData) {
                    outInfos.m_CurrentWeightedReflectances[i] = fPrevReflect;
                    outInfos.m_CurrentPixelWeights[i] = fPrevWeight;
                } else {
                    outInfos.m_CurrentWeightedReflectances[i] = fCurReflectance;
                    outInfos.m_CurrentPixelWeights[i] = fCurrentWeight;
                }
                // if we have no data so far, then set also the flag as no data
                if(bIsPrevReflNoData && bIsCurReflNoData) {
                    outInfos.m_nCurrentPixelFlag[i] = IMG_FLG_NO_DATA;
                    outInfos.m_nCurrentPixelWeightedDate[i] = DATE_NO_DATA;
                } else {
                    if (!bIsCurReflNoData){
                        outInfos.m_nCurrentPixelWeightedDate[i] = m_nCurrentDate;
                    }   // otherwise, it remains to the value from the previous product, which might be a valid or no data value
                }
            }
        } else {
            // TODO: This code can be removed as the initialization is made in ResetCurrentPixelValues
            // L2A band missing - as for LANDSAT 8
            outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
            outInfos.m_CurrentPixelWeights[i] = GetPrevL3AWeightValue(A, i);
        }
    }

    // if all reflectances are no data for a pixel, we will keep the previous flag
    if(bAllReflsAreNoData) {
        for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++) {
            outInfos.m_nCurrentPixelFlag[i] = GetPrevL3APixelFlagValue(A, i);
            outInfos.m_nCurrentPixelWeightedDate[i] = GetPrevL3AWeightedAvDateValue(A, i);
        }
    }
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleSnowOrWaterPixel(const TInput & A, OutFunctorInfos& outInfos)
{
    FlagType curFlgType = IsWaterPixel(A) ? IMG_FLG_WATER : IMG_FLG_SNOW;
    bool bCurrentPixelWeightedDateSet = false;

    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
        // band available
        if(nCurrentBandIndex != -1)
        {
            outInfos. m_nCurrentPixelFlag[i] = curFlgType;
            float fPrevWeight = GetPrevL3AWeightValue(A, i);
            // if pixel never observed without cloud, water or snow ON THIS BAND
            // but we might have bands with completely NO_DATA (missing bands in 2 satellites configuration)
            if(IsNoDataValue(fPrevWeight, 0)) {
                float fCurRefl = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
                // check if the current reflectance is valid (maybe we have missing areas)
                // in this case, we will keep the existing reflectance
                if(IsNoDataValue(fCurRefl, NO_DATA_VALUE)) {
                    outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                } else {
                    outInfos.m_CurrentWeightedReflectances[i] = fCurRefl;
                }
                outInfos.m_CurrentPixelWeights[i] = 0;
                if(!bCurrentPixelWeightedDateSet)
                {
                    outInfos.m_nCurrentPixelWeightedDate[i] = m_nCurrentDate;
                    // we do not set the bCurrentPixelWeightedDateSet to TRUE here as we want the next
                    // check of it to have more priority as we might have bands missing in prev L3A
                }
            } else {
                // pixel already observed cloud free, keep the previous weighted average
                float fPrevRefl = GetPrevL3AReflectanceValue(A, i);
                if(IsNoDataValue(fPrevRefl, NO_DATA_VALUE)) {
                    // if we had previously LAND, we need to check for all bands if the prev reflectance is NO_DATA
                    // This might happen for combinations on 2 satellites, when we had only few bands valid, and the rest missing
                    outInfos.m_CurrentWeightedReflectances[i] = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
                    outInfos.m_CurrentPixelWeights[i] = WEIGHT_NO_DATA;
                } else {
                    outInfos.m_CurrentWeightedReflectances[i] = fPrevRefl;
                    outInfos.m_CurrentPixelWeights[i] = fPrevWeight;
                    if(!bCurrentPixelWeightedDateSet)
                    {
                        outInfos.m_nCurrentPixelWeightedDate[i] = GetPrevL3AWeightedAvDateValue(A, i);
                        outInfos.m_nCurrentPixelFlag[i] = IMG_FLG_LAND;
                        bCurrentPixelWeightedDateSet = true;
                    }
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
    for(int i = 0; i<m_nNbOfL3AReflectanceBands; i++)
    {
        short nPrevL3AFlagVal = GetPrevL3APixelFlagValue(A, i);
        // if flagN-1 is no-data => replace nodata with cloud
        if(nPrevL3AFlagVal == IMG_FLG_NO_DATA)
        {

            int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
            // band available
            if(nCurrentBandIndex != -1)
            {
                float fCurReflectance = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
                bool bIsCurReflNoData = IsNoDataValue(fCurReflectance, NO_DATA_VALUE);
                // This step is needed to remove the eventual false clouds when all reflectance bands are NO_DATA
                // and we previously didn't detected anything
                outInfos.m_nCurrentPixelFlag[i] = bIsCurReflNoData ? IMG_FLG_NO_DATA : IMG_FLG_CLOUD;

                outInfos.m_CurrentWeightedReflectances[i] = fCurReflectance;
                outInfos.m_CurrentPixelWeights[i] = 0;
                outInfos.m_nCurrentPixelWeightedDate[i] = m_nCurrentDate;
                //outInfos.m_nCurrentPixelFlag[i] = IMG_FLG_CLOUD;
            } else {
                outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                float fPrevWeight = GetPrevL3AWeightValue(A, i);
                if(IsNoDataValue(fPrevWeight, WEIGHT_NO_DATA)) {
                    outInfos.m_CurrentPixelWeights[i] = WEIGHT_NO_DATA; // TODO: This is not conform to ATBD but seems more natural
                                                                    // in order to have no data for bands that are completely missing
                } else {
                    outInfos.m_CurrentPixelWeights[i] = 0;
                }
            }
        } else {
            if((nPrevL3AFlagVal == IMG_FLG_CLOUD) || (nPrevL3AFlagVal == IMG_FLG_CLOUD_SHADOW))
            {
                float fBlueReflectance = GetL2AReflectanceForPixelVal(A[m_nL2ABlueBandIndex]);
                float fRelBlueReflectance = GetPrevL3AReflectanceValue(A, m_nL3ABlueBandIndex);
                // replace value only if the new reflectance in blue is smaller
                if(fBlueReflectance < fRelBlueReflectance)
                {
                    int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
                    // band available
                    if(nCurrentBandIndex != -1)
                    {
                        outInfos.m_CurrentWeightedReflectances[i] = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
                        outInfos.m_nCurrentPixelWeightedDate[i] = m_nCurrentDate;
                        outInfos.m_nCurrentPixelFlag[i] = IMG_FLG_CLOUD;
                    } else {
                        outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                        outInfos.m_nCurrentPixelFlag[i] = nPrevL3AFlagVal;
                    }
                    outInfos.m_CurrentPixelWeights[i] = 0;
                } else {
                    outInfos.m_CurrentWeightedReflectances[i] = GetPrevL3AReflectanceValue(A, i);
                    outInfos.m_nCurrentPixelWeightedDate[i] = GetPrevL3AWeightedAvDateValue(A, i);
                    outInfos.m_CurrentPixelWeights[i] = 0;
                    outInfos.m_nCurrentPixelFlag[i] = nPrevL3AFlagVal;
                }
            } else if(nPrevL3AFlagVal == IMG_FLG_LAND) {
                // if we had previously LAND, we need to check for all bands if the prev reflectance is NO_DATA
                // This might happen for combinations on 2 satellites, when we had only few bands valid, and the rest missing
                int nCurrentBandIndex = GetAbsoluteL2ABandIndex(i);
                // band available
                if(nCurrentBandIndex != -1)
                {
                    float fPrevRefl = GetPrevL3AReflectanceValue(A, i);
                    if(IsNoDataValue(fPrevRefl, NO_DATA_VALUE)) {
                        outInfos.m_CurrentWeightedReflectances[i] = GetL2AReflectanceForPixelVal(A[nCurrentBandIndex]);
                    }
                }
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
    // No data should not be considered as snow
    if(IsNoDataValue(val, 0)) {
        return false;
    }
    return (val != 0);
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsWaterPixel(const TInput & A)
{
    if(m_nWaterMaskBandIndex == -1)
        return false;

    int val = (int)static_cast<float>(A[m_nWaterMaskBandIndex]);
    // No data should not be considered as water
    if(IsNoDataValue(val, 0)) {
        return false;
    }
    return (val != 0);
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsCloudPixel(const TInput & A)
{
    if(m_nCloudMaskBandIndex== -1)
        return false;

    int val = (int)static_cast<float>(A[m_nCloudMaskBandIndex]);
    // No data should not be considered as cloud
    if(IsNoDataValue(val, 0)) {
        return false;
    }
    return (val != 0);
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsLandPixel(const TInput & A)
{
    return (!IsSnowPixel(A) && !IsWaterPixel(A) && !IsCloudPixel(A));
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetCurrentL2AWeightValue(const TInput & A)
{
    // TODO: Normally, this should not happen so we should log this error and maybe throw an exception
    if(m_nCurrentL2AWeightBandIndex == -1)
        return WEIGHT_NO_DATA;

    // TODO: the Total Weight computed for L2A does not have a quantification value yet. Should it be added as now it is just a temp file?
    return static_cast<float>(A[m_nCurrentL2AWeightBandIndex]);// / m_fQuantificationValue);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3AWeightValue(const TInput & A, int offset)
{
    if(!m_bPrevL3ABandsAvailable || m_nPrevL3AWeightBandStartIndex == -1)
        return WEIGHT_NO_DATA;

    // the L3A bands presence follow the L2A bands presence.
    // If we have missing L3A bands in input, then we need to recompute the relative offset
    int relIdx = offset;//m_arrL2ABandPresence[offset];
    if(relIdx != -1) {
        return (static_cast<float>(A[m_nPrevL3AWeightBandStartIndex+relIdx]) / WEIGHT_QUANTIF_VALUE);
    }
    return WEIGHT_NO_DATA;
}

template< class TInput, class TOutput>
short UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3AWeightedAvDateValue(const TInput & A, int offset)
{
    if(!m_bPrevL3ABandsAvailable || m_nPrevL3AWeightedAvDateBandIndex == -1)
        return DATE_NO_DATA;

    // the L3A bands presence follow the L2A bands presence.
    // If we have missing L3A bands in input, then we need to recompute the relative offset
    int relIdx = offset;//m_arrL2ABandPresence[offset];
    if(relIdx != -1) {
        return (static_cast<short>(A[m_nPrevL3AWeightedAvDateBandIndex+relIdx]));
    }
    return DATE_NO_DATA;
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3AReflectanceValue(const TInput & A, int offset)
{
    if(!m_bPrevL3ABandsAvailable || m_nPrevL3AReflectanceBandStartIndex == -1)
        return NO_DATA_VALUE;

    // the L3A bands presence follow the L2A bands presence.
    // If we have missing L3A bands in input, then we need to recompute the relative offset
    int relIdx = offset;//m_arrL2ABandPresence[offset];
    if(relIdx != -1) {
        // Here we convert to quantified value only if different of NO_DATA
        float fPixelVal = static_cast<float>(A[m_nPrevL3AReflectanceBandStartIndex + relIdx]);
        if(fPixelVal < 0) {
            return NO_DATA_VALUE;
        }
        return fPixelVal/DEFAULT_COMPOSITION_QUANTIF_VALUE;
    }
    return NO_DATA_VALUE;
}

template< class TInput, class TOutput>
short UpdateSynthesisFunctor<TInput,TOutput>::GetPrevL3APixelFlagValue(const TInput & A, int offset)
{
    if(!m_bPrevL3ABandsAvailable || m_nPrevL3APixelFlagBandIndex == -1)
        return IMG_FLG_NO_DATA;

    // the L3A bands presence follow the L2A bands presence.
    // If we have missing L3A bands in input, then we need to recompute the relative offset
    int relIdx = offset;//m_arrL2ABandPresence[offset];
    if(relIdx != -1) {
        return (static_cast<short>(A[m_nPrevL3APixelFlagBandIndex+relIdx] + 0.5));
    }
    return IMG_FLG_NO_DATA;
}

template< class TInput, class TOutput>
int UpdateSynthesisFunctor<TInput,TOutput>::GetBlueBandIndex()
{
    return m_nL2ABlueBandIndex;
}

template< class TInput, class TOutput>
bool UpdateSynthesisFunctor<TInput,TOutput>::IsNoDataValue(float fValue, float fNoDataValue)
{
    return ((fValue + NO_DATA_EPSILON) < 0) || (fabs(fValue - fNoDataValue) < NO_DATA_EPSILON);
}
