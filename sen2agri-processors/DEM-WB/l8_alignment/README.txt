
###########################
#       Installation      #
###########################
$ cd directory/of/dem/product
$ mkdir build
$ cd build
$ ccmake ../DowloadSWBD/

Type "c", then "c" and normally, there are no errors, type "g"

$ make -j2


############################
#       Configuration      #
############################
Edit the file config.py.
line 35, edit app_directory and write the path to your build directory.
ex : app_directory=/home/foo/DEMProduct/build

launcher and app_otb_directory are for using a custom compiled OTB.

If your OTB installation supports OpenJPEG, skip this part.
Else, download portable OTB version for Linux.
https://www.orfeo-toolbox.org/packages/OTB-5.2.1-Linux64.run
Run the script.
Then, set the following environment variables :
export PATH=/home/amondot/Downloads/OTB-5.2.1-Linux64/bin/:$PATH
export OTB_APPLICATION_PATH=/home/amondot/Downloads/OTB-5.2.1-Linux64/lib/otb/applications/:$OTB_APPLICATION_PATH
export GDAL_DATA=/home/amondot/Downloads/OTB-5.2.1-Linux64/share/gdal/

Be sure to have access to the network.


############################
#           Usage          #
############################

Using for Sentinel-2:
s2agri_dem_product.py -i /media/S2/S2A_OPER_PRD_MSIL1C_PDMC_20150603T124211_R074_V20130708T020051_20130708T020051.SAFE/GRANULE/S2A_OPER_MSI_L1C_TL_SGS__20150530T125341_A005365_T50HQE_N06.08/S2A_OPER_MTD_L1C_TL_SGS__20150530T125341_A005365_T50HQE.xml -w /media/DEM/WD_s2agri_s2/ -o /media/DEM/OUT_s2agri_s2/ -d /media/DEM/ -s s2

Usung for L8:
s2agri_dem_product.py -i /media/DEM/Input/China/122034/LC81220342014288LGN00/LC81220342014288LGN00_MTL.txt -w /media/DEM/WD_s2agri/ -v wrs2_descending/wrs2_descending.shp -o /media/DEM/OUT_s2agri/ -d /media/DEM/ -s l8

Arguments description:
    -i Input metadata file
    -o output directory
    -t ID of the working tile/area (used to fing files and for naming)
    -d folder containing STRM tiles from CGIAR-CSI SRTM 90m
    -w working directory
    -v Shape containing l8 extents













