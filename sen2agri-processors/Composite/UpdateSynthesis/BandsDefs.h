#ifndef BANDSDEFS_H
#define BANDSDEFS_H

#define S2_L2A_10M_BANDS_NO     4
#define L8_L2A_10M_BANDS_NO     3
#define SPOT4_L2A_10M_BANDS_NO  3

#define WEIGHTED_REFLECTANCE_10M_BANDS_NO   S2_L2A_10M_BANDS_NO

//S2 10m Bands defines
//#define S2_L2A_10M_BANDS_START_IDX      0
#define S2_L2A_10M_BLUE_BAND_IDX        0
#define S2_L2A_10M_RED_BAND_IDX         2
//#define S2_L2A_10M_CLD_MASK_IDX         4
//#define S2_L2A_10M_WATER_MASK_IDX       5
//#define S2_L2A_10M_SNOW_MASK_IDX        6
//#define S2_L2A_10M_TOTAL_WEIGHT_IDX     7
//#define S2_L3A_10M_WEIGHT_START_IDX     8
//#define S2_L3A_10M_W_AV_DATE_IDX        12
//#define S2_L3A_10M_REFL_START_IDX       13
//#define S2_L3A_10M_PIXEL_STATUS_IDX     17

//Landsat 8 10m Bands defines
//#define L8_L2A_10M_BANDS_START_IDX      0
#define L8_L2A_10M_BLUE_BAND_IDX        0
#define L8_L2A_10M_RED_BAND_IDX         2
//#define L8_L2A_10M_CLD_MASK_IDX         3
//#define L8_L2A_10M_WATER_MASK_IDX       4
//#define L8_L2A_10M_SNOW_MASK_IDX        5
//#define L8_L2A_10M_TOTAL_WEIGHT_IDX     6
//#define L8_L3A_10M_WEIGHT_START_IDX     7
//#define L8_L3A_10M_W_AV_DATE_IDX        11
//#define L8_L3A_10M_REFL_START_IDX       12
//#define L8_L3A_10M_PIXEL_STATUS_IDX     16

//SPOT4 10m Bands defines
//#define SPOT4_L2A_10M_BANDS_START_IDX      0
// For SPOT4 the blue band is actually the green band
#define SPOT4_L2A_10M_BLUE_BAND_IDX        0
#define SPOT4_L2A_10M_RED_BAND_IDX         1
//#define SPOT4_L2A_10M_CLD_MASK_IDX         3
//#define SPOT4_L2A_10M_WATER_MASK_IDX       4
//#define SPOT4_L2A_10M_SNOW_MASK_IDX        5
//#define SPOT4_L2A_10M_TOTAL_WEIGHT_IDX     6
//#define SPOT4_L3A_10M_WEIGHT_START_IDX     7
//#define SPOT4_L3A_10M_W_AV_DATE_IDX        11
//#define SPOT4_L3A_10M_REFL_START_IDX       12
//#define SPOT4_L3A_10M_PIXEL_STATUS_IDX     16


// 20M Positions Definition
#define S2_L2A_20M_BANDS_NO     6
#define L8_L2A_20M_BANDS_NO     3
#define SPOT4_L2A_20M_BANDS_NO  1
#define WEIGHTED_REFLECTANCE_20M_BANDS_NO   S2_L2A_20M_BANDS_NO

//S2 20m Bands defines
//#define S2_L2A_20M_BANDS_START_IDX      0
#define S2_L2A_20M_BLUE_BAND_IDX        -1
#define S2_L2A_20M_RED_BAND_IDX         -1
//#define S2_L2A_20M_CLD_MASK_IDX         6
//#define S2_L2A_20M_WATER_MASK_IDX       7
//#define S2_L2A_20M_SNOW_MASK_IDX        8
//#define S2_L2A_20M_TOTAL_WEIGHT_IDX     9
//#define S2_L3A_20M_WEIGHT_START_IDX     10
//#define S2_L3A_20M_W_AV_DATE_IDX        16
//#define S2_L3A_20M_REFL_START_IDX       17
//#define S2_L3A_20M_PIXEL_STATUS_IDX     23

//Landsat 8 20m Bands defines
//#define L8_L2A_20M_BANDS_START_IDX      0
#define L8_L2A_20M_BLUE_BAND_IDX        -1
#define L8_L2A_20M_RED_BAND_IDX         -1
//#define L8_L2A_20M_CLD_MASK_IDX         3
//#define L8_L2A_20M_WATER_MASK_IDX       4
//#define L8_L2A_20M_SNOW_MASK_IDX        5
//#define L8_L2A_20M_TOTAL_WEIGHT_IDX     6
//#define L8_L3A_20M_WEIGHT_START_IDX     7
//#define L8_L3A_20M_W_AV_DATE_IDX        13
//#define L8_L3A_20M_REFL_START_IDX       14
//#define L8_L3A_20M_PIXEL_STATUS_IDX     20

//SPOT4 20m Bands defines
//#define SPOT4_L2A_20M_BANDS_START_IDX      0
#define SPOT4_L2A_20M_BLUE_BAND_IDX        -1
#define SPOT4_L2A_20M_RED_BAND_IDX         -1
//#define SPOT4_L2A_20M_CLD_MASK_IDX         1
//#define SPOT4_L2A_20M_WATER_MASK_IDX       2
//#define SPOT4_L2A_20M_SNOW_MASK_IDX        3
//#define SPOT4_L2A_20M_TOTAL_WEIGHT_IDX     4
//#define SPOT4_L3A_20M_WEIGHT_START_IDX     5
//#define SPOT4_L3A_20M_W_AV_DATE_IDX        11
//#define SPOT4_L3A_20M_REFL_START_IDX       12
//#define SPOT4_L3A_20M_PIXEL_STATUS_IDX     18

// These defines are for the case when all the bands of 10 AND 20m are resampled at the specified resolution
// and are all present

#define S2_L2A_ALL_BANDS_NO     10
#define L8_L2A_ALL_BANDS_NO     6
#define SPOT4_L2A_ALL_BANDS_NO  4

#define WEIGHTED_REFLECTANCE_ALL_BANDS_NO   S2_L2A_ALL_BANDS_NO

//S2 Bands defines
//#define S2_L2A_ALL_BANDS_START_IDX      0
#define S2_L2A_ALL_BLUE_BAND_IDX        0
#define S2_L2A_ALL_RED_BAND_IDX         2
//#define S2_L2A_ALL_CLD_MASK_IDX         10
//#define S2_L2A_ALL_WATER_MASK_IDX       11
//#define S2_L2A_ALL_SNOW_MASK_IDX        12
//#define S2_L2A_ALL_TOTAL_WEIGHT_IDX     13
//#define S2_L3A_ALL_WEIGHT_START_IDX     14
//#define S2_L3A_ALL_W_AV_DATE_IDX        24
//#define S2_L3A_ALL_REFL_START_IDX       25
//#define S2_L3A_ALL_PIXEL_STATUS_IDX     35

//Landsat 8 Bands defines
//#define L8_L2A_ALL_BANDS_START_IDX      0
#define L8_L2A_ALL_BLUE_BAND_IDX        0
#define L8_L2A_ALL_RED_BAND_IDX         2
//#define L8_L2A_ALL_CLD_MASK_IDX         6
//#define L8_L2A_ALL_WATER_MASK_IDX       7
//#define L8_L2A_ALL_SNOW_MASK_IDX        8
//#define L8_L2A_ALL_TOTAL_WEIGHT_IDX     9
//#define L8_L3A_ALL_WEIGHT_START_IDX     10
//#define L8_L3A_ALL_W_AV_DATE_IDX        20
//#define L8_L3A_ALL_REFL_START_IDX       21
//#define L8_L3A_ALL_PIXEL_STATUS_IDX     31

//SPOT4 Bands defines
//#define SPOT4_L2A_ALL_BANDS_START_IDX      0
// For SPOT4 the blue band is actually the green band
#define SPOT4_L2A_ALL_BLUE_BAND_IDX        0
#define SPOT4_L2A_ALL_RED_BAND_IDX         1
//#define SPOT4_L2A_ALL_CLD_MASK_IDX         4
//#define SPOT4_L2A_ALL_WATER_MASK_IDX       5
//#define SPOT4_L2A_ALL_SNOW_MASK_IDX        6
//#define SPOT4_L2A_ALL_TOTAL_WEIGHT_IDX     7
//#define SPOT4_L3A_ALL_WEIGHT_START_IDX     8
//#define SPOT4_L3A_ALL_W_AV_DATE_IDX        18
//#define SPOT4_L3A_ALL_REFL_START_IDX       19
//#define SPOT4_L3A_ALL_PIXEL_STATUS_IDX     29


#define L3A_WEIGHTED_REFLECTANCES_MAX_NO       S2_L2A_ALL_BANDS_NO

#endif // BANDSDEFS_H

