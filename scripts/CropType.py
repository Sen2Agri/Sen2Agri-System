#!/usr/bin/python

import os
import os.path
import glob
import argparse
import csv
from sys import argv
import datetime

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

args = parser.parse_args()

reference_polygons=args.ref
sample_ratio=args.ratio

indesc = " "
for desc in args.input:
   indesc = indesc + '"' + desc + '"' + " "

t0=args.t0
tend=args.tend
sp=args.rate
radius=args.radius
classifier=args.classifier
random_seed=args.rseed
crop_mask=args.mask
pixsize=args.pixsize

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
crop_type_map=os.path.join(args.outdir, "crop_type_map.tif")
confusion_matrix_validation=os.path.join(args.outdir, "confusion-matrix-validation.csv")
quality_metrics=os.path.join(args.outdir, "quality-metrics.txt")

# Bands Extractor
print "Executing BandsExtractor at " + str(datetime.datetime.now())
beCmdLine = "otbApplicationLauncherCommandLine BandsExtractor "+os.path.join(buildFolder,"CropType/BandsExtractor")+" -il "+indesc+" -out "+rawtocr+" -mask "+rawmask+" -outdate "+dates+" -shape "+shape+" -pixsize "+pixsize
print beCmdLine
result = os.system(beCmdLine)

if result != 0 :
   print "Error running BandsExtractor"
   exit(1)
print "BandsExtractor done at " + str(datetime.datetime.now())

# ogr2ogr
print "Executing ogr2ogr at " + str(datetime.datetime.now())
ooCmdLine = "/usr/local/bin/ogr2ogr -clipsrc "+shape+" -overwrite "+reference_polygons_clip+" "+reference_polygons
print ooCmdLine
result = os.system(ooCmdLine)

if result != 0 :
   print "Error running ogr2ogr"
   exit(1)
print "ogr2ogr done at " + str(datetime.datetime.now())


# Sample Selection
print "Executing SampleSelection at " + str(datetime.datetime.now())
ssCmdLine = "otbApplicationLauncherCommandLine SampleSelection "+os.path.join(buildFolder,"CropType/SampleSelection")+" -ref "+reference_polygons_clip+" -ratio "+sample_ratio+" -seed "+random_seed+" -tp "+training_polygons+" -vp "+validation_polygons
print ssCmdLine
result = os.system(ssCmdLine)

if result != 0 :
   print "Error running SampleSelection"
   exit(1)
print "SampleSelection done at " + str(datetime.datetime.now())

# gdalwarp
print "Executing gdalwarp at " + str(datetime.datetime.now())

gwCmdLine = "/usr/local/bin/gdalwarp -multi -wm 2048 -dstnodata \"-10000\" -overwrite -cutline "+shape+" -crop_to_cutline "+rawtocr+" "+tocr
print gwCmdLine
result = os.system(gwCmdLine)

if result != 0 :
   print "Error running gdalwarp"
   exit(1)

os.system("rm " + rawtocr)

gwCmdLine = "/usr/local/bin/gdalwarp -multi -wm 2048 -dstnodata 1 -overwrite -cutline "+shape+" -crop_to_cutline "+rawmask+" "+mask
print gwCmdLine
result = os.system(gwCmdLine)

if result != 0 :
   print "Error running gdalwarp"
   exit(1)

os.system("rm " + rawmask)

print "gdalwarp done at " + str(datetime.datetime.now())

# Temporal Resampling
print "Executing TemporalResampling at " + str(datetime.datetime.now())
trCmdLine = "otbApplicationLauncherCommandLine TemporalResampling "+os.path.join(buildFolder,"CropType/TemporalResampling")+" -tocr "+tocr+" -mask "+mask+" -ind "+dates+" -sp "+sp+" -t0 "+t0+" -tend "+tend+" -radius "+radius+" -rtocr "+rtocr
print trCmdLine
result = os.system(trCmdLine)

if result != 0 :
   print "Error running TemporalResampling"
   exit(1)

os.system("rm " + tocr)
os.system("rm " + mask)

print "TemporalResampling done at " + str(datetime.datetime.now())

# Feature Extraction
print "Executing FeatureExtraction at " + str(datetime.datetime.now())
feCmdLine = "otbApplicationLauncherCommandLine FeatureExtraction "+os.path.join(buildFolder,"CropType/FeatureExtraction")+" -rtocr "+rtocr+" -fts "+fts
print feCmdLine
result = os.system(feCmdLine)

if result != 0 :
   print "Error running FeatureExtraction"
   exit(1)

os.system("rm " + rtocr)

print "FeatureExtraction done at " + str(datetime.datetime.now())

# Image Statistics
print "Executing ComputeImagesStatistics at " + str(datetime.datetime.now())
isCmdLine = "otbcli_ComputeImagesStatistics -il "+fts+" -out "+statistics
print isCmdLine
result = os.system(isCmdLine)

if result != 0 :
   print "Error running ComputeImagesStatistics"
   exit(1)
print "ComputeImagesStatistics done at " + str(datetime.datetime.now())

#Train Image Classifier
print "Executing TrainImagesClassifier at " + str(datetime.datetime.now())
tcCmdLine = "otbcli_TrainImagesClassifier -io.il "+fts+" -io.vd "+training_polygons+" -io.imstat "+statistics+" -rand "+random_seed+" -sample.bm 0 -io.confmatout "+confmatout+" -io.out "+model+" -sample.mt -1 -sample.mv -1 -sample.vfn CODE -sample.vtr "+sample_ratio+" -classifier "+classifier

if classifier == "rf" :
   tcCmdLine += " -classifier.rf.nbtrees 100 -classifier.rf.min 5 -classifier.rf.max 25"
elif classifier == "svm" :
   tcCmdLine += " -classifier.svm.k rbf -classifier.svm.opt 1"
print tcCmdLine
result = os.system(tcCmdLine)

if result != 0 :
   print "Error running TrainImagesClassifier"
   exit(1)
print "TrainImagesClassifier done at " + str(datetime.datetime.now())

#Image Classifier
print "Executing ImageClassifier at " + str(datetime.datetime.now())
icCmdLine = "otbcli_ImageClassifier -in "+fts+" -imstat "+statistics+" -model "+model+" -out "+crop_type_map
if os.path.isfile(crop_mask) :
   icCmdLine += " -mask "+crop_mask
print icCmdLine
result = os.system(icCmdLine)

if result != 0 :
   print "Error running ImageClassifier"
   exit(1)
print "ImageClassifier done at " + str(datetime.datetime.now())

#Validation
print "Executing ComputeConfusionMatrix at " + str(datetime.datetime.now())
vdCmdLine = "otbcli_ComputeConfusionMatrix -in "+crop_type_map+" -out "+confusion_matrix_validation+" -ref vector -ref.vector.in "+validation_polygons+" -ref.vector.field CODE -nodatalabel 10 > "+quality_metrics
print vdCmdLine
result = os.system(vdCmdLine)

if result != 0 :
   print "Error running ComputeConfusionMatrix"
   exit(1)
print "ComputeConfusionMatrix done at " + str(datetime.datetime.now())

#Product creation
print "Executing ProductFormatter at " + str(datetime.datetime.now())
pfCmdLine = "otbApplicationLauncherCommandLine ProductFormatter "+os.path.join(buildFolder,"MACCSMetadata/src")+" -destroot "+args.outdir+" -fileclass SVT1 -level L4B -timeperiod "+t0+"_"+tend+" -baseline 01.00 -processor croptype -processor.croptype.file "+crop_type_map
print pfCmdLine
result = os.system(pfCmdLine)

if result != 0 :
   print "Error running ProductFormatter"
   exit(1)
print "ProductFormatter done at " + str(datetime.datetime.now())


print "Execution successfull !"

