#ifndef UPDATESYNTHESIS_H
#define UPDATESYNTHESIS_H

typedef enum {SENSOR_S2, SENSOR_L8} SensorType;
typedef enum {RES_10M, RES_20M} ResolutionType;

typedef enum {LAND=1, WATER,SNOW,CLOUD,CLOUD_SHADOW} FlagType;

#define MAX_WEIGHTED_REFLECTANCES 6

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

    void Initialize(SensorType sensorType, ResolutionType resolution, bool bPrevL3ABandsAvailable);
    void SetReflectanceQuantificationValue(float fQuantifVal);
    void SetCurrentDate(int nDate);

private:
    void ResetCurrentPixelValues();
    int GetAbsoluteL2ABandIndex(int index);
    float GetReflectanceForPixelVal(float fPixelVal);
    void HandleLandPixel(const TInput & A);
    void HandleSnowOrWaterPixel(const TInput & A);
    void HandleCloudOrShadowPixel(const TInput & A);

    bool IsLandPixel(const TInput & A);
    bool IsSnowPixel(const TInput & A);
    bool IsWaterPixel(const TInput & A);
    bool IsCloudPixel(const TInput & A);
    bool IsCloudShadowPixel(const TInput & A);
    bool IsRedBand(int index);

    float GetPrevWeightValue(const TInput & A);
    float GetPrevWeightedAvDateValue(const TInput & A);
    float GetPrevReflectanceValue(const TInput & A);
    float GetPrevPixelFlagValue(const TInput & A);
    int GetBlueBandIndex();

private:
    SensorType m_sensorType;
    ResolutionType m_resolution;

    float m_fQuantificationValue;
    int m_nCurrentDate;
    bool m_bPrevL3ABandsAvailable;
    int m_nNbOfReflectanceBands;
    int m_nNbL2ABands;

    int m_nCloudMaskBandIndex;
    int m_nCloudShadowMaskBandIndex;
    int m_nSnowMaskBandIndex;
    int m_nWaterMaskBandIndex;
    int m_nPrevWeightBandIndex;
    int m_nPrevWeightedAvDateBandIndex;
    int m_nPrevReflectanceBandIndex;
    int m_nPrevPixelFlagBandIndex;
    int m_nRedBandIndex;

    // output pixel values
    float m_CurrentWeightedReflectances[MAX_WEIGHTED_REFLECTANCES];
    float m_CurrentPixelWeights[MAX_WEIGHTED_REFLECTANCES];
    float m_fCurrentPixelFlag;
    float m_fCurrentPixelWeightedDate;
};

#endif // UPDATESYNTHESIS_H
