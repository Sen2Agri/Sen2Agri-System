#!/usr/bin/env python
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

args = parser.parse_args()

appLocation = args.applocation
outDir = args.outdir
siteId = "nn"
if args.siteid:
    siteId = args.siteid

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

#OUT_BANDS="$OUT_FOLDER/spot_bands.tif"
outBands = "{}/spot_bands.tif".format(outDir)
#OUT_MASKS="$OUT_FOLDER/spot_masks.tif"
outMasks = "{}/spot_masks.tif".format(outDir)
#OUT_DATES="$OUT_FOLDER/spot_dates.txt"
outDates = "{}/spot_dates.txt".format(outDir)
#OUT_SHAPE="$OUT_FOLDER/spot_shapes.shp"
outShape = "{}/spot_shapes.shp".format(outDir)
#OUT_NDVI="$OUT_FOLDER/spot_ndvi.tif"
outNdvi = "{}/spot_ndvi.tif".format(outDir)
#OUT_SIGMO="$OUT_FOLDER/spot_sigmo.tif"
outSigmo = "{}/spot_sigmo.tif".format(outDir)
#OUT_METRIC="$OUT_FOLDER/metric_estimation.tif"
outMetric = "{}/metric_estimation.tif".format(outDir)
outMetricParams = "{}/metric_estimation_params.tif".format(outDir)
outMetricFlags = "{}/metric_estimation_flags.tif".format(outDir)

print("Processing started: " + str(datetime.datetime.now()))
start = time.time()

runCmd(["otbcli", "BandsExtractor", appLocation, "-il"] + args.input + ["-pixsize", args.resolution, "-merge", "true", "-ndh", "true", "-out", outBands, "-allmasks", outMasks, "-outdate", outDates])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
runCmd(["otbcli", "FeatureExtraction", appLocation, "-rtocr", outBands, "-ndvi", outNdvi])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
runCmd(["otbcli", "PhenologicalNDVIMetrics", appLocation, "-in", outNdvi, "-mask", outMasks, "-dates", outDates,"-out", outMetric])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

# DEPRECATED CALLS
#runCmd(["otbcli", "SigmoFitting2", vegetationStatusLocation, "-in", outNdvi, "-mask", outMasks, "-dates", outDates,"-out", outSigmo])
#try otbcli MetricsEstimation $VEGETATIONSTATUS_OTB_LIBS_ROOT -ipf $OUT_SIGMO -indates $OUT_DATES -opf $OUT_METRIC
#runCmd(["otbcli", "MetricsEstimation2", vegetationStatusLocation, "-ipf", outSigmo, "-opf", outMetric])

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
os.remove(outBands)
#os.remove(outDates)
#os.remove(outNdvi)
#os.remove(outSigmo)

