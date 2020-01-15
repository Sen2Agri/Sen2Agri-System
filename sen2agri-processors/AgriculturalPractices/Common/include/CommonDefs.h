#ifndef CommonDefs_h
#define CommonDefs_h

#define NOT_AVAILABLE               -10000
#define NR                          -10001
#define NOT_AVAILABLE_1             -10002

#define NA_STR                      "NA"
#define NR_STR                      "NR"
#define NA1_STR                     "NA1"


// TODO: To remove the CR_CAT_VAL and all its references. LC should be used instead
#define CR_CAT_VAL                      "CR_CAT"
#define LC_VAL                          "LC"

// The unique sequencial ID as it appears in the database or in the shapefile
#define SEQ_UNIQUE_ID                       "NewID"
#define ORIG_UNIQUE_ID                      "ori_id"
#define ORIG_CROP                           "ori_crop"

#define CATCH_CROP_VAL                  "CatchCrop"
#define FALLOW_LAND_VAL                 "Fallow"
#define NITROGEN_FIXING_CROP_VAL        "NFC"

#define CATCH_CROP_VAL_ID                  1
#define FALLOW_LAND_VAL_ID                 2
#define NITROGEN_FIXING_CROP_VAL_ID        3

#define SEC_IN_DAY                   86400          // seconds in day = 24 * 3600

#define NDVI_STR    "_SNDVI_"
#define VV_STR      "_VV_"
#define VH_STR      "_VH_"
#define LAI_STR     "_SLAI_"

#define AMP_STR      "_AMP"
#define COHE_STR     "_COHE"


#define NDVI_REGEX          R"(S2AGRI_L3B_SNDVI_A(\d{8})T.*\.TIF)"
#define LAI_REGEX           R"(S2AGRI_L3B_SLAI_A(\d{8})T.*\.TIF)"

// 2017 naming format for coherence and amplitude
#define COHE_VV_REGEX_OLD       R"((\d{8})-(\d{8})_.*_(\d{3})_VV_.*\.tiff)"
#define COHE_VH_REGEX_OLD       R"((\d{8})-(\d{8})_.*_(\d{3})_VH_.*\.tiff)"
#define AMP_VV_REGEX_OLD        R"((\d{8})_.*_VV_.*\.tiff)"
#define AMP_VH_REGEX_OLD        R"((\d{8})_.*_VH_.*\.tiff)"

// 2018 naming format for coherence and amplitude
#define COHE_VV_REGEX       R"(SEN4CAP_L2A_.*_V(\d{8})T\d{6}_(\d{8})T\d{6}_VV_(\d{3})_(?:.+)?COHE\.tif)"
#define COHE_VH_REGEX       R"(SEN4CAP_L2A_.*_V(\d{8})T\d{6}_(\d{8})T\d{6}_VH_(\d{3})_(?:.+)?COHE\.tif)"
#define AMP_VV_REGEX        R"(SEN4CAP_L2A_.*_V(\d{8})T\d{6}_(\d{8})T\d{6}_VV_\d{3}_(?:.+)?AMP\.tif)"
#define AMP_VH_REGEX        R"(SEN4CAP_L2A_.*_V(\d{8})T\d{6}_(\d{8})T\d{6}_VH_\d{3}_(?:.+)?AMP\.tif)"

#define NDVI_REGEX_DATE_IDX         1

#define COHE_REGEX_DATE_IDX         1           // this is the same for 2017 and 2018 formats
#define COHE_REGEX_DATE2_IDX        2           // this is the same for 2017 and 2018 formats

#define AMP_REGEX_DATE_IDX          1
#define AMP_REGEX_DATE2_IDX         2           // this does not exists for 2017

#define COHE_REGEX_ORBIT_IDX        3           // this is the same for 2017 and 2018 formats

#define LAI_REGEX_DATE_IDX          1

#define NDVI_FT         "NDVI"
#define AMP_FT          "AMP"
#define COHE_FT         "COHE"


#endif
