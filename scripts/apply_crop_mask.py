#!/usr/bin/env python

from __future__ import print_function

import argparse
import glob
import os
import os.path
import re
import multiprocessing.dummy

from sen2agri_common import Step, run_step, format_otb_filename


def run_step_list(steps):
    for step in steps:
        run_step(step)


def main():
    parser = argparse.ArgumentParser(description='Mask L4B raw maps')

    parser.add_argument('l4b_path', metavar='l4b-path', help='The L4B path')
    parser.add_argument('l4a_path', metavar='l4a-path', help='The L4A path')

    args = parser.parse_args()

    tile_id_re = re.compile(r"/crop_type_map_([0-9a-zA-Z]+)\.tif$")
    steps = []
    for tile_crop_map in glob.glob(os.path.join(args.l4b_path, "crop_type_map_*.tif")):
        m = tile_id_re.search(tile_crop_map, re.IGNORECASE)
        if not m:
            continue

        tile_id = m.group(1)

        tile_mask = os.path.join(args.l4a_path, "crop-mask-segmented-{}.tif".format(tile_id))
        if not os.path.exists(tile_mask):
            tile_mask = os.path.join(args.l4a_path, "crop_mask_map_{}.tif".format(tile_id))
        if not os.path.exists(tile_mask):
            tile_mask = None

        if tile_mask is None:
            print("No crop mask found for tile {}".format(tile_id))
            continue

        print("Using crop mask {} for tile {}".format(tile_mask, tile_id))
        tile_crop_map_masked = os.path.join(args.l4b_path, "crop_type_map_masked_{}.tif".format(tile_id))

        step_args = ["otbcli_BandMath",
                     "-exp", "im2b1 == 0 ? 0 : im1b1",
                     "-il", tile_crop_map, tile_mask,
                     "-out", format_otb_filename(tile_crop_map_masked, compression='DEFLATE'), "int16"]

        steps.append([Step("Mask by crop mask " + tile_id, step_args),
                      Step("Nodata_" + tile_id,
                           ["gdal_edit.py",
                            "-a_nodata", -10000,
                            tile_crop_map_masked])])

    pool = multiprocessing.dummy.Pool(multiprocessing.cpu_count())
    pool.map(run_step_list, steps)
    pool.close()
    pool.join()


if __name__ == '__main__':
    main()
