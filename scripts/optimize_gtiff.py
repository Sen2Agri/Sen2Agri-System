#!/usr/bin/env python
import argparse
import os
import pipes
import subprocess
import sys

from osgeo import gdal


def run_command(args, env=None):
    args = map(str, args)
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

    overviews = parser.add_mutually_exclusive_group(required=False)
    overviews.add_argument('--overviews', help="Add overviews (default)", default=True, action='store_true')
    overviews.add_argument('--no-overviews', help="Don't add overviews", dest='overviews', action='store_false')

    compress = parser.add_mutually_exclusive_group(required=False)
    compress.add_argument('--compress', help="Compress output (default: DEFLATE)", nargs='?', choices=['LZW', 'DEFLATE'], const='DEFLATE', default='DEFLATE')
    compress.add_argument('--no-compress', help="Don't compress output", dest='compress', action='store_const', const=None)

    tiling = parser.add_mutually_exclusive_group(required=False)
    tiling.add_argument('--tiled', help="Create a tiled image (default)", default=True, action='store_true')
    tiling.add_argument('--stripped', help="Create a stripped image", dest='tiled', action='store_false')

    return parser.parse_args()


def get_image_size(image):
    ds = gdal.Open(image, gdal.gdalconst.GA_ReadOnly)
    return (ds.RasterXSize, ds.RasterYSize)


def get_overview_levels(size, min_size):
    levels = []
    f = 1
    while size > min_size:
        size = size / 2
        f = f * 2
        levels.append(f)
    return levels


def main():
    args = parse_args()
    print(args)

    run_command(['gdaladdo', '-clean', args.input])

    env = {'GDAL_NUM_THREADS': 'ALL_CPUS'}
    command = ['gdal_translate', '-co', 'BIGTIFF=NO', '-co', 'INTERLEAVE=' + args.interleave]
    if args.overviews:
        command += ['-co', 'COPY_SRC_OVERVIEWS=YES']

        size = get_image_size(args.input)
        levels = get_overview_levels(max(size[0], size[1]), args.min_size)
        if levels:
            run_command(['gdaladdo', '-r', args.resampler, args.input] + levels, env)

    parts = os.path.splitext(args.input)
    temp = "".join([parts[0], ".tmpcog", parts[1]])

    if args.compress:
        command += ['-co', 'COMPRESS=' + args.compress, '-co', 'PREDICTOR=2']

    if args.tiled:
        command += ['-co', 'TILED=YES', '-co', 'BLOCKXSIZE=' + str(args.block_size), '-co', 'BLOCKYSIZE=' + str(args.block_size)]
        env['GDAL_TIFF_OVR_BLOCKSIZE'] = str(args.block_size)

    command += [args.input, temp]
    run_command(command, env)

    os.rename(temp, args.input)


if __name__ == '__main__':
    sys.exit(main())
