#ifndef UPDATESYNTHESISFUNCTOR_H
#define UPDATESYNTHESISFUNCTOR_H

#include "BandsDefs.h"

typedef enum {SENSOR_S2, SENSOR_LANDSAT8, SENSOR_SPOT4} SensorType;
typedef enum {RES_10M, RES_20M} ResolutionType;

typedef enum {FLAG_NO_DATA=0, CLOUD=1, SNOW=2, WATER=3, LAND=4, CLOUD_SHADOW=5} FlagType;

#define DATE_QUANTIF_VALUE      10000
#define WEIGHT_QUANTIF_VALUE    10000

#define NO_DATA                 -10000
#define DATE_NO_DATA            (NO_DATA/DATE_QUANTIF_VALUE)
#define WEIGHT_NO_DATA          (NO_DATA/WEIGHT_QUANTIF_VALUE)      //  NO_DATA / WEIGHT_QUANTIF_VALUE
#define NO_DATA_EPSILON         0.00001f    //

class OutFunctorInfos
{
public:
    float m_CurrentWeightedReflectances[L3A_WEIGHTED_REFLECTANCES_MAX_NO];
    float m_CurrentPixelWeights[L3A_WEIGHTED_REFLECTANCES_MAX_NO];
    short m_nCurrentPixelFlag;
    float m_fCurrentPixelWeightedDate;
} ;

template< class TInput, class TOutput>
class UpdateSynthesisFunctor
{
public:
    UpdateSynthesisFunctor();
    UpdateSynthesisFunctor& operator =(const UpdateSynthesisFunctor& copy);
    bool operator!=( const UpdateSynthesisFunctor & other) const;
    bool operator==( const UpdateSynthesisFunctor & other ) const;
    TOutput operator()( const TInput & A );
    void Initialize(SensorType sensorType, ResolutionType resolution, bool bPrevL3ABandsAvailable,
                    int nDate, float fReflQuantifVal, bool bAllInOne = false, bool bMissingL3ABands = false);
    int GetNbOfL3AReflectanceBands() { return m_nNbOfL3AReflectanceBands; }
    int GetNbOfOutputComponents() { return 2*m_nNbOfL3AReflectanceBands+2;}

    const char * GetNameOfClass() { return "UpdateSynthesisFunctor"; }

private:
    void ResetCurrentPixelValues(const TInput & A, OutFunctorInfos& outInfos);
    int GetAbsoluteL2ABandIndex(int index);
    float GetL2AReflectanceForPixelVal(float fPixelVal);
    void HandleLandPixel(const TInput & A, OutFunctorInfos& outInfos);
    void HandleSnowOrWaterPixel(const TInput & A, OutFunctorInfos& outInfos);
    void HandleCloudOrShadowPixel(const TInput & A, OutFunctorInfos& outInfos);
    bool IsSnowPixel(const TInput & A);
    bool IsWaterPixel(const TInput & A);
    bool IsCloudPixel(const TInput & A);
    bool IsLandPixel(const TInput & A);
    bool IsRedBand(int index);
    float GetCurrentL2AWeightValue(const TInput & A);
    float GetPrevL3AWeightValue(const TInput & A, int offset);
    float GetPrevL3AWeightedAvDateValue(const TInput & A);
    float GetPrevL3AReflectanceValue(const TInput & A, int offset);
    short GetPrevL3APixelFlagValue(const TInput & A);
    int GetBlueBandIndex();
    bool InitBandInfos(SensorType sensorType, ResolutionType resolution);
    bool InitAllBandInResolutionInfos(SensorType sensorType);
    bool IsNoDataValue(float fValue, float fNoDataValue);

private:
    SensorType m_sensorType;
    ResolutionType m_resolution;

    float m_fReflQuantifValue;
    int m_nCurrentDate;

    bool m_bPrevL3ABandsAvailable;
    int m_nNbOfL3AReflectanceBands;

    int m_nRealNbOfL3AReflectanceBands;
    bool m_bMissingL3ABands;

    int m_nNbL2ABands;

    // this array holds the availability information of a certain band and
    // its absolute index in the input bands array
    int m_arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO];
    int m_arrL3AReflBandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO];

    int m_nL2ABandStartIndex;
    int m_nCloudMaskBandIndex;
    int m_nSnowMaskBandIndex;
    int m_nWaterMaskBandIndex;
    int m_nCurrentL2AWeightBandIndex;
    int m_nPrevL3AWeightBandStartIndex;
    int m_nPrevL3AWeightedAvDateBandIndex;
    int m_nPrevL3AReflectanceBandStartIndex;
    int m_nPrevL3APixelFlagBandIndex;
    int m_nRedBandIndex;
    int m_nBlueBandIndex;

    // precomputed value for the reflectance no data (in order to avoid its calculation each pixel)
    float m_fReflNoDataValue;

};

#include "UpdateSynthesisFunctor.txx"

#endif // UPDATESYNTHESISFUNCTOR_H
