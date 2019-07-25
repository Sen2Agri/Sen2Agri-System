#!/bin/bash

function usage() {
    echo "Usage: ./grassland_mowing_execute_s1.sh --user <user> --script-path <script_path> --s4c-config-file <s4c_config_file> --site-id <site_id> --config-file <config_file> --input-shape-file <input_shape_file> --output-data-dir <output_data_dir> --new-acq-date <new_acq_date> --older-acq-date <older_acq_date> --input-files-list <input_files_list> --seg-parcel-id-attribute <seg_parcel_id_attribute> --output-shapefile <output_shapefile> --do-cmpl <do_cmpl> --test <test> --season-start <season_start> --season-end <season_end>"
    exit 1
}

conda_user="sen2agri-service"

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -u|--user)
    conda_user="$2"
    shift # past argument
    shift # past value
    ;;
    -p|--script-path)
    script_path="$2"
    shift # past argument
    shift # past value
    ;;
    -c|--s4c-config-file)
    s4c_config_file="$2"
    shift # past argument
    shift # past value
    ;;
    -s|--site-id)
    site_id="$2"
    shift # past argument
    shift # past value
    ;;
    -f|--config-file)
    config_file="$2"
    shift # past argument
    shift # past value
    ;;
    -i|--input-shape-file)
    input_shape_file="$2"
    shift # past argument
    shift # past value
    ;;
    -o|--output-data-dir)
    output_data_dir="$2"
    shift # past argument
    shift # past value
    ;;
    -e|--new-acq-date)
    new_acq_date="$2"
    shift # past argument
    shift # past value
    ;;
    -b|--older-acq-date)
    older_acq_date="$2"
    shift # past argument
    shift # past value
    ;;
    -l|--input-files-list)
    input_files_list="$2"
    shift # past argument
    shift # past value
    ;;
    -a|--seg-parcel-id-attribute)
    seg_parcel_id_attribute="$2"
    shift # past argument
    shift # past value
    ;;
    -x|--output-shapefile)
    output_shapefile="$2"
    shift # past argument
    shift # past value
    ;;
    -m|--do-cmpl)
    do_cmpl="$2"
    shift # past argument
    shift # past value
    ;;
    -t|--test)
    test="$2"
    shift # past argument
    shift # past value
    ;;

    --season-start)
    season_start="$2"
    shift # past argument
    shift # past value
    ;;
    --season-end)
    season_end="$2"
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

#echo COUNTRY        = "${s4c_config_file}"
#echo YEAR           = "${YEAR}"
if [ -z ${script_path} ] ; then
    echo "No script-path provided!" && usage
fi 

if [ -z ${s4c_config_file} ] ; then
    echo "No s4c-config-file provided!"
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
    if [ -z ${input_files_list} ] ; then
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

PY_CMD="python ${script_path} --s4c-config-file ${s4c_config_file} --site-id ${site_id} --config-file ${config_file} --input-shape-file ${input_shape_file} --output-data-dir ${output_data_dir} --new-acq-date ${new_acq_date} --older-acq-date ${older_acq_date} --seg-parcel-id-attribute ${seg_parcel_id_attribute} --output-shapefile ${output_shapefile} --do-cmpl ${do_cmpl} --test ${test}"
if [ -z ${input_files_list} ] ; then
    PY_CMD="${PY_CMD} --season-start ${season_start} --season-end ${season_end}"
else 
    PY_CMD="${PY_CMD} --input-files-list ${input_files_list}"
fi

CMD="${CONDA_CMD} && ${PY_CMD}"
CMD="${CMD}${CMD_TERM}"

echo "Executing ${CMD}"

#Execute the command
eval $CMD    
