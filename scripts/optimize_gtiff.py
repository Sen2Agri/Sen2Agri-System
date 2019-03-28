#!/usr/bin/env python
from __future__ import print_function

import argparse
import os
import pipes
import subprocess
import sys

from osgeo import gdal
from gdal import gdalconst


def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)


def parse_args():
    parser = argparse.ArgumentParser(description='Create cloud-optimized GeoTIFFs')
    parser.add_argument('input', help='Input image')
    parser.add_argument('-b', '--block-size', help='GeoTIFF block size', type=int, default=1024)
    parser.add_argument('-s', '--min-size', help='Smallest overview size', type=int, default=1024)
    parser.add_argument('-r', '--resampler', help='Overview resampling method', default='average')
    parser.add_argument('-i', '--interleave', help='Interleaving type', choices=['BAND', 'PIXEL'], default='BAND')
    parser.add_argument('--no-data', help='No data value', default=-10000)

    bigtiff = parser.add_mutually_exclusive_group(required=False)
    bigtiff.add_argument('--bigtiff', help="Save as BigTIFF", default=False, action='store_true')
    bigtiff.add_argument('--no-bigtiff', help="Don't save as BigTIFF (default)", dest='bigtiff', action='store_false')

    overviews = parser.add_mutually_exclusive_group(required=False)
    overviews.add_argument('--overviews', help="Add overviews (default)", default=True, action='store_true')
    overviews.add_argument('--no-overviews', help="Don't add overviews", dest='overviews', action='store_false')

    compress = parser.add_mutually_exclusive_group(required=False)
    compress.add_argument('--compress', help="Compress output (default: DEFLATE)", nargs='?', choices=['LZW', 'DEFLATE'], const='DEFLATE', default='DEFLATE')
    compress.add_argument('--no-compress', help="Don't compress output", dest='compress', action='store_const', const=None)

    tiling = parser.add_mutually_exclusive_group(required=False)
    tiling.add_argument('--tiled', help="Create a tiled image (default)", default=True, action='store_true')
    tiling.add_argument('--stripped', help="Create a stripped image", dest='tiled', action='store_false')

    threaded = parser.add_mutually_exclusive_group(required=False)
    threaded.add_argument('--threaded', help='Use multiple threads (default)', default=True, action='store_true')
    threaded.add_argument('--no-threaded', help='Use a single thread', dest='threaded', action='store_false')

    return parser.parse_args()


def get_overview_levels(size, min_size):
    levels = []
    f = 1
    while size > min_size:
        size = size / 2
        f = f * 2
        levels.append(f)
    return levels


def get_predictor(dataset):
    band = dataset.GetRasterBand(1)
    if band is not None and band.DataType in (gdalconst.GDT_Float32, gdalconst.GDT_Float64):
        return 3
    else:
        return 2


def main():
    args = parse_args()

    if args.no_data != 'none':
        run_command(['gdal_edit.py', '-a_nodata', args.no_data, args.input])
    else:
        run_command(['gdal_edit.py', '-unsetnodata', args.input])

    run_command(['gdaladdo', '-q', '-clean', args.input])

    env = {}
    if args.threaded:
        env['GDAL_NUM_THREADS'] = 'ALL_CPUS'

    dataset = gdal.Open(args.input, gdal.gdalconst.GA_ReadOnly)

    # why?
    # command = ['gdal_translate', '-q', '-co', 'BIGTIFF=NO', '-co', 'INTERLEAVE=' + args.interleave, '-a_nodata', args.no_data]
    command = ['gdal_translate', '-q', '-co', 'BIGTIFF=NO', '-co', 'INTERLEAVE=' + args.interleave]

    if args.bigtiff:
        command += ['-co', 'BIGTIFF=YES']

    if args.overviews:
        command += ['-co', 'COPY_SRC_OVERVIEWS=YES']

        size = (dataset.RasterXSize, dataset.RasterYSize)
        levels = get_overview_levels(max(size[0], size[1]), args.min_size)
        if levels:
            run_command(['gdaladdo', '-q', '-r', args.resampler, args.input] + levels, env)

    parts = os.path.splitext(args.input)
    temp = parts[0] + ".tmpcog.tif"

    if args.compress:
        predictor = get_predictor(dataset)
        command += ['-co', 'COMPRESS=' + args.compress, '-co', 'PREDICTOR=' + str(predictor)]

    if args.tiled:
        command += ['-co', 'TILED=YES', '-co', 'BLOCKXSIZE=' + str(args.block_size), '-co', 'BLOCKYSIZE=' + str(args.block_size)]
        env['GDAL_TIFF_OVR_BLOCKSIZE'] = str(args.block_size)

    del dataset

    command += [args.input, temp]
    run_command(command, env)

    if parts[1] == ".tif":
        os.rename(temp, args.input)
    elif parts[1] == ".vrt":
        os.rename(temp, parts[0] + ".tif")
        os.remove(args.input)
        os.remove(args.input + ".ovr")


if __name__ == '__main__':
    sys.exit(main())
