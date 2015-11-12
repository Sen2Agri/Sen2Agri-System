#!/usr/bin/python

import os
import glob
import argparse
import csv
from sys import argv
import datetime
from common import executeStep  

def inSituDataAvailable() :
	# Temporal Features (Step 6)
	#executeStep("TemporalFeatures", "otbApplicationLauncherCommandLine", "TemporalFeatures", os.path.join(buildFolder,"CropMask/TemporalFeatures"),"-ndvi",ndvi,"-dates",outdays,"-window", window, "-tf",temporal_features, skip=fromstep>6)

	# Statistic Features (Step 7)
	#executeStep("StatisticFeatures", "otbApplicationLauncherCommandLine", "StatisticFeatures", os.path.join(buildFolder,"CropMask/StatisticFeatures"),"-ndwi",ndwi,"-brightness",brightness,"-sf",statistic_features, skip=fromstep>7, rmfiles=[] if keepfiles else [ndwi, brightness])

	# Concatenate Features (Step 8)
	#executeStep("ConcatenateFeatures", "otbcli_ConcatenateImages", "-il", temporal_features,statistic_features,"-out",features, skip=fromstep>8, rmfiles=[] if keepfiles else [temporal_features, statistic_features])

	#Features when insitu data is available (Step 6 or 7 or 8)
	executeStep("FeaturesWithInsitu", "otbApplicationLauncherCommandLine", "FeaturesWithInsitu", os.path.join(buildFolder,"CropMask/FeaturesWithInsitu"),"-ndvi",ndvi,"-ndwi",ndwi,"-brightness",brightness,"-dates",outdays,"-window", window, "-out",features, skip=fromstep>6, rmfiles=[] if keepfiles else [ndwi, brightness])

	# Image Statistics (Step 9)
	executeStep("ComputeImagesStatistics", "otbcli_ComputeImagesStatistics", "-il", features,"-out",statistics, skip=fromstep>9)

	# ogr2ogr Reproject insitu data (Step 10)
	executeStep("ogr2ogr", "/usr/local/bin/ogr2ogr", "-t_srs", shape_proj,"-progress","-overwrite",reference_polygons_reproject, reference_polygons, skip=fromstep>10)

	# ogr2ogr Crop insitu data (Step 11)
	executeStep("ogr2ogr", "/usr/local/bin/ogr2ogr", "-clipsrc", shape,"-progress","-overwrite",reference_polygons_clip, reference_polygons_reproject, skip=fromstep>11)

	# Sample Selection (Step 12)
	executeStep("SampleSelection", "otbApplicationLauncherCommandLine","SampleSelection", os.path.join(buildFolder,"CropType/SampleSelection"), "-ref",reference_polygons_clip,"-ratio", sample_ratio, "-seed", random_seed, "-tp", training_polygons, "-vp", validation_polygons,"-nofilter","true", skip=fromstep>12)

	#Train Image Classifier (Step 13)
	executeStep("TrainImagesClassifier", "otbcli_TrainImagesClassifier", "-io.il", features,"-io.vd",training_polygons,"-io.imstat", statistics, "-rand", random_seed, "-sample.bm", "0", "-io.confmatout", confmatout,"-io.out",model,"-sample.mt", nbtrsample,"-sample.mv","-1","-sample.vfn","CROP","-sample.vtr",sample_ratio,"-classifier","rf", "-classifier.rf.nbtrees",rfnbtrees,"-classifier.rf.min",rfmin,"-classifier.rf.max",rfmax, skip=fromstep>13)


	#Image Classifier (Step 14)
	executeStep("ImageClassifier", "otbcli_ImageClassifier", "-in", features,"-imstat",statistics,"-model", model, "-out", raw_crop_mask, skip=fromstep>14, rmfiles=[] if keepfiles else [features])

	return;
#end inSituDataAvailable

def noInSituDataAvailable() :
	global validation_polygons
	#Data Smoothing for NDVI (Step 15)
	executeStep("DataSmoothing for NDVI", "otbApplicationLauncherCommandLine", "DataSmoothing", os.path.join(buildFolder,"CropMask/DataSmoothing"),"-ts",ndvi,"-bands", "1", "-lambda", lmbd, "-weight", weight, "-sts",ndvi_smooth, skip=fromstep>15)
	
	#Data Smoothing for Reflectances (Step 16)
	executeStep("DataSmoothing for Reflectances", "otbApplicationLauncherCommandLine", "DataSmoothing", os.path.join(buildFolder,"CropMask/DataSmoothing"),"-ts",rtocr,"-bands", "4", "-lambda", lmbd, "-weight", weight, "-sts",rtocr_smooth, skip=fromstep>16, rmfiles=[] if keepfiles else [rtocr])

	# Temporal Features (Step 16)
	#executeStep("TemporalFeatures", "otbApplicationLauncherCommandLine", "TemporalFeaturesNoInsitu", os.path.join(buildFolder,"CropMask/TemporalFeaturesNoInsitu"),"-ndvi",ndvi_smooth,"-ts", rtocr_smooth, "-dates", outdays, "-tf", tf_noinsitu, skip=fromstep>16)

	# Spectral Features (Step 17)
	#executeStep("SpectralFeatures", "otbApplicationLauncherCommandLine", "SpectralFeatures", os.path.join(buildFolder,"CropMask/SpectralFeatures"),"-ts", rtocr_smooth, "-tf", tf_noinsitu, "-sf", spectral_features, skip=fromstep>17, rmfiles=[] if keepfiles else [ndvi_smooth, rtocr_smooth, tf_noinsitu])

	#Features when no insitu data is available (Step 17)
	executeStep("FeaturesWithoutInsitu", "otbApplicationLauncherCommandLine", "FeaturesWithoutInsitu", os.path.join(buildFolder,"CropMask/FeaturesWithoutInsitu"),"-ndvi",ndvi_smooth,"-ts", rtocr_smooth, "-dates", outdays , "-sf", spectral_features, skip=fromstep>17, rmfiles=[] if keepfiles else [ndvi_smooth, rtocr_smooth])

	# Image Statistics (Step 18)
	executeStep("ComputeImagesStatistics", "otbcli_ComputeImagesStatistics", "-il", spectral_features, "-out", statistics_noinsitu, skip=fromstep>18)

	# Reference Map preparation (Step 19)
	executeStep("gdalwarp for Reference map", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "0", "-overwrite", "-tr", pixsize, pixsize, "-cutline", shape, "-crop_to_cutline", reference, crop_reference, skip=fromstep>19)
	
	# Erosion (Step 20)
	executeStep("Erosion", "otbApplicationLauncherCommandLine", "Erosion", os.path.join(buildFolder,"CropMask/Erosion"),"-in", crop_reference, "-out", eroded_reference, "-radius", erode_radius, skip=fromstep>20, rmfiles=[] if keepfiles else [crop_reference])


	# Trimming (Step 21)
	executeStep("Trimming", "otbApplicationLauncherCommandLine", "Trimming", os.path.join(buildFolder,"CropMask/Trimming"),"-feat", spectral_features, "-ref", eroded_reference, "-out", trimmed_reference_shape, "-alpha", alpha, "-nbsamples", "0", "-seed", random_seed, skip=fromstep>21, rmfiles=[] if keepfiles else [eroded_reference])
	
	#Train Image Classifier (Step 22)
	executeStep("TrainImagesClassifier", "otbcli_TrainImagesClassifier", "-io.il", spectral_features,"-io.vd",trimmed_reference_shape,"-io.imstat", statistics_noinsitu, "-rand", random_seed, "-sample.bm", "0", "-io.confmatout", confmatout,"-io.out",model,"-sample.mt", nbtrsample,"-sample.mv","-1","-sample.vfn","CROP","-sample.vtr",sample_ratio,"-classifier","rf", "-classifier.rf.nbtrees",rfnbtrees,"-classifier.rf.min",rfmin,"-classifier.rf.max",rfmax, skip=fromstep>22)

	#Image Classifier (Step 23)
	executeStep("ImageClassifier", "otbcli_ImageClassifier", "-in", spectral_features,"-imstat",statistics_noinsitu,"-model", model, "-out", raw_crop_mask, skip=fromstep>23, rmfiles=[] if keepfiles else [spectral_features])

	#use the shape built from the reference image for validation
	validation_polygons = trimmed_reference_shape
	return
#end noInSituDataAvailable

#Path to build folder
defaultBuildFolder="~/sen2agri-build/"

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

parser.add_argument('-window', help='The window, expressed in number of records, used for the temporal features extraction', required=False, metavar='window', default=6)
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

parser.add_argument('-minarea', help="The minium number of pixel in an area where, for an equal number of crop and nocrop samples, the crop decision is taken (default 20)", required=False, metavar='minarea', default=20)

parser.add_argument('-pixsize', help='The size, in meters, of a pixel (default 10)', required=False, metavar='pixsize', default=10)
parser.add_argument('-outdir', help="Output directory", default=defaultBuildFolder)
parser.add_argument('-buildfolder', help="Build folder", default=defaultBuildFolder)
parser.add_argument('-targetfolder', help="The folder where the target product is built", default="")

parser.add_argument('-keepfiles', help="Keep all intermediate files (default false)", default=False, action='store_true')
parser.add_argument('-fromstep', help="Run from the selected step (default 1)", type=int, default=1)

args = parser.parse_args()

reference_polygons=args.refp
sample_ratio=str(args.ratio)

indesc = args.input

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
minarea=str(args.minarea)
window=str(args.window)


buildFolder=args.buildfolder
targetFolder=args.targetfolder if args.targetfolder != "" else args.outdir
reference=args.refr
erode_radius=str(args.eroderad)
alpha=str(args.alpha)

reference_polygons_reproject=os.path.join(args.outdir, "reference_polygons_reproject.shp")
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
shape_proj=os.path.join(args.outdir, "shape.prj")
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
triming_features=os.path.join(args.outdir, "triming_features.tif")

eroded_reference=os.path.join(args.outdir, "eroded_reference.tif")
crop_reference=os.path.join(args.outdir, "crop_reference.tif")
trimmed_reference_shape=os.path.join(args.outdir, "trimmed_reference_shape.shp")
statistics_noinsitu=os.path.join(args.outdir, "statistics_noinsitu.xml")

tmpfolder=args.outdir

pca=os.path.join(args.outdir, "pca.tif")
mean_shift_smoothing=os.path.join(args.outdir, "mean_shift_smoothing.tif")
mean_shift_smoothing_spatial=os.path.join(args.outdir, "mean_shift_smoothing_spatial.tif")
segmented=os.path.join(args.outdir, "segmented.tif")
segmented_merged=os.path.join(args.outdir, "segmented_merged.tif")

confmatout=os.path.join(args.outdir, "confusion-matrix.csv")
model=os.path.join(args.outdir, "crop-mask-model.txt")
raw_crop_mask=os.path.join(args.outdir, "raw_crop_mask.tif")
raw_crop_mask_confusion_matrix_validation=os.path.join(args.outdir, "raw-crop-mask-confusion-matrix-validation.csv")
raw_crop_mask_quality_metrics=os.path.join(args.outdir, "raw-crop-mask-quality-metrics.txt")

crop_mask_uncut=os.path.join(args.outdir, "crop_mask_uncut.tif")
crop_mask_uncompressed=os.path.join(args.outdir, "crop_mask_uncomp.tif")
crop_mask=os.path.join(args.outdir, "crop_mask.tif")


confusion_matrix_validation=os.path.join(args.outdir, "crop-mask-confusion-matrix-validation.csv")
quality_metrics=os.path.join(args.outdir, "crop-mask-quality-metrics.txt")
xml_validation_metrics=os.path.join(args.outdir, "crop-mask-validation-metrics.xml")

keepfiles = args.keepfiles
fromstep = args.fromstep

globalStart = datetime.datetime.now()

# Bands Extractor (Step 1)
executeStep("BandsExtractor", "otbApplicationLauncherCommandLine", "BandsExtractor", os.path.join(buildFolder,"CropType/BandsExtractor"),"-out",rawtocr,"-mask",rawmask,"-outdate", dates, "-shape", shape, "-pixsize", pixsize,"-merge", "true", "-il", *indesc, skip=fromstep>1)

# gdalwarp (Step 2 and 3)
executeStep("gdalwarp for reflectances", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", rawtocr, tocr, skip=fromstep>2, rmfiles=[] if keepfiles else [rawtocr])

executeStep("gdalwarp for masks", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", rawmask, mask, skip=fromstep>3, rmfiles=[] if keepfiles else [rawmask])

# Temporal Resampling (Step 4)
executeStep("TemporalResampling", "otbApplicationLauncherCommandLine", "TemporalResampling", os.path.join(buildFolder,"CropType/TemporalResampling"), "-tocr", tocr, "-mask", mask, "-ind", dates, "-sp", sp, "-t0", t0, "-tend", tend, "-radius", radius, "-rtocr", rtocr, "-outdays", outdays, skip=fromstep>4, rmfiles=[] if keepfiles else [tocr, mask])

# Feature Extraction (Step 5)
if reference_polygons != "" :
	executeStep("FeatureExtraction", "otbApplicationLauncherCommandLine", "FeatureExtraction", os.path.join(buildFolder,"CropType/FeatureExtraction"), "-rtocr", rtocr, "-ndvi", ndvi, "-ndwi", ndwi, "-brightness", brightness, skip=fromstep>5, rmfiles=[] if keepfiles else [rtocr])
else:
	executeStep("FeatureExtraction", "otbApplicationLauncherCommandLine", "FeatureExtraction", os.path.join(buildFolder,"CropType/FeatureExtraction"), "-rtocr", rtocr, "-ndvi", ndvi, skip=fromstep>5)

# Select either inSitu or noInSitu branches
if reference_polygons != "" :
	inSituDataAvailable()
else:
	noInSituDataAvailable()

#Validation (Step 24)
executeStep("Validation for Raw Cropmask", "otbcli_ComputeConfusionMatrix", "-in", raw_crop_mask, "-out", raw_crop_mask_confusion_matrix_validation, "-ref", "vector", "-ref.vector.in", validation_polygons, "-ref.vector.field", "CROP", "-nodatalabel", "-10000", outf=raw_crop_mask_quality_metrics, skip=fromstep>24)

#Dimension reduction (Step 25)
executeStep("PrincipalComponentAnalysis", "otbApplicationLauncherCommandLine", "PrincipalComponentAnalysis",os.path.join(buildFolder,"CropMask/PrincipalComponentAnalysis") ,"-ndvi", ndvi, "-nc", nbcomp, "-out", pca, skip=fromstep>25, rmfiles=[] if keepfiles else [ndvi])

#Mean-Shift segmentation (Step 26, 27 and 28)
executeStep("MeanShiftSmoothing", "otbcli_MeanShiftSmoothing", "-in", pca,"-modesearch","0", "-spatialr", spatialr, "-ranger", ranger, "-foutpos", mean_shift_smoothing_spatial, "-fout", mean_shift_smoothing, "uint32", skip=fromstep>26, rmfiles=[] if keepfiles else [pca])

executeStep("LSMSSegmentation", "otbcli_LSMSSegmentation", "-in", mean_shift_smoothing,"-inpos", mean_shift_smoothing_spatial, "-spatialr", spatialr, "-ranger", ranger, "-minsize", "0", "-tmpdir", tmpfolder, "-out", segmented, "uint32", skip=fromstep>27, rmfiles=[] if keepfiles else [mean_shift_smoothing_spatial])

executeStep("LSMSSmallRegionsMerging", "otbcli_LSMSSmallRegionsMerging", "-in", mean_shift_smoothing,"-inseg", segmented, "-minsize", minsize, "-out", segmented_merged, "uint32", skip=fromstep>28, rmfiles=[] if keepfiles else [mean_shift_smoothing, segmented])

#Majority voting (Step 29)
executeStep("MajorityVoting", "otbApplicationLauncherCommandLine", "MajorityVoting",os.path.join(buildFolder,"CropMask/MajorityVoting") ,"-nodatasegvalue", "0", "-nodataclassifvalue", "-10000", "-minarea", minarea, "-inclass", raw_crop_mask, "-inseg", segmented_merged, "-rout", crop_mask_uncut, skip=fromstep>29, rmfiles=[] if keepfiles else [segmented_merged])

# gdalwarp (Step 30)
executeStep("gdalwarp for crop mask", "/usr/local/bin/gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", crop_mask_uncut, crop_mask_uncompressed, skip=fromstep>30)

#Validation (Step 31)
executeStep("Validation for Cropmask", "otbcli_ComputeConfusionMatrix", "-in", crop_mask_uncompressed, "-out", confusion_matrix_validation, "-ref", "vector", "-ref.vector.in", validation_polygons, "-ref.vector.field", "CROP", "-nodatalabel", "-10000", outf=quality_metrics, skip=fromstep>31)

#Compression (Step 32)
executeStep("Compression", "otbcli_Convert", "-in", crop_mask_uncompressed, "-out", crop_mask+"?gdal:co:COMPRESS=DEFLATE", "int16",  skip=fromstep>32, rmfiles=[] if keepfiles else [crop_mask_uncompressed])

#XML conversion (Step 33)
executeStep("XML Conversion for Crop Mask", "otbApplicationLauncherCommandLine", "XMLStatistics", os.path.join(buildFolder,"Common/XMLStatistics"), "-confmat", confusion_matrix_validation, "-quality", quality_metrics, "-root", "CropMask", "-out", xml_validation_metrics,  skip=fromstep>33)

#Product creation (Step 34)
executeStep("ProductFormatter", "otbApplicationLauncherCommandLine", "ProductFormatter", os.path.join(buildFolder,"MACCSMetadata/src"), "-destroot", targetFolder, "-fileclass", "SVT1", "-level", "L4A", "-timeperiod", t0+"_"+tend, "-baseline", "-01.00", "-processor", "cropmask", "-processor.cropmask.file", crop_mask, skip=fromstep>34)

globalEnd = datetime.datetime.now()

print "Processor CropMask finished in " + str(globalEnd-globalStart)




