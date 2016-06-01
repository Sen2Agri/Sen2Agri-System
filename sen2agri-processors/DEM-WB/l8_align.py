#!/usr/bin/env python
 # -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:         MACCS

   Author:          CS SI, (Alexia Mondot alexia.mondot@c-s.fr)
   Copyright:       CS SI
   Licence:         See Licence.txt

   Language:        Python
  _____________________________________________________________________________

  HISTORY

  VERSION: 01.00.00 | DATE: <29/06/2015> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________

This script allows to align product on the bb of the given shapefile

Usage :
    python processing.py -i path/to/my/product/directory - -o path/to/my/output/dir
"""
# import system libraries
import sys
import os
import glob
import getopt
import time
import datetime

# project libraries
import config

# import GDAL and QGIS libraries
from osgeo import gdal, osr, ogr
gdal.UseExceptions()
import gdalconst

# import loggin for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger('l8_align')
logger.setLevel(config.level_working_modules)
fh = logging.FileHandler(config.fichier_log)
formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s - %(message)s')
fh.setFormatter(formatter)
fh.setLevel(logging.DEBUG)
logger.addHandler(fh)

# hard-coded values
launcher_base = config.launcher
libs = config.app_directory
lib_otb = config.app_otb_directory


def crop_with_mask(image, mask, output_filename):
    """
    Cut the given raster image with the vector mask given.
    Returns the cropped raster image
    Removed : -dstnodata -32768
    """
    command = "gdalwarp -overwrite -q -cutline " + mask + " -crop_to_cutline -of GTiff " + image + " " + output_filename
    print command
    os.system(command)


def get_image_info_with_gdal(image_in):
    """
    Extract epsg code from the given image
    """
    try:
        dataset = gdal.Open(str(image_in), gdalconst.GA_ReadOnly)
    except RuntimeError:  # OSError is get when the access to a folder is refused
        logger.exception("Error: Opening " + str(image_in))
        return

    spatialReference = osr.SpatialReference()
    spatialReference.ImportFromWkt(dataset.GetProjectionRef())
    codeEPSG = str(spatialReference.GetAttrValue("AUTHORITY", 1))
    logger.debug("EPSG: " + str(codeEPSG))
    return codeEPSG


def get_feature_enveloppe(l8_vector, working_dir, epsg_code):
    """
    Get the enveloppe of the feature of l8_vector
    """
    ds = ogr.Open(l8_vector, 1)
    if ds is None:
        print "ogr_get_area: " + l8_vector + " Open failed.\n"
        return None

    lyr = ds.GetLayer(0)
    lyr.ResetReading()

    feat = lyr.GetNextFeature()
    geom = feat.GetGeometryRef()

    env = geom.GetEnvelope()  # "minX: %d, minY: %d, maxX: %d, maxY: %d" %(env[0],env[2],env[1],env[3])
    logger.debug(env)

    # Create ring
    ring = ogr.Geometry(ogr.wkbLinearRing)
    ring.AddPoint(env[0], env[2])
    ring.AddPoint(env[0], env[3])
    ring.AddPoint(env[1], env[3])
    ring.AddPoint(env[1], env[2])
    ring.AddPoint(env[0], env[2])

    # Create polygon
    poly = ogr.Geometry(ogr.wkbPolygon)
    poly.AddGeometry(ring)
    feat_new = ogr.Feature(lyr.GetLayerDefn())
    feat_new.SetGeometry(poly)
    extent = writtingOneEmptyFeatureVectorLayerWithOgr(feat_new, working_dir, epsg_code)

    logger.debug("extent")
    logger.debug(extent)

    ds = None
    return extent, [env[0], env[3], env[1], env[2]]


def writtingOneEmptyFeatureVectorLayerWithOgr(feat, outputDirectory, epsg_code):
    """
    Writes the given feature in an empty temporary shapefile in the given directory

    feat               --    current feature
    outputDirectory    --    current directory

    Returns the created vector layer
    """
    logger.debug("begin writting layer")

    # output SpatialReference
    outSpatialRef = osr.SpatialReference()
    outSpatialRef.ImportFromEPSG(epsg_code)

    # creating an empty vector layer
    driverName = "ESRI Shapefile"
    drv = ogr.GetDriverByName(driverName)
    if drv is None:
        print "%s driver not available.\n" % driverName
        sys.exit(1)

    outputLayer = os.path.join(outputDirectory, "oneFeature_" + datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + ".shp")
    # avoiding over writting which does not work
    while os.path.isfile(outputLayer):
        outputLayer = outputLayer[:-4] + "_" + ".shp"

    logger.debug("outputVectorLayer" + str(outputLayer))

    ds = drv.CreateDataSource(outputLayer)
    if ds is None:
        print "Creation of output file failed.\n"
        sys.exit(1)

    # end creating vector layer
    # creating a layer in the vector file
    lyr = ds.CreateLayer("point_out", outSpatialRef, ogr.wkbPolygon)
    if lyr is None:
        print "Layer creation failed.\n"
        sys.exit(1)

    # creating a field inside the layer
    field_defn = ogr.FieldDefn("field", ogr.OFTString)

    if lyr.CreateField(field_defn) != 0:
        print "Creating Name field failed.\n"
        sys.exit(1)

    # creating the feature
    if lyr.CreateFeature(feat) != 0:
        print "Failed to create feature in shapefile.\n"
        sys.exit(1)

    logger.debug("end writting layer")
    ds = None
    return outputLayer


def convert_extent(extent, epsg_code_in, epsg_code_out = 4326):
    """
    Convert the given extent expressed in espg_code_in into epsg_code_out
    """
    pointX1 = extent[0]
    pointY1 = extent[1]
    pointX2 = extent[2]
    pointY2 = extent[3]

    # Spatial Reference System
    inputEPSG = int(epsg_code_in)
    outputEPSG = int(epsg_code_out)

    # create a geometry from coordinates
    point1 = ogr.Geometry(ogr.wkbPoint)
    point1.AddPoint(pointX1, pointY1)
    point2 = ogr.Geometry(ogr.wkbPoint)
    point2.AddPoint(pointX2, pointY2)

    # create coordinate transformation
    inSpatialRef = osr.SpatialReference()
    inSpatialRef.ImportFromEPSG(inputEPSG)

    outSpatialRef = osr.SpatialReference()
    outSpatialRef.ImportFromEPSG(outputEPSG)

    coordTransform = osr.CoordinateTransformation(inSpatialRef, outSpatialRef)

    # transform point
    point1.Transform(coordTransform)
    point2.Transform(coordTransform)

    # print point in EPSG 4326
    return point1.GetX(), point1.GetY(), point2.GetX(), point2.GetY()


def update_txt(txt_file, shape_env_points, output_dir, epsg_code):
    """
    Update MTL file with new extent
    """
    # convert shape_env_points in meters in lat/long
    shape_env_points_wgs84 = convert_extent(shape_env_points, epsg_code)
    logger.debug(shape_env_points)

    logger.debug("reading " + txt_file)
    f_in = open(txt_file, 'r')

    file_out = os.path.join(output_dir, os.path.basename(txt_file))
    logger.debug(file_out)
    f_out = open(file_out, 'w')

    for line in f_in:
        # UTM coordinates
        if "CORNER_UL_PROJECTION_X_PRODUCT" in line:
            line = "    CORNER_UL_PROJECTION_X_PRODUCT = " + str(shape_env_points[0]) + "\n"
        if "CORNER_UL_PROJECTION_Y_PRODUCT" in line:
            line = "    CORNER_UL_PROJECTION_Y_PRODUCT = " + str(shape_env_points[1]) + "\n"
        if "CORNER_UR_PROJECTION_X_PRODUCT" in line:
            line = "    CORNER_UR_PROJECTION_X_PRODUCT = " + str(shape_env_points[2]) + "\n"
        if "CORNER_UR_PROJECTION_Y_PRODUCT" in line:
            line = "    CORNER_UR_PROJECTION_Y_PRODUCT = " + str(shape_env_points[1]) + "\n"
        if "CORNER_LL_PROJECTION_X_PRODUCT" in line:
            line = "    CORNER_LL_PROJECTION_X_PRODUCT = " + str(shape_env_points[0]) + "\n"
        if "CORNER_LL_PROJECTION_Y_PRODUCT" in line:
            line = "    CORNER_LL_PROJECTION_Y_PRODUCT = " + str(shape_env_points[3]) + "\n"
        if "CORNER_LR_PROJECTION_X_PRODUCT" in line:
            line = "    CORNER_LR_PROJECTION_X_PRODUCT = " + str(shape_env_points[2]) + "\n"
        if "CORNER_LR_PROJECTION_Y_PRODUCT" in line:
            line = "    CORNER_LR_PROJECTION_Y_PRODUCT = " + str(shape_env_points[3]) + "\n"
        # latlong coordinates
        if "CORNER_UL_LAT_PRODUCT" in line:
            line = "    CORNER_UL_LAT_PRODUCT = " + str(shape_env_points_wgs84[0]) + "\n"
        if "CORNER_UL_LON_PRODUCT" in line:
            line = "    CORNER_UL_LON_PRODUCT = " + str(shape_env_points_wgs84[1]) + "\n"
        if "CORNER_UR_LAT_PRODUCT" in line:
            line = "    CORNER_UR_LAT_PRODUCT = " + str(shape_env_points_wgs84[2]) + "\n"
        if "CORNER_UR_LON_PRODUCT" in line:
            line = "    CORNER_UR_LON_PRODUCT = " + str(shape_env_points_wgs84[1]) + "\n"
        if "CORNER_LL_LAT_PRODUCT" in line:
            line = "    CORNER_LL_LAT_PRODUCT = " + str(shape_env_points_wgs84[0]) + "\n"
        if "CORNER_LL_LON_PRODUCT" in line:
            line = "    CORNER_LL_LON_PRODUCT = " + str(shape_env_points_wgs84[3]) + "\n"
        if "CORNER_LR_LAT_PRODUCT" in line:
            line = "    CORNER_LR_LAT_PRODUCT = " + str(shape_env_points_wgs84[2]) + "\n"
        if "CORNER_LR_LON_PRODUCT" in line:
            line = "    CORNER_LR_LON_PRODUCT = " + str(shape_env_points_wgs84[3]) + "\n"

        f_out.write(line)

    f_in .close()
    f_out.close()


def usage(return_code = 1):
    """
    Usage
    """
    print "Usage : python l8_align.py -i input/dir/ -v landsat8_extents.shp -o output/data/folder -w working/dir"
    print 'exemple : python l8_align.py -i /media/Data/LC81980302013177LGN01/ -v extent.shp \
        -o /media/Data/LC81980302013177LGN01/OUTPUT_DIR -w /media/Data/LC81980302013177LGN01/working_dir'
    sys.exit(return_code)


def get_arguments():
    # ####### MANAGING ARGUMENTS ########
    # TODO : set path to downloadwb as an argument

    # check inputs
    logger.info("######## MANAGING ARGUMENTS ########")
    logger.debug("Checking arguments")
    print len(sys.argv)
    if len(sys.argv) != 9:
        usage()

    try:
        opts, args = getopt.getopt(sys.argv[1:], "i:o:v:w:", ["--input_dir=", "--output_directory="])
    except getopt.GetoptError:
        usage(2)

    input_dir = output_data_directory = working_dir = l8_vector = None

    for opt, arg in opts:
        if opt in ("-i", "--input_dir"):
            input_dir = os.path.realpath(arg)
        elif opt in ("-o", "--output_directory"):
            output_data_directory = os.path.realpath(arg)
        elif opt in ("-v"):
            l8_vector = arg
        elif opt in ("-w"):
            working_dir = arg

    logger.debug("Arguments ok")

    # crating directories if they don't exist
    logger.debug("Managing output directories")
    if not os.path.isdir(output_data_directory):
        os.mkdir(output_data_directory)
    if not os.path.isdir(working_dir):
        os.mkdir(working_dir)
    if not os.path.isdir(input_dir):
        print "Error, input dir is missing "
        usage(2)
    logger.debug("Output directories ok")

    # checking if inputs exist
    if not input_dir:
        print "input image missing"
        sys.exit(2)
    if not l8_vector:
        print "l8_vector missing"
        sys.exit(2)

    return input_dir, l8_vector, output_data_directory, working_dir


time_start = time.time()

input_dir, l8_vector, output_data_directory, working_dir = get_arguments()

image = glob.glob(os.path.join(input_dir, "*.TIF"))[0]
logger.debug(image)

# txt file
txt_file = glob.glob(os.path.join(input_dir, "*_MTL.txt"))[0]
logger.debug(txt_file)

epsg_code = get_image_info_with_gdal(image)
shape_temp = os.path.join(working_dir, os.path.splitext(os.path.basename(l8_vector))[0] + "_reprojected" + os.path.splitext(os.path.basename(l8_vector))[1])
logger.debug(shape_temp)
ogr_command = "ogr2ogr -t_srs EPSG:" + str(epsg_code) + " " + shape_temp + " " + l8_vector
os.system(ogr_command)

# shape_env = os.path.join(working_dir, os.path.splitext(os.path.basename(l8_vector))[0] + "_env" + os.path.splitext(os.path.basename(l8_vector))[1])
shape_env, shape_env_points = get_feature_enveloppe(shape_temp, working_dir, int(epsg_code))
update_txt(txt_file, shape_env_points, output_data_directory, epsg_code)

for tif_file in glob.glob(os.path.join(input_dir, "*.TIF")):
    data_out = os.path.join(output_data_directory, os.path.basename(tif_file))
    crop_with_mask(tif_file, shape_env, data_out)

interval = time.time() - time_start
logger.info('Total time in seconds:' + str(interval))
