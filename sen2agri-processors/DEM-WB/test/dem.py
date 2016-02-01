#!/usr/bin/env python
from __future__ import print_function
import argparse
import re
import glob
import gdal
import osr
import subprocess
import lxml.etree
from lxml.builder import E
#import logging
import math
import os
import sys
import pipes
import zipfile


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


def resample_dataset(src_file_name, dst_file_name, dst_spacing_x, dst_spacing_y):
    dataset = gdal.Open(src_file_name, gdal.gdalconst.GA_ReadOnly)

    src_x_size = dataset.RasterXSize
    src_y_size = dataset.RasterYSize

    print("Source dataset {} of size {}x{}".format(src_file_name, src_x_size, src_y_size))

    src_geo_transform = dataset.GetGeoTransform()
    (ulx, uly) = (src_geo_transform[0], src_geo_transform[3])
    (lrx, lry) = (src_geo_transform[0] + src_geo_transform[1] * src_x_size,
                  src_geo_transform[3] + src_geo_transform[5] * src_y_size)

    print("Source coordinates ({}, {})-({},{})".format(ulx, uly, lrx, lry))

    dst_x_size = int(round((lrx - ulx) / dst_spacing_x))
    dst_y_size = int(round((lry - uly) / dst_spacing_y))

    print("Destination dataset {} of size {}x{}".format(dst_file_name, dst_x_size, dst_y_size))

    dst_geo_transform = (ulx, dst_spacing_x, src_geo_transform[2],
                         uly, src_geo_transform[4], dst_spacing_y)

    (ulx, uly) = (dst_geo_transform[0], dst_geo_transform[3])
    (lrx, lry) = (dst_geo_transform[0] + dst_geo_transform[1] * dst_x_size,
                  dst_geo_transform[3] + dst_geo_transform[5] * dst_y_size)
    print("Destination coordinates ({}, {})-({},{})".format(ulx, uly, lrx, lry))

    drv = gdal.GetDriverByName('GTiff')
    dest = drv.Create(dst_file_name, dst_x_size, dst_y_size, 1, gdal.GDT_Float32)
    dest.SetGeoTransform(dst_geo_transform)
    dest.SetProjection(dataset.GetProjection())
    gdal.ReprojectImage(dataset, dest, dataset.GetProjection(), dest.GetProjection(),
                        gdal.GRA_Bilinear)


def get_dtm_tiles(points):
    """
    Returns a list of dtm tiles covering the given extent
    """
    a_x, a_y, b_x, b_y = points

    if a_x < b_x and a_y > b_y:
        a_bb_x = int(math.floor(a_x / 5) * 5)
        a_bb_y = int(math.floor((a_y + 5) / 5) * 5)
        b_bb_x = int(math.floor((b_x + 5) / 5) * 5)
        b_bb_y = int(math.floor(b_y / 5) * 5)

        print("bounding box {} {} {} {}".format(
            a_bb_x, a_bb_y, b_bb_x, b_bb_y))

        x_numbers_list = [(x + 180) / 5 + 1
                          for x in range(min(a_bb_x, b_bb_x), max(a_bb_x, b_bb_x), 5)]
        x_numbers_list_format = ["%02d" % (x,) for x in x_numbers_list]

        y_numbers_list = [(60 - x) / 5
                          for x in range(min(a_bb_y, b_bb_y), max(a_bb_y, b_bb_y), 5)]
        y_numbers_list_format = ["%02d" % (x,) for x in y_numbers_list]

        srtm_zips = ["srtm_" + str(x) + "_" + str(y) + ".tif"
                     for x in x_numbers_list_format
                     for y in y_numbers_list_format]

        return srtm_zips


def run_command(args):
    print(" ".join(map(pipes.quote, args)))
    subprocess.call(args)


def get_landsat_tile_id(image):
    m = re.match("[A-Z][A-Z]\d(\d{6})\d{4}\d{3}[A-Z]{3}\d{2}_B\d{1,2}\.TIF", image)
    return m and ('LANDSAT', m.group(1))


def get_sentinel2_tile_id(image):
    m = re.match("\w+_T(\w{5})_B\d{2}\.\w{3}", image)
    return m and ('SENTINEL', m.group(1))


def get_tile_id(image):
    name = os.path.basename(image)
    return get_landsat_tile_id(name) or get_sentinel2_tile_id(name)


class Context(object):

    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)


def format_filename(output_directory, tile_id, suffix):
    filename_template = "L8_TEST_AUX_REFDE2_{0}_0001_{1}.TIF"
    filename_template = "S2__TEST_AUX_REFDE2_{0}____5001_{1}.TIF"
    filename_template = "S2A_TEST_AUX_REFDE2_{0}_0001_{1}.TIF"
    directory_template = "S2A_TEST_AUX_REFDE2_{0}_0001.DBL.DIR"

    return filename_template.format(tile_id, suffix)
    directory = os.path.join(output_directory, directory_template.format(tile_id))
    return os.path.join(directory,
                        filename_template.format(tile_id, suffix))


def create_context(args):
    mode, tile_id = get_tile_id(args.input)

    dataset = gdal.Open(args.input, gdal.gdalconst.GA_ReadOnly)

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

    directory_template = "S2A_TEST_AUX_REFDE2_{0}_0001.DBL.DIR"
    image_directory = os.path.join(args.output, directory_template.format(tile_id))

    metadata_template = "S2A_TEST_AUX_REFDE2_{0}_0001.HDR"

    d = dict(image=args.input,
             mode=mode,
             srtm_directory=args.srtm,
             swbd_directory=args.swbd,
             working_directory=args.working_dir,
             output=args.output,
             image_directory=image_directory,
             metadata_file=os.path.join(args.output, metadata_template.format(tile_id)),
             swbd_list=os.path.join(args.working_dir, "swbd.txt"),
             tile_id=tile_id,
             dem_vrt=os.path.join(args.working_dir, "dem.vrt"),
             dem_nodata=os.path.join(args.working_dir, "dem.tif"),
             dem_coarse=os.path.join(image_directory, format_filename(image_directory, tile_id,
                 "ALC")),
             slope_degrees=os.path.join(args.working_dir, "slope_degrees.tif"),
             aspect_degrees=os.path.join(args.working_dir, "aspect_degrees.tif"),
             slope_coarse=os.path.join(image_directory, format_filename(image_directory, tile_id,
                 "SLC")),
             aspect_coarse=os.path.join(image_directory, format_filename(image_directory, tile_id,
                 "ASC")),
             wb=os.path.join(args.working_dir, "wb.shp"),
             wb_reprojected=os.path.join(
                 args.working_dir, "wb_reprojected.shp"),
             water_mask=os.path.join(image_directory, format_filename(image_directory, tile_id,
                 "MSK")),
             size_x=size_x, size_y=size_y,
             spacing_x=spacing_x, spacing_y=spacing_y,
             extent=extent, wgs84_extent=wgs84_extent,
             epsg_code=epsg_code)

    if mode == 'LANDSAT':
        d['dem_r1'] = os.path.join(image_directory, format_filename(image_directory, tile_id,
            "ALT"))
        d['slope_r1'] = os.path.join(image_directory, format_filename(image_directory, tile_id,
            "SLP"))
        d['aspect_r1'] = os.path.join(image_directory, format_filename(image_directory, tile_id,
            "ASP"))

        d['dem_r2'] = None
        d['slope_r2'] = None
        d['aspect_r2'] = None
    else:
        d['dem_r1'] = os.path.join(image_directory, format_filename(image_directory, tile_id,
            "ALT_R1"))
        d['dem_r2'] = os.path.join(image_directory, format_filename(image_directory, tile_id,
            "ALT_R2"))
        d['slope_r1'] = os.path.join(image_directory, format_filename(image_directory, tile_id,
            "SLP_R1"))
        d['slope_r2'] = os.path.join(image_directory, format_filename(image_directory, tile_id,
            "SLP_R2"))
        d['aspect_r1'] = os.path.join(image_directory, format_filename(image_directory, tile_id,
            "ASP_R1"))
        d['aspect_r2'] = os.path.join(image_directory, format_filename(image_directory, tile_id,
            "ASP_R2"))

    return Context(**d)


def create_metadata(context):
    file_names = [context.dem_r1, context.dem_r2, context.dem_coarse,
                  context.slope_r1, context.slope_r2, context.slope_coarse,
                  context.aspect_r1, context.aspect_r2, context.aspect_coarse,
                  context.water_mask]

    files = []
    index = 1
    for f in file_names:
        if f is not None:
            files.append(
                    E.Packaged_DBL_File(
                        E.Relative_File_Path(os.path.relpath(f, context.output)),
                        sn=str(index)))
            index = index + 1

    return E.Earth_Explorer_Header(
            E.Fixed_Header(
                E.Mission('SENTINEL-2_'),
                E.File_Type('AUX_REFDE2')),
            E.Variable_Header(
                E.Specific_Product_Header(
                    E.DBL_Organization(
                        E.List_of_Packaged_DBL_Files(
                            *files,
                            count=str(len(files)))))))

def process_DTM(context):
    if abs(context.spacing_x) > abs(context.spacing_y):
        grid_spacing = abs(context.spacing_x)
    else:
        grid_spacing = abs(context.spacing_y)

    dtm_tiles = get_dtm_tiles([context.wgs84_extent[0][0],
                               context.wgs84_extent[0][1],
                               context.wgs84_extent[2][0],
                               context.wgs84_extent[2][1]])

    dtm_tiles = [os.path.join(context.srtm_directory, tile)
                 for tile in dtm_tiles]

    missing_tiles = []
    for tile in dtm_tiles:
        if not os.path.exists(tile):
            missing_tiles.append(tile)

    if missing_tiles:
        print("The following SRTM tiles are missing: {}. Please provide them in the SRTM directory ({}).".format(
            [os.path.basename(tile) for tile in missing_tiles], context.srtm_directory))
        sys.exit(1)

    run_command(["gdalbuildvrt",
                 context.dem_vrt] + dtm_tiles)
    run_command(["otbcli_BandMath",
                 "-il", context.dem_vrt,
                 "-out", context.dem_nodata,
                 "-exp", "im1b1 == -32768 ? 0 : im1b1"])
    run_command(["otbcli_OrthoRectification",
                 "-io.in", context.dem_nodata,
                 "-io.out", context.dem_r1,
                 "-map", "epsg",
                 "-map.epsg.code", context.epsg_code,
                 "-outputs.sizex", str(context.size_x),
                 "-outputs.sizey", str(context.size_y),
                 "-outputs.spacingx", str(context.spacing_x),
                 "-outputs.spacingy", str(context.spacing_y),
                 "-outputs.ulx", str(context.extent[0][0]),
                 "-outputs.uly", str(context.extent[0][1]),
                 "-opt.gridspacing", str(grid_spacing)])

    if context.dem_r2:
        # run_command(["gdal_translate",
        #              "-outsize", str(int(round(context.size_x / 2.0))), str(int(round(context.size_y
        #                                                                               / 2.0))),
        #              context.dem_r1,
        #              context.dem_r2])
        resample_dataset(context.dem_r1, context.dem_r2, 20, -20)
        # run_command(["otbcli_RigidTransformResample",
        #              "-in", context.dem_r1,
        #              "-out", context.dem_r2,
        #              "-transform.type.id.scalex", "0.5",
        #              "-transform.type.id.scaley", "0.5"])

    if context.mode == 'LANDSAT':
        scale = 1.0 / 8
        inv_scale = 8.0
    else:
        # scale = 1.0 / 23.9737991266  # almost 1/24
        scale = 1.0 / 24
        inv_scale = 24.0

    # run_command(["gdal_translate",
    #              "-outsize", str(int(round(context.size_x / inv_scale))), str(int(round(context.size_y /
    #                                                                                     inv_scale))),
    #              context.dem_r1,
    #              context.dem_coarse])
    resample_dataset(context.dem_r2, context.dem_coarse, 240, -240)
    # run_command(["otbcli_RigidTransformResample",
    #              "-in", context.dem_r1,
    #              "-out", context.dem_coarse,
    #              "-transform.type.id.scalex", str(scale),
    #              "-transform.type.id.scaley", str(scale)])

    run_command(["gdaldem", "slope",
                 # "-s", "111120",
                 "-compute_edges",
                 context.dem_r1,
                 context.slope_degrees])
    run_command(["gdaldem", "aspect",
                 # "-s", "111120",
                 "-compute_edges",
                 context.dem_r1,
                 context.aspect_degrees])

    run_command(["gdal_translate",
                 "-ot", "Int16",
                 "-scale", "0", "90", "0", "157",
                 context.slope_degrees,
                 context.slope_r1])
    run_command(["gdal_translate",
                 "-ot", "Int16",
                 "-scale", "0", "368", "0", "628",
                 context.aspect_degrees,
                 context.aspect_r1])

    if context.slope_r2:
        run_command(["otbcli_Superimpose",
                     "-inr", context.dem_r2,
                     "-inm", context.slope_r1,
                     "-interpolator", "linear",
                     "-lms", "40",
                     "-out", context.slope_r2])

    if context.aspect_r2:
        run_command(["otbcli_Superimpose",
                     "-inr", context.dem_r2,
                     "-inm", context.aspect_r1,
                     "-interpolator", "linear",
                     "-lms", "40",
                     "-out", context.aspect_r2])

    run_command(["otbcli_Superimpose",
                 "-inr", context.dem_coarse,
                 "-inm", context.slope_r1,
                 "-interpolator", "linear",
                 "-lms", "40",
                 "-out", context.slope_coarse])

    run_command(["otbcli_Superimpose",
                 "-inr", context.dem_coarse,
                 "-inm", context.aspect_r1,
                 "-interpolator", "linear",
                 "-lms", "40",
                 "-out", context.aspect_coarse])


def process_WB(context):
    run_command(["otbcli",
                 "DownloadSWBDTiles",
                 "-il", context.dem_r1,
                 "-mode", "list",
                 "-mode.list.indir", context.swbd_directory,
                 "-mode.list.outlist", context.swbd_list])

    with open(context.swbd_list) as f:
        swbd_tiles = f.read().splitlines()

    run_command(["otbcli_ConcatenateVectorData",
                 "-out", context.wb,
                 "-vd"] + swbd_tiles)

    run_command(["ogr2ogr",
                 "-s_srs", "EPSG:4326",
                 "-t_srs", "EPSG:" + context.epsg_code,
                 context.wb_reprojected,
                 context.wb])

    run_command(["otbcli_Rasterization",
                 "-in", context.wb_reprojected,
                 "-out", context.water_mask, "uint8",
                 "-im", context.dem_coarse,
                 "-mode.binary.foreground", "1"])


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Creates DEM and WB data for MACCS")
    parser.add_argument('input', help="input image")
    parser.add_argument('--srtm', required=True, help="SRTM dataset path")
    parser.add_argument('--swbd', required=True, help="SWBD dataset path")
    parser.add_argument('-w', '--working-dir', required=True,
                        help="working directory")
    parser.add_argument('output', help="output location")

    args = parser.parse_args()

    return create_context(args)

context = parse_arguments()
try:
    print(context.image_directory)
    os.makedirs(context.image_directory)
except:
    pass
metadata = create_metadata(context)
with open(context.metadata_file, 'w') as f:
    lxml.etree.ElementTree(metadata).write(f, pretty_print=True)

process_DTM(context)
process_WB(context)
