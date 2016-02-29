#!/usr/bin/env python
from __future__ import print_function
import argparse
import re
import os
from os.path import isfile, isdir, join
import glob
import sys
import time, datetime
import gdal
from common import *
from osgeo import ogr

general_log_path = "/tmp/"
general_log_filename = "demmacs_launcher"

def GetExtent(gt, cols, rows):
    ext = []
    xarr = [0, cols]
    yarr = [0, rows]

    for px in xarr:
        for py in yarr:
            x = gt[0] + px * gt[1] + py * gt[2]
            y = gt[3] + px * gt[4] + py * gt[5]
            ext.append([x, y])
        yarr.reverse()
    return ext


def ReprojectCoords(coords, src_srs, tgt_srs):
    trans_coords = []
    transform = osr.CoordinateTransformation(src_srs, tgt_srs)
    for x, y in coords:
        x, y, z = transform.TransformPoint(x, y)
        trans_coords.append([x, y])
    return trans_coords


def get_envelope(footprints):
    geomCol = ogr.Geometry(ogr.wkbGeometryCollection)

    for footprint in footprints:
        #ring = ogr.Geometry(ogr.wkbLinearRing)
        for pt in footprint:
            #ring.AddPoint(pt[0], pt[1])
            point = ogr.Geometry(ogr.wkbPoint)
            point.AddPoint_2D(pt[0], pt[1])
            geomCol.AddGeometry(point)

        #poly = ogr.Geometry(ogr.wkbPolygon)
        #poly

    hull = geomCol.ConvexHull()
    return hull.ExportToWkt()


def get_footprint(image_filename):
    dataset = gdal.Open(image_filename, gdal.gdalconst.GA_ReadOnly)

    size_x = dataset.RasterXSize
    size_y = dataset.RasterYSize

    geo_transform = dataset.GetGeoTransform()

    spacing_x = geo_transform[1]
    spacing_y = geo_transform[5]

    extent = GetExtent(geo_transform, size_x, size_y)

    source_srs = osr.SpatialReference()
    source_srs.ImportFromWkt(dataset.GetProjection())
    epsg_code = source_srs.GetAttrValue("AUTHORITY", 1)
    target_srs = osr.SpatialReference()
    target_srs.ImportFromEPSG(4326)

    wgs84_extent = ReprojectCoords(extent, source_srs, target_srs)
    return wgs84_extent


parser = argparse.ArgumentParser(
    description="Launcher for DEM MACCS script")
parser.add_argument('-c', '--config', default="/etc/sen2agri/sen2agri.conf", help="configuration file")

args = parser.parse_args()


config = Config()
if not config.loadConfig(args.config):
    log(general_log_path, "Could not load the config from configuration file", general_log_filename)
    sys.exit(-1)

l1c_db = L1CInfo(config.host, config.database, config.user, config.password)
demmaccs_config = l1c_db.get_demmacs_config()
if demmaccs_config is None:
    log(general_log_path, "Could not load the config from database", general_log_filename)
    sys.exit(-1)

l1c_list = l1c_db.get_unprocessed_l1c()

if len(l1c_list) == 0:
    log(general_log_path, "No unprocessed L1C found in DB", general_log_filename)
    sys.exit(0)

#by convention, the processor ID for dem-maccs will be always 1 within the DB
processor_short_name = l1c_db.get_short_name("processor", 1)
base_output_path = demmaccs_config.output_path.replace("{processor}", processor_short_name)

processed_ids = []
tiles_dir_list = []

for l1c in l1c_list:
    #get site short name
    site_short_name = l1c_db.get_short_name("site", l1c[1])
    if l1c[2].endswith("/"):
        l2a_basename = os.path.basename(l1c[2][:len(l1c[2]) - 1])
    else:
        l2a_basename = os.path.basename(l1c[2])

    if l2a_basename.startswith("S2"):
        l2a_basename = l2a_basename.replace("L1C", "L2A")
    elif l2a_basename.startswith("LC8"):
        l2a_basename += "L2A"
    else:
        log(general_log_path, "The L1C product names is bad:{}".format(l2a_basename), general_log_filename)
        sys.exit(-1)
    #create the output_path. it will hold all the tiles found inside the l1c
    #for sentinel, this output path will be something like /path/to/poduct/site_name/processor_name/....MSIL2A.../
    #for landsat, this output path will be something like /path/to/poduct/site_name/processor_name/LC8...._L2A/
    output_path = base_output_path.replace("{site}", site_short_name)
    if not output_path.endswith("/"):
        output_path += "/"
    output_path = output_path + l2a_basename + "/"
    log(output_path, "Creating the output path {}".format(output_path), "demmacs_launcher.log")
    log(output_path, "Launching demmaccs.py", "demmacs_launcher.log")
    if run_command([demmaccs_config.launcher, "--srtm", demmaccs_config.srtm_path, "--swbd", demmaccs_config.swbd_path, "-p", "5", "--gip-dir", demmaccs_config.gips_path, "--working-dir", demmaccs_config.working_dir, "--maccs-address", demmaccs_config.maccs_ip_address, "--maccs-launcher", demmaccs_config.maccs_launcher, "--dem-launcher", "/home/agrosu/sen2agri/sen2agri-processors/DEM-WB/test/dem.py", l1c[2], output_path]) == 0:
        #processed_ids.append(l1c[0])
        tiles_dir_list = (glob.glob("{}/*.DBL.DIR".format(output_path)))
        log(output_path, "Creating common footprint for tiles: DBL.DIR List: {}".format(tiles_dir_list), "demmacs_launcher.log")

        wgs84_extent_list = []
        for tile_dir in tiles_dir_list:
            tile_img = (glob.glob("{}/*_FRE_R1.DBL.TIF".format(tile_dir)))
            if len(tile_img) > 0:
                wgs84_extent_list.append(get_footprint(tile_img[0]))
        wkt = get_envelope(wgs84_extent_list)
        if len(wkt) == 0:
            log(output_path, "Could not create the footprint", "demmacs_launcher.log")
        else:
            log(output_path, "Mark the state as present in the database for product {}".format(output_path), "demmacs_launcher.log")
            l1c_db.mark_product_as_present(l1c[0], 1, l1c[1], output_path, os.path.basename(output_path[:len(output_path) - 1]), wkt)
    else:
        log(output_path, "demmaccs.py script didn't work!", "demmacs_launcher.log")








