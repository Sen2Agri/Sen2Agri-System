#!/bin/bash

function usage() {
    echo "Usage: ./generate_grassland_mowing_input_shp.sh --user <user> --script-path <script_path> --site-id <site_id> --config-file <config_file> --path <working_path> --year <year> --filter-ctnum <filter_ct_num> --force"
    exit 1
}

conda_user="sen2agri-service"
script_path="generate_grassland_mowing_input_shp.py"
config_file="/etc/sen2agri/sen2agri.conf"
filter_ctnum=""
add_decl_cols=""
filter_ids_table=""
force=0


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
    -s|--site-id)
    site_id="$2"
    shift # past argument
    shift # past value
    ;;
    -c|--config-file)
    config_file="$2"
    shift # past argument
    shift # past value
    ;;
    -o|--path)
    path="$2"
    shift # past argument
    shift # past value
    ;;
    -y|--year)
    year="$2"
    shift # past argument
    shift # past value
    ;;
    -f|--filter-ctnum)
    filter_ctnum="$2"
    shift # past argument
    shift # past value
    ;;
    -a|--add-decl-cols)
    add_decl_cols="$2"
    shift # past argument
    shift # past value
    ;;

    -t|--filter-ids-table)
    filter_ids_table="$2"
    shift # past argument
    shift # past value
    ;;
    --force)
    force=1
    shift # past argument
    shift # past value
    ;;
    --srid)
    srid="$2"
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

if [ -z ${site_id} ] ; then
    echo "No site-id provided!" && usage
fi 
if [ -z ${config_file} ] ; then
    echo "No config-file provided!"
fi 
if [ -z ${path} ] ; then
    echo "No working path provided!" && usage
fi 
if [ -z ${year} ] ; then
    echo "No year provided!" && usage
fi 
if [ -z ${filter_ctnum} ] ; then
    echo "No filter-ctnum provided!"
fi 
if [ -z ${filter_ids_table} ] ; then
    echo "No filter-ids-table provided!"
fi 


echo "$USER"
echo "${conda_user}"

if [ $USER == ${conda_user} ] ; then
    echo "Activating conda sen4cap for user $USER"
    CONDA_CMD="source ~/.bashrc && conda activate sen4cap"
    CMD_TERM=""
else 
    echo "Activating conda sen4cap from user $USER for user ${conda_user}"
    CONDA_CMD="sudo su -l ${conda_user} -c 'conda activate sen4cap"
    CMD_TERM="'"
fi    

CFG_FILE=""
if [ ! -z ${config_file} ] ; then
    CFG_FILE="--config-file ${config_file}" 
fi
FORCE_OPT=""
if [ "${force}" == "1" ] ; then
    FORCE_OPT="--force" 
fi

FILTER_CT_NUM_OPT=""
if [ ! -z "${filter_ctnum}" ] ; then
    FILTER_CT_NUM_OPT="--filter-ctnum ${filter_ctnum}" 
fi

ADD_DECL_COLS_OPT=""
if [ ! -z "${add_decl_cols}" ] ; then
    ADD_DECL_COLS_OPT="--add-decl-cols ${add_decl_cols}" 
fi

FILTER_IDS_TABLE_OPT=""
if [ ! -z "${filter_ids_table}" ] ; then
    FILTER_IDS_TABLE_OPT="--filter-ids-table ${filter_ids_table}" 
fi

SRID_OPT=""
if [ ! -z "${srid}" ] ; then
    SRID_OPT="--srid ${srid}" 
fi

PY_CMD="python ${script_path} ${CFG_FILE} --site-id ${site_id} --path ${path} --year ${year} ${FILTER_CT_NUM_OPT} ${ADD_DECL_COLS_OPT} ${FILTER_IDS_TABLE_OPT} ${FORCE_OPT} ${SRID_OPT}"

CMD="${CONDA_CMD} && ${PY_CMD}"
CMD="${CMD}${CMD_TERM}"

echo "Executing ${CMD}"

#Execute the command
eval $CMD    

