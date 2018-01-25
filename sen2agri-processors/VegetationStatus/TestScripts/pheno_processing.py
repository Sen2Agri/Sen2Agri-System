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
from __future__ import print_function

import os
import shutil
import glob
import argparse
import csv
from sys import argv
import datetime
import subprocess
import pipes
import time
import xml.etree.ElementTree as ET
import math
from xml.dom import minidom

def runCmd(cmdArray):
    start = time.time()
    print(" ".join(map(pipes.quote, cmdArray)))
    res = subprocess.call(cmdArray)
    print("OTB app finished in: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    if res != 0:
        print("OTB application error")
        exit(1)
    return res

parser = argparse.ArgumentParser(description='Phenological NDVI processor')

parser.add_argument('--applocation', help='The path where the sen2agri is built', default="")
parser.add_argument('--input', help='The list of products xml descriptors', required=True, nargs='+')
parser.add_argument('--outdir', help="Output directory", required=True)
parser.add_argument('--tileid', help="Tile id", required=False)
parser.add_argument('--resolution', help="Resample to this resolution. Use the same resolution as input files if you don't want any resample", required=True)
parser.add_argument('--siteid', help='The site ID', required=False)
parser.add_argument('--mainmission', help='The primary mission: SENTINEL|LANDSAT|SPOT', required=False)

args = parser.parse_args()

appLocation = args.applocation
outDir = args.outdir
siteId = "nn"
if args.siteid:
    siteId = args.siteid

mission = "SENTINEL"
if args.mainmission and args.mainmission in ['SENTINEL', 'LANDSAT', 'SPOT']:
    mission = args.mainmission
    
tileID="TILE_none"
if args.tileid:
    tileID = "TILE_{}".format(args.tileid)

if os.path.exists(outDir):
    if not os.path.isdir(outDir):
        print("Can't create the output directory because there is a file with the same name")
        print("Remove: " + outDir)
        exit(1)
else:
    os.makedirs(outDir)

outBands = "{}/all_bands.tif".format(outDir)
outMasks = "{}/all_masks.tif".format(outDir)
outDates = "{}/all_dates.txt".format(outDir)
outNdvi = "{}/all_ndvis.tif".format(outDir)
outMetric = "{}/metric_estimation.tif".format(outDir)
outMetricParams = "{}/metric_estimation_params.tif".format(outDir)
outMetricFlags = "{}/metric_estimation_flags.tif".format(outDir)

print("Processing started: " + str(datetime.datetime.now()))
start = time.time()

runCmd(["otbcli", "NdviMaskSeriesExtractor", appLocation, "-il"] + args.input + ["-pixsize", args.resolution, "-outndvis", outNdvi, "-outmasks", outMasks, "-ndh", "true", "-outdate", outDates, "-mission", mission])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
runCmd(["otbcli", "PhenologicalNDVIMetrics", appLocation, "-in", outNdvi, "-mask", outMasks, "-dates", outDates,"-out", outMetric])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

runCmd(["otbcli", "PhenoMetricsSplitter", appLocation, "-in", outMetric, "-outparams", outMetricParams, "-outflags", outMetricFlags, "-compress", "1"])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

runCmd(["otbcli", "ProductFormatter", appLocation, 
    "-destroot", outDir, 
    "-fileclass", "SVT1", 
    "-level", "L3E", 
    "-baseline",  "01.00", 
    "-siteid", siteId,  
    "-processor", "phenondvi", 
    "-processor.phenondvi.metrics", tileID, outMetricParams, 
    "-processor.phenondvi.flags", tileID, outMetricFlags, "-il"] + args.input)

print("Processing finished: " + str(datetime.datetime.now()))
print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

#os.remove(outMasks)
#os.remove(outBands)
#os.remove(outDates)
#os.remove(outNdvi)
#os.remove(outSigmo)

