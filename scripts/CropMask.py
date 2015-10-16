#!/usr/bin/python

import os
import glob
import argparse
import csv
from sys import argv

def inSituDataAvailable() :
	# Temporal Features
	print "Executing TemporalFeatures..."
	tfCmdLine = "otbApplicationLauncherCommandLine TemporalFeatures "+buildFolder+"CropMask/TemporalFeatures -ndvi "+ndvi+" -dates "+outdays+" -tf "+temporal_features
	print tfCmdLine
	result = os.system(tfCmdLine)

	if result != 0 :
	   print "Error running TemporalFeatures"
	   exit(1)

	print "TemporalFeatures done!"

	# Statistic Features
	print "Executing StatisticFeatures..."
	sfCmdLine = "otbApplicationLauncherCommandLine StatisticFeatures "+buildFolder+"CropMask/StatisticFeatures -ndwi "+ndwi+" -brightness "+brightness+" -sf "+statistic_features
	print sfCmdLine
	result = os.system(sfCmdLine)

	if result != 0 :
	   print "Error running StatisticFeatures"
	   exit(1)

	print "StatisticFeatures done!"

	# _Concatenate Images
	print "Executing ConcatenateImages..."
	ciCmdLine = "otbcli_ConcatenateImages -il "+temporal_features+" "+statistic_features+" -out " + features
	print ciCmdLine
	result = os.system(ciCmdLine)

	if result != 0 :
	   print "Error running ConcatenateImages"
	   exit(1)

	#os.system("rm " + temporal_features)
	#os.system("rm " + statistic_features)

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
#	print "Executing RandomSelection..."
#	rsCmdLine = "otbApplicationLauncherCommandLine RandomSelection "+buildFolder+"CropMask/RandomSelection -ref "+training_polygons+" -nbtrsample "+nbtrsample+" -seed "+random_seed+" -trp "+random_training_polygons+" -tsp "+random_testing_polygons
#	print rsCmdLine
#	result = os.system(rsCmdLine)

#	if result != 0 :
#	   print "Error running RandomSelection"
#	   exit(1)
#	print "RandomSelection done!"

	#Train Image Classifier
	print "Executing TrainImagesClassifier..."
	tcCmdLine = "otbcli_TrainImagesClassifier -io.il "+features+" -io.vd "+training_polygons+" -io.imstat "+statistics+" -rand "+random_seed+" -sample.bm 0 -io.confmatout "+confmatout+" -io.out "+model+" -sample.mt "+nbtrsample+" -sample.mv -1 -sample.vfn CROP -sample.vtr "+sample_ratio+" -classifier rf -classifier.rf.nbtrees "+rfnbtrees+" -classifier.rf.min "+rfmin+" -classifier.rf.max "+rfmax

	print tcCmdLine
	result = os.system(tcCmdLine)

	if result != 0 :
	   print "Error running TrainImagesClassifier"
	   exit(1)
	print "TrainImagesClassifier done!"

	#Random Forest Training
#	print "Executing RandomForestTraining..."
#	tcCmdLine = "otbApplicationLauncherCommandLine RandomForestTraining "+buildFolder+"CropMask/RandomForestTraining -fvi "+features+" -shp "+training_polygons+" -seed "+random_seed+" -rfnbtrees "+rfnbtrees+" -rfmin "+rfmin+" -rfmax "+rfmax +" -nbsamples "+nbsamples+" -out "+model

#	print tcCmdLine
#	result = os.system(tcCmdLine)

#	if result != 0 :
#	   print "Error running RandomForestTraining"
#	   exit(1)
#	print "RandomForestTraining done!"

	#Image Classifier
	print "Executing ImageClassifier..."
	icCmdLine = "otbcli_ImageClassifier -in "+features+" -imstat "+statistics+" -model "+model+" -out "+raw_crop_mask
	print icCmdLine
	result = os.system(icCmdLine)

	if result != 0 :
	   print "Error running ImageClassifier"
	   exit(1)
	print "ImageClassifier done!"

def noInSituDataAvailable() :
	#Data Smoothing
	print "Executing DataSmoothing..."
	dsCmdLine = "otbApplicationLauncherCommandLine DataSmoothing "+buildFolder+"CropMask/DataSmoothing -ts "+ndvi+" -bands 1 -lambda "+lmbd+" -weight "+weight+" -sts "+ndvi_smooth

	print dsCmdLine
	result = os.system(dsCmdLine)

	if result != 0 :
	   print "Error running DataSmoothing"
	   exit(1)

	dsCmdLine = "otbApplicationLauncherCommandLine DataSmoothing "+buildFolder+"CropMask/DataSmoothing -ts "+rtocr+" -bands 4 -lambda "+lmbd+" -weight "+weight+" -sts "+rtocr_smooth

	print dsCmdLine
	result = os.system(dsCmdLine)

	if result != 0 :
	   print "Error running DataSmoothing"
	   exit(1)

	print "DataSmoothing done!"

	# Temporal Features
	print "Executing TemporalFeatures..."
	tfCmdLine = "otbApplicationLauncherCommandLine TemporalFeaturesNoInsitu "+buildFolder+"CropMask/TemporalFeaturesNoInsitu -ndvi "+ndvi_smooth+" -ts " + rtocr_smooth + " -dates "+outdays+" -tf "+tf_noinsitu
	print tfCmdLine
	result = os.system(tfCmdLine)

	if result != 0 :
	   print "Error running TemporalFeatures"
	   exit(1)

	print "TemporalFeatures done!"

	# Spectral Features
	print "Executing SpectralFeatures..."
	sfCmdLine = "otbApplicationLauncherCommandLine SpectralFeatures "+buildFolder+"CropMask/SpectralFeatures -ts " + rtocr_smooth + " -tf "+tf_noinsitu + " -sf " + spectral_features
	print sfCmdLine
	result = os.system(sfCmdLine)

	if result != 0 :
	   print "Error running SpectralFeatures"
	   exit(1)

	print "SpectralFeatures done!"

	# Image Statistics
	print "Executing ComputeImagesStatistics..."
	isCmdLine = "otbcli_ComputeImagesStatistics -il "+spectral_features+" -out "+statistics_noinsitu
	print isCmdLine
	result = os.system(isCmdLine)

	if result != 0 :
	   print "Error running ComputeImagesStatistics"
	   exit(1)
	print "ComputeImagesStatistics done!"


	# Reference Map preparation
	print "Executing erosion with GrayScaleMorphologicalOperation application..."
	rmCmdLine = "otbcli_GrayScaleMorphologicalOperation -in " + reference + " -out "+eroded_reference + " -channel 1 -filter erode -structype.ball.xradius " + erode_radius + " -structype.ball.yradius " + erode_radius
	print rmCmdLine
	result = os.system(rmCmdLine)

	if result != 0 :
	   print "Error running GrayScaleMorphologicalOperation"
	   exit(1)

	print "Erosion done!"

	# Trimming
	print "Executing Trimming application..."
	trCmdLine = "otbApplicationLauncherCommandLine Trimming "+buildFolder+"CropMask/Trimming -feat " + spectral_features + " -ref "+eroded_reference + " -out " + trimmed_reference_shape + " -alpha " + alpha + " -nbsamples 400000 -seed " + random_seed
	print trCmdLine
	result = os.system(trCmdLine)

	if result != 0 :
	   print "Error running Trimming"
	   exit(1)

	print "Trimming done!"

	# TrainImagesClassifier
	print "Executing TrainImagesClassifier..."
	tcCmdLine = "otbcli_TrainImagesClassifier -io.il "+spectral_features+" -io.vd "+trimmed_reference_shape+" -io.imstat "+statistics_noinsitu+" -rand "+random_seed+" -sample.bm 0 -io.confmatout "+confmatout+" -io.out "+model+" -sample.mt "+nbtrsample+" -sample.mv -1 -sample.vfn CROP -sample.vtr "+sample_ratio+" -classifier rf -classifier.rf.nbtrees "+rfnbtrees+" -classifier.rf.min "+rfmin+" -classifier.rf.max "+rfmax

	print tcCmdLine
	result = os.system(tcCmdLine)

	if result != 0 :
	   print "Error running TrainImagesClassifier"
	   exit(1)
	print "TrainImagesClassifier done!"

	#Image Classifier
	print "Executing ImageClassifier..."
	icCmdLine = "otbcli_ImageClassifier -in "+spectral_features+" -imstat "+statistics_noinsitu+" -model "+model+" -out "+raw_crop_mask
	print icCmdLine
	result = os.system(icCmdLine)

	if result != 0 :
	   print "Error running ImageClassifier"
	   exit(1)
	print "ImageClassifier done!"



#Path to build folder
buildFolder="../../sen2agri-build/"

parser = argparse.ArgumentParser(description='CropMask Python processor')

parser.add_argument('-refp', help='The reference polygons', required=False, metavar='reference_polygons', default='')
parser.add_argument('-ratio', help='The ratio between the validation and training polygons', required=False, metavar='sample_ratio', default=0.75)
parser.add_argument('-input', help='The list of products descriptors', required=True, metavar='product_descriptor', nargs='+')
parser.add_argument('-t0', help='The start date for the temporal resampling interval (in format YYYYMMDD)', required=True, metavar='YYYYMMDD')
parser.add_argument('-tend', help='The end date for the temporal resampling interval (in format YYYYMMDD)', required=True, metavar='YYYYMMDD')
parser.add_argument('-rate', help='The sampling rate for the temporal series, in days', required=False, metavar='sampling_rate', default=5)
parser.add_argument('-radius', help='The radius used for gapfilling, in days', required=False, metavar='radius', default=15)
parser.add_argument('-nbtrsample', help='The number of samples included in the training set.', required=False, metavar='nbtrsample', default=1000)
parser.add_argument('-rseed', help='The random seed used for training', required=False, metavar='random_seed', default=0)

parser.add_argument('-lmbd', help='The lambda parameter used in data smoothing (default 2)', required=False, metavar='lmbd', default=2)
parser.add_argument('-weight', help='The weight factor for data smoothing (default 1)', required=False, metavar='weight', default=1)
parser.add_argument('-nbcomp', help='The number of components used by dimensionality reduction (default 6)', required=False, metavar='nbcomp', default=6)
parser.add_argument('-spatialr', help='The spatial radius of the neighborhood used for segmentation (default 10)', required=False, metavar='spatialr', default=10)
parser.add_argument('-ranger', help='The range radius defining the radius (expressed in radiometry unit) in the multispectral space (default 0.65)', required=False, metavar='ranger', default=0.65)
parser.add_argument('-minsize', help='Minimum size of a region (in pixel unit) in segmentation. (default 10)', required=False, metavar='minsize', default=10)

parser.add_argument('-refr', help='The reference raster when insitu data is not available', required=False, metavar='reference', default='')
parser.add_argument('-eroderad', help='The radius used for erosion (default 1)', required=False, metavar='erode_radius', default='1')
parser.add_argument('-alpha', help='The parameter alpha used by the mahalanobis function (default 0.01)', required=False, metavar='alpha', default='0.01')

parser.add_argument('-rfnbtrees', help='The number of trees used for training (default 100)', required=False, metavar='rfnbtrees', default=100)
parser.add_argument('-rfmax', help='maximum depth of the trees used for Random Forest classifier (default 25)', required=False, metavar='rfmax', default=25)
parser.add_argument('-rfmin', help='minimum number of samples in each node used by the classifier (default 5)', required=False, metavar='rfmin', default=5)

parser.add_argument('-pixsize', help='The size, in meters, of a pixel (default 10)', required=False, metavar='pixsize', default=10)
parser.add_argument('-outdir', help="Output directory", default=buildFolder)

args = parser.parse_args()

reference_polygons=args.refp
sample_ratio=str(args.ratio)

indesc = " "
for desc in args.input:
   indesc = indesc + desc + " "

t0=args.t0
tend=args.tend
sp=str(args.rate)
radius=str(args.radius)
random_seed=str(args.rseed)
nbtrsample=str(args.nbtrsample)
rfnbtrees=str(args.rfnbtrees)
rfmax=str(args.rfmax)
rfmin=str(args.rfmin)
lmbd=str(args.lmbd)
weight=str(args.weight)
nbcomp=str(args.nbcomp)
spatialr=str(args.spatialr)
ranger=str(args.ranger)
minsize=str(args.minsize)
pixsize=str(args.pixsize)

reference=args.refr
erode_radius=str(args.eroderad)
alpha=str(args.alpha)

reference_polygons_clip=os.path.join(args.outdir, "reference_clip.shp")
training_polygons=os.path.join(args.outdir, "training_polygons.shp")
validation_polygons=os.path.join(args.outdir, "validation_polygons.shp")
random_training_polygons=os.path.join(args.outdir, "random_training_polygons.shp")
random_testing_polygons=os.path.join(args.outdir, "random_testing_polygons.shp")

rawtocr=os.path.join(args.outdir, "rawtocr.tif")
tocr=os.path.join(args.outdir, "tocr.tif")
rawmask=os.path.join(args.outdir, "rawmask.tif")
mask=os.path.join(args.outdir, "mask.tif")
dates=os.path.join(args.outdir, "dates.txt")
outdays=os.path.join(args.outdir, "days.txt")
shape=os.path.join(args.outdir, "shape.shp")
rtocr=os.path.join(args.outdir, "rtocr.tif")
ndvi=os.path.join(args.outdir, "ndvi.tif")
ndwi=os.path.join(args.outdir, "ndwi.tif")
brightness=os.path.join(args.outdir, "brightness.tif")
temporal_features=os.path.join(args.outdir, "tf.tif")
statistic_features=os.path.join(args.outdir, "sf.tif")
features=os.path.join(args.outdir, "concat_features.tif")
statistics=os.path.join(args.outdir, "statistics.xml")

ndvi_smooth=os.path.join(args.outdir, "ndvi_smooth.tif")
rtocr_smooth=os.path.join(args.outdir, "rtocr_smooth.tif")
tf_noinsitu=os.path.join(args.outdir, "tf_noinsitu.tif")
spectral_features=os.path.join(args.outdir, "spectral_features.tif")

eroded_reference=os.path.join(args.outdir, "eroded_reference.tif")
trimmed_reference_shape=os.path.join(args.outdir, "trimmed_reference_shape.shp")
statistics_noinsitu=os.path.join(args.outdir, "statistics_noinsitu.xml")

tmpfolder="."

pca=os.path.join(args.outdir, "pca.tif")
mean_shift_smoothing=os.path.join(args.outdir, "mean_shift_smoothing.tif")
segmented=os.path.join(args.outdir, "segmented.tif")
segmented_merged=os.path.join(args.outdir, "segmented_merged.tif")

confmatout=os.path.join(args.outdir, "confusion-matrix.csv")
model=os.path.join(args.outdir, "crop-mask-model.txt")
raw_crop_mask=os.path.join(args.outdir, "raw_crop_mask.tif")
crop_mask=os.path.join(args.outdir, "crop_mask.tif")
confusion_matrix_validation=os.path.join(args.outdir, "crop-mask-confusion-matrix-validation.csv")
quality_metrics=os.path.join(args.outdir, "crop-mask-quality-metrics.txt")

# Bands Extractor
print "Executing BandsExtractor..."
beCmdLine = "otbApplicationLauncherCommandLine BandsExtractor "+buildFolder+"CropType/BandsExtractor -il "+indesc+" -out "+rawtocr+" -mask "+rawmask+" -outdate "+dates+" -shape "+shape+" -pixsize "+pixsize
print beCmdLine
result = os.system(beCmdLine)

if result != 0 :
   print "Error running BandsExtractor"
   exit(1)
print "BandsExtractor done!"

# gdalwarp
print "Executing gdalwarp..."

gwCmdLine = "gdalwarp -multi -wm 2048 -dstnodata \"-10000\" -overwrite -cutline "+shape+" -crop_to_cutline "+rawtocr+" "+tocr
print gwCmdLine
result = os.system(gwCmdLine)

if result != 0 :
   print "Error running gdalwarp"
   exit(1)

#os.system("rm " + rawtocr)

gwCmdLine = "gdalwarp -multi -wm 2048 -dstnodata 1 -overwrite -cutline "+shape+" -crop_to_cutline "+rawmask+" "+mask
print gwCmdLine
result = os.system(gwCmdLine)

if result != 0 :
   print "Error running gdalwarp"
   exit(1)

#os.system("rm " + rawmask)

print "gdalwarp done!"

# Temporal Resampling
print "Executing TemporalResampling..."
trCmdLine = "otbApplicationLauncherCommandLine TemporalResampling "+buildFolder+"CropType/TemporalResampling -tocr "+tocr+" -mask "+mask+" -ind "+dates+" -sp "+sp+" -t0 "+t0+" -tend "+tend+" -radius "+radius+" -rtocr "+rtocr+" -outdays "+outdays
print trCmdLine
result = os.system(trCmdLine)

if result != 0 :
   print "Error running TemporalResampling"
   exit(1)

#os.system("rm " + tocr)
#os.system("rm " + mask)

print "TemporalResampling done!"


# Feature Extraction
print "Executing FeatureExtraction..."
feCmdLine = "otbApplicationLauncherCommandLine FeatureExtraction "+buildFolder+"CropType/FeatureExtraction -rtocr "+rtocr+" -ndvi "+ndvi+" -ndwi "+ndwi+" -brightness "+brightness
print feCmdLine
result = os.system(feCmdLine)

if result != 0 :
   print "Error running FeatureExtraction"
   exit(1)

print "FeatureExtraction done!"

if reference_polygons != "" :
	inSituDataAvailable()
else:
	noInSituDataAvailable()

#Validation
print "Executing ComputeConfusionMatrix..."
vdCmdLine = "otbcli_ComputeConfusionMatrix -in "+raw_crop_mask+" -out "+confusion_matrix_validation+" -ref vector -ref.vector.in "+validation_polygons+" -ref.vector.field CROP -nodatalabel 10 > "+quality_metrics
print vdCmdLine
result = os.system(vdCmdLine)

if result != 0 :
   print "Error running ComputeConfusionMatrix"
   exit(1)
print "ComputeConfusionMatrix done!"

#Dimension reduction
print "Executing DimensionalityReduction..."
drCmdLine = "otbcli_DimensionalityReduction -in "+ndvi+" -method pca -nbcomp "+nbcomp+" -out "+pca
print drCmdLine
result = os.system(drCmdLine)

if result != 0 :
   print "Error running DimensionalityReduction"
   exit(1)
print "DimensionalityReduction done!"

#Mean-Shift segmentation
print "Executing MeanShiftSmoothing..."
msCmdLine = "otbcli_MeanShiftSmoothing -in "+pca+" -modesearch 0 -spatialr "+spatialr+" -ranger "+ranger+" -fout "+mean_shift_smoothing +" uint32"
print msCmdLine
result = os.system(msCmdLine)

if result != 0 :
   print "Error running MeanShiftSmoothing"
   exit(1)
print "MeanShiftSmoothing done!"

print "Executing LSMSSegmentation..."
msCmdLine = "otbcli_LSMSSegmentation -in "+mean_shift_smoothing+" -spatialr "+spatialr+" -ranger "+ranger+" -minsize "+minsize+" -tmpdir " +tmpfolder+ " -out "+segmented+" uint32"
print msCmdLine
result = os.system(msCmdLine)

if result != 0 :
   print "Error running LSMSSegmentation"
   exit(1)
print "LSMSSegmentation done!"

print "Executing LSMSSmallRegionsMerging..."
msCmdLine = "otbcli_LSMSSmallRegionsMerging -in "+pca+" -inseg "+segmented+" -minsize "+minsize+" -out "+segmented_merged+" uint32"
print msCmdLine
result = os.system(msCmdLine)

if result != 0 :
   print "Error running LSMSSmallRegionsMerging"
   exit(1)
print "LSMSSmallRegionsMerging done!"

#Majority voting
print "Executing MajorityVoting..."
drCmdLine = "otbApplicationLauncherCommandLine MajorityVoting "+buildFolder+"CropMask/MajorityVoting  -inclass "+raw_crop_mask+" -inseg "+segmented_merged+" -rout "+crop_mask
print drCmdLine
result = os.system(drCmdLine)

if result != 0 :
   print "Error running MajorityVoting"
   exit(1)
print "MajorityVoting done!"

print "Execution successfull !"


