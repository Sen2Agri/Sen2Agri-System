#!/usr/bin/env python
import argparse
import os
import pipes
import subprocess
import sys

from osgeo import gdal


def run_command(args):
    args = map(str, args)
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args)


def ilog2(n):
    return len(bin(n)) - 3


def parse_args():
    parser = argparse.ArgumentParser(description='Create cloud-optimized GeoTIFFs')
    parser.add_argument('input', help='Input image')
    parser.add_argument('-s', '--min-size', help='Smallest overview size', type=int, default=256)

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


def main():
    args = parse_args()
    print(args)

    if args.overviews:
        size = get_image_size(args.input)
        max_level = ilog2(max(size[0], size[1]) / args.min_size)
        levels = [2**(x + 1) for x in xrange(max_level + 1)]
        run_command(['gdaladdo', '-r', 'bilinear', args.input] + levels)

    parts = os.path.splitext(args.input)
    temp = "".join([parts[0], ".tmpcog", parts[1]])
    command = ['gdal_translate', '-co', 'BIGTIFF=YES', '-co', 'COPY_SRC_OVERVIEWS=YES']

    if args.compress:
        command += ['-co', 'COMPRESS=' + args.compress]

    if args.tiled:
        command += ['-co', 'TILED=YES']

    command += [args.input, temp]
    run_command(command)

    os.rename(temp, args.input)


if __name__ == '__main__':
    sys.exit(main())
