#!/bin/bash

#echo "Running CropMask with insitu for Ukraine ..."
./CropMask-Ukraine.sh > ~/logs/CropMask-Ukraine-insitu.txt
#echo "Running CropMask with insitu for South Africa ..."
./CropMask-SouthAfrica.sh > ~/logs/CropMask-SouthAfrica-insitu.txt
#echo "Running CropMask with insitu for Sudmipy West ..."
./CropMask-CSudmipy-O.sh > ~/logs/CropMask-CSudmipy-O-insitu.txt
#echo "Running CropMask with insitu for Sudmipy East ..."
./CropMask-CSudmipy-E.sh > ~/logs/CropMask-CSudmipy-E-insitu.txt

#echo "Running CropType with insitu mask for Ukraine ..."
./CropType-Ukraine.sh > ~/logs/CropType-Ukraine-insitu.txt
#echo "Running CropType with insitu mask for South Africa ..."
./CropType-SouthAfrica.sh > ~/logs/CropType-SouthAfrica-insitu.txt
#echo "Running CropType with insitu mask for Sudmipy West ..."
./CropType-CSudmipy-O.sh > ~/logs/CropType-CSudmipy-O-insitu.txt
#echo "Running CropType with insitu mask for Sudmipy West ..."
./CropType-CSudmipy-E.sh > ~/logs/CropType-CSudmipy-E-insitu.txt

echo "Running CropMask with no insitu for Sudmipy West ..."
./CropMask-CSudmipy-O-noinsitu.sh > ~/logs/CropMask-CSudmipy-O-noinsitu.txt
echo "Running CropMask with no insitu for Sudmipy East ..."
./CropMask-CSudmipy-E-noinsitu.sh > ~/logs/CropMask-CSudmipy-E-noinsitu.txt

echo "Running CropType with no insitu mask for Sudmipy West ..."
./CropType-CSudmipy-O-noinsitu.sh > ~/logs/CropType-CSudmipy-O-noinsitu.txt
echo "Running CropType with no insitu mask for Sudmipy East ..."
./CropType-CSudmipy-E-noinsitu.sh > ~/logs/CropType-CSudmipy-E-noinsitu.txt

echo "Running CropMask with no insitu for Ukraine ..."
./CropMask-Ukraine-noinsitu.sh > ~/logs/CropMask-Ukraine-noinsitu.txt
echo "Running CropMask with no insitu for South Africa ..."
./CropMask-SouthAfrica-noinsitu.sh > ~/logs/CropMask-SouthAfrica-noinsitu.txt

echo "Running CropType with no insitu mask for Ukraine ..."
./CropType-Ukraine-noinsitu.sh > ~/logs/CropType-Ukraine-noinsitu.txt
echo "Running CropType with no insitu mask for South Africa ..."
./CropType-SouthAfrica-noinsitu.sh > ~/logs/CropType-SouthAfrica-noinsitu.txt

