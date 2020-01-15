#!/bin/bash

function usage() {
    echo "Usage: ./grassland_mowing_execute_s1.sh --user <user> --script-path <script_path> --s4c-config-file <s4c_config_file> --site-id <site_id> --config-file <config_file> --input-shape-file <input_shape_file> --output-data-dir <output_data_dir> --end-date <end_date> --start-date <start_date> --input-products-list <input_products_list> --seg-parcel-id-attribute <seg_parcel_id_attribute> --output-shapefile <output_shapefile> --do-cmpl <do_cmpl> --test <test> --season-start <season_start> --season-end <season_end>"
    exit 1
}

conda_user="sen2agri-service"
config_file="/etc/sen2agri/sen2agri.conf"

POSITIONAL=()
input_products_list=()
IS_INSIDE_INPUT_PRDS_LIST=0

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -u|--user)
    conda_user="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -p|--script-path)
    script_path="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -c|--s4c-config-file)
    s4c_config_file="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -s|--site-id)
    site_id="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -f|--config-file)
    config_file="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -i|--input-shape-file)
    input_shape_file="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -o|--output-data-dir)
    output_data_dir="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -e|--end-date)
    new_acq_date="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -b|--start-date)
    older_acq_date="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -l|--input-products-list)
    IS_INSIDE_INPUT_PRDS_LIST=1
    shift # past argument
    ;;
    --prds-are-tif)
    prds_are_tif="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past argument
    ;;
    -a|--seg-parcel-id-attribute)
    seg_parcel_id_attribute="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -x|--output-shapefile)
    output_shapefile="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -m|--do-cmpl)
    do_cmpl="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    -t|--test)
    test="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;

    --season-start)
    season_start="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    --season-end)
    season_end="$2"
    IS_INSIDE_INPUT_PRDS_LIST=0
    shift # past argument
    shift # past value
    ;;
    
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    if [ "$IS_INSIDE_INPUT_PRDS_LIST" == "1" ] ; then
        input_products_list+=("$1")
    fi
    
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

#echo "conda_user   = ${conda_user}"
#echo "script_path   = ${script_path}"
#echo "s4c_config_file   = ${s4c_config_file}"
#echo "site_id   = ${site_id}"
#echo "config_file   = ${config_file}"
#echo "input_shape_file   = ${input_shape_file}"
#echo "output_data_dir   = ${output_data_dir}"
#echo "new_acq_date   = ${new_acq_date}"
#echo "older_acq_date   = ${older_acq_date}"
#echo "seg_parcel_id_attribute   = ${seg_parcel_id_attribute}"
#echo "output_shapefile   = ${output_shapefile}"
#echo "do_cmpl   = ${do_cmpl}"
#echo "test   = ${test}"
#echo "season_start   = ${season_start}"
#echo "season_end   = ${season_end}"
#
#echo "input-products-list = ${input_products_list}"


if [ -z ${script_path} ] ; then
    echo "No script-path provided!" && usage
else
    if [[ "${script_path}" = /* ]] ; then
        echo "${script_path} is absolute!"
    else
        path_to_executable=$(which ${script_path})
        if [ -x "$path_to_executable" ] ; then
            echo "${script_path} found in path!"
        else 
            # check if in the same directory as the sh script
            SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
            script_path1="${SCRIPT_DIR}/${script_path}"
            if [ ! -f ${script_path1} ] ; then 
                echo "Cannot find $script_path anywhere!"
                exit 1
            fi
            echo "${script_path1} found in the same folder with sh script (${SCRIPT_DIR}). Using it ..."
            script_path=${script_path1}
        fi
    fi
fi 

if [ -z ${s4c_config_file} ] ; then
    echo "The parameter s4c-config-file defining the database parameters was not provided!"
    echo "The default /etc/sen2agri/sen2agri.conf will be used"
    s4c_config_file="/etc/sen2agri/sen2agri.conf"
fi 
if [ -z ${site_id} ] ; then
    echo "No site-id provided!" && usage
fi 
if [ -z ${config_file} ] ; then
    echo "No config-file provided!" && usage
fi 
if [ -z ${input_shape_file} ] ; then
    echo "No input-shape-file provided!" && usage
fi 
if [ -z ${output_data_dir} ] ; then
    echo "No output-data-dir provided!" && usage
fi 
if [ -z ${new_acq_date} ] ; then
    echo "No new-acq-date provided!" && usage
fi 
if [ -z ${older_acq_date} ] ; then
    echo "No older-acq-date provided!" && usage
fi 
if [ -z ${seg_parcel_id_attribute} ] ; then
    echo "No seg-parcel-id-attribute provided!" && usage
fi 
if [ -z ${output_shapefile} ] ; then
    echo "No output-shapefile provided!" && usage
fi 
if [ -z ${do_cmpl} ] ; then
    echo "No do-cmpl provided!" && usage
fi 
if [ -z ${test} ] ; then
    echo "No test provided!" && usage
fi 

if [ -z ${season_start} ] || [ -z ${season_end} ] ; then
    if [[ -z ${input_products_list} || ${#input_products_list[@]} == 0 ]] ; then
        echo "You should provide either input-files-list or both season-start and season-end!" && usage
    fi 
fi 

if [ $USER == ${conda_user} ] ; then
    echo "Activating conda sen4cap for user $USER"
    CONDA_CMD="source ~/.bashrc && conda activate sen4cap"
    CMD_TERM=""
else 
    echo "Activating conda sen4cap from user $USER for user ${conda_user}"
    CONDA_CMD="sudo su -l ${conda_user} -c 'conda activate sen4cap"
    CMD_TERM="'"
fi    

PRDS_ARE_TIFF_CMD=""
if [ ! -z ${prds_are_tif} ] ; then
    PRDS_ARE_TIFF_CMD="--prds-are-tif ${prds_are_tif}"
fi

PY_CMD="python ${script_path} --s4c-config-file ${s4c_config_file} --site-id ${site_id} --config-file ${config_file} --input-shape-file ${input_shape_file} --output-data-dir ${output_data_dir} --new-acq-date ${new_acq_date} --older-acq-date ${older_acq_date} --seg-parcel-id-attribute ${seg_parcel_id_attribute} --output-shapefile ${output_shapefile} --do-cmpl ${do_cmpl} --test ${test} ${PRDS_ARE_TIFF_CMD}"
if [ -z ${input_products_list} ] ; then
    PY_CMD="${PY_CMD} --season-start ${season_start} --season-end ${season_end}"
else 
    PY_CMD="${PY_CMD} --input-products-list"
    for input_prd in "${input_products_list[@]}"
    do
       PY_CMD="${PY_CMD} ${input_prd}"
    done
fi

CMD="${CONDA_CMD} && ${PY_CMD}"
CMD="${CMD}${CMD_TERM}"

echo "Executing ${CMD}"

#Execute the command
eval $CMD    
