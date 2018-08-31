#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:      Sen2Agri-Processors
   Language:     Python
   Copyright:    2015-2016, CS Romania, office@c-s.ro
   See COPYRIGHT file for details.
   
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
_____________________________________________________________________________

"""
import argparse
import glob
import os
import gdal
from gdalconst import GA_ReadOnly
import osr
import pipes
import subprocess
import itertools
import uuid
import shutil

def get_projection(file):
    dataset = gdal.Open(file, GA_ReadOnly)
    if not dataset:
        raise Exception("Unable to open dataset", file)
    return dataset.GetProjection()


def get_projection_name(proj):
    srs = osr.SpatialReference(wkt=proj)
    if srs.IsProjected:
        return srs.GetAttrValue('PROJCS')
    return srs.GetAttrValue('GEOGCS')


def get_tile(file):
    name = os.path.basename(file)
    parts = name.split('_')
    if len(parts) < 6:
        raise Exception("Unable to parse file name", file)
    return parts[5]


def run_command(args):
    args = [str(arg) for arg in args]
    print(' '.join(map(pipes.quote, args)))
    subprocess.call(args)


def remove_files(files):
    for file in files:
        try:
            os.remove(file)
        except:
            pass

parser = argparse.ArgumentParser()
parser.add_argument('-i', '--input', help="Input directory")
parser.add_argument('-w', '--work_dir', help="Working directory", default=".")
parser.add_argument('-r', '--resolution', help="Target resolution",
                    default=120)
parser.add_argument('-p', '--reproject', help="Always reproject to EPSG:4326",
                    action='store_true')
args = parser.parse_args()

temp_dir = os.path.join(args.work_dir, str(uuid.uuid4()))

os.makedirs(temp_dir)

files = glob.glob(os.path.join(args.input, "**/*FRE_R1*.TIF"))
if not files:
    files = glob.glob(os.path.join(args.input, "**/*FRE*.TIF"))
if not files:
    print("Unable to find a MACCS L2A product in the input directory")
    exit(1)

print("Using input files: {}".format(files))

projection_list = set([get_projection(file) for file in files])
if len(projection_list) > 1:
    reproject = True
    print("Different SRSs detected, reprojecting to EPSG:4326")
else:
    reproject = False
    srs = projection_list.pop()
    print("Detected SRS: {}".format(get_projection_name(srs)))

if args.reproject and not reproject:
    reproject = True
    print("Reprojecting to EPSG:4326 due to user request")

bands = [3, 2, 1]
resolution = 240
# resampler = 'cubic'
resampler = 'average'

band_args = list(itertools.chain(*[['-b', b] for b in bands]))
mosaic_tiles = []
temp_files = []
for file in files:
    tile = get_tile(file)
    vrt = os.path.join(temp_dir, "{}.vrt".format(tile))
    temp_files.append(vrt)
    run_command(['gdalbuildvrt', '-srcnodata', '-10000'] + band_args +
                [vrt, file])

    resampled_tile = os.path.join(temp_dir,
                                  "{}_resampled.tif".format(tile))
    temp_files.append(resampled_tile)
    run_command(['gdalwarp', '-multi', '-wm', 2048, '-srcnodata', -10000,
                 '-tr', resolution, resolution, '-r',
                 resampler, vrt, resampled_tile])

    if reproject:
        reprojected_tile = os.path.join(temp_dir,
                                        "{}_reprojected.vrt".format(tile))
        temp_files.append(reprojected_tile)
        mosaic_tiles.append(reprojected_tile)
        run_command(['gdalwarp', '-t_srs', 'EPSG:4326', '-of', 'vrt',
                     '-r', resampler, resampled_tile, reprojected_tile])
    else:
        mosaic_tiles.append(resampled_tile)

mosaic_16bit = os.path.join(temp_dir, "mosaic_16bit.tif")
temp_files.append(mosaic_16bit)
mosaic = os.path.join(args.input, "mosaic.jpg")
mosaic_aux = os.path.join(args.input, "mosaic.jpg.aux.xml")
temp_files.append(mosaic_aux)
if reproject:
    run_command(['gdalwarp', '-multi', '-wm', 2048,
                 '-r', resampler, '-co', 'PHOTOMETRIC=RGB', '-dstnodata', 0] +
                mosaic_tiles + [mosaic_16bit])
else:
    run_command(['gdalwarp', '-multi', '-wm', 2048, '-dstnodata', 0,
                 '-co', 'PHOTOMETRIC=RGB'] + mosaic_tiles +
                [mosaic_16bit])

run_command(['gdal_translate', '-scale', '-ot', 'Byte', '-of', 'JPEG',
             mosaic_16bit, mosaic])

remove_files(temp_files)
#os.rmdir(temp_dir)
shutil.rmtree(temp_dir)
