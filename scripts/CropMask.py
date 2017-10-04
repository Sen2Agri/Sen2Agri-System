#!/usr/bin/python

import os
import glob
import argparse
import csv
import sys
from sys import argv
import datetime
from sen2agri_common import executeStep


def inSituDataAvailable() :
    #Features when insitu data is available (Step 6 or 7 or 8)
    executeStep("FeaturesWithInsitu", "otbcli", "FeaturesWithInsitu", buildFolder,"-ndvi",ndvi,"-ndwi",ndwi,"-brightness",brightness,"-dates",outdays,"-window", window,"-bm", "true", "-out",features, skip=fromstep>6, rmfiles=[] if keepfiles else [ndwi, brightness])

    # Image Statistics (Step 9)
    executeStep("ComputeImagesStatistics", "otbcli_ComputeImagesStatistics", "-il", features,"-out",statistics, skip=fromstep>9)

    # ogr2ogr Reproject insitu data (Step 10)
    with open(shape_proj, 'r') as file:
        shape_wkt = "ESRI::" + file.read()

    executeStep("ogr2ogr", "ogr2ogr", "-t_srs", shape_wkt,"-progress","-overwrite",reference_polygons_reproject, reference_polygons, skip=fromstep>10)

    # ogr2ogr Crop insitu data (Step 11)
    executeStep("ogr2ogr", "ogr2ogr", "-clipsrc", shape,"-progress","-overwrite",reference_polygons_clip, reference_polygons_reproject, skip=fromstep>11)

    # Sample Selection (Step 12)
    executeStep("SampleSelection", "otbcli","SampleSelection", buildFolder, "-ref",reference_polygons_clip,"-ratio", sample_ratio, "-seed", random_seed, "-tp", training_polygons, "-vp", validation_polygons,"-nofilter","true", skip=fromstep>12)

    #Train Image Classifier (Step 13)
    executeStep("TrainImagesClassifier", "otbcli_TrainImagesClassifier", "-io.il",
                features,"-io.vd",training_polygons,"-io.imstat", statistics, "-rand", random_seed,
                "-sample.bm", "0", "-sample.vtr", "0.5", "-io.confmatout",
                confmatout,"-io.out",model,"-sample.mt", "4000","-sample.mv","-1","-sample.vfn","CROP","-classifier","rf", "-classifier.rf.nbtrees",rfnbtrees,"-classifier.rf.min",rfmin,"-classifier.rf.max",rfmax, skip=fromstep>13)
    #Image Classifier (Step 14)
    executeStep("ImageClassifier", "otbcli_ImageClassifier", "-in", features,"-imstat",statistics,"-model", model, "-out", raw_crop_mask_uncompressed, skip=fromstep>14, rmfiles=[] if keepfiles else [features])

    return;
#end inSituDataAvailable

def noInSituDataAvailable() :
    global validation_polygons

    #Data Smoothing for Reflectances (Step 16)
    executeStep("DataSmoothing for Reflectances", "otbcli", "DataSmoothing", buildFolder,"-ts", tocr, "-mask", mask, "-dates", dates, "-lambda", lmbd, "-sts",rtocr_smooth, "-outdays", outdays_smooth, skip=fromstep>16, rmfiles=[] if keepfiles else [rtocr])

    #Features when no insitu data is available (Step 17)
    executeStep("FeaturesWithoutInsitu", "otbcli", "FeaturesWithoutInsitu", buildFolder, "-ts", rtocr_smooth, "-dates", outdays_smooth , "-sf", spectral_features, skip=fromstep>17, rmfiles=[] if keepfiles else [rtocr_smooth])

    # Image Statistics (Step 18)
    executeStep("ComputeImagesStatistics", "otbcli_ComputeImagesStatistics", "-il", spectral_features, "-out", statistics_noinsitu, skip=fromstep>18)

    # Reference Map preparation (Step 19 and 20)
    with open(shape_proj, 'r') as file:
        shape_wkt = "ESRI::" + file.read()

    executeStep("gdalwarp for cropping Reference map", "gdalwarp", "-dstnodata", "0", "-overwrite", "-cutline", shape, "-crop_to_cutline", reference, cropped_reference, skip=fromstep>19)
    executeStep("gdalwarp for reprojecting Reference map", "gdalwarp", "-dstnodata", "0", "-overwrite", "-t_srs", shape_wkt, cropped_reference, reprojected_reference, skip=fromstep>19, rmfiles=[] if keepfiles else [cropped_reference])
    executeStep("gdalwarp for resampling Reference map", "gdalwarp", "-dstnodata", "0", "-overwrite", "-tr", pixsize, pixsize, "-cutline", shape, "-crop_to_cutline", reprojected_reference, crop_reference, skip=fromstep>20, rmfiles=[] if keepfiles else [reprojected_reference])

    # Erosion (Step 21)
    executeStep("Erosion", "otbcli", "Erosion", buildFolder,"-in", crop_reference, "-out", eroded_reference, "-radius", erode_radius, skip=fromstep>21, rmfiles=[] if keepfiles else [crop_reference])


    # Trimming (Step 22)
    executeStep("Trimming", "otbcli", "Trimming", buildFolder,"-feat", spectral_features, "-ref", eroded_reference, "-out", trimmed_reference_raster, "-alpha", alpha, "-nbsamples", "0", "-seed", random_seed, skip=fromstep>22, rmfiles=[] if keepfiles else [eroded_reference])

    #Train Image Classifier (Step 23)
    executeStep("TrainImagesClassifierNew", "otbcli", "TrainImagesClassifierNew", buildFolder, "-io.il", spectral_features,"-io.rs",trimmed_reference_raster,"-nodatalabel", "-10000", "-io.imstat", statistics_noinsitu, "-rand", random_seed, "-sample.bm", "0", "-io.confmatout", confmatout,"-io.out",model,"-sample.mt", nbtrsample,"-sample.mv","4000","-sample.vtr",sample_ratio,"-classifier","rf", "-classifier.rf.nbtrees",rfnbtrees,"-classifier.rf.min",rfmin,"-classifier.rf.max",rfmax, skip=fromstep>23)

    #Image Classifier (Step 24)
    executeStep("ImageClassifier", "otbcli_ImageClassifier", "-in", spectral_features,"-imstat",statistics_noinsitu,"-model", model, "-out", raw_crop_mask_uncompressed, skip=fromstep>24, rmfiles=[] if keepfiles else [spectral_features])

    return
#end noInSituDataAvailable


parser = argparse.ArgumentParser(description='CropMask Python processor')

parser.add_argument('-mission', help='The main mission for the time series', required=False, default='SPOT')

parser.add_argument('-refp', help='The reference polygons', required=False, metavar='reference_polygons', default='')
parser.add_argument('-ratio', help='The ratio between the validation and training polygons (default 0.75)', required=False, metavar='sample_ratio', default=0.75)
parser.add_argument('-input', help='The list of products descriptors', required=True, metavar='product_descriptor', nargs='+')
parser.add_argument('-trm', help='The temporal resampling mode (default gapfill)', choices=['resample', 'gapfill'], required=False, default='gapfill')
# parser.add_argument('-radius', help='The radius used for gapfilling, in days (default 15)', required=False, metavar='radius', default=15)
parser.add_argument('-nbtrsample', help='The number of samples included in the training set (default 4000)', required=False, metavar='nbtrsample', default=4000)
parser.add_argument('-rseed', help='The random seed used for training (default 0)', required=False, metavar='random_seed', default=0)

parser.add_argument('-window', help='The window, expressed in number of records, used for the temporal features extraction (default 6)', required=False, metavar='window', default=6)
parser.add_argument('-lmbd', help='The lambda parameter used in data smoothing (default 2)', required=False, metavar='lmbd', default=2)
parser.add_argument('-nbcomp', help='The number of components used by dimensionality reduction (default 6)', required=False, metavar='nbcomp', default=6)
parser.add_argument('-spatialr', help='The spatial radius of the neighborhood used for segmentation (default 10)', required=False, metavar='spatialr', default=10)
parser.add_argument('-ranger', help='The range radius defining the radius (expressed in radiometry unit) in the multispectral space (default 0.65)', required=False, metavar='ranger', default=0.65)
parser.add_argument('-minsize', help='Minimum size of a region (in pixel unit) for segmentation.(default 10)', required=False, metavar='minsize', default=10)

parser.add_argument('-refr', help='The reference raster when insitu data is not available', required=False, metavar='reference', default='')
parser.add_argument('-eroderad', help='The radius used for erosion (default 1)', required=False, metavar='erode_radius', default='1')
parser.add_argument('-alpha', help='The parameter alpha used by the mahalanobis function (default 0.01)', required=False, metavar='alpha', default='0.01')

parser.add_argument('-rfnbtrees', help='The number of trees used for training (default 100)', required=False, metavar='rfnbtrees', default=100)
parser.add_argument('-rfmax', help='maximum depth of the trees used for Random Forest classifier (default 25)', required=False, metavar='rfmax', default=25)
parser.add_argument('-rfmin', help='minimum number of samples in each node used by the classifier (default 25)', required=False, metavar='rfmin', default=25)

parser.add_argument('-minarea', help="The minium number of pixel in an area where, for an equal number of crop and nocrop samples, the crop decision is taken (default 20)", required=False, metavar='minarea', default=20)

parser.add_argument('-pixsize', help='The size, in meters, of a pixel (default 10)', required=False, metavar='pixsize', default=10)
parser.add_argument('-tilename', help="The name of the tile", default="T0000")
parser.add_argument('-outdir', help="Output directory", default=".")
parser.add_argument('-buildfolder', help="Build folder", default="")
parser.add_argument('-targetfolder', help="The folder where the target product is built", default="")

parser.add_argument('-keepfiles', help="Keep all intermediate files (default false)", default=False, action='store_true')
parser.add_argument('-fromstep', help="Run from the selected step (default 1)", type=int, default=1)
parser.add_argument('-siteid', help='The site ID', required=False)

args = parser.parse_args()

mission=args.mission

reference_polygons=args.refp
sample_ratio=str(args.ratio)

indesc = args.input

trm=args.trm
# radius=str(args.radius)
radius="15"  # not used
random_seed=str(args.rseed)
nbtrsample=str(args.nbtrsample)
rfnbtrees=str(args.rfnbtrees)
rfmax=str(args.rfmax)
rfmin=str(args.rfmin)
lmbd=str(args.lmbd)
nbcomp=str(args.nbcomp)
spatialr=str(args.spatialr)
ranger=str(args.ranger)
minsize=str(args.minsize)
pixsize=str(args.pixsize)
minarea=str(args.minarea)
window=str(args.window)
siteId = "nn"
if args.siteid:
    siteId = str(args.siteid)

buildFolder=args.buildfolder
targetFolder=args.targetfolder if args.targetfolder != "" else args.outdir
reference=args.refr
erode_radius=str(args.eroderad)
alpha=str(args.alpha)

reference_polygons_reproject=os.path.join(args.outdir, "reference_polygons_reproject.shp")
reference_polygons_clip=os.path.join(args.outdir, "reference_clip.shp")
training_polygons=os.path.join(args.outdir, "training_polygons.shp")
validation_polygons=os.path.join(args.outdir, "validation_polygons.shp")

rawtocr=os.path.join(args.outdir, "rawtocr.tif")
tocr=os.path.join(args.outdir, "tocr.tif")
rawmask=os.path.join(args.outdir, "rawmask.tif")

statusFlags=os.path.join(args.outdir, "status_flags.tif")

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

rndvi=os.path.join(args.outdir, "rndvi.tif")
rtocr_smooth=os.path.join(args.outdir, "rtocr_smooth.tif")
outdays_smooth=os.path.join(args.outdir, "days_smooth.txt")
tf_noinsitu=os.path.join(args.outdir, "tf_noinsitu.tif")
spectral_features=os.path.join(args.outdir, "spectral_features.tif")
triming_features=os.path.join(args.outdir, "triming_features.tif")

eroded_reference=os.path.join(args.outdir, "eroded_reference.tif")
cropped_reference=os.path.join(args.outdir, "crop_reference.tif")
reprojected_reference = os.path.join(args.outdir, "reprojected_reference.tif")
crop_reference=os.path.join(args.outdir, "crop_reference.tif")
trimmed_reference_raster=os.path.join(args.outdir, "trimmed_reference_raster.tif")
statistics_noinsitu=os.path.join(args.outdir, "statistics_noinsitu.xml")

tmpfolder=args.outdir
tilename=args.tilename

pca=os.path.join(args.outdir, "pca.tif")
mean_shift_smoothing=os.path.join(args.outdir, "mean_shift_smoothing.tif")
mean_shift_smoothing_spatial=os.path.join(args.outdir, "mean_shift_smoothing_spatial.tif")
segmented=os.path.join(args.outdir, "segmented.tif")
segmented_merged=os.path.join(args.outdir, "segmented_merged.tif")

confmatout=os.path.join(args.outdir, "confusion-matrix.csv")
model=os.path.join(args.outdir, "crop-mask-model.txt")
raw_crop_mask_uncompressed=os.path.join(args.outdir, "raw_crop_mask_uncomp.tif")
raw_crop_mask_cut_uncompressed=os.path.join(args.outdir, "raw_crop_mask_cut_uncomp.tif")
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

if not os.path.exists(args.outdir):
    os.makedirs(args.outdir)

globalStart = datetime.datetime.now()

try:
# Bands Extractor (Step 1)
    executeStep("BandsExtractor", "otbcli", "BandsExtractor", buildFolder,"-mission",mission,"-out",rawtocr,"-mask",rawmask, "-outdate", dates, "-shape", shape, "-pixsize", pixsize,"-merge", "false", "-il", *indesc, skip=fromstep>1)

# gdalwarp (Step 2 and 3)
    executeStep("gdalwarp for reflectances", "gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", rawtocr, tocr, skip=fromstep>2, rmfiles=[] if keepfiles else [rawtocr])

    executeStep("gdalwarp for masks", "gdalwarp", "-multi", "-wm", "2048", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", rawmask, mask, skip=fromstep>3, rmfiles=[] if keepfiles else [rawmask])

# Select either inSitu or noInSitu branches
    if reference_polygons != "" :
            # Temporal Resampling (Step 4)
            executeStep("TemporalResampling", "otbcli", "TemporalResampling", buildFolder, "-tocr", tocr, "-mask", mask, "-ind", dates, "-sp", "SENTINEL", "5", "SPOT", "5", "LANDSAT", "16", "-rtocr", rtocr, "-outdays", outdays, "-mode", trm, "-merge", "1", skip=fromstep>4, rmfiles=[] if keepfiles else [tocr, mask])

            # Feature Extraction with insitu (Step 5)
            executeStep("FeatureExtraction", "otbcli", "FeatureExtraction", buildFolder, "-rtocr", rtocr, "-ndvi", ndvi, "-ndwi", ndwi, "-brightness", brightness, skip=fromstep>5, rmfiles=[] if keepfiles else [rtocr])

            #Perform insitu specific steps.
            inSituDataAvailable()

            #Validation (Step 25)
            executeStep("Validation for Raw Cropmask with insitu", "otbcli_ComputeConfusionMatrix", "-in", raw_crop_mask_uncompressed, "-out", raw_crop_mask_confusion_matrix_validation, "-ref", "vector", "-ref.vector.in", validation_polygons, "-ref.vector.field", "CROP", "-nodatalabel", "-10000", outf=raw_crop_mask_quality_metrics, skip=fromstep>25)

            rndvi=ndvi
    else:
        # Temporal Resampling (Step 5)
            executeStep("TemporalResampling", "otbcli", "TemporalResampling",
                    buildFolder, "-tocr", tocr, "-mask",
                    mask, "-ind", dates, "-sp", "SENTINEL", "5", "SPOT", "5", "LANDSAT", "16",
                    "-rtocr", rtocr, "-mode", trm, "-merge", "1", skip=fromstep>5)

            # Feature Extraction without insitu (Step 6)
            executeStep("FeatureExtraction", "otbcli", "FeatureExtraction",
                    buildFolder, "-rtocr", rtocr,
                    "-ndvi", rndvi, skip=fromstep>6)

            #Perform Noinsitu specific steps
            noInSituDataAvailable()

            #Validation (Step 25)
            executeStep("Validation for Raw Cropmask without insitu", "otbcli_ComputeConfusionMatrix", "-in", raw_crop_mask_uncompressed, "-out", raw_crop_mask_confusion_matrix_validation, "-ref", "raster", "-ref.raster.in", trimmed_reference_raster, "-nodatalabel", "-10000", outf=raw_crop_mask_quality_metrics, skip=fromstep>25)

#Dimension reduction (Step 26)
    executeStep("DimensionalityReduction", "otbcli_DimensionalityReduction",
            "-method", "pca", "-nbcomp", nbcomp,
            "-in", rndvi, "-out", pca, skip=fromstep>26, rmfiles=[] if keepfiles else [rndvi])

#Mean-Shift segmentation (Step 27, 28 and 29)
    executeStep("MeanShiftSmoothing", "otbcli_MeanShiftSmoothing", "-in", pca,"-modesearch","0", "-spatialr", spatialr, "-ranger", ranger, "-maxiter", "20", "-foutpos", mean_shift_smoothing_spatial, "-fout", mean_shift_smoothing, skip=fromstep>27, rmfiles=[] if keepfiles else [pca])

    executeStep("LSMSSegmentation", "otbcli_LSMSSegmentation", "-in", mean_shift_smoothing,"-inpos",
            mean_shift_smoothing_spatial, "-spatialr", spatialr, "-ranger", ranger, "-minsize",
            "0", "-tilesizex", "1024", "-tilesizey", "1024", "-tmpdir", tmpfolder, "-out", segmented, "uint32", skip=fromstep>28, rmfiles=[] if keepfiles else [mean_shift_smoothing_spatial])

    executeStep("LSMSSmallRegionsMerging", "otbcli_LSMSSmallRegionsMerging", "-in", mean_shift_smoothing,"-inseg", segmented, "-minsize", minsize, "-tilesizex", "1024", "-tilesizey", "1024", "-out", segmented_merged, "uint32", skip=fromstep>29, rmfiles=[] if keepfiles else [mean_shift_smoothing, segmented])

#Segmentation (Step 29)
    # executeStep("Segmentation", "otbcli_Segmentation", "-in", pca, "-filter", "meanshift", "-filter.meanshift.spatialr", spatialr, "-filter.meanshift.ranger", ranger, "-filter.meanshift.maxiter", "100", "-filter.meanshift.minsize", minsize, "-mode", "raster", "-mode.raster.out", segmented_merged, "uint32", skip=fromstep>29, rmfiles=[] if keepfiles else [pca])

#Majority voting (Step 30)
    executeStep("MajorityVoting", "otbcli", "MajorityVoting",buildFolder ,"-nodatasegvalue", "0", "-nodataclassifvalue", "-10000", "-minarea", minarea, "-inclass", raw_crop_mask_uncompressed, "-inseg", segmented_merged, "-rout", crop_mask_uncut, skip=fromstep>30, rmfiles=[] if keepfiles else [segmented_merged])

# gdalwarp (Step 31)
    executeStep("gdalwarp for raw crop mask", "gdalwarp", "-dstnodata", "\"-10000\"", "-ot", "Int16", "-overwrite", "-cutline", shape, "-crop_to_cutline", raw_crop_mask_uncompressed, raw_crop_mask_cut_uncompressed, skip=fromstep>31)
    executeStep("gdalwarp for segmented crop mask", "gdalwarp", "-dstnodata", "\"-10000\"", "-overwrite", "-cutline", shape, "-crop_to_cutline", crop_mask_uncut, crop_mask_uncompressed, skip=fromstep>31)

#Validation (Step 32)
    if reference_polygons != "" :
            executeStep("Validation for Cropmask with insitu", "otbcli_ComputeConfusionMatrix", "-in", crop_mask_uncompressed, "-out", confusion_matrix_validation, "-ref", "vector", "-ref.vector.in", validation_polygons, "-ref.vector.field", "CROP", "-nodatalabel", "-10000", outf=quality_metrics, skip=fromstep>32)
    else:
            executeStep("Validation for Cropmask without insitu", "otbcli_ComputeConfusionMatrix", "-in", crop_mask_uncompressed, "-out", confusion_matrix_validation, "-ref", "raster", "-ref.raster.in", trimmed_reference_raster, "-nodatalabel", "-10000", outf=quality_metrics, skip=fromstep>32, rmfiles=[] if keepfiles else [trimmed_reference_raster])

#Compression (Step 33)
    executeStep("Compression", "otbcli_Convert", "-in", crop_mask_uncompressed, "-out", crop_mask+"?gdal:co:COMPRESS=DEFLATE", "int16",  skip=fromstep>33, rmfiles=[] if keepfiles else [crop_mask_uncompressed])
    executeStep("Compression", "otbcli_Convert", "-in", raw_crop_mask_cut_uncompressed, "-out", raw_crop_mask+"?gdal:co:COMPRESS=DEFLATE", "int16",  skip=fromstep>33, rmfiles=[] if keepfiles else [raw_crop_mask_cut_uncompressed])

#XML conversion (Step 34)
    executeStep("XML Conversion for Crop Mask", "otbcli", "XMLStatistics", buildFolder, "-confmat", confusion_matrix_validation, "-quality", quality_metrics, "-root", "CropMask", "-out", xml_validation_metrics,  skip=fromstep>34)

# Quality Flags Extractor (Step 35)
    executeStep("QualityFlagsExtractor", "otbcli", "QualityFlagsExtractor", buildFolder,"-mission",mission,"-out",statusFlags+"?gdal:co:COMPRESS=DEFLATE","-pixsize", pixsize,"-il", *indesc, skip=fromstep>35)

#Product creation (Step 36)
    executeStep("ProductFormatter", "otbcli", "ProductFormatter", buildFolder, "-destroot", targetFolder, "-fileclass", "SVT1", "-level", "L4A", "-baseline", "01.00", "-siteid", siteId, "-processor", "cropmask", "-processor.cropmask.file", "TILE_"+tilename, crop_mask, "-processor.cropmask.rawfile", "TILE_"+tilename, raw_crop_mask, "-processor.cropmask.quality",  "TILE_"+tilename, xml_validation_metrics, "-processor.cropmask.flags", "TILE_"+tilename, statusFlags, "-il", *indesc, skip=fromstep>36)

except:
    print sys.exc_info()
finally:
    globalEnd = datetime.datetime.now()
    print "Processor CropMask finished in " + str(globalEnd-globalStart)
