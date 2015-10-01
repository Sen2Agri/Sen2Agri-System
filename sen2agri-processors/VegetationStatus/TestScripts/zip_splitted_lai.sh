#! /bin/bash

if [ $# -lt 1 ]
then
  echo "Usage: $0 <out folder name>"
  echo "The output directory should be given" 1>&2  
  exit
fi

OUT_FOLDER=$1
FITTED_FILE="$OUT_FOLDER/FittedFilesList.txt"
REPROCESSED_FILE="$OUT_FOLDER/ReprocessedFilesList.txt"


while read line           
do           
    echo "Zipping file $line as "$line".zip"
    zip -j $line".zip" $line
done <$FITTED_FILE    

while read line           
do  
    echo "Zipping file $line as "$line".zip"         
    zip -j $line".zip" $line
done <$REPROCESSED_FILE    