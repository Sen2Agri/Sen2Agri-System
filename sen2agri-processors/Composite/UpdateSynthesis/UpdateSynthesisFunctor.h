#ifndef UPDATESYNTHESIS_H
#define UPDATESYNTHESIS_H

typedef enum {SENSOR_S2, SENSOR_LANDSAT8, SENSOR_SPOT4} SensorType;
typedef enum {RES_10M, RES_20M} ResolutionType;

typedef enum {LAND=1, WATER,SNOW,CLOUD,CLOUD_SHADOW} FlagType;

#define FLAG_NO_DATA -1.0f
#define DATE_NO_DATA -1.0f
#define REFLECTANCE_NO_DATA -1.0f
#define WEIGHT_NO_DATA -1.0f

#define S2_L2A_10M_BANDS_NO     4
#define L8_L2A_10M_BANDS_NO     3
#define SPOT4_L2A_10M_BANDS_NO  3

#define WEIGHTED_REFLECTANCE_10M_BANDS_NO   S2_L2A_10M_BANDS_NO

//S2 10m Bands defines
#define S2_L2A_10M_BANDS_START_IDX      0
#define S2_L2A_10M_BLUE_BAND_IDX        0
#define S2_L2A_10M_RED_BAND_IDX         2
#define S2_L2A_10M_CLD_MASK_IDX         4
#define S2_L2A_10M_WATER_MASK_IDX       5
#define S2_L2A_10M_SNOW_MASK_IDX        6
#define S2_L2A_10M_TOTAL_WEIGHT_IDX     7
#define S2_L3A_10M_WEIGHT_START_IDX     8
#define S2_L3A_10M_W_AV_DATE_IDX        12
#define S2_L3A_10M_REFL_START_IDX       13
#define S2_L3A_10M_PIXEL_STATUS_IDX     17

//Landsat 8 10m Bands defines
#define L8_L2A_10M_BANDS_START_IDX      0
#define L8_L2A_10M_BLUE_BAND_IDX        0
#define L8_L2A_10M_RED_BAND_IDX         2
#define L8_L2A_10M_CLD_MASK_IDX         3
#define L8_L2A_10M_WATER_MASK_IDX       4
#define L8_L2A_10M_SNOW_MASK_IDX        5
#define L8_L2A_10M_TOTAL_WEIGHT_IDX     6
#define L8_L3A_10M_WEIGHT_START_IDX     7
#define L8_L3A_10M_W_AV_DATE_IDX        11
#define L8_L3A_10M_REFL_START_IDX       12
#define L8_L3A_10M_PIXEL_STATUS_IDX     16

//SPOT4 10m Bands defines
#define SPOT4_L2A_10M_BANDS_START_IDX      0
// For SPOT4 the blue band is actually the green band
#define SPOT4_L2A_10M_BLUE_BAND_IDX        0
#define SPOT4_L2A_10M_RED_BAND_IDX         1
#define SPOT4_L2A_10M_CLD_MASK_IDX         3
#define SPOT4_L2A_10M_WATER_MASK_IDX       4
#define SPOT4_L2A_10M_SNOW_MASK_IDX        5
#define SPOT4_L2A_10M_TOTAL_WEIGHT_IDX     6
#define SPOT4_L3A_10M_WEIGHT_START_IDX     7
#define SPOT4_L3A_10M_W_AV_DATE_IDX        11
#define SPOT4_L3A_10M_REFL_START_IDX       12
#define SPOT4_L3A_10M_PIXEL_STATUS_IDX     16


// 20M Positions Definition
#define S2_L2A_20M_BANDS_NO     6
#define L8_L2A_20M_BANDS_NO     3
#define SPOT4_L2A_20M_BANDS_NO  1
#define WEIGHTED_REFLECTANCE_20M_BANDS_NO   S2_L2A_20M_BANDS_NO

//S2 20m Bands defines
#define S2_L2A_20M_BANDS_START_IDX      0
#define S2_L2A_20M_BLUE_BAND_IDX        -1
#define S2_L2A_20M_RED_BAND_IDX         -1
#define S2_L2A_20M_CLD_MASK_IDX         6
#define S2_L2A_20M_WATER_MASK_IDX       7
#define S2_L2A_20M_SNOW_MASK_IDX        8
#define S2_L2A_20M_TOTAL_WEIGHT_IDX     9
#define S2_L3A_20M_WEIGHT_START_IDX     10
#define S2_L3A_20M_W_AV_DATE_IDX        16
#define S2_L3A_20M_REFL_START_IDX       17
#define S2_L3A_20M_PIXEL_STATUS_IDX     23

//Landsat 8 20m Bands defines
#define L8_L2A_20M_BANDS_START_IDX      0
#define L8_L2A_20M_BLUE_BAND_IDX        -1
#define L8_L2A_20M_RED_BAND_IDX         -1
#define L8_L2A_20M_CLD_MASK_IDX         3
#define L8_L2A_20M_WATER_MASK_IDX       4
#define L8_L2A_20M_SNOW_MASK_IDX        5
#define L8_L2A_20M_TOTAL_WEIGHT_IDX     6
#define L8_L3A_20M_WEIGHT_START_IDX     7
#define L8_L3A_20M_W_AV_DATE_IDX        13
#define L8_L3A_20M_REFL_START_IDX       14
#define L8_L3A_20M_PIXEL_STATUS_IDX     20

//SPOT4 20m Bands defines
#define SPOT4_L2A_20M_BANDS_START_IDX      0
#define SPOT4_L2A_20M_BLUE_BAND_IDX        -1
#define SPOT4_L2A_20M_RED_BAND_IDX         -1
#define SPOT4_L2A_20M_CLD_MASK_IDX         1
#define SPOT4_L2A_20M_WATER_MASK_IDX       2
#define SPOT4_L2A_20M_SNOW_MASK_IDX        3
#define SPOT4_L2A_20M_TOTAL_WEIGHT_IDX     4
#define SPOT4_L3A_20M_WEIGHT_START_IDX     5
#define SPOT4_L3A_20M_W_AV_DATE_IDX        11
#define SPOT4_L3A_20M_REFL_START_IDX       12
#define SPOT4_L3A_20M_PIXEL_STATUS_IDX     18

#define L3A_WEIGHTED_REFLECTANCES_MAX_NO       S2_L2A_20M_BANDS_NO

enum {B2_MSK=1, B3_MSK=2, B4_MSK=4, B8_MSK=8, ALL_10M_MSK=0x0F};
enum {B5_MSK=1, B6_MSK=2, B7_MSK=4, B8A_MSK=8, B11_MSK=16, B12_MSK=32, ALL_20M_MSK=0x3F};

class OutFunctorInfos
{
public:
    float m_CurrentWeightedReflectances[L3A_WEIGHTED_REFLECTANCES_MAX_NO];
    float m_CurrentPixelWeights[L3A_WEIGHTED_REFLECTANCES_MAX_NO];
    float m_fCurrentPixelFlag;
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
    void Initialize(SensorType sensorType, ResolutionType resolution, bool bPrevL3ABandsAvailable);
    void SetReflectanceQuantificationValue(float fQuantifVal);
    void SetCurrentDate(int nDate);

    const char * GetNameOfClass() { return "UpdateSynthesisFunctor"; }

private:
    void ResetCurrentPixelValues(OutFunctorInfos& outInfos);
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
    float GetPrevL3APixelFlagValue(const TInput & A);
    int GetBlueBandIndex();

private:
    SensorType m_sensorType;
    ResolutionType m_resolution;

    float m_fQuantificationValue;
    int m_nCurrentDate;

    bool m_bPrevL3ABandsAvailable;
    int m_nNbOfL3AReflectanceBands;
    int m_nNbL2ABands;

    // this array holds the availability information of a certain band and
    // its absolute index in the input bands array
    int m_arrL2ABandPresence[L3A_WEIGHTED_REFLECTANCES_MAX_NO];
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

};

#include "UpdateSynthesisFunctor.txx"

#endif // UPDATESYNTHESIS_H
