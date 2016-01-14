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

parser.add_argument('-mission', help='The main mission for the series', required=False, default='SPOT')

parser.add_argument('-ref', help='The reference polygons', required=True, metavar='reference_polygons')
parser.add_argument('-ratio', help='The ratio between the validation and training polygons (default 0.75)', required=False, metavar='sample_ratio', default=0.75)
parser.add_argument('-input', help='The list of products descriptors', required=True, metavar='product_descriptor', nargs='+')
parser.add_argument('-t0', help='The start date for the temporal resampling interval (in format YYYYMMDD)', required=True, metavar='YYYYMMDD')
parser.add_argument('-tend', help='The end date for the temporal resampling interval (in format YYYYMMDD)', required=True, metavar='YYYYMMDD')
parser.add_argument('-rate', help='The sampling rate for the temporal series, in days (default 5)', required=False, metavar='sampling_rate', default=5)
# parser.add_argument('-radius', help='The radius used for gapfilling, in days (default 15)', required=False, metavar='radius', default=15)
parser.add_argument('-classifier', help='The classifier (rf or svm) used for training (default rf)',
        required=False, metavar='classifier', choices=['rf','svm'], default='rf')
parser.add_argument('-rseed', help='The random seed used for training (default 0)', required=False, metavar='random_seed', default=0)
parser.add_argument('-mask', help='The crop mask', required=False, metavar='crop_mask', default='')
parser.add_argument('-pixsize', help='The size, in meters, of a pixel (default 10)', required=False, metavar='pixsize', default=10)
parser.add_argument('-tilename', help="The name of the tile", default="T0000")
parser.add_argument('-outdir', help="Output directory", default=defaultBuildFolder)
parser.add_argument('-buildfolder', help="Build folder", default=defaultBuildFolder)
parser.add_argument('-targetfolder', help="The folder where the target product is built", default="")

parser.add_argument('-rfnbtrees', help='The number of trees used for training (default 100)', required=False, metavar='rfnbtrees', default=100)
parser.add_argument('-rfmax', help='maximum depth of the trees used for Random Forest classifier (default 25)', required=False, metavar='rfmax', default=25)
parser.add_argument('-rfmin', help='minimum number of samples in each node used by the classifier (default 25)', required=False, metavar='rfmin', default=25)

parser.add_argument('-keepfiles', help="Keep all intermediate files (default false)", default=False, action='store_true')
parser.add_argument('-fromstep', help="Run from the selected step (default 1)", type=int, default=1)

args = parser.parse_args()

mission=args.mission

reference_polygons=args.ref
sample_ratio=str(args.ratio)

indesc = args.input

t0=args.t0
tend=args.tend
sp=args.rate
#radius=args.radius
radius="15"  # not used
classifier=args.classifier
random_seed=args.rseed
crop_mask=args.mask
pixsize=args.pixsize
rfnbtrees=str(args.rfnbtrees)
rfmax=str(args.rfmax)
rfmin=str(args.rfmin)

buildFolder=args.buildfolder

targetFolder=args.targetfolder if args.targetfolder != "" else args.outdir
tilename = args.tilename

reference_polygons_reproject=os.path.join(args.outdir, "reference_polygons_reproject.shp")
reference_polygons_clip=os.path.join(args.outdir, "reference_clip.shp")
training_polygons=os.path.join(args.outdir, "training_polygons.shp")
validation_polygons=os.path.join(args.outdir, "validation_polygons.shp")
rawtocr=os.path.join(args.outdir, "rawtocr.tif")
tocr=os.path.join(args.outdir, "tocr.tif")
rawmask=os.path.join(args.outdir, "rawmask.tif")
mask=os.path.join(args.outdir, "mask.tif")
dates=os.path.join(args.outdir, "dates.txt")
shape=os.path.join(args.outdir, "shape.shp")
shape_proj=os.path.join(args.outdir, "shape.prj")
rtocr=os.path.join(args.outdir, "rtocr.tif")
fts=os.path.join(args.outdir, "feature-time-series.tif")
statistics=os.path.join(args.outdir, "statistics.xml")
confmatout=os.path.join(args.outdir, "confusion-matrix.csv")
model=os.path.join(args.outdir, "model.txt")
lut=os.path.join(args.outdir, "lut.txt")

reprojected_crop_mask=os.path.join(args.outdir, "reprojected_crop_mask.tif")
cropped_crop_mask=os.path.join(args.outdir, "cropped_crop_mask.tif")

crop_type_map_uncut=os.path.join(args.outdir, "crop_type_map_uncut.tif")
crop_type_map_uncompressed=os.path.join(args.outdir, "crop_type_map_uncompressed.tif")
crop_type_map=os.path.join(args.outdir, "crop_type_map.tif")
color_crop_type_map=os.path.join(args.outdir, "color_crop_type_map.tif")

confusion_matrix_validation=os.path.join(args.outdir, "confusion-matrix-validation.csv")
quality_metrics=os.path.join(args.outdir, "quality-metrics.txt")
xml_validation_metrics=os.path.join(args.outdir, "validation-metrics.xml")

keepfiles = args.keepfiles
fromstep = args.fromstep

if not os.path.exists(args.outdir):
    os.makedirs(args.outdir)

globalStart = datetime.datetime.now()

try:
# Bands Extractor (Step 1)
    executeStep("BandsExtractor", "otbcli", "BandsExtractor", os.path.join(buildFolder,"CropType/BandsExtractor"),"-mission",mission,"-out",rawtocr,"-mask",rawmask,"-outdate", dates, "-shape", shape, "-pixsize", pixsize, "-il", *indesc, skip=fromstep>1)

# gdalwarp (Step 2 and 3)
    executeStep("gdalwarp for reflectances", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", rawtocr, tocr, skip=fromstep>2, rmfiles=[] if keepfiles else [rawtocr])

    executeStep("gdalwarp for masks", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", rawmask, mask, skip=fromstep>3, rmfiles=[] if keepfiles else [rawmask])

# Temporal Resampling (Step 4)
    executeStep("TemporalResampling", "otbcli", "TemporalResampling", os.path.join(buildFolder,"CropType/TemporalResampling"), "-tocr", tocr, "-mask", mask, "-ind", dates, "-sp", sp, "-t0", t0, "-tend", tend, "-radius", radius, "-rtocr", rtocr, skip=fromstep>4, rmfiles=[] if keepfiles else [tocr, mask])

# Feature Extraction (Step 5)
    executeStep("FeatureExtraction", "otbcli", "FeatureExtraction", os.path.join(buildFolder,"CropType/FeatureExtraction"), "-rtocr", rtocr, "-fts", fts, skip=fromstep>5, rmfiles=[] if keepfiles else [rtocr])

# ogr2ogr Reproject insitu data (Step 6)
    with open(shape_proj, 'r') as file:
    shape_wkt = "ESRI::" + file.read()

    executeStep("ogr2ogr", "/usr/local/bin/ogr2ogr", "-t_srs", shape_wkt, "-progress", "-overwrite",reference_polygons_reproject, reference_polygons, skip=fromstep>6)

# ogr2ogr Crop insitu data (Step 7)
    executeStep("ogr2ogr", "/usr/local/bin/ogr2ogr", "-clipsrc", shape, "-progress", "-overwrite",reference_polygons_clip, reference_polygons_reproject, skip=fromstep>7)

# Sample Selection (Step 8)
    executeStep("SampleSelection", "otbcli","SampleSelection", os.path.join(buildFolder,"CropType/SampleSelection"), "-ref",reference_polygons_clip,"-ratio", sample_ratio, "-seed", random_seed, "-tp", training_polygons, "-vp", validation_polygons, "-lut", lut, skip=fromstep>8)

# Image Statistics (Step 9)
    executeStep("ComputeImagesStatistics", "otbcli_ComputeImagesStatistics", "-il", fts,"-out",statistics, skip=fromstep>9)

#Train Image Classifier (Step 10)
    if classifier == "rf" :
    executeStep("TrainImagesClassifier", "otbcli_TrainImagesClassifier", "-io.il", fts,"-io.vd",training_polygons,"-io.imstat", statistics, "-rand", random_seed, "-sample.bm", "0", "-io.confmatout", confmatout,"-io.out",model,"-sample.mt", "-1","-sample.mv","-1","-sample.vfn","CODE","-sample.vtr","0.9","-classifier","rf", "-classifier.rf.nbtrees",rfnbtrees,"-classifier.rf.min", rfmin,"-classifier.rf.max",rfmax, skip=fromstep>10)
    elif classifier == "svm":
    executeStep("TrainImagesClassifier", "otbcli_TrainImagesClassifier", "-io.il", fts,"-io.vd",training_polygons,"-io.imstat", statistics, "-rand", random_seed, "-sample.bm", "0", "-io.confmatout", confmatout,"-io.out",model,"-sample.mt", "-1","-sample.mv","-1","-sample.vfn","CODE","-sample.vtr","0.9","-classifier","svm", "-classifier.svm.k", rbf,"-classifier.svm.opt",1, skip=fromstep>10)

# Crop Mask Preparation (Step 11 and 12)
    if os.path.isfile(crop_mask):
    executeStep("gdalwarp for reprojecting crop mask", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "0", "-overwrite", "-t_srs", shape_wkt, crop_mask, reprojected_crop_mask, skip=fromstep>11)

    executeStep("gdalwarp for cutting crop mask", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "0", "-overwrite", "-tr", pixsize, pixsize, "-cutline", shape, "-crop_to_cutline", reprojected_crop_mask, cropped_crop_mask, skip=fromstep>12)

#Image Classifier (Step 13)
    if os.path.isfile(crop_mask):
    executeStep("ImageClassifier", "otbcli_ImageClassifier", "-in", fts,"-imstat",statistics,"-model", model, "-mask", cropped_crop_mask, "-out", crop_type_map_uncut, "int16", skip=fromstep>13, rmfiles=[] if keepfiles else [fts])
    else:
    executeStep("ImageClassifier", "otbcli_ImageClassifier", "-in", fts,"-imstat",statistics,"-model", model, "-out", crop_type_map_uncut, "int16", skip=fromstep>13, rmfiles=[] if keepfiles else [fts])

# gdalwarp (Step 14)
    executeStep("gdalwarp for crop type", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", crop_type_map_uncut, crop_type_map_uncompressed, skip=fromstep>14)

#Validation (Step 15)
    executeStep("Validation for Crop Type", "otbcli_ComputeConfusionMatrix", "-in",  crop_type_map_uncompressed, "-out", confusion_matrix_validation, "-ref", "vector", "-ref.vector.in", validation_polygons, "-ref.vector.field", "CODE", "-nodatalabel", "-10000", outf=quality_metrics, skip=fromstep>15)

#Color image (Step 16)
    executeStep("ColorMapping", "otbcli_ColorMapping", "-in", crop_type_map_uncompressed,"-method","custom","-method.custom.lut", lut, "-out", color_crop_type_map, "int32", skip=fromstep>16)

#Compression (Step 17)
    executeStep("Compression", "otbcli_Convert", "-in",  crop_type_map_uncompressed, "-out", crop_type_map+"?gdal:co:COMPRESS=DEFLATE", "int16",  skip=fromstep>17, rmfiles=[] if keepfiles else [ crop_type_map_uncompressed])

#XML conversion (Step 18)
    executeStep("XML Conversion for Crop Type", "otbcli", "XMLStatistics", os.path.join(buildFolder,"Common/XMLStatistics"), "-confmat", confusion_matrix_validation, "-quality", quality_metrics, "-root", "CropType", "-out", xml_validation_metrics,  skip=fromstep>18)

#Product creation (Step 19)
    executeStep("ProductFormatter", "otbcli", "ProductFormatter", os.path.join(buildFolder,"MACCSMetadata/src"), "-destroot", targetFolder, "-fileclass", "SVT1", "-level", "L4B", "-timeperiod", t0+"_"+tend, "-baseline", "-01.00", "-processor", "croptype", "-processor.croptype.file", "TILE_"+tilename, crop_type_map, "-processor.croptype.quality", xml_validation_metrics, skip=fromstep>19)

finally:
    globalEnd = datetime.datetime.now()
    print "Processor CropType finished in " + str(globalEnd-globalStart)
