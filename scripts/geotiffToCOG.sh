#### call example  
###
###  ./gTranslate2COG S2A_MSIL2A_20170803T094031_N0205_R036_T34UFF_20170803T094046.SAFE OUT/
###
### Atentie la / de la OUT
######

#### input path is here but Must be arg
if [ "$#" -ne 2 ]; then
 echo "gTranslate2COG S2A_MSIL2A_20170803T094031_N0205_R036_T34UFF_20170803T094046.SAFE OUT/"; 
 echo " Trebuie setat inputPath mai jos in script";
 echo " OUTPUT-ul se trimite  ca argument" 

 exit 1; 
fi

### !!!!!!   de setat pentru fiecare bloc !!!! sau gasita alta optiune. deocamdata asta-i cea mai simpla (pt mine)  ###
#
#inputPath="/mnt/archive/maccs_def/lithuania/l2a/"
inputPath="/d38/sen4cap/czech_2016/l2a/"


###
outputPath=${2}
inputProduct=${1}

##inputProduct=$(basename ${1})

###move not .TIF file ##
rsync -av --exclude="*.TIF" --exclude='*_QCK_*' --exclude="PRIVATE" --exclude="*.EEF" --exclude="*.log" --exclude=".DBL"  ${inputPath}${inputProduct} ${outputPath}

### get list of .TIF files
listTIF=$(find ${inputPath}${1} -maxdepth 2 -iname "*.TIF")

### one by oone from lit - translate
for f in $listTIF; do 
#  pName=${inputProduct}
  o1=${f#$inputPath}
#  echo "gdal_translate $f ${outputPath}${o1} -co TILED=YES -co COPY_SRC_OVERVIEWS=YES -co COMPRESS=LZW"
#  gdal_translate $f ${outputPath}${o1} -co TILED=YES -co COPY_SRC_OVERVIEWS=YES -co COMPRESS=LZW
  echo "gdal_translate $f ${outputPath}${o1} -co TILED=YES -co COPY_SRC_OVERVIEWS=YES"
  gdal_translate $f ${outputPath}${o1} -co TILED=YES -co COPY_SRC_OVERVIEWS=YES
done

#gdal_translate $1 out.tif -co TILED=YES -co COPY_SRC_OVERVIEWS=YES -co COMPRESS=LZW 
