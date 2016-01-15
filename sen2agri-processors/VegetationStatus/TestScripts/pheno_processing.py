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

parser.add_argument('--applocation', help='The path where the sen2agri is built', required=True)
parser.add_argument('--input', help='The list of products xml descriptors', required=True, nargs='+')
parser.add_argument('--t0', help='The start date for the temporal resampling interval (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--tend', help='The end date for the temporal resampling interval (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--outdir', help="Output directory", required=True)
parser.add_argument('--tileid', help="Tile id", required=False)

args = parser.parse_args()

appLocation = args.applocation
outDir = args.outdir
t0 = args.t0
tend = args.tend

vegetationStatusLocation = "{}/VegetationStatus/phenotb/src/Applications".format(appLocation)
cropTypeLocation = "{}/CropType".format(appLocation)
productFormatterLocation = "{}/MACCSMetadata/src".format(appLocation)

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

print("Processing started: " + str(datetime.datetime.now()))
start = time.time()

#try otbcli BandsExtractor $CROPTTYPE_OTB_LIBS_ROOT/BandsExtractor/ -il $TIME_SERIES_XMLS -merge true -ndh true -out $OUT_BANDS -allmasks $OUT_MASKS -outdate $OUT_DATES
runCmd(["otbcli", "BandsExtractor", cropTypeLocation, "-il"] + args.input + ["-pixsize", "20", "-merge", "true", "-ndh", "true", "-out", outBands, "-allmasks", outMasks, "-outdate", outDates])
#try otbcli FeatureExtraction $CROPTTYPE_OTB_LIBS_ROOT/FeatureExtraction -rtocr $OUT_BANDS -ndvi $OUT_NDVI
runCmd(["otbcli", "FeatureExtraction", cropTypeLocation, "-rtocr", outBands, "-ndvi", outNdvi])
#try otbcli SigmoFitting $VEGETATIONSTATUS_OTB_LIBS_ROOT -in $OUT_NDVI -mask $OUT_MASKS -dates $OUT_DATES -out $OUT_SIGMO
runCmd(["otbcli", "SigmoFitting", vegetationStatusLocation, "-in", outNdvi, "-mask", outMasks, "-dates", outDates,"-out", outSigmo])
#try otbcli MetricsEstimation $VEGETATIONSTATUS_OTB_LIBS_ROOT -ipf $OUT_SIGMO -indates $OUT_DATES -opf $OUT_METRIC
runCmd(["otbcli", "MetricsEstimation", vegetationStatusLocation, "-ipf", outSigmo, "-indates", outDates, "-opf", outMetric])

runCmd(["otbcli", "ProductFormatter", productFormatterLocation, "-destroot", outDir, "-fileclass", "SVT1", "-level", "L3B", "-timeperiod", t0 + '_' + tend, "-baseline", "01.00", "-processor", "vegetation", "-processor.vegetation.pheno", tileID, outMetric, "-il", args.input[0]])

print("Processing finished: " + str(datetime.datetime.now()))
print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

#os.remove(outMasks)
os.remove(outBands)
#os.remove(outDates)
#os.remove(outNdvi)
#os.remove(outSigmo)

