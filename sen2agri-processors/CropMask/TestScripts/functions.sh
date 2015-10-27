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

# Parameter 1 - raster file
# Parameter 2 - expected raster file
function validateRasterFile {
	raster_file=$1
	reference_file=$2

	raster_size=$(stat -c%s "$raster_file")
	reference_size=$(stat -c%s "$reference_file")

	raster_info="$(otbcli_ReadImageInfo -in $raster_file | grep "Number of bands")"
	raster_bands="${raster_info##* }"
	
	reference_info="$(otbcli_ReadImageInfo -in $reference_file | grep "Number of bands")"
	reference_bands="${reference_info##* }"

	echo "Information for file '$1':"

	if [ $raster_bands == $reference_bands ] ; then
	    echo "Number of bands: PASSED"
	    if [ $raster_size == $reference_size ] ; then    
		echo "File size      : PASSED"
		if [[ ! $(diff "$raster_file" "$reference_file") ]] ; then
	    	    echo "File content   : PASSED"
		else
		    echo "File content   : FAILED"
		fi
	    else
		echo "File size      : FAILED"
	        echo "File content   : FAILED"
	    fi
	else
	    echo "Number of bands: FAILED"
	    echo "File size      : FAILED"
	    echo "File content   : FAILED"
	fi	
}

# Parameter 1 - text file
# Parameter 2 - expected text file
function validateTextFile {
	text_file=$1
	reference_file=$2

	text_size=$(stat -c%s "$text_file")
	reference_size=$(stat -c%s "$reference_file")

	echo "Information for file '$1':"

	if [ $text_size == $reference_size ] ; then    
	    echo "File size      : PASSED"
	    if [[ ! $(diff "$text_file" "$reference_file") ]] ; then
	        echo "File content   : PASSED"
	    else
	        echo "File content   : FAILED"
	    fi
	else
	    echo "File size      : FAILED"
	    echo "File content   : FAILED"
	fi
}

# Parameter 1 - text file
# Parameter 2 - expected text file
function validateShapeFile {
	shape_file=$1
	reference_file=$2

	shape_size=$(stat -c%s "$shape_file")
	reference_size=$(stat -c%s "$reference_file")

	echo "Information for file '$1':"

	if [ $shape_size == $reference_size ] ; then    
	    echo "File size      : PASSED"
	    if [[ ! $(diff "$shape_file" "$reference_file") ]] ; then
	        echo "File content   : PASSED"
	    else
	        echo "File content   : FAILED"
	    fi
	else
	    echo "File size      : FAILED"
	    echo "File content   : FAILED"
	fi
}
