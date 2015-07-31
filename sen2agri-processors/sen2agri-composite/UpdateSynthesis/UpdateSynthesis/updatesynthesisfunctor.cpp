#include "updatesynthesisfunctor.h"

#define FLAG_NO_DATA -1.0f
#define DATE_NO_DATA -1.0f
#define REFLECTANCE_NO_DATA -1.0f
#define WEIGHT_NO_DATA -1.0f

template< class TInput, class TOutput>
UpdateSynthesisFunctor<TInput,TOutput>::UpdateSynthesisFunctor()
{
    m_fQuantificationValue = -1;
    m_nTotalNbOfBands = 0;
    m_nCloudShadowMaskBandIndex = -1;
    m_nSnowMaskBandIndex = -1;
    m_nWaterMaskBandIndex = -1;
    m_nPrevWeightBandIndex = -1;
    m_nPrevWeightedAvDateBandIndex = -1;
    m_nPrevWeightedAvReflectanceBandIndex = -1;
    m_nPrevPixelStatusBandIndex = -1;
    m_nRedBandIndex = -1;
    m_nNbOfReflectanceBands = 0;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetReflectanceQuantificationValue(float fQuantifVal)
{
    m_fQuantificationValue = fQuantifVal;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetTotalNbOfBands(int nBandsNo)
{
    m_nTotalNbOfBands = nBandsNo;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetCloudMaskBandIndex(int nIndex)
{
    m_nCloudMaskBandIndex = nIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetCloudShadowMaskBandIndex(int nIndex)
{
    m_nCloudShadowMaskBandIndex = nIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetSnowMaskBandIndex(int nIndex)
{
    m_nSnowMaskBandIndex = nIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetWaterMaskBandIndex(int nIndex)
{
    m_nWaterMaskBandIndex = nIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetPrevWeightBandIndex(int nIndex)
{
    m_nPrevWeightBandIndex = nIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetWeightedAvDateBandIndex(int nIndex)
{
    m_nPrevWeightedAvDateBandIndex = nIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetWeightedAvReflectanceBandIndex(int nIndex)
{
    m_nPrevWeightedAvReflectanceBandIndex = nIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetPixelStatusBandIndex(int nIndex)
{
    m_nPrevPixelStatusBandIndex = nIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetRedBandIndex(int nIndex)
{
    m_nRedBandIndex = nIndex;
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::SetNbOfReflectanceBands(int nReflectanceBandNo)
{
    // TODO: this could be also hardcoded
    // but also could be added a new degree of flexibility by adding the indexof the first reflectance band
    m_nNbOfReflectanceBands = nReflectanceBandNo;
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
    // TODO: Here we should return a raster with 4 bands for
    //  float m_fCurrentPixelWeight;
    //  m_fCurrentPixelReflectance;
    //  m_fCurrentPixelFlag;
    //  m_fCurrentPixelWeightedDate;
    TOutput var(4);
    //var.SetSize(4);
    var[0] = m_fCurrentPixelWeight;
    var[1] = m_fCurrentPixelReflectance;
    var[2] = m_fCurrentPixelFlag;
    var[3] = m_fCurrentPixelWeightedDate;

    return var;
}

// private functions

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::ResetCurrentPixelValues()
{
    m_fCurrentPixelWeight = WEIGHT_NO_DATA;
    m_fCurrentPixelReflectance = REFLECTANCE_NO_DATA;
    m_fCurrentPixelFlag = FLAG_NO_DATA;
    m_fCurrentPixelWeightedDate = DATE_NO_DATA;
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::ComputeReflectanceForPixelVal(float fPixelVal)
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
bool UpdateSynthesisFunctor<TInput,TOutput>::IsCloudShadowPixel(const TInput & A)
{
    if(m_nCloudShadowMaskBandIndex == -1)
        return false;

    int val = (int)static_cast<float>(A[m_nCloudShadowMaskBandIndex]);
    return (val != 0);
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
float UpdateSynthesisFunctor<TInput,TOutput>::GetPrevWeightedAvReflectanceValue(const TInput & A)
{
    if(m_nPrevWeightedAvReflectanceBandIndex == -1)
        return REFLECTANCE_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevWeightedAvReflectanceBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
float UpdateSynthesisFunctor<TInput,TOutput>::GetPixelStatusValue(const TInput & A)
{
    if(m_nPrevPixelStatusBandIndex == -1)
        return FLAG_NO_DATA;

    int val = (int)static_cast<float>(A[m_nPrevPixelStatusBandIndex]);
    return (val != 0);
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleLandPixel(const TInput & A)
{
    // we assume that the reflectance bands start from index 0
    for(int i = 0; i<m_nNbOfReflectanceBands; i++)
    {
        float fCurReflectance = ComputeReflectanceForPixelVal(A[i]);
        float fPrevWeight = GetPrevWeightValue(A);
        float fPrevReflect = GetPrevWeight(A);

        // TODO: See what means "if band is available in the case of LANDSAT 8, some bands are not available\"

    }
}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleSnowOrWaterPixel(const TInput & A)
{

}

template< class TInput, class TOutput>
void UpdateSynthesisFunctor<TInput,TOutput>::HandleCloudOrShadowPixel(const TInput &A)
{

}
