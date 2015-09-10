#!/usr/bin/python

import os
import glob
import argparse
import csv
from sys import argv

#Path to build folder
buildFolder="../../sen2agri-build/"

parser = argparse.ArgumentParser(description='CropMask Python processor')

parser.add_argument('-ref', help='The reference polygons', required=True, metavar='reference_polygons')
parser.add_argument('-ratio', help='The ratio between the validation and training polygons', required=False, metavar='sample_ratio', default=0.75)
parser.add_argument('-input', help='The list of products descriptors', required=True, metavar='product_descriptor', nargs='+')
parser.add_argument('-t0', help='The start date for the temporal resampling interval (in format YYYYMMDD)', required=True, metavar='YYYYMMDD')
parser.add_argument('-tend', help='The end date for the temporal resampling interval (in format YYYYMMDD)', required=True, metavar='YYYYMMDD')
parser.add_argument('-rate', help='The sampling rate for the temporal series, in days', required=False, metavar='sampling_rate', default=5)
parser.add_argument('-radius', help='The radius used for gapfilling, in days', required=False, metavar='radius', default=15)
parser.add_argument('-nbtrsample', help='The number of samples included in the training set.', required=False, metavar='nbtrsample', default=1000)
parser.add_argument('-rseed', help='The random seed used for training', required=False, metavar='random_seed', default=0)


parser.add_argument('-rfnbtrees', help='The number of trees used for training (default 100)', required=False, metavar='rfnbtrees', default=100)
parser.add_argument('-rfmax', help='maximum depth of the trees used for Random Forest classifier (default 25)', required=False, metavar='rfmax', default=25)
parser.add_argument('-rfmin', help='minimum number of samples in each node used by the classifier (default 5)', required=False, metavar='rfmin', default=5)

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
random_seed=args.rseed
nbtrsample=args.nbtrsample
rfnbtrees=args.rfnbtrees
rfmax=args.rfmax
rfmin=args.rfmin

reference_polygons_clip=buildFolder+"reference_clip.shp"
training_polygons=buildFolder+"training_polygons.shp"
validation_polygons=buildFolder+"validation_polygons.shp"
random_training_polygons=buildFolder+"random_training_polygons.shp"
random_testing_polygons=buildFolder+"random_testing_polygons.shp"

rawtocr=buildFolder+"rawtocr.tif"
tocr=buildFolder+"tocr.tif"
rawmask=buildFolder+"rawmask.tif"
mask=buildFolder+"mask.tif"
dates=buildFolder+"dates.txt"
outdays=buildFolder+"days.txt"
shape=buildFolder+"shape.shp"
rtocr=buildFolder+"rtocr.tif"
ndvi=buildFolder+"ndvi.tif"
ndwi=buildFolder+"ndwi.tif"
brightness=buildFolder+"brightness.tif"
temporal_features=buildFolder+"tf.tif"
statistic_features=buildFolder+"sf.tif"
features=buildFolder+"concat_features.tif"
statistics=buildFolder+"statistics.xml"

confmatout=buildFolder+"confusion-matrix.csv"
model=buildFolder+"crop-mask-model.txt"
crop_mask=buildFolder+"crop_mask.tif"
confusion_matrix_validation=buildFolder+"crop-mask-confusion-matrix-validation.csv"
quality_metrics=buildFolder+"crop-mask-quality-metrics.txt"

# Bands Extractor
print "Executing BandsExtractor..."
beCmdLine = "otbApplicationLauncherCommandLine BandsExtractor "+buildFolder+"CropType/BandsExtractor -il "+indesc+" -out "+rawtocr+" -mask "+rawmask+" -outdate "+dates+" -shape "+shape
print beCmdLine
result = os.system(beCmdLine)

if result != 0 :
   print "Error running BandsExtractor"
   exit(1)
print "BandsExtractor done!"

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
trCmdLine = "otbApplicationLauncherCommandLine TemporalResampling "+buildFolder+"CropType/TemporalResampling -tocr "+tocr+" -mask "+mask+" -ind "+dates+" -sp "+sp+" -t0 "+t0+" -tend "+tend+" -radius "+radius+" -rtocr "+rtocr+" -outdays "+outdays
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
feCmdLine = "otbApplicationLauncherCommandLine FeatureExtraction "+buildFolder+"CropType/FeatureExtraction -rtocr "+rtocr+" -ndvi "+ndvi+" -ndwi "+ndwi+" -brightness "+brightness
print feCmdLine
result = os.system(feCmdLine)

if result != 0 :
   print "Error running FeatureExtraction"
   exit(1)

os.system("rm " + rtocr)

print "FeatureExtraction done!"

# Temporal Features
print "Executing TemporalFeatures..."
tfCmdLine = "otbApplicationLauncherCommandLine TemporalFeatures "+buildFolder+"CropMask/TemporalFeatures -ndvi "+ndvi+" -dates "+outdays+" -tf "+temporal_features
print tfCmdLine
result = os.system(tfCmdLine)

if result != 0 :
   print "Error running TemporalFeatures"
   exit(1)

os.system("rm " + ndvi)

print "TemporalFeatures done!"

# Statistic Features
print "Executing StatisticFeatures..."
sfCmdLine = "otbApplicationLauncherCommandLine StatisticFeatures "+buildFolder+"CropMask/StatisticFeatures -ndwi "+ndwi+" -brightness "+brightness+" -sf "+statistic_features
print sfCmdLine
result = os.system(sfCmdLine)

if result != 0 :
   print "Error running StatisticFeatures"
   exit(1)

os.system("rm " + ndwi)
os.system("rm " + brightness)

print "StatisticFeatures done!"

# _Concatenate Images
print "Executing ConcatenateImages..."
ciCmdLine = "otbcli_ConcatenateImages -il "+temporal_features+" "+statistic_features+" -out " + features
print ciCmdLine
result = os.system(ciCmdLine)

if result != 0 :
   print "Error running ConcatenateImages"
   exit(1)

os.system("rm " + temporal_features)
os.system("rm " + statistic_features)

print "ConcatenateImages done!"


# Image Statistics
print "Executing ComputeImagesStatistics..."
isCmdLine = "otbcli_ComputeImagesStatistics -il "+features+" -out "+statistics
print isCmdLine
result = os.system(isCmdLine)

if result != 0 :
   print "Error running ComputeImagesStatistics"
   exit(1)
print "ComputeImagesStatistics done!"

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
ssCmdLine = "otbApplicationLauncherCommandLine SampleSelection "+buildFolder+"CropType/SampleSelection -ref "+reference_polygons_clip+" -ratio "+sample_ratio+" -seed "+random_seed+" -tp "+training_polygons+" -vp "+validation_polygons + " -nofilter true"
print ssCmdLine
result = os.system(ssCmdLine)

if result != 0 :
   print "Error running SampleSelection"
   exit(1)
print "SampleSelection done!"

# Random Selection
print "Executing RandomSelection..."
rsCmdLine = "otbApplicationLauncherCommandLine RandomSelection "+buildFolder+"CropMask/RandomSelection -ref "+training_polygons+" -nbtrsample "+nbtrsample+" -seed "+random_seed+" -trp "+random_training_polygons+" -tsp "+random_testing_polygons
print rsCmdLine
result = os.system(rsCmdLine)

if result != 0 :
   print "Error running RandomSelection"
   exit(1)
print "RandomSelection done!"




#Train Image Classifier
print "Executing TrainImagesClassifier..."
tcCmdLine = "otbcli_TrainImagesClassifier -io.il "+features+" -io.vd "+random_training_polygons+" -io.imstat "+statistics+" -rand "+random_seed+" -sample.bm 0 -io.confmatout "+confmatout+" -io.out "+model+" -sample.mt -1 -sample.mv -1 -sample.vfn CROP -sample.vtr "+sample_ratio+" -classifier rf -classifier.rf.nbtrees "+rfnbtrees+" -classifier.rf.min "+rfmin+" -classifier.rf.max "+rfmax 

print tcCmdLine
result = os.system(tcCmdLine)

if result != 0 :
   print "Error running TrainImagesClassifier"
   exit(1)
print "TrainImagesClassifier done!"

#Image Classifier
print "Executing ImageClassifier..."
icCmdLine = "otbcli_ImageClassifier -in "+features+" -imstat "+statistics+" -model "+model+" -out "+crop_mask
print icCmdLine
result = os.system(icCmdLine)

if result != 0 :
   print "Error running ImageClassifier"
   exit(1)
print "ImageClassifier done!"

#Validation
print "Executing ComputeConfusionMatrix..."
vdCmdLine = "otbcli_ComputeConfusionMatrix -in "+crop_mask+" -out "+confusion_matrix_validation+" -ref vector -ref.vector.in "+validation_polygons+" -ref.vector.field CROP -nodatalabel 10 > "+quality_metrics
print vdCmdLine
result = os.system(vdCmdLine)

if result != 0 :
   print "Error running ComputeConfusionMatrix"
   exit(1)
print "ComputeConfusionMatrix done!"

print "Execution successfull !"
