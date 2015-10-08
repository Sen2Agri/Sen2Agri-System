#ifndef DIRECTIONALCORRECTIONFUNCTOR_H
#define DIRECTIONALCORRECTIONFUNCTOR_H

#define NO_DATA_EPSILON 0.00001f

class ScaterringFunctionCoefficients
{
public:
    float V0;
    float V1;
    float R0;
    float R1;
};

template< class TInput, class TOutput>
class DirectionalCorrectionFunctor
{
public:
    DirectionalCorrectionFunctor();
    DirectionalCorrectionFunctor& operator =(const DirectionalCorrectionFunctor& copy);
    bool operator!=( const DirectionalCorrectionFunctor & other) const;
    bool operator==( const DirectionalCorrectionFunctor & other ) const;
    TOutput operator()( const TInput & A );
    void Initialize(const std::vector<ScaterringFunctionCoefficients> &coeffs, float fReflQuantifVal);

    const char * GetNameOfClass() { return "DirectionalCorrectionFunctor"; }

private:
    bool IsSnowPixel(const TInput & A);
    bool IsWaterPixel(const TInput & A);
    bool IsCloudPixel(const TInput & A);
    bool IsLandPixel(const TInput & A);
    float GetCurrentL2AWeightValue(const TInput & A);
    bool IsNoDataValue(float fValue, float fNoDataValue);

private:
    float m_fReflQuantifValue;

    std::vector<ScaterringFunctionCoefficients> m_ScatteringCoeffs;

    int m_nReflBandsCount;

    int m_nCloudMaskBandIndex;
    int m_nSnowMaskBandIndex;
    int m_nWaterMaskBandIndex;
    int m_nNdviBandIdx;
    int m_nSunAnglesBandStartIdx;
    int m_nSensoAnglesBandStartIdx;

    float m_fReflNoDataValue;

};

#include "DirectionalCorrectionFunctor.txx"

#endif // DIRECTIONALCORRECTIONFUNCTOR_H
