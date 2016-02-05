#!/bin/sh

echo "----------------------"
echo "RUN Crop Mask Belgique"
echo "----------------------"
/home/achenebert/src/S5T5-scripts/CropMask/CropMask_Belgique.sh > /data/s2agri/output/Belgique/Belgique/cropMask/cropMask.log
#echo "-------------------------"
#echo "RUN Crop Mask Burkina"
#echo "-------------------------"
#/home/achenebert/src/S5T5-scripts/CropMask/CropMask_Burkina.sh > /data/s2agri/output/Burkina/Burkina/cropMask/cropMask.log
echo "----------------------"
echo "RUN Crop Mask China   "
echo "----------------------"
/home/achenebert/src/S5T5-scripts/CropMask/CropMask_China.sh > /data/s2agri/output/China/Shandong/cropMask/cropMask.log
echo "----------------------"
echo "RUN Crop Mask Russia"
echo "----------------------"
/home/achenebert/src/S5T5-scripts/CropMask/CropMask_Russia.sh > /data/s2agri/output/Russia/Toula/cropMask/cropMask.log
#echo "-------------------------"
#echo "RUN Crop Mask SouthAfrica"
#echo "-------------------------"
#/home/achenebert/src/S5T5-scripts/CropMask/CropMask_SouthAfrica.sh > /data/s2agri/output/SouthAfrica/SouthAfrica/cropMask/cropMask.log
echo "-------------------------"
echo "RUN Crop Mask Ukraine noinsitu"
echo "-------------------------"
/home/achenebert/src/S5T5-scripts/CropMask/CropMask_Ukraine_noinsitu.sh > /data/s2agri/output/Ukraine/Ukraine_noinsitu/cropMask/cropMask.log
echo "-------------------------"
echo "RUN Crop Mask Ukraine"
echo "-------------------------"
/home/achenebert/src/S5T5-scripts/CropMask/CropMask_Ukraine.sh > /data/s2agri/output/Ukraine/Ukraine/cropMask/cropMask.log
