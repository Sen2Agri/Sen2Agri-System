#!/usr/bin/python

import os
import os.path
import glob
import argparse
import csv
from sys import argv
import datetime
from common import executeStep 

#Path to build folder
defaultBuildFolder="~/sen2agri-build/"

parser = argparse.ArgumentParser(description='CropType Python processor')

parser.add_argument('-ref', help='The reference polygons', required=True, metavar='reference_polygons')
parser.add_argument('-ratio', help='The ratio between the validation and training polygons', required=False, metavar='sample_ratio', default=0.75)
parser.add_argument('-input', help='The list of products descriptors', required=True, metavar='product_descriptor', nargs='+')
parser.add_argument('-t0', help='The start date for the temporal resampling interval (in format YYYYMMDD)', required=True, metavar='YYYYMMDD')
parser.add_argument('-tend', help='The end date for the temporal resampling interval (in format YYYYMMDD)', required=True, metavar='YYYYMMDD')
parser.add_argument('-rate', help='The sampling rate for the temporal series, in days', required=False, metavar='sampling_rate', default=5)
parser.add_argument('-radius', help='The radius used for gapfilling, in days', required=False, metavar='radius', default=15)
parser.add_argument('-classifier', help='The classifier used for training (either rf or svm)', required=True, metavar='classifier', choices=['rf','svm'])
parser.add_argument('-rseed', help='The random seed used for training', required=False, metavar='random_seed', default=0)
parser.add_argument('-mask', help='The crop mask', required=False, metavar='crop_mask', default='')
parser.add_argument('-pixsize', help='The size, in meters, of a pixel (default 10)', required=False, metavar='pixsize', default=10)
parser.add_argument('-outdir', help="Output directory", default=defaultBuildFolder)
parser.add_argument('-buildfolder', help="Build folder", default=defaultBuildFolder)

parser.add_argument('-rfnbtrees', help='The number of trees used for training (default 100)', required=False, metavar='rfnbtrees', default=100)
parser.add_argument('-rfmax', help='maximum depth of the trees used for Random Forest classifier (default 25)', required=False, metavar='rfmax', default=25)
parser.add_argument('-rfmin', help='minimum number of samples in each node used by the classifier (default 5)', required=False, metavar='rfmin', default=5)

parser.add_argument('-keepfiles', help="Keep all intermediate files (default false)", default=False, action='store_true')
parser.add_argument('-fromstep', help="Run from the selected step (default 1)", type=int, default=1)

args = parser.parse_args()

reference_polygons=args.ref
sample_ratio=args.ratio

indesc = args.input

t0=args.t0
tend=args.tend
sp=args.rate
radius=args.radius
classifier=args.classifier
random_seed=args.rseed
crop_mask=args.mask
pixsize=args.pixsize
rfnbtrees=str(args.rfnbtrees)
rfmax=str(args.rfmax)
rfmin=str(args.rfmin)

buildFolder=args.buildfolder

reference_polygons_clip=os.path.join(args.outdir, "reference_clip.shp")
training_polygons=os.path.join(args.outdir, "training_polygons.shp")
validation_polygons=os.path.join(args.outdir, "validation_polygons.shp")
rawtocr=os.path.join(args.outdir, "rawtocr.tif")
tocr=os.path.join(args.outdir, "tocr.tif")
rawmask=os.path.join(args.outdir, "rawmask.tif")
mask=os.path.join(args.outdir, "mask.tif")
dates=os.path.join(args.outdir, "dates.txt")
shape=os.path.join(args.outdir, "shape.shp")
rtocr=os.path.join(args.outdir, "rtocr.tif")
fts=os.path.join(args.outdir, "feature-time-series.tif")
statistics=os.path.join(args.outdir, "statistics.xml")
confmatout=os.path.join(args.outdir, "confusion-matrix.csv")
model=os.path.join(args.outdir, "model.txt")
crop_type_map_uncut=os.path.join(args.outdir, "crop_type_map_uncut.tif")
crop_type_map=os.path.join(args.outdir, "crop_type_map.tif")
confusion_matrix_validation=os.path.join(args.outdir, "confusion-matrix-validation.csv")
quality_metrics=os.path.join(args.outdir, "quality-metrics.txt")

keepfiles = args.keepfiles
fromstep = args.fromstep

globalStart = datetime.datetime.now()

# Bands Extractor (Step 1)
executeStep("BandsExtractor", "otbApplicationLauncherCommandLine", "BandsExtractor", os.path.join(buildFolder,"CropType/BandsExtractor"),"-out",rawtocr,"-mask",rawmask,"-outdate", dates, "-shape", shape, "-pixsize", pixsize, "-il", *indesc, skip=fromstep>1)

# gdalwarp (Step 2 and 3)
executeStep("gdalwarp for reflectances", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", rawtocr, tocr, skip=fromstep>2, rmfiles=[] if keepfiles else [rawtocr])

executeStep("gdalwarp for masks", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", rawmask, mask, skip=fromstep>3, rmfiles=[] if keepfiles else [rawmask])

# Temporal Resampling (Step 4)
executeStep("TemporalResampling", "otbApplicationLauncherCommandLine", "TemporalResampling", os.path.join(buildFolder,"CropType/TemporalResampling"), "-tocr", tocr, "-mask", mask, "-ind", dates, "-sp", sp, "-t0", t0, "-tend", tend, "-radius", radius, "-rtocr", rtocr, skip=fromstep>4, rmfiles=[] if keepfiles else [tocr, mask])

# Feature Extraction (Step 5)
executeStep("FeatureExtraction", "otbApplicationLauncherCommandLine", "FeatureExtraction", os.path.join(buildFolder,"CropType/FeatureExtraction"), "-rtocr", rtocr, "-fts", fts, skip=fromstep>5, rmfiles=[] if keepfiles else [rtocr])

# ogr2ogr (Step 6)
executeStep("ogr2ogr", "/usr/local/bin/ogr2ogr", "-clipsrc", shape,"-overwrite",reference_polygons_clip, reference_polygons, skip=fromstep>6)

# Sample Selection (Step 7)
executeStep("SampleSelection", "otbApplicationLauncherCommandLine","SampleSelection", os.path.join(buildFolder,"CropType/SampleSelection"), "-ref",reference_polygons_clip,"-ratio", sample_ratio, "-seed", random_seed, "-tp", training_polygons, "-vp", validation_polygons, skip=fromstep>7)

# Image Statistics (Step 8)
executeStep("ComputeImagesStatistics", "otbcli_ComputeImagesStatistics", "-il", fts,"-out",statistics, skip=fromstep>8)

#Train Image Classifier (Step 9)
if classifier == "rf" :
	executeStep("TrainImagesClassifier", "otbcli_TrainImagesClassifier", "-io.il", fts,"-io.vd",training_polygons,"-io.imstat", statistics, "-rand", random_seed, "-sample.bm", "0", "-io.confmatout", confmatout,"-io.out",model,"-sample.mt", "-1","-sample.mv","-1","-sample.vfn","CODE","-sample.vtr",sample_ratio,"-classifier","rf", "-classifier.rf.nbtrees",rfnbtrees,"-classifier.rf.min",rfmin,"-classifier.rf.max",rfmax, skip=fromstep>9)
elif classifier == "svm":
	executeStep("TrainImagesClassifier", "otbcli_TrainImagesClassifier", "-io.il", fts,"-io.vd",training_polygons,"-io.imstat", statistics, "-rand", random_seed, "-sample.bm", "0", "-io.confmatout", confmatout,"-io.out",model,"-sample.mt", "-1","-sample.mv","-1","-sample.vfn","CODE","-sample.vtr",sample_ratio,"-classifier","svm", "-classifier.svm.k",rbf,"-classifier.svm.opt",1, skip=fromstep>9)

#Image Classifier (Step 10)
if os.path.isfile(crop_mask) :
	executeStep("ImageClassifier", "otbcli_ImageClassifier", "-in", fts,"-imstat",statistics,"-model", model, "-mask", crop_mask, "-out", crop_type_map_uncut, "int16", skip=fromstep>10, rmfiles=[] if keepfiles else [fts])
else:
	executeStep("ImageClassifier", "otbcli_ImageClassifier", "-in", fts,"-imstat",statistics,"-model", model, "-out", crop_type_map_uncut, "int16", skip=fromstep>10, rmfiles=[] if keepfiles else [fts])

# gdalwarp (Step 11)
executeStep("gdalwarp for crop type", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", crop_type_map_uncut, crop_type_map, skip=fromstep>11)

#Validation (Step 12)
executeStep("Validation for Crop Type", "otbcli_ComputeConfusionMatrix", "-in", crop_type_map, "-out", confusion_matrix_validation, "-ref", "vector", "-ref.vector.in", validation_polygons, "-ref.vector.field", "CODE", "-nodatalabel", "-10000", outf=quality_metrics, skip=fromstep>12)

#Product creation (Step 13)
executeStep("ProductFormatter", "otbApplicationLauncherCommandLine", "ProductFormatter", os.path.join(buildFolder,"MACCSMetadata/src"), "-destroot", args.outdir, "-fileclass", "SVT1", "-level", "L4B", "-timeperiod", t0+"_"+tend, "-baseline", "-01.00", "-processor", "croptype", "-processor.croptype.file", crop_type_map, skip=fromstep>13)

globalEnd = datetime.datetime.now()

print "Processor CropType finished in " + str(globalEnd-globalStart)


