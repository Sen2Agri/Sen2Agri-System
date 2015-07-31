#ifndef UPDATESYNTHESIS_H
#define UPDATESYNTHESIS_H

template< class TInput, class TOutput>
class UpdateSynthesisFunctor
{
public:
    UpdateSynthesisFunctor();

    bool operator!=( const UpdateSynthesisFunctor & ) const
    {
        return false;
    }
    bool operator==( const UpdateSynthesisFunctor & other ) const
    {
        return !(*this != other);
    }

    TOutput operator()( const TInput & A ) const;

//    void SetL2AProduct();
//    void SetPreviousL3AProduct();
//    void SetCurrentResolution();

    void SetReflectanceQuantificationValue(float fQuantifVal);
    void SetTotalNbOfBands(int nBandsNo);
    void SetCloudMaskBandIndex(int nIndex);
    void SetCloudShadowMaskBandIndex(int nIndex);
    void SetSnowMaskBandIndex(int nIndex);
    void SetWaterMaskBandIndex(int nIndex);
    void SetPrevWeightBandIndex(int nIndex);
    void SetWeightedAvDateBandIndex(int nIndex);
    void SetWeightedAvReflectanceBandIndex(int nIndex);
    void SetPixelStatusBandIndex(int nIndex);
    void SetRedBandIndex(int nIndex);
    void SetNbOfReflectanceBands(int nReflectanceBandNo);

private:
    void ResetCurrentPixelValues();
    float ComputeReflectanceForPixelVal(float fPixelVal);
    void HandleLandPixel(const TInput & A);
    void HandleSnowOrWaterPixel(const TInput & A);
    void HandleCloudOrShadowPixel(const TInput & A);

    bool IsLandPixel(const TInput & A);
    bool IsSnowPixel(const TInput & A);
    bool IsWaterPixel(const TInput & A);
    bool IsCloudPixel(const TInput & A);
    bool IsCloudShadowPixel(const TInput & A);

    float GetPrevWeightValue(const TInput & A);
    float GetPrevWeightedAvDateValue(const TInput & A);
    float GetPrevWeightedAvReflectanceValue(const TInput & A);
    float GetPixelStatusValue(const TInput & A);

private:
    float m_fQuantificationValue;
    int m_nTotalNbOfBands;
    int m_nCloudMaskBandIndex;
    int m_nCloudShadowMaskBandIndex;
    int m_nSnowMaskBandIndex;
    int m_nWaterMaskBandIndex;
    int m_nPrevWeightBandIndex;
    int m_nPrevWeightedAvDateBandIndex;
    int m_nPrevWeightedAvReflectanceBandIndex;
    int m_nPrevPixelStatusBandIndex;
    int m_nRedBandIndex;
    int m_nNbOfReflectanceBands;

    // output pixel values
    float m_fCurrentPixelWeight;
    float m_fCurrentPixelReflectance;
    float m_fCurrentPixelFlag;
    float m_fCurrentPixelWeightedDate;


};

#endif // UPDATESYNTHESIS_H
