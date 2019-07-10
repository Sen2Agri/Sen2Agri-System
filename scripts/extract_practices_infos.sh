#!/bin/bash

function usage() {
    echo "Usage: ./extract_practices_infos.sh -c <COUNTRY_CODE - (NLD|CZE|LTU|ESP|ITA|ROU)> -y <YEAR>"
    exit 1
}

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -c|--country)
    COUNTRY_AND_REGION="$2"
    shift # past argument
    shift # past value
    ;;
    -y|--year)
    YEAR="$2"
    shift # past argument
    shift # past value
    ;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

echo COUNTRY        = "${COUNTRY_AND_REGION}"
echo YEAR           = "${YEAR}"

if [ -z ${COUNTRY_AND_REGION} ] ; then
    echo "No country provided!"
    usage
fi 

if [ -z ${YEAR} ] ; then
    echo "No year provided!"
    usage
fi 

COUNTRY="${COUNTRY_AND_REGION%%_*}"
COUNTRY_REGION=""
if [ "$1" != "$COUNTRY" ] ; then
    COUNTRY_REGION="${1##*_}"
fi    

SHP_PATH=""
IN_SHP_NAME=""

WORKING_DIR_ROOT="/mnt/archive/agric_practices"
INSITU_ROOT="$WORKING_DIR_ROOT/insitu/"
OUT_DIR="$INSITU_ROOT/PracticesInfos/"

if [ -z $COUNTRY_REGION ] ; then 
    FILTER_IDS_FILE="${OUT_DIR}/Sen4CAP_L4C_${YEAR}_FilterIDs.csv"
else 
    FILTER_IDS_FILE="${OUT_DIR}/Sen4CAP_L4C_${YEAR}_${COUNTRY_REGION}_FilterIDs.csv"
fi

CC_OUT_FILE="${OUT_DIR}/Sen4CAP_L4C_Catch_${COUNTRY_AND_REGION}_${YEAR}.csv"
NFC_OUT_FILE="${OUT_DIR}/Sen4CAP_L4C_NFC_${COUNTRY_AND_REGION}_${YEAR}.csv"
FL_OUT_FILE="${OUT_DIR}/Sen4CAP_L4C_Fallow_${COUNTRY_AND_REGION}_${YEAR}.csv"
NA_OUT_FILE="${OUT_DIR}/Sen4CAP_L4C_NA_${COUNTRY_AND_REGION}_${YEAR}.csv"

VEG_START=""
CC_VEG_START=""
NFC_VEG_START=""
FL_VEG_START=""
NA_VEG_START=""

CC_HSTART=""
CC_HEND=""
CC_HSTARTW=""
CC_PSTART=""
CC_PEND=""
CC_PSTARTW=""
CC_PENDW=""
CC_ADD_FILES=""

NFC_HSTART=""
NFC_HEND=""
NFC_HSTARTW=""
NFC_PSTART=""
NFC_PEND=""
NFC_PSTARTW=""
NFC_PENDW=""
NFC_ADD_FILES=""

FL_HSTART=""
FL_HEND=""
FL_HSTARTW=""
FL_PSTART=""
FL_PEND=""
FL_PSTARTW=""
FL_PENDW=""
FL_ADD_FILES=""

NA_HSTART=""
NA_HEND=""
NA_HSTARTW=""
NA_PSTART=""
NA_PEND=""
NA_PSTARTW=""        
NA_PENDW=""
NA_ADD_FILES=""

IGNORED_IDS_FILE=""
CC_IGNORED_IDS_FILE=""
NFC_IGNORED_IDS_FILE=""
FL_IGNORED_IDS_FILE=""
NA_IGNORED_IDS_FILE=""


case "$COUNTRY" in
    NLD)
        COUNTRY="NL"
        IN_SHP_NAME="NLD_${YEAR}_DeclSTD_quality_indic.shp"
        VEG_START="${YEAR}-05-01"

        CC_HSTART="${YEAR}-05-15"
        CC_HSTARTW="${YEAR}-07-15"
        CC_HEND="${YEAR}-10-15"
        CC_PSTART="${YEAR}-05-15"
        CC_PEND="${YEAR}-07-15"
        CC_PSTARTW="${YEAR}-10-15"
        
        NA_HSTART="${YEAR}-06-01"
        NA_HSTARTW="${YEAR}-07-15"
        NA_HEND="${YEAR}-12-15"
        NA_PSTART="${YEAR}-05-15"
        NA_PEND="${YEAR}-07-15"
        NA_PSTARTW="${YEAR}-10-15"        
        ;;
    CZE)
        IN_SHP_NAME="CZE_${YEAR}_DeclSTD_quality_indic.shp"
        
        VEG_START="${YEAR}-05-01"

        CC_HSTART="${YEAR}-05-01"
        CC_HEND="${YEAR}-10-31"
        CC_PSTART="${YEAR}-07-31"
        CC_PEND="${YEAR}-09-24"
        CC_PSTARTW="${YEAR}-09-06"
        CC_PENDW="${YEAR}-10-31"
        CC_ADD_FILES="${INSITU_ROOT}/CZ_SubsidyApplication_${YEAR}.csv ${INSITU_ROOT}/CZ_EFA_${YEAR}.csv"

        NFC_HSTART="${YEAR}-05-01"
        NFC_HEND="${YEAR}-10-31"
        NFC_PSTART="${YEAR}-06-01"
        NFC_PEND="${YEAR}-07-15"
        NFC_ADD_FILES="${INSITU_ROOT}/CZ_SubsidyApplication_${YEAR}.csv ${INSITU_ROOT}/CZ_EFA_${YEAR}.csv"

        FL_VEG_START="${YEAR}-03-01"
        FL_HSTART="${YEAR}-03-01"
        FL_HEND="${YEAR}-10-31"
        FL_PSTART="${YEAR}-03-01"
        FL_PEND="${YEAR}-07-15"
        FL_ADD_FILES="${INSITU_ROOT}/CZ_SubsidyApplication_${YEAR}.csv ${INSITU_ROOT}/CZ_EFA_${YEAR}.csv"

        NA_HSTART="${YEAR}-05-01"
        NA_HEND="${YEAR}-12-15"
        NA_ADD_FILES="${INSITU_ROOT}/CZ_SubsidyApplication_${YEAR}.csv ${INSITU_ROOT}/CZ_EFA_${YEAR}.csv"

        ;;
    LTU)
        IN_SHP_NAME="LTU_${YEAR}_DeclSTD_quality_indic.shp"
        
        VEG_START="${YEAR}-04-01"

        CC_HSTART="${YEAR}-06-03"
        CC_HEND="${YEAR}-09-01"
        CC_PSTART="${YEAR}-09-01"
        CC_PEND="${YEAR}-10-15"
        if [ "${YEAR}" == "2018" ] ; then
            CC_ADD_FILES="${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Catch_crops_PO.csv ${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Catch_crops_IS.csv"
            CC_IGNORED_IDS_FILE="${INSITU_ROOT}/CC_Ignored_Orig_IDs.csv"            
        else 
            CC_ADD_FILES="${INSITU_ROOT}/ApplicationsEFA_${YEAR}_LT.csv ${INSITU_ROOT}/CC_IS_${YEAR}.csv ${INSITU_ROOT}/CC_PS_${YEAR}.csv"
        fi

        NFC_HSTART="${YEAR}-06-03"
        NFC_HEND="${YEAR}-10-31"
        NFC_PSTART="${YEAR}-04-01"
        NFC_PEND="${YEAR}-10-31"
        NFC_ADD_FILES="${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Nitrogen_fixing.csv"
        if [ "${YEAR}" == "2018" ] ; then
            NFC_ADD_FILES="${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Nitrogen_fixing.csv"
        else 
            NFC_ADD_FILES="${INSITU_ROOT}/ApplicationsEFA_${YEAR}_LT.csv"
        fi
        
        FL_HSTART="${YEAR}-04-01"
        FL_HSTARTW="${YEAR}-06-03"
        FL_HEND="${YEAR}-09-01"
        FL_PSTART="${YEAR}-04-01"
        FL_PEND="${YEAR}-06-30"
        FL_PENDW="${YEAR}-09-01"
        if [ "${YEAR}" == "2018" ] ; then
            FL_ADD_FILES="${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Black_fallow.csv ${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Green_fallow.csv"
            FL_IGNORED_IDS_FILE="${INSITU_ROOT}/FL_Ignored_Orig_IDs.csv"
        else 
            FL_ADD_FILES="${INSITU_ROOT}/ApplicationsEFA_${YEAR}_LT.csv"
        fi

        NA_HSTART="${YEAR}-06-03"
        NA_HEND="${YEAR}-12-15"
        if [ "${YEAR}" == "2018" ] ; then
            NA_ADD_FILES="${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Catch_crops_PO.csv ${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Catch_crops_IS.csv ${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Black_fallow.csv ${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Green_fallow.csv ${INSITU_ROOT}/ApplicationsEFA_${YEAR}_Nitrogen_fixing.csv"

        else 
            NA_ADD_FILES="${INSITU_ROOT}/ApplicationsEFA_${YEAR}_LT.csv"
        fi
        
        ;;
    ESP)
        IN_SHP_NAME="ESP_${YEAR}_DeclSTD_quality_indic.shp"
        
        VEG_START="${YEAR}-04-02"

        NFC_HSTART="${YEAR}-04-02"
        NFC_HEND="${YEAR}-08-15"
        NFC_PSTART="${YEAR}-03-01"
        NFC_PEND="${YEAR}-08-31"

        FL_HSTART="${YEAR}-04-02"
        FL_HEND="${YEAR}-08-15"
        FL_PSTART="${YEAR}-02-01"
        FL_PEND="${YEAR}-06-30"

        NA_HSTART="${YEAR}-04-02"
        NA_HEND="${YEAR}-08-15"
        ;;
    ITA)
        if [ "$COUNTRY_REGION" == "FML" ] ; then
            IN_SHP_NAME="ITA_FRIULI_MARCHE_LAZIO_${YEAR}_DeclSTD_quality_indic.shp"
        elif [ "$COUNTRY_REGION" == "CP1" ] ; then
            IN_SHP_NAME="ITA_CAMPANIA_PUGLIA_${YEAR}_DeclSTD_quality_indic_part1.shp"
        elif [ "$COUNTRY_REGION" == "CP2" ]  ; then
            IN_SHP_NAME="ITA_CAMPANIA_PUGLIA_${YEAR}_DeclSTD_quality_indic_part2.shp"
        else
            echo "Error executing practices infos for ITA. Unknown region ${COUNTRY_REGION}"
            exit 1
        fi    

        VEG_START="${YEAR}-01-01"

        NFC_VEG_START="${YEAR}-03-01"
        NFC_HSTART="${YEAR}-04-02"
        NFC_HEND="${YEAR}-08-31"
        NFC_PSTART="${YEAR}-03-01"
        NFC_PEND="${YEAR}-08-31"

        FL_HSTART="${YEAR}-01-01"
        FL_HEND="${YEAR}-09-30"
        FL_PSTART="${YEAR}-01-01"
        FL_PEND="${YEAR}-06-30"
        
        NA_VEG_START="${YEAR}-04-02"
        NA_HSTART="${YEAR}-04-02"
        NA_HEND="${YEAR}-12-15"
        ;;
    ROU)
        IN_SHP_NAME="ROU_${YEAR}_DeclSTD_quality_indic.shp"
        
        VEG_START="${YEAR}-04-02"

        CC_HSTART="${YEAR}-06-15"
        CC_HEND="${YEAR}-10-31"
        CC_PSTART="${YEAR}-10-01"
        CC_ADD_FILES="${INSITU_ROOT}/RO_CatchCrops_${YEAR}.csv"

        NFC_VEG_START="${YEAR}-03-19"
        NFC_HSTART="${YEAR}-04-02"
        NFC_HEND="${YEAR}-10-31"
        NFC_PSTART="${YEAR}-04-02"
        NFC_PEND="${YEAR}-07-29"
        NFC_ADD_FILES="${INSITU_ROOT}/Sen4CAP_L4C_NFC_ROU_${YEAR}.csv"
        # TODO: Here handle the other dates according to the specified fields

        NA_HSTART="${YEAR}-04-02"
        NA_HEND="${YEAR}-10-31"
        ;;
    *)
        echo $"Usage: $0 {NLD|CZE|LTU|ESP|ITA|ROU}"
        exit 1
esac

SHP_PATH="$INSITU_ROOT$IN_SHP_NAME"

# Normally, we have only one VEG_START but we have countries where we have also different dates for practices
if [ -z "$CC_VEG_START" ]; then CC_VEG_START="$VEG_START" ; fi
if [ -z "$NFC_VEG_START" ]; then NFC_VEG_START="$VEG_START" ; fi
if [ -z "$FL_VEG_START" ]; then FL_VEG_START="$VEG_START" ; fi
if [ -z "$NA_VEG_START" ]; then NA_VEG_START="$VEG_START" ; fi

CC_VEG_START="-vegstart \"$CC_VEG_START\" "
NFC_VEG_START="-vegstart \"$NFC_VEG_START\" "
FL_VEG_START="-vegstart \"$FL_VEG_START\" "
NA_VEG_START="-vegstart \"$NA_VEG_START\" "

if [ -n "$CC_HSTART" ]; then CC_HSTART="-hstart \"$CC_HSTART\" " ; fi
if [ -n "$CC_HEND" ]; then CC_HEND="-hend \"$CC_HEND\" " ; fi
if [ -n "$CC_HSTARTW" ]; then CC_HSTARTW="-hwinterstart \"$CC_HSTARTW\" " ; fi
if [ -n "$CC_PSTART" ]; then CC_PSTART="-pstart \"$CC_PSTART\" " ; fi
if [ -n "$CC_PEND" ]; then CC_PEND="-pend \"$CC_PEND\" " ; fi
if [ -n "$CC_PSTARTW" ]; then CC_PSTARTW="-wpstart \"$CC_PSTARTW\" " ; fi
if [ -n "$CC_PENDW" ]; then CC_PENDW="-wpend \"$CC_PENDW\" " ; fi
if [ -n "$CC_ADD_FILES" ]; then CC_ADD_FILES="-addfiles $CC_ADD_FILES " ; fi

if [ -n "$NFC_HSTART" ] ; then NFC_HSTART="-hstart \"$NFC_HSTART\" " ; fi
if [ -n "$NFC_HEND" ] ; then NFC_HEND="-hend \"$NFC_HEND\" " ; fi
if [ -n "$NFC_HSTARTW" ]; then NFC_HSTARTW="-hwinterstart \"$NFC_HSTARTW\" " ; fi
if [ -n "$NFC_PSTART" ]; then NFC_PSTART="-pstart \"$NFC_PSTART\" " ; fi
if [ -n "$NFC_PEND" ]; then NFC_PEND="-pend \"$NFC_PEND\" " ; fi
if [ -n "$NFC_PSTARTW" ]; then NFC_PSTARTW="-wpstart \"$NFC_PSTARTW\" " ; fi
if [ -n "$NFC_PENDW" ]; then NFC_PENDW="-wpend \"$NFC_PENDW\" " ; fi
if [ -n "$NFC_ADD_FILES" ]; then NFC_ADD_FILES="-addfiles $NFC_ADD_FILES " ; fi

if [ -n "$FL_HSTART" ]; then FL_HSTART="-hstart \"$FL_HSTART\" " ; fi
if [ -n "$FL_HEND" ]; then FL_HEND="-hend \"$FL_HEND\" " ; fi
if [ -n "$FL_HSTARTW" ]; then FL_HSTARTW="-hwinterstart \"$FL_HSTARTW\" " ; fi
if [ -n "$FL_PSTART" ]; then FL_PSTART="-pstart \"$FL_PSTART\" " ; fi
if [ -n "$FL_PEND" ]; then FL_PEND="-pend \"$FL_PEND\" " ; fi
if [ -n "$FL_PSTARTW" ]; then FL_PSTARTW="-wpstart \"$FL_PSTARTW\" " ; fi
if [ -n "$FL_PENDW" ]; then FL_PENDW="-wpend \"$FL_PENDW\" " ; fi
if [ -n "$FL_ADD_FILES" ]; then FL_ADD_FILES="-addfiles $FL_ADD_FILES " ; fi

if [ -n "$NA_HSTART" ]; then NA_HSTART="-hstart \"$NA_HSTART\" " ; fi
if [ -n "$NA_HEND" ]; then NA_HEND="-hend \"$NA_HEND\" " ; fi
if [ -n "$NA_HSTARTW" ]; then NA_HSTARTW="-hwinterstart \"$NA_HSTARTW\" " ; fi
if [ -n "$NA_PSTART" ]; then NA_PSTART="-pstart \"$NA_PSTART\" " ; fi
if [ -n "$NA_PEND" ]; then NA_PEND="-pend \"$NA_PEND\" " ; fi
if [ -n "$NA_PSTARTW" ]; then NA_PSTARTW="-wpstart \"$NA_PSTARTW\" " ; fi
if [ -n "$NA_PENDW" ]; then NA_PENDW="-wpend \"$NA_PENDW\" " ; fi
if [ -n "$NA_ADD_FILES" ]; then NA_ADD_FILES="-addfiles $NA_ADD_FILES " ; fi

if [ -n "$IGNORED_IDS_FILE" ]; then IGNORED_IDS_FILE="-ignoredids \"$IGNORED_IDS_FILE\" " ; fi
if [ -n "$CC_IGNORED_IDS_FILE" ]; then CC_IGNORED_IDS_FILE="-ignoredids \"$CC_IGNORED_IDS_FILE\" " ; fi
if [ -n "$NFC_IGNORED_IDS_FILE" ]; then NFC_IGNORED_IDS_FILE="-ignoredids \"$NFC_IGNORED_IDS_FILE\" " ; fi
if [ -n "$FL_IGNORED_IDS_FILE" ]; then FL_IGNORED_IDS_FILE="-ignoredids \"$FL_IGNORED_IDS_FILE\" " ; fi
if [ -n "$NA_IGNORED_IDS_FILE" ]; then NA_IGNORED_IDS_FILE="-ignoredids \"$NA_IGNORED_IDS_FILE\" " ; fi

mkdir -p "$OUT_DIR"

echo "Veg start: $VEG_START"
echo "CC hstart: $CC_HSTART" 
echo "CC hend: $CC_HEND" 
echo "CC winter hstart: $CC_HSTARTW"
echo "CC pstart: $CC_PSTART"
echo "CC pend: $CC_PEND"
echo "CC winter pstart: $CC_PSTARTW" 
echo "CC winter pend: $CC_PENDW"

# Extract the unique IDs
echo "Executing otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH -country $COUNTRY -seqidsonly 1 $NA_ADD_FILES $IGNORED_IDS_FILE -out $FILTER_IDS_FILE"
otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH -country $COUNTRY -seqidsonly 1 $NA_ADD_FILES $IGNORED_IDS_FILE -out $FILTER_IDS_FILE

# Extract the Practiced information for parcels with CC
if [ ! -z "$CC_HSTART" ] ; then
    echo "Executing: otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH $CC_ADD_FILES -practice CatchCrop -country $COUNTRY -year $YEAR $CC_VEG_START $CC_HSTART $CC_HEND $CC_HSTARTW $CC_PSTART $CC_PEND $CC_PSTARTW $CC_PENDW $CC_IGNORED_IDS_FILE -out $CC_OUT_FILE"

    otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH $CC_ADD_FILES -practice CatchCrop -country $COUNTRY -year $YEAR $CC_VEG_START $CC_HSTART $CC_HEND $CC_HSTARTW $CC_PSTART $CC_PEND $CC_PSTARTW $CC_PENDW $CC_IGNORED_IDS_FILE -out $CC_OUT_FILE
fi

# Extract the Practiced information for parcels with NFC
if [ ! -z "$NFC_HSTART" ] ; then
    echo "Executing: otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH $NFC_ADD_FILES -practice NFC -country $COUNTRY -year $YEAR $NFC_VEG_START $NFC_HSTART $NFC_HEND $NFC_HSTARTW $NFC_PSTART $NFC_PEND $NFC_PSTARTW $NFC_PENDW $NFC_IGNORED_IDS_FILE -out $NFC_OUT_FILE"

    otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH $NFC_ADD_FILES -practice NFC -country $COUNTRY -year $YEAR $NFC_VEG_START $NFC_HSTART $NFC_HEND $NFC_HSTARTW $NFC_PSTART $NFC_PEND $NFC_PSTARTW $NFC_PENDW $NFC_IGNORED_IDS_FILE -out $NFC_OUT_FILE
fi
    
# Extract the Practiced information for parcels with Fallow Land
if [ ! -z "$FL_HSTART" ] ; then
    echo "Executing: otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH $FL_ADD_FILES -practice Fallow -country $COUNTRY -year $YEAR $FL_VEG_START $FL_HSTART $FL_HEND $FL_HSTARTW $FL_PSTART $FL_PEND $FL_PSTARTW $FL_PENDW $FL_IGNORED_IDS_FILE -out $FL_OUT_FILE"

    otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH $FL_ADD_FILES -practice Fallow -country $COUNTRY -year $YEAR $FL_VEG_START $FL_HSTART $FL_HEND $FL_HSTARTW $FL_PSTART $FL_PEND $FL_PSTARTW $FL_PENDW $FL_IGNORED_IDS_FILE -out $FL_OUT_FILE
fi
    
# Extract the Practiced information for parcels witout EFA
echo "Executing: otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH $NA_ADD_FILES -practice NA -country $COUNTRY -year $YEAR $NA_VEG_START $NA_HSTART $NA_HEND $NA_HSTARTW $NA_PSTART $NA_PEND $NA_PSTARTW $NA_PENDW $NA_IGNORED_IDS_FILE -out $NA_OUT_FILE"

otbcli LPISDataSelection sen2agri-processors-build -inshp $SHP_PATH $NA_ADD_FILES -practice "NA" -country $COUNTRY -year $YEAR $NA_VEG_START $NA_HSTART $NA_HEND $NA_HSTARTW $NA_PSTART $NA_PEND $NA_PSTARTW $NA_PENDW $NA_IGNORED_IDS_FILE -out $NA_OUT_FILE

