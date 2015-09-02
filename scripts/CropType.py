#!/usr/bin/python

import os
import os.file
import glob
import argparse
import csv
from sys import argv

#Path to build folder
buildFolder="../../sen2agri-build/"

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

args = parser.parse_args()

reference_polygons=args.ref
sample_ratio=args.ratio

indesc = " "
for desc in args.input:
   indesc = indesc + desc + " "

t0=args.t0
tend=args.tend
sp=args.rate
radius=args.radius
classifier=args.classifier
random_seed=args.rseed
crop_mask=args.mask

reference_polygons_clip=buildFolder+"reference_clip.shp"
training_polygons=buildFolder+"training_polygons.shp"
validation_polygons=buildFolder+"validation_polygons.shp"
rawtocr=buildFolder+"rawtocr.tif"
tocr=buildFolder+"tocr.tif"
rawmask=buildFolder+"rawmask.tif"
mask=buildFolder+"mask.tif"
dates=buildFolder+"dates.txt"
shape=buildFolder+"shape.shp"
rtocr=buildFolder+"rtocr.tif"
fts=buildFolder+"feature-time-series.tif"
statistics=buildFolder+"statistics.xml"
confmatout=buildFolder+"confusion-matrix.csv"
model=buildFolder+"model.txt"
crop_type_map=buildFolder+"crop_type_map.tif"
confusion_matrix_validation=buildFolder+"confusion-matrix-validation.csv"
quality_metrics=buildFolder+"quality-metrics.txt"

# Bands Extractor
print "Executing BandsExtractor..."
beCmdLine = "otbApplicationLauncherCommandLine BandsExtractor "+buildFolder+"CropType/BandsExtractor -il "+indesc+" -out "+rawtocr+" -mask "+rawmask+" -outdate "+dates+" -shape "+shape
print beCmdLine
result = os.system(beCmdLine)

if result != 0 :
   print "Error running BandsExtractor"
   exit(1)
print "BandsExtractor done!"

# ogr2ogr
print "Executing ogr2ogr"
ooCmdLine = "ogr2ogr -clipsrc "+shape+" -overwrite "+reference_polygons_clip+" "+reference_polygons
print ooCmdLine
result = os.system(ooCmdLine)

if result != 0 :
   print "Error running ogr2ogr"
   exit(1)
print "ogr2ogr done!"


# Sample Selection
print "Executing SampleSelection..."
ssCmdLine = "otbApplicationLauncherCommandLine SampleSelection "+buildFolder+"CropType/SampleSelection -ref "+reference_polygons_clip+" -ratio "+sample_ratio+" -seed "+random_seed+" -tp "+training_polygons+" -vp "+validation_polygons
print ssCmdLine
result = os.system(ssCmdLine)

if result != 0 :
   print "Error running SampleSelection"
   exit(1)
print "SampleSelection done!"

# gdalwarp
print "Executing gdalwarp..."

gwCmdLine = "gdalwarp -dstnodata \"-10000\" -overwrite -cutline "+shape+" -crop_to_cutline "+rawtocr+" "+tocr
print gwCmdLine
result = os.system(gwCmdLine)

if result != 0 :
   print "Error running gdalwarp"
   exit(1)

os.system("rm " + rawtocr)

gwCmdLine = "gdalwarp -dstnodata 1 -overwrite -cutline "+shape+" -crop_to_cutline "+rawmask+" "+mask
print gwCmdLine
result = os.system(gwCmdLine)

if result != 0 :
   print "Error running gdalwarp"
   exit(1)

os.system("rm " + rawmask)

print "gdalwarp done!"

# Temporal Resampling
print "Executing TemporalResampling..."
trCmdLine = "otbApplicationLauncherCommandLine TemporalResampling "+buildFolder+"CropType/TemporalResampling -tocr "+tocr+" -mask "+mask+" -ind "+dates+" -sp "+sp+" -t0 "+t0+" -tend "+tend+" -radius "+radius+" -rtocr "+rtocr
print trCmdLine
result = os.system(trCmdLine)

if result != 0 :
   print "Error running TemporalResampling"
   exit(1)

os.system("rm " + tocr)
os.system("rm " + mask)

print "TemporalResampling done!"

# Feature Extraction
print "Executing FeatureExtraction..."
feCmdLine = "otbApplicationLauncherCommandLine FeatureExtraction "+buildFolder+"CropType/FeatureExtraction -rtocr "+rtocr+" -fts "+fts
print feCmdLine
result = os.system(feCmdLine)

if result != 0 :
   print "Error running FeatureExtraction"
   exit(1)

os.system("rm " + rtocr)

print "FeatureExtraction done!"

# Image Statistics
print "Executing ComputeImagesStatistics..."
isCmdLine = "otbcli_ComputeImagesStatistics -il "+fts+" -out "+statistics
print isCmdLine
result = os.system(isCmdLine)

if result != 0 :
   print "Error running ComputeImagesStatistics"
   exit(1)
print "ComputeImagesStatistics done!"

#Train Image Classifier
print "Executing TrainImagesClassifier..."
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
print "TrainImagesClassifier done!"

#Image Classifier
print "Executing ImageClassifier..."
icCmdLine = "otbcli_ImageClassifier -in "+fts+" -imstat "+statistics+" -model "+model+" -out "+crop_type_map
if os.file.exists(crop_mask) :
   icCmdLine += " -mask "+crop_mask
print icCmdLine
result = os.system(icCmdLine)

if result != 0 :
   print "Error running ImageClassifier"
   exit(1)
print "ImageClassifier done!"

#Validation
print "Executing ComputeConfusionMatrix..."
vdCmdLine = "otbcli_ComputeConfusionMatrix -in "+crop_type_map+" -out "+confusion_matrix_validation+" -ref vector -ref.vector.in "+validation_polygons+" -ref.vector.field CODE > "+quality_metrics
print vdCmdLine
result = os.system(vdCmdLine)

if result != 0 :
   print "Error running ComputeConfusionMatrix"
   exit(1)
print "ComputeConfusionMatrix done!"

print "Execution successfull !"

