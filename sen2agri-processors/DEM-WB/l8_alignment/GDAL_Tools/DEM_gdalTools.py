# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:         DEM&WaterBodyModule

   Author:          CS SI, (Alexia Mondot alexia.mondot@c-s.fr)
   Copyright:       CS SI
   Licence:         See Licence.txt

   Language:        Python
  _____________________________________________________________________________

  HISTORY

  VERSION: 01.00.00 | DATE: <11/03/2016> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________
"""

# import system libraries
import sys
import os
import datetime
import math

# project libraries
import config

# import GDAL and QGIS libraries
from osgeo import gdal, osr, ogr
gdal.UseExceptions()
import gdalconst

#import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger( 'DEM gdal Tools' )
logger.setLevel(config.level_working_modules)
fh = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s - %(message)s')
fh.setFormatter(formatter)
fh.setLevel(logging.DEBUG)
logger.addHandler(fh)

# hard-coded values
launcher_base = config.launcher
libs = config.app_directory
lib_otb = config.app_otb_directory


def get_extent_from_l8_extent(l8_vector, path, row, working_dir, epsg_code):
    """
    Extract the feature defined by path and row.
    Returns :
        - the extent of the bounding box of the feature (shapefile)
        - the coordinates of the bounding box
        - the extent of the feature (shapefile)

    :param l8_vector:
    :param path:
    :param row:
    :param working_dir:
    :param epsg_code:
    :return:
    """

    ds = ogr.Open(l8_vector)
    if ds is None:
        print "ogr_get_area: " + l8_vector + " Open failed.\n"
        return None

    lyr = ds.GetLayer(0)
    lyr.ResetReading()

    feats = []
    logger.debug(path)
    logger.debug(row)
    logger.debug(type(path))
    for feat in lyr:
        if feat.GetField("PATH") == path and feat.GetField("ROW") == row:
            feats.append(feat)

    logger.debug("feats")
    logger.debug(feats)
    if not feats:
        print "Error getting extent : row/path are incorrect or the given l8_vector file is wrong"
        sys.exit(1)
    else:
        logger.debug(feats)
        feat = feats[0]
        extent_feature = writtingOneEmptyFeatureVectorLayerWithOgr(feat, working_dir, epsg_code)
        geom = feat.GetGeometryRef()

        env = geom.GetEnvelope()  # "minX: %d, minY: %d, maxX: %d, maxY: %d" %(env[0],env[2],env[1],env[3])

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
    del(ds)
    print "extent", extent
    return extent, [env[0], env[3], env[1], env[2]], extent_feature


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
    outSpatialRef.ImportFromEPSG(int(epsg_code))

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
    del(ds)
    return outputLayer


def get_extent(image_in, EPSG_dest=None):
    """
    Get extent in wgs84
    """
    try:
        dataset = gdal.Open(str(image_in), gdalconst.GA_ReadOnly)
    except RuntimeError:  # OSError is get when the access to a folder is refused
        logger.exception("Error: Opening " + str(image_in))
        return

    # get input image extent
    geotransform = dataset.GetGeoTransform()
    if geotransform is None:
        return
    a_x = geotransform[0]
    a_y = geotransform[3]
    b_x = a_x + geotransform[1] * dataset.RasterXSize
    b_y = a_y + geotransform[5] * dataset.RasterYSize


    if EPSG_dest:
        # get input image epsg code
        spatialReference = osr.SpatialReference()
        spatialReference.ImportFromWkt(dataset.GetProjectionRef())
        codeEPSG = int(str(spatialReference.GetAttrValue("AUTHORITY", 1)))
        logger.debug("EPSG: " + str(codeEPSG))

        # EPSG_dest = 4326

        # create a geometry from coordinates
        point_a = ogr.Geometry(ogr.wkbPoint)
        point_a.AddPoint(a_x, a_y)

        point_b = ogr.Geometry(ogr.wkbPoint)
        point_b.AddPoint(b_x, b_y)

        # create coordinate transformation
        inSpatialRef = osr.SpatialReference()
        inSpatialRef.ImportFromEPSG(codeEPSG)

        outSpatialRef = osr.SpatialReference()
        outSpatialRef.ImportFromEPSG(EPSG_dest)

        coordTransform = osr.CoordinateTransformation(inSpatialRef, outSpatialRef)

        # transform point
        point_a.Transform(coordTransform)
        point_b.Transform(coordTransform)

        # print point in EPSG 4326
        return point_a.GetX(), point_a.GetY(), point_b.GetX(), point_b.GetY()
    return a_x, a_y, b_x, b_y



def get_feature_enveloppe(l8_vector, working_dir, epsg_code, scale_factor=None):
    """
    Get the enveloppe of the feature of l8_vector

    :param l8_vector:
    :param working_dir:
    :param epsg_code:
    :return:
    """
    ds = ogr.Open(l8_vector)
    if ds is None:
        print "ogr_get_area: " + l8_vector + " Open failed.\n"
        return None

    lyr = ds.GetLayer(0)
    lyr.ResetReading()

    feat = lyr.GetNextFeature()
    geom = feat.GetGeometryRef()

    env = geom.GetEnvelope()  # "minX: %d, minY: %d, maxX: %d, maxY: %d" %(env[0],env[2],env[1],env[3])
    logger.debug(env)

    A, C, D, B = env[0:4]


    if scale_factor:

        newA = (round(A/30))*30+15
        newB = (round(B/30))*30+15

        # sizeX = C-A
        # sizeY = D-B

        Ctemp = (round(C/30))*30+15  # newA + sizeX
        Dtemp = (round(D/30))*30+15  # newB + sizeY

        newC = Ctemp
        newD = Dtemp

        sizeX = newC-newA
        sizeY = newD-newB

        # print "sizeX {}".format(sizeX)

        # print "sizeX % scale_factor {}".format(sizeX % scale_factor)
        # print "sizeY % scale_factor {}".format(sizeY % scale_factor)
        # print "scale_factor-(sizeX%scale_factor) {}".format(scale_factor-(sizeX%scale_factor))
        # print "C {}".format(C)

        print "Old : {} {} {} {}".format(A, B, C, D)
        print "Old on 30 grid : {} {} {} {}".format(newA, newB, Ctemp, Dtemp)

        # scale_factor = 1/scale_factor

        # print "scale_factor : {}".format(scale_factor)

        # print "sizeX % scale_factor {}".format(sizeX % scale_factor)
        # print "sizeY % scale_factor {}".format(sizeY % scale_factor)
        # print "scale_factor-(Ctemp%scale_factor) {}".format(scale_factor-(sizeX%scale_factor))
        # print "C {}".format(C)

        if (sizeX % scale_factor) != 0:
            newC = Ctemp + (scale_factor-(sizeX%scale_factor))
        if (sizeY % scale_factor) != 0:
            newD = Dtemp + (scale_factor-(sizeY%scale_factor))

        print "New : {} {} {} {}".format(newA, newB, newC, newD)
        A = newA
        B = newB
        C = newC
        D = newD
        # sys.exit(1)

    # Create ring
    ring = ogr.Geometry(ogr.wkbLinearRing)
    ring.AddPoint(A, C)
    ring.AddPoint(A, D)
    ring.AddPoint(B, D)
    ring.AddPoint(B, C)
    ring.AddPoint(A, C)

    # Create polygon
    poly = ogr.Geometry(ogr.wkbPolygon)
    poly.AddGeometry(ring)
    feat_new = ogr.Feature(lyr.GetLayerDefn())
    feat_new.SetGeometry(poly)
    extent = writtingOneEmptyFeatureVectorLayerWithOgr(feat_new, working_dir, epsg_code)

    logger.debug("extent")
    logger.debug(extent)

    del(ds)
    if scale_factor:
        return extent, "{} {} {} {}".format(A, B, C, D)

    return extent, [env[0], env[3], env[1], env[2]]



def crop_with_mask(image, mask, output_filename):
    """
    Cut the given raster image with the vector mask given.
    Returns the cropped raster image
    Removed : -dstnodata -32768

    :param image:
    :param mask:
    :param output_filename:
    :return:
    """
    if not os.path.exists(os.path.join(os.path.dirname(__file__),"crop.sh")):
        print "crop.sh is missing"
        sys.exit(1)
    command = " ".join([os.path.join(os.path.dirname(__file__),"crop.sh"), mask, image, output_filename])

    # command = "gdalwarp -overwrite -q -cutline " + mask + " -crop_to_cutline -of GTiff " + image + " " + output_filename
    logging.debug("command {}".format(command))
    print "command {}".format(command)
    os.system(command)


def crop_with_extent(image, extent, output_filename):
    commandGDAL = "gdal_translate -projwin {} -of GTiff {} {}".format(extent,
                                                             image, output_filename)
    print "command GDAL", commandGDAL
    os.system( commandGDAL )


def get_image_info_with_gdal(image_in, get_minimum = False, get_only_epsg = False, get_stats=False):
    """
    Extract the following information from the given image:
        - epsg code
        - up left corner
        - bottom right corner
        - spacing x
        - spacing y
        - size x
        - size y
        - min of the band if asked
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
    if get_only_epsg:
        return codeEPSG

    # get input image extent
    geotransform = dataset.GetGeoTransform()
    if geotransform is None:
        return
    a_x = geotransform[0]
    a_y = geotransform[3]
    b_x = a_x + geotransform[1] * dataset.RasterXSize
    b_y = a_y + geotransform[5] * dataset.RasterYSize

    if get_minimum:
        band = dataset.GetRasterBand(1)
        min, _, _, _ = band.ComputeStatistics(False)
        return codeEPSG, a_x, a_y, b_x, b_y, geotransform[1], geotransform[5], dataset.RasterXSize, dataset.RasterYSize, min
    if get_stats:
        band = dataset.GetRasterBand(1)
        stats = band.ComputeStatistics(False)
        return codeEPSG, a_x, a_y, b_x, b_y, geotransform[1], geotransform[5], dataset.RasterXSize, dataset.RasterYSize, stats


    return codeEPSG, a_x, a_y, b_x, b_y, geotransform[1], geotransform[5], dataset.RasterXSize, dataset.RasterYSize


def convert_extent(extent, epsg_code_in, epsg_code_out = 4326):
    """
    Convert the given extent expressed in espg_code_in into epsg_code_out

    :param extent:
    :param epsg_code_in:
    :param epsg_code_out:
    :return:
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
