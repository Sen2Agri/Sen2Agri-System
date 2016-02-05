#!/bin/sh

#echo "----------------------"
#echo "RUN Crop Type Belgique"
#echo "----------------------"
#/home/achenebert/src/S5T5-scripts/CropType/CropType_Belgique.sh > /data/s2agri/output/Belgique/Belgique/cropType/CropType.log
#echo "-------------------------"
#echo "RUN Crop Type Burkina"
#echo "-------------------------"
#/home/achenebert/src/S5T5-scripts/CropType/CropType_Burkina.sh > /data/s2agri/output/Burkina/Burkina/cropType/CropType.log
#echo "----------------------"
#echo "RUN Crop Type China   "
#echo "----------------------"
#/home/achenebert/src/S5T5-scripts/CropType/CropType_China.sh > /data/s2agri/output/China/Shandong/cropType/CropType.log
echo "----------------------"
echo "RUN Crop Type Russia"
echo "----------------------"
/home/achenebert/src/S5T5-scripts/CropType/CropType_Russia.sh > /data/s2agri/output/Russia/Toula/cropType/CropType.log
#echo "-------------------------"
#echo "RUN Crop Type SouthAfrica"
#echo "-------------------------"
#/home/achenebert/src/S5T5-scripts/CropType/CropType_SouthAfrica.sh > /data/s2agri/output/SouthAfrica/SouthAfrica/cropType/CropType.log
#echo "-------------------------"
#echo "RUN Crop Type Ukraine noinsitu"
#echo "-------------------------"
#/home/achenebert/src/S5T5-scripts/CropType/CropType_Ukraine_noinsitu.sh > /data/s2agri/output/Ukraine/Ukraine_noinsitu/cropType/CropType.log
echo "-------------------------"
echo "RUN Crop Type Ukraine"
echo "-------------------------"
/home/achenebert/src/S5T5-scripts/CropType/CropType_Ukraine.sh > /data/s2agri/output/Ukraine/Ukraine/cropType/CropType.log
