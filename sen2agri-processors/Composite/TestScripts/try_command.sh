#! /bin/bash

function try {
    echo
    echo
    echo "$@"
    "$@"
    code=$?
    if [ $code -ne 0 ]
    then
        echo "$1 did not work: exit status $code"
        exit 1
    fi
}

function ut_output_info {
    if [ $# != 4 ] ; then
	echo " Wrong call for ut_output_info. This function should receive 4 params"
	exit
    fi
    echo "Information for $1 file:"
    OUTPUT_IMAGE_INFO="$(otbcli_ReadImageInfo -in $1 | grep "Number of bands")"

    BANDS_NB="${OUTPUT_IMAGE_INFO: -2}"
    COMPARISION_FILE="$3"
#"./qr_cmp_southafrica/ReprocessedTimeSeries.tif"

    if [ $BANDS_NB == $2 ] ; then
	echo "Number of bands: PASSED"
	FILESIZE=$(stat -c%s "$1")
	if [ $FILESIZE == $4 ] ; then    
	    echo "File size      : PASSED"
	    if [[ ! $(diff "$1" "$COMPARISION_FILE") ]] ; then
		echo "Comp ref file  : PASSED"
	    else
		echo "Comp ref file  : FAILED"
	    fi
	else
	    echo "File size      : FAILED"
	fi
    else
	echo "Number of bands: FAILED"
	echo "File size      : FAILED"
	echo "Comp ref file  : FAILED"
    fi

}
