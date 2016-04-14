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

  VERSION: 01.00.00 | DATE: <29/05/2015> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________

This script allows to create a DTM product according to a RE tile.
It produces the following files :
    - DTM at L2 resolution
    - DTM at L2_Coarse resolution
    - WaterBodies  at L2_Coarse resolution
    - Slope at L2 resolution
    - Slope at L2_Coarse resolution
    - Aspect at L2 resolution
    - Aspect at L2_Coarse resolution

Usage :
    python processing.py -i path/to/my/product/directory - -o path/to/my/output/dir
"""
# import system libraries
import sys
import os
import glob
import getopt
import shutil
import math
import time
import re
import datetime

# project libraries
import config

# import GDAL and QGIS libraries
from osgeo import gdal, osr, ogr
gdal.UseExceptions()
import gdalconst

#import loggin for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger( 'DEM_Product' )
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


def get_extent_from_l8_extent(l8_vector, path, row, tile_id, working_dir, epsg_code):
    """
    Extract the feature defined by path and row.
    Returns :
        - the extent of the bounding box of the feature (shapefile)
        - the coordinates of the bounding box
        - the extent of the feature (shapefile)
    """
    ds = ogr.Open(l8_vector, 1)
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
    ds = None
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
    ds = None
    return outputLayer


def get_extent(image_in):
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

    # get input image epsg code
    spatialReference = osr.SpatialReference()
    spatialReference.ImportFromWkt(dataset.GetProjectionRef())
    codeEPSG = int(str(spatialReference.GetAttrValue("AUTHORITY", 1)))
    logger.debug("EPSG: " + str(codeEPSG))

    EPSG_dest = 4326

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


def get_tiles(points):
    """
    Returns a list of dtm tiles covering the given extent
    """
    # extract
    a_x, a_y, b_x, b_y = points
    logger.debug("points ")
    logger.debug(points)

    # check a is upper left corner
    if a_x < b_x and a_y > b_y:
        a_bb_x = int(math.floor(a_x / 5) * 5)
        a_bb_y = int(math.floor((a_y + 5) / 5) * 5)
        b_bb_x = int(math.floor((b_x + 5) / 5) * 5)
        b_bb_y = int(math.floor(b_y / 5) * 5)

        print "bounding box", a_bb_x, a_bb_y, b_bb_x, b_bb_y

        # get list of zip
        x_numbers_list = [((x + 180) / 5) + 1 for x in range(min(a_bb_x, b_bb_x), max(a_bb_x, b_bb_x), 5)]
        x_numbers_list_format = ["%02d" % (x,) for x in x_numbers_list]
        y_numbers_list = [(60 - x) / 5 for x in range(min(a_bb_y, b_bb_y), max(a_bb_y, b_bb_y), 5)]
        y_numbers_list_format = ["%02d" % (x,) for x in y_numbers_list]

        logger.debug(x_numbers_list_format)
        logger.debug(y_numbers_list_format)

        srtm_zips = ["srtm_" + str(x) + "_" + str(y) + ".zip" for x in x_numbers_list_format for y in y_numbers_list_format]

        logger.debug("zip to take ")
        logger.debug(srtm_zips)

        return srtm_zips


def unzip_files(dtm_folder):
    """
    This function unzip all zip of the given folder
    """
    # unzip downloaded files
    for zip_file in glob.glob(os.path.join(dtm_folder, "*.zip")):
        # fileName, fileExtension = os.path.splitext( os.path.basename( zip_file ))
        command_unzip = "unzip " + zip_file + " -d " + dtm_folder
        logger.debug("command : " + command_unzip)
        os.system(command_unzip)


def get_image_info_with_gdal(image_in, get_minimum = False):
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

    return codeEPSG, a_x, a_y, b_x, b_y, geotransform[1], geotransform[5], dataset.RasterXSize, dataset.RasterYSize


def bandmath(images, expression, output_filename, type = ""):
    """
    This function applies otbcli_BandMath to image with the given expression

    Keyword arguments:
        image               --    raster layer to process
        expression          --    expression to apply
        output_filename     --    output raster image
        type                --    type of the output image
    """
    command = ('%s '
                '-il %s '
                '-exp "%s" '
                '-out "%s"'
                % ("otbcli_BandMath",
                    " ".join(images),
                    expression,
                    output_filename))
    if type:
        command += " " + type

    logger.info("command : " + command)
    os.system(command)


def superimpose(image_in, image_ref, image_out, lms = 4, type = ""):
    """
    Runs the OTB Application Superimpose
    """
    command = ('%s '
                '-inr "%s" '
                '-inm "%s" '
                '-interpolator "linear" '
                '-lms "%s" '
                '-out "%s"'
                % ("otbcli_Superimpose",
                    image_ref,
                    image_in,
                    str(lms),
                    image_out))

    if type:
        command += " " + type

    logger.info("command : " + command)
    os.system(command)


def crop_with_mask(image, mask, output_filename):
    """
    Cut the given raster image with the vector mask given.
    Returns the cropped raster image
    Removed : -dstnodata -32768
    """
    command = "gdalwarp -overwrite -q -cutline " + mask + " -crop_to_cutline -of GTiff " + image + " " + output_filename
    logger.info(command)
    os.system(command)


def image_envelope(image, outputDirectory):
    """
    Runs OTB Application Image enveloppe for the given image.
    Returns the output shapefile
    """
    output = None
    if image and outputDirectory:
        output = os.path.join(outputDirectory, os.path.splitext(str(image))[0] + "_extent.shp")

        # create the Segmentation application
        command = "otbcli_ImageEnvelope "
        # set the arguments of the application
        command += " -in " + image
        command += " -out " + output

        os.system(command)

    return output


def gdal_reprojection_and_no_data(image_in, image_out, image_out_srs, image_in_srs = "EPSG:4326"):
    """
    Runs a reprojection of image_in into image_out with the new image_out_src coordinate system

    Keyword arguments:
        image_in          --    input image
        image_out         --    output image
        image_out_srs     --    CRS of the output image
    """
    # input no data is set to -32768 and as output, no data are set to 0 (for the sea)
    command = "gdalwarp "
    command += "-t_srs " + str(image_out_srs) + " -of GTiff "  # " -srcnodata -32768 -dstnodata 0 -of GTiff "
    command += image_in + " " + image_out

    logger.debug("command reprojection : " + command)
    os.system(command)


def DownloadSWBDTiles_dl(image, output_directory):
    """
    This function calls the otb application DownloadSRTMTiles with the given image.
    The STRM tiles will be downloaded in the outputDirectory.

    Keyword arguments:
        image               --    raster layer to process
        outputDirectory     --    working directory
    """
    if image and output_directory:

        launcher = launcher_base + " DownloadSWBDTiles " + libs
        command = ('%s '
                    '-il "%s" '
                    '-mode "download" '
                    '-mode.download.outdir "%s" '
                    % (launcher,
                       image,
                       output_directory))

    logger.info("command : " + command)
    os.system(command)


def gdal_ressampling(input_image, output_image, ratio_for_otb):
    """
    Applies a ressampling on the given image with the given ratio
    """
    # ratio 2 : -> 200%

    command = 'gdal_translate -outsize ' + str(ratio_for_otb * 100) + '% ' + \
        str(ratio_for_otb * 100) + '% ' + input_image + ' ' + output_image

    logger.info("command : " + command)
    os.system(command)


def rasterizationWithSupportImage(vectorIn, supportImage, outputDirectory, field = None):
    """
    Runs the Rasterization OTB application.

    Keyword arguments:
        vectorIn         --    vector to rasterize
        supportImage     --    support image
        outputDirectory  --    working directory
        field            --    specific field to rasterize
    """

    if vectorIn and supportImage and outputDirectory:
        # create the output
        if not field:
            field = ""
        filenameWithoutExtension = os.path.splitext(vectorIn)[0]
        output = os.path.join(outputDirectory, filenameWithoutExtension + "_" + \
                               field + "_raster.tif")  # + temp[index:]

        # create the Segmentation application
        command = ('%s '
                    '-in "%s" '
                    '-out "%s" uint8 '
                    '-im "%s" '
                    '-background "%s" '
                    % ("otbcli_Rasterization",
                        vectorIn,
                        output,
                        supportImage,
                        str(0)))
        if field:
            command += '-mode "attribute" '
            command += '-mode.attribute.field ' + field
        else:
            command += '-mode "binary" '
            command += '-mode.binary.foreground "1"'

        logger.info("command : " + command)
        os.system(command)

    return output


def clipVectorWithOGR_Extent(vectorIn, extent, outputDirectory, keyword = "", sqlite = False):
    """
    Calls the command ogr2ogr to clip the vector layer to the given extent.

    Keyword Arguments :
        vectorIn        --    vector to crop
        extent          --    list of the 4 coordinates to define the extent
        outputDirectory --    directory to save the cropped file

    Returns the path of the cropped shapefile
    """
    if sqlite:
        newvector = os.path.join(outputDirectory, os.path.basename(vectorIn)[:-7] + "_cropped_" + datetime.datetime.now().strftime("%H%M%S") + keyword + ".shp")
    else:
        newvector = os.path.join(outputDirectory, os.path.basename(vectorIn)[:-4] + "_cropped_" + datetime.datetime.now().strftime("%H%M%S") + keyword + ".shp")
    commandOGR = "ogr2ogr -clipsrc " + str(extent[0]) + " " + str(extent[1]) + " " + str(extent[2]) + " " + str(extent[3]) + " " + newvector + " " + vectorIn
    os.system(commandOGR)
    return newvector


def get_DTM(data, dem_tiles, output_dem_directory, tile_id, extent_wgs84_vector):
    """
    Behaviour for downloaded tiles from DownloadSRTMTiles application.

    It runs the following steps:
        -unzip all downloaded tiles
        -creates a vrt with all hgt files
        -crop the vrt with the given image as reference
    """
    # ####### MANAGING DTM ########
    logger.info("######## MANAGING DTM ########")
    working_dir_dtm = os.path.join(dem_tiles, "working_dir_dtm")
    if not os.path.isdir(working_dir_dtm):
        os.mkdir(working_dir_dtm)

    epsg_code, a_x, a_y, b_x, b_y, spacing_x, spacing_y, size_x, size_y = get_image_info_with_gdal(data)
    logger.debug("epsg tile : " + str(epsg_code))

    # make a vrt with all srtm tif tiles
    output_vrt = os.path.join(working_dir_dtm, tile_id + ".vrt")
    if not os.path.isfile(output_vrt):
        logger.debug("output_vrt : " + output_vrt)
        logger.debug("Bulding vrt")
        command_build_vrt = "gdalbuildvrt " + output_vrt
        for srtm in glob.glob(os.path.join(dem_tiles, "*.tif")):
            command_build_vrt += " " + srtm
        logger.info(command_build_vrt)
        os.system(command_build_vrt)
    logger.debug("VRT ok")

    dtm_temp_nodata = os.path.join(working_dir_dtm, tile_id + "_DTM_no_data.tif")
    bandmath([output_vrt], "(im1b1==-32768?0:im1b1)", dtm_temp_nodata)

    dtm_temp = os.path.join(working_dir_dtm, tile_id + "_DTM_90m_no_data.tif")
    gdal_reprojection_and_no_data(dtm_temp_nodata, dtm_temp, "EPSG:" + str(epsg_code))
    logger.debug("vrt reprojete et nodata set : " + dtm_temp)

    shape_temp = os.path.join(working_dir, os.path.splitext(os.path.basename(l8_vector))[0] + "_reprojected" + os.path.splitext(os.path.basename(l8_vector))[1])
    ogr_command = "ogr2ogr -t_srs EPSG:" + str(epsg_code) + " " + shape_temp + " " + extent_wgs84_vector
    os.system(ogr_command)

    dtm_temp_nodata_r = os.path.join(working_dir_dtm, tile_id + "_DTM_90m_no_data_cropped.tif")
    crop_with_mask(dtm_temp, shape_temp, dtm_temp_nodata_r)

    # superimpose the vrt on the input data (5m resolution)
    dtm_l1 = os.path.join(working_dir_dtm, tile_id + "_DTM_5m.tif")
    if not os.path.isfile(dtm_l1):
        superimpose(dtm_temp, data, dtm_l1, 40)

    # ressampling to L2 and L2_Coarse resolutions
    logger.debug("Ressampling")
    dtm_l2_no_data_to_zero = os.path.join(output_dem_directory, tile_id + "_ALT.TIF")
    dtm_l2_coarse_no_data_to_zero = os.path.join(output_dem_directory, tile_id + "_ALC.TIF")
    if not os.path.isfile(dtm_l2_no_data_to_zero):
        gdal_ressampling(dtm_l1, dtm_l2_no_data_to_zero, ratio_l2)
    if not os.path.isfile(dtm_l2_coarse_no_data_to_zero):
        gdal_ressampling(dtm_l1, dtm_l2_coarse_no_data_to_zero, ratio_l2_coarse)
    logger.debug("Ressampling ok")

    logger.info("Output DTM")
    logger.info("\t DTM L1 resolution          : " + dtm_l1)
    # logger.info( "\t DTM L3A resolution          : " + dtm_l1 )
    logger.info("\t DTM L2 resolution        : " + dtm_l2_no_data_to_zero)
    logger.info("\t DTM L2 coarse resolution : " + dtm_l2_coarse_no_data_to_zero)

    logger.debug("######## END MANAGING DTM ########")
    return dtm_l2_no_data_to_zero, dtm_l2_coarse_no_data_to_zero, dtm_l1
    # ####### END MANAGING DTM ########


def get_wb(data, output_dem_directory, working_dir, dtm_l2_coarse, tile_id):
    """
    Behaviour for downloaded tiles from DownloadSRTMTiles application.

    It runs the following steps:
        -unzip all downloaded tiles
        -creates a vrt with all hgt files
        -crop the vrt with the given image as reference
    """
    # ####### MANAGING WB ########
    logger.info("######## MANAGING WB ########")
    working_dir_wb = os.path.join(working_dir, "working_dir_wb")
    if not os.path.isdir(working_dir_wb):
        os.mkdir(working_dir_wb)

    # dowload wb tiles
    logger.debug("Downloading srtm wb tiles")
    DownloadSWBDTiles_dl(data, working_dir_wb)
    logger.debug("Srtm wb tiles ok")

    logger.debug("Unziping srtm tiles")

    epsg_code, a_x, a_y, b_x, b_y, _, _, _, _ = get_image_info_with_gdal(dtm_l2_coarse)

    # unzip downloaded files
    for zip_file in glob.glob(os.path.join(working_dir_wb, "*.zip")):
        # fileName, fileExtension = os.path.splitext( os.path.basename( zip_file ))
        command_unzip = "unzip " + zip_file + " -d " + working_dir_wb
        logger.debug("command_unzip" + command_unzip)
        os.system(command_unzip)
    logger.debug("Unzip ok")

    logger.debug("Reproject DTM coarse in wgs84")
    # set a projection to all shapefiles and rasterize on dtm at L2_Coarse resolution
    logger.debug("Assigniing projection to wb")
    listWaterBodies = []
    for shapefile in glob.glob(os.path.join(working_dir_wb, "*.shp")):
        # set projection to wb
        projfile = os.path.splitext(shapefile)[0] + ".prj"
        logger.debug("projfile :" + projfile)
        my_proj = open(projfile, 'w')
        my_proj.write('GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["Degree",0.017453292519943295]]')
        my_proj.close()
        logger.debug("rasterizing wb")
        shape_temp = os.path.splitext(shapefile)[0] + "_reproject" + os.path.splitext(shapefile)[1]

        ogr_command = "ogr2ogr -t_srs EPSG:" + str(epsg_code) + " " + shape_temp + " " + shapefile
        os.system(ogr_command)

        shape_clipped = clipVectorWithOGR_Extent(shape_temp, [a_x, a_y, b_x, b_y], working_dir_wb)

        temp = rasterizationWithSupportImage(shape_clipped, dtm_l2_coarse, working_dir_wb)
        logger.debug("rasterizing wb ok")
        listWaterBodies.append(temp)
    logger.debug("Projection to wb ok")
    logger.debug(listWaterBodies)

    wb = os.path.join(output_dem_directory, tile_id + "_MSK.TIF")
    logger.debug("OR between images")
    if not listWaterBodies == []:
        formula = "im1b1 "
        for i in range(1, len(listWaterBodies)):
            formula += 'or im' + str(i + 1) + "b1 "
        bandmath(listWaterBodies, formula, wb, "uint8")

    logger.info("Output WB")
    logger.info("\t WB L3BOA coarse resolution : " + wb)
    logger.debug("######## END MANAGING WB ########")
    return wb
    # ####### END MANAGING WB ########


def get_slope(dtm_temp, dtm_l2, dtm_l2_coarse, working_dir_aspect_and_slope, tile_id, output_dem_directory):
    """
    Get slope at l2 and l2coarse resolution
    """
    # ####### MANAGING SLOPE ########
    logger.info("######## MANAGING SLOPE ########")
    scale = True
    # running gdaldem to get slope
    logger.debug("gdal dem")
    slope = os.path.join(working_dir_aspect_and_slope, tile_id + "_slope_90m.tif")
    logger.debug("slope file : " + slope)
    commandGDAL = "gdaldem slope " + dtm_temp + " "
    if scale:
        commandGDAL += " -compute_edges "
    commandGDAL += slope
    logger.debug("command : " + commandGDAL)
    os.system(commandGDAL)
    logger.debug("gdal dem ok")

    slope_radian = os.path.join(working_dir_aspect_and_slope, tile_id + "_slope_rad.tif")
    #la pente est exprimée en degre float 32, on remet en radians * 100 codé sur 16 bits
    # les degres vont de 0 à 90, onr emet entre 0 et pi/2*100
    cmd = "gdal_translate -ot Int16 -scale 0 90 0 157 " + slope + " " + slope_radian
    os.system(cmd)

    logger.debug("Ressampling")
    # ressampling to L2 and L2_Coarse resolutions
    slope_l2 = os.path.join(working_dir_aspect_and_slope, tile_id + "_slope_l2.tif")
    slope_l2_coarse = os.path.join(working_dir_aspect_and_slope, tile_id + "_slope_l2_coarse.tif")
    slope_l2_no_data_to_zero = os.path.join(output_dem_directory, tile_id + "_SLP.TIF")
    slope_l2_coarse_no_data_to_zero = os.path.join(output_dem_directory, tile_id + "_SLC.TIF")
    if not os.path.isfile(slope_l2):
        superimpose(slope_radian, dtm_l2, slope_l2, 40)
    if not os.path.isfile(slope_l2_coarse):
        superimpose(slope_radian, dtm_l2_coarse, slope_l2_coarse, 200)
    logger.debug("Ressampling ok")

    seuil = -500
    bandmath([slope_l2], "(im1b1<" + str(seuil) + "?0:im1b1)", slope_l2_no_data_to_zero)
    bandmath([slope_l2_coarse], "(im1b1<" + str(seuil) + "?0:im1b1)", slope_l2_coarse_no_data_to_zero)
    logger.info("Output Slope")
    logger.info("\t Slope L3BOA 90m               : " + slope)
    logger.info("\t Slope L3BOA resolution        : " + slope_l2_no_data_to_zero)
    logger.info("\t Slope L3BOA coarse resolution : " + slope_l2_coarse_no_data_to_zero)

    logger.debug("######## END MANAGING SLOPE ########")
    return slope_l2_no_data_to_zero, slope_l2_coarse_no_data_to_zero
    # ####### END MANAGING SLOPE ########


def get_aspect(dtm_temp, dtm_l2, dtm_l2_coarse, working_dir_aspect_and_slope, tile_id, output_dem_directory):
    """
    Get slope at l2 and l2coarse resolution
    """
    #  ####### MANAGING ASPECT ########
    logger.info("######## MANAGING ASPECT ########")
    scale = True
    # running gdaldem to get aspect
    logger.debug("gdal dem")
    aspect = os.path.join(working_dir_aspect_and_slope, tile_id + "_aspect_90m.tif")
    logger.debug("slope file : " + aspect)
    commandGDAL = "gdaldem aspect " + dtm_temp + " "
    if scale:
        commandGDAL += " -compute_edges "
    commandGDAL += aspect
    logger.debug("command : " + commandGDAL)
    os.system(commandGDAL)
    logger.debug("gdal dem ok")

    aspect_rad = os.path.join(working_dir_aspect_and_slope, tile_id + "_aspect_rad.tif")
    # l'orientation est exprimée en degre float 32, on remet en radians * 100 codé sur 16 bits
    # les degres vont de 0 à 360, on remet entre 0 et 2pi*100
    cmd = "gdal_translate -ot Int16 -scale 0 360 0 628 " + aspect + " " + aspect_rad
    os.system(cmd)

    logger.debug("Ressampling")
    # ressampling to L2 and L2_Coarse resolutions
    aspect_l2 = os.path.join(working_dir_aspect_and_slope, tile_id + "_aspect_l2.tif")
    aspect_l2_coarse = os.path.join(working_dir_aspect_and_slope, tile_id + "_aspect_l2_coarse.tif")
    aspect_l2_no_data_to_zero = os.path.join(output_dem_directory, tile_id + "_ASP.TIF")
    aspect_l2_coarse_no_data_to_zero = os.path.join(output_dem_directory, tile_id + "_ASC.TIF")
    if not os.path.isfile(aspect_l2):
        superimpose(aspect_rad, dtm_l2, aspect_l2, 40)
    if not os.path.isfile(aspect_l2_coarse):
        superimpose(aspect_rad, dtm_l2_coarse, aspect_l2_coarse, 200)
    logger.debug("Ressampling ok")

    seuil = -500
    bandmath([aspect_l2], "(im1b1<" + str(seuil) + "?0:im1b1)", aspect_l2_no_data_to_zero)
    bandmath([aspect_l2_coarse], "(im1b1<" + str(seuil) + "?0:im1b1)", aspect_l2_coarse_no_data_to_zero)

    logger.info("Output aspect")
    logger.info("\t aspect L3BOA 90m               : " + aspect)
    logger.info("\t aspect L3BOA resolution        : " + aspect_l2_no_data_to_zero)
    logger.info("\t aspect L3BOA coarse resolution : " + aspect_l2_coarse_no_data_to_zero)

    logger.debug("######## END MANAGING ASPECT ########")
    return aspect_l2_no_data_to_zero, aspect_l2_coarse_no_data_to_zero
    # ####### END MANAGING ASPECT ########


def usage(return_code = 1):
    """
    Usage
    """
    print "Usage : python preprocessing -i input/image/ -o output/dtm/folder \
                -t tile_id -d strm_directory -w working_dir -v landsat8_extents.shp"
    print 'exemple : python dem_product.py -i /media/Data/LC81980302013177LGN01/LC81980302013177LGN01_B5.TIF \
    -o /media/Data/LC81980302013177LGN01/DEM_Product -t "LC81980302013177LGN01" -d /media/Data/DEMProduct/Srtm_folder/ \
    -w /media/Data/LC81980302013177LGN01/working_dir/ -v /media/Data/L8/wrs2_descending/wrs2_descending.shp'
    sys.exit(return_code)


def get_arguments():
    """
    Manages inputs
    """
    # ####### MANAGING ARGUMENTS ########
    # TODO : set path to downloadwb as an argument

    # check inputs
    logger.info("######## MANAGING ARGUMENTS ########")
    logger.debug("Checking arguments")
    logger.debug(len(sys.argv))
    if len(sys.argv) != 13:
        usage()

    try:
        opts, args = getopt.getopt(sys.argv[1:], "i:o:t:d:w:v:", ["--input_image=", "--output_directory="])
    except getopt.GetoptError:
        usage(2)

    input_image = output_dem_directory = tile_id = srtm_dir = working_dir = l8_vector = None

    for opt, arg in opts:
        if opt in ("-i", "--input_image"):
            input_image = os.path.realpath(arg)
        elif opt in ("-o", "--output_directory"):
            output_dem_directory = os.path.realpath(arg)
        elif opt in ("-v"):
            l8_vector = arg
        elif opt in ("-t"):
            tile_id = arg
        elif opt in ("-d"):
            srtm_dir = arg
        elif opt in ("-w"):
            working_dir = arg

    logger.debug("Arguments ok")

    # creating directories if they don't exist
    logger.debug("Managing output directories")
    if not os.path.isdir(output_dem_directory):
        os.mkdir(output_dem_directory)
    if not os.path.isdir(working_dir):
        os.mkdir(working_dir)
    logger.debug("Output directories ok")

    # checking if inputs exists
    if not input_image:
        print "input image missing"
        sys.exit(2)
    if not l8_vector:
        print "l8_vector missing"
        sys.exit(2)

    # get path, row of the product
    txt = os.path.join(os.path.dirname(input_image), tile_id + "_MTL.txt")
    if not os.path.isfile(txt):
        print "Input prodict incomplete. Missing txt", txt
        sys.exit()
    else:
        path, row = "", ""
        with open(txt, 'r') as fd:
            for line in fd:
                if re.search("\s*WRS_PATH = (\d*)", line) is not None:
                    path = int(re.search("\s*WRS_PATH = (\d*)", line).group(1))
                elif re.search("\s*WRS_ROW = (\d*)", line) is not None:
                    row = int(re.search("\s*WRS_ROW = (\d*)", line).group(1))
            print "path, row", path, row

    return input_image, srtm_dir, l8_vector, tile_id, output_dem_directory, working_dir, path, row


time_start = time.time()

input_image, srtm_dir, l8_vector, tile_id, output_dem_directory, working_dir, path, row = get_arguments()
extent_wgs84_vector, extent_wgs84, extent_feature = get_extent_from_l8_extent(l8_vector, path, row, tile_id, working_dir, "4326")
tile_id = config.tile_id_prefix + "%03d%03d" % (path, row) + config.tile_id_suffix

ouput_product_dir = os.path.join(output_dem_directory, os.path.basename(os.path.normpath(os.path.dirname(input_image))))

# crop all input tiles to match l8 real extent
command_list = ["python"]
command_list.append(os.path.join(os.path.dirname(__file__), "l8_align.py -i"))
command_list.append(os.path.dirname(input_image))
command_list.append("-v")
command_list.append(extent_feature)
command_list.append("-o")
command_list.append(ouput_product_dir)
command_list.append("-w")
command_list.append(working_dir)
logger.info(" ".join(command_list))
os.system(" ".join(command_list))

input_image = os.path.join(ouput_product_dir, os.path.basename(input_image))

output_dem_directory = os.path.join(output_dem_directory, tile_id + ".DBL.DIR")
if not os.path.isdir(output_dem_directory):
    os.mkdir(output_dem_directory)


logger.debug(extent_wgs84)
tiles = get_tiles(extent_wgs84)

for tile in tiles:
    full_path = os.path.join(srtm_dir, tile)
    logger.debug(full_path)
    if not os.path.isfile(full_path):
        logger.info(tile + " is missing")
    else:
        shutil.copyfile(full_path, os.path.join(working_dir, tile))

unzip_files(working_dir)

# computing the ratio to L2 and L2_Coarse resolutions
logger.debug("Getting ratios")
# ratio_l2         = float(L3BOA_constants.L3A_raw_dtm_resolution) / float(L3BOA_constants.L3A_BOA_resolution)
# ratio_l2_coarse  = float(L3BOA_constants.L3A_raw_dtm_resolution) / float(L3BOA_constants.L3A_BOA_coarse_resolution)
ratio_l2 = float(config.L1_resolution) / float(config.L2_resolution)
ratio_l2_coarse = float(config.L1_resolution) / float(config.L2_coarse_resolution)
logger.info("Ratio ratio_l2 : " + str(ratio_l2) + " ; ratio ratio_l2_coarse : " + str(ratio_l2_coarse))

logger.debug("######## END MANAGING ARGUMENTS ########")
# ####### END MANAGING ARGUMENTS ########

dtm_l2_no_data_to_zero, dtm_l2_coarse_no_data_to_zero, dtm_temp = get_DTM(input_image, working_dir, output_dem_directory, tile_id, extent_wgs84_vector)

# dtm_l2_coarse_no_data_to_zero = os.path.join(output_dem_directory, tile_id + "_ALC.TIF")
wb = get_wb(input_image, output_dem_directory, working_dir, dtm_l2_coarse_no_data_to_zero, tile_id)

working_dir_aspect_and_slope = os.path.join(working_dir, "working_dir_aspect_and_slope")
if not os.path.isdir(working_dir_aspect_and_slope):
    os.mkdir(working_dir_aspect_and_slope)

slope_l2_no_data_to_zero, slope_l2_coarse_no_data_to_zero = get_slope(dtm_temp, dtm_l2_no_data_to_zero, dtm_l2_coarse_no_data_to_zero, working_dir_aspect_and_slope, tile_id, output_dem_directory)
aspect_l2_no_data_to_zero, aspect_l2_coarse_no_data_to_zero = get_aspect(dtm_temp, dtm_l2_no_data_to_zero, dtm_l2_coarse_no_data_to_zero, working_dir_aspect_and_slope, tile_id, output_dem_directory)


interval = time.time() - time_start

logger.info('Total time in seconds:' + str(interval))
