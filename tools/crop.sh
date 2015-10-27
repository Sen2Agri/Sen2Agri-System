#!/usr/bin/zsh
function crop_image() {
        echo otbcli_ExtractROI -in "$1" -out "$2" "$3" -startx "$4" -starty "$5" -sizex "$6" -sizey "$7"
        otbcli_ExtractROI -in "$1" -out "$2" "$3" -startx "$4" -starty "$5" -sizex "$6" -sizey "$7"
}

function process_dir() {
    for dir in "$1"/*; do
        local dest="$(dirname ${dir})"
        dest="$(basename "${dest}")"
        local dirname="$(basename $dir)"
        dest="$dest/$dirname"

        mkdir -p $dest/MASK
        local -a xml_name
        xml_name=( $dir/*.xml )
        cp $xml_name $dest

        sed -e 's|<NB_ROWS>.*</NB_ROWS>|<NB_ROWS>1000</NB_ROWS>|' -e 's|<NB_COLS>.*</NB_COLS>|<NB_COLS>1000</NB_COLS>|' -i $dest/$(basename "$xml_name")

        for img in $dir/*.TIF; do
            local name=$(basename $img)
            crop_image $img "$dest/$name" int16 $2 $3 $4 $5 &
        done

        local -a mask
        local -a mask_name

        mask=( $dir/MASK/*_NUA.TIF )
        mask_name="$(basename "$mask")"
        crop_image "$mask" "$dest/MASK/$mask_name" int16 $2 $3 $4 $5 &

        mask=( $dir/MASK/*_DIV.TIF )
        mask_name="$(basename "$mask")"
        crop_image "$mask" "$dest/MASK/$mask_name" uint8 $2 $3 $4 $5 &

        mask=( $dir/MASK/*_SAT.TIF )
        mask_name="$(basename "$mask")"
        crop_image "$mask" "$dest/MASK/$mask_name" uint8 $2 $3 $4 $5 &

        wait
    done
}

#process_dir /mnt/Imagery_S2A/L2A/Spot4-T5/South\ Africa 1500 2000 1000 1000
#process_dir /mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-E_LEVEL2A 3200 1400 1000 1000 # for composite
process_dir /mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-E_LEVEL2A 1750 8000 1000 1000 # for crop mask/type
#process_dir /mnt/Imagery_S2A/L2A/Spot4-T5/CSudmipy-O_LEVEL2A 4600 8000 1000 1000
process_dir /mnt/Imagery_S2A/L2A/Spot4-T5/Ukraine 1500 1600 1000 1000
