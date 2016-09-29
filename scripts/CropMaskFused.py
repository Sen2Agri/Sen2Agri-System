#!/usr/bin/python

from __future__ import print_function

from osgeo import ogr, osr
import gdal
import os
import os.path
import shutil
import glob
import argparse
import csv
import re
import sys
from sys import argv
import traceback
import datetime
from sen2agri_common import *


class CropMaskProcessor(ProcessorBase):
    def create_context(self):
        parser = argparse.ArgumentParser(description='Crop Mask Processor')

        parser.add_argument('-mission', help='The main mission for the series',
                            required=False, default='SPOT')
        parser.add_argument('-refp', help='The reference polygons',
                            required=True, metavar='reference_polygons')
        parser.add_argument('-ratio', help='The ratio between the validation and training polygons (default 0.75)',
                            required=False, metavar='sample_ratio', default=0.75)
        parser.add_argument('-input', help='The list of products descriptors',
                            required=True, metavar='product_descriptor', nargs='+')
        parser.add_argument('-prodspertile', help='Number of products for each tile',
                            type=int, nargs='+')
        parser.add_argument('-trm', help='The temporal resampling mode (default resample)',
                            choices=['resample', 'gapfill'], required=False, default='resample')
        parser.add_argument('-classifier', help='The classifier (rf or svm) used for training (default rf)',
                            required=False, metavar='classifier', choices=['rf', 'svm'], default='rf')
        parser.add_argument('-normalize', help='Normalize the input before classification', default=False,
                            required=False, action='store_true')
        parser.add_argument('-rseed', help='The random seed used for training (default 0)',
                            required=False, metavar='random_seed', default=0)
        parser.add_argument('-pixsize', help='The size, in meters, of a pixel (default 10)',
                            required=False, metavar='pixsize', default=10)
        parser.add_argument('-sp', help='Per-sensor sampling rates (default SENTINEL 10 SPOT 5 LANDSAT 16)',
                            required=False, nargs='+', default=["SENTINEL", "10", "SPOT", "5", "LANDSAT", "16"])

        parser.add_argument('-outdir', help="Output directory", default=".")
        parser.add_argument('-buildfolder', help="Build folder", default="")
        parser.add_argument(
            '-targetfolder', help="The folder where the target product is built", default="")

        parser.add_argument('-rfnbtrees', help='The number of trees used for training (default 100)',
                            required=False, metavar='rfnbtrees', default=100)
        parser.add_argument('-rfmax', help='maximum depth of the trees used for Random Forest classifier (default 25)',
                            required=False, metavar='rfmax', default=25)
        parser.add_argument('-rfmin', help='minimum number of samples in each node used by the classifier (default 25)',
                            required=False, metavar='rfmin', default=25)

        parser.add_argument('-bm', help='Use benchmarking (vs. ATBD) features (default False)', required=False, type=bool, default=False)
        parser.add_argument('-window', help='The window, expressed in number of records, used for the temporal features extraction (default 6)', required=False, type=int, default=6)

        parser.add_argument('-nbcomp', help='The number of components used for dimensionality reduction (default 6)', required=False, type=int, default=6)
        parser.add_argument('-spatialr', help='The spatial radius of the neighborhood used for segmentation (default 10)', required=False, default=10)
        parser.add_argument('-ranger', help='The range radius defining the radius (expressed in radiometry unit) in the multispectral space (default 0.65)', required=False, default=0.65)
        parser.add_argument('-minsize', help='Minimum size of a region (in pixel unit) for segmentation.(default 10)', required=False, default=10)
        parser.add_argument('-minarea', help="The minium number of pixel in an area where, for an equal number of crop and nocrop samples, the crop decision is taken (default 20)", required=False, default=20)

        parser.add_argument('-keepfiles', help="Keep all intermediate files (default false)",
                            default=False, action='store_true')
        parser.add_argument('-siteid', help='The site ID', required=False, default='nn')
        parser.add_argument(
            '-strata', help='Shapefiles with polygons for the strata')
        # parser.add_argument('-min-coverage', help="Minimum coverage (0 to 1) for considering a tile for stratification (default 0.1)", required=False, type=float, default=0.1)
        parser.add_argument('-mode', help='The execution mode',
                            required=False, choices=['prepare-site', 'prepare-tiles', 'train', 'classify', 'merge', 'postprocess-tiles', 'validate'], default=None)
        parser.add_argument('-stratum-filter', help='The list of strata to use in training and classification',
                            required=False, type=int, nargs='+', default=None)
        parser.add_argument('-tile-filter', help='The list of tiles to apply the classification to',
                            required=False, nargs='+', default=None)
        parser.add_argument('-skip-segmentation', help="Skip the segmentation step, creating the product with just the raw mask (default false)", default=False, action='store_true')
        parser.add_argument('-skip-quality-flags', help="Skip quality flags extraction, (default false)", default=False, action='store_true')
        self.args = parser.parse_args()

        self.args.min_coverage = 0

        self.args.tmpfolder = self.args.outdir

    def train_stratum(self, stratum):
        features_shapefile = self.get_output_path("features-{}.shp", stratum.id)

        split_features(stratum, self.args.refp, self.args.outdir)

        area_training_polygons = self.get_output_path("training_polygons-{}.shp", stratum.id)
        area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)
        area_statistics = self.get_output_path("statistics-{}.xml", stratum.id)
        area_model = self.get_output_path("model-{}.txt", stratum.id)
        area_confmatout = self.get_output_path("confusion-matrix-training-{}.csv", stratum.id)
        area_days = self.get_output_path("days-{}.txt", stratum.id)

        area_descriptors = []
        area_prodpertile = []
        for tile in stratum.tiles:
            area_descriptors += tile.descriptors
            area_prodpertile.append(len(tile.descriptors))

        run_step(Step("SampleSelection", ["otbcli", "SampleSelection", self.args.buildfolder,
                                        "-ref", features_shapefile,
                                        "-ratio", self.args.ratio,
                                        "-seed", self.args.rseed,
                                        "-nofilter", "true",
                                        "-tp", area_training_polygons,
                                        "-vp", area_validation_polygons]))
        step_args = ["otbcli", "CropMaskTrainImagesClassifier", self.args.buildfolder,
                        "-mission", self.args.mission,
                        "-nodatalabel", -10000,
                        "-pixsize", self.args.pixsize,
                        "-outdays", area_days,
                        "-mode", self.args.trm,
                        "-io.vd", area_training_polygons,
                        "-rand", self.args.rseed,
                        "-sample.bm", 0,
                        "-io.confmatout", area_confmatout,
                        "-io.out", area_model,
                        #"-sample.mt", 400000,
                        "-sample.mt", 40000,
                        "-sample.mv", 1000,
                        "-sample.vfn", "CROP",
                        "-sample.vtr", 0.01,
                        "-window", self.args.window,
                        "-bm", "true" if self.args.bm else "false",
                        "-classifier", self.args.classifier]
        step_args += ["-sp"] + self.args.sp
        step_args += ["-prodpertile"] + area_prodpertile
        step_args += ["-il"] + area_descriptors
        if self.args.classifier == "rf":
            step_args += ["-classifier.rf.nbtrees", self.args.rfnbtrees,
                            "-classifier.rf.min", self.args.rfmin,
                            "-classifier.rf.max", self.args.rfmax]
            if self.args.normalize:
                step_args += ["-outstat", area_statistics]
        else:
            step_args += ["-classifier.svm.k", "rbf",
                            "-classifier.svm.opt", 1,
                            "-outstat", area_statistics]

        run_step(Step("TrainImagesClassifier", step_args))

    def classify_tile(self, stratum, tile):
        area_model = self.get_output_path("model-{}.txt", stratum.id)
        area_days = self.get_output_path("days-{}.txt", stratum.id)
        area_statistics = self.get_output_path("statistics-{}.xml", stratum.id)

        tile_crop_mask_uncompressed = self.get_output_path("crop_mask_map_uncut_{}_{}_uncompressed.tif", stratum.id, tile.id)
        stratum_tile_mask = self.get_stratum_tile_mask(stratum, tile)

        step_args = ["otbcli", "CropMaskImageClassifier", self.args.buildfolder,
                        "-mission", self.args.mission,
                        "-pixsize", self.args.pixsize,
                        "-indays", area_days,
                        "-singletile", "true" if len(stratum.tiles) == 1 else "false",
                        "-bv", -10000,
                        "-window", self.args.window,
                        "-bm", "true" if self.args.bm else "false",
                        "-model", area_model,
                        "-out", tile_crop_mask_uncompressed]
        step_args += ["-il"] + tile.descriptors
        step_args += ["-sp"] + self.args.sp
        if self.args.classifier == "svm" or self.args.normalize:
            step_args += ["-outstat", area_statistics]

        step_args += ["-mask", stratum_tile_mask]

        run_step(Step("ImageClassifier_{}_{}".format(stratum.id, tile.id), step_args, retry=True))

        tile_crop_mask_map_uncut = self.get_stratum_tile_classification_output(stratum, tile)
        step_args = ["otbcli_Convert",
                        "-in", tile_crop_mask_uncompressed,
                        "-out", format_otb_filename(tile_crop_mask_map_uncut, compression='DEFLATE'), "int16"]
        run_step(Step("Compression_{}_{}".format(stratum.id, tile.id), step_args))

        if not self.args.keepfiles:
            os.remove(tile_crop_mask_uncompressed)

    def postprocess_tile(self, tile):
        if self.args.skip_segmentation:
            return

        tile_crop_mask = self.get_tile_classification_output(tile)

        tile_ndvi = self.get_output_path("ndvi-{}.tif", tile.id)
        tile_pca = self.get_output_path("pca-{}.tif", tile.id)

        step_args = ["otbcli", "NDVISeries", self.args.buildfolder,
                        "-mission", self.args.mission,
                        "-pixsize", self.args.pixsize,
                        "-mode", "gapfill",
                        "-out", tile_ndvi]
        step_args += ["-il"] + tile.descriptors
        step_args += ["-sp"] + self.args.sp

        run_step(Step("NDVI Series " + tile.id, step_args))

        step_args = ["otbcli_DimensionalityReduction",
                        "-method", "pca",
                        "-nbcomp", self.args.nbcomp,
                        "-in", tile_ndvi,
                        "-out", tile_pca]

        run_step(Step("NDVI PCA " + tile.id, step_args))

        if not self.args.keepfiles:
            os.remove(tile_ndvi)

        tile_smoothed = self.get_output_path("smoothed-{}.tif", tile.id)
        tile_smoothed_spatial = self.get_output_path("smoothed-spatial-{}.tif", tile.id)
        tile_segmentation = self.get_output_path("segmentation-{}.tif", tile.id)
        tile_segmentation_merged = self.get_output_path("segmentation-merged-{}.tif", tile.id)
        tile_segmented = self.get_output_path("crop-mask-segmented-{}.tif", tile.id)

        step_args = ["otbcli_MeanShiftSmoothing",
                     "-in", tile_pca,
                     "-modesearch", 0,
                     "-spatialr", self.args.spatialr,
                     "-ranger", self.args.ranger,
                     "-maxiter", 20,
                     "-fout", tile_smoothed,
                     "-foutpos", tile_smoothed_spatial]
        run_step(Step("Mean-Shift Smoothing " + tile.id, step_args))

        if not self.args.keepfiles:
            os.remove(tile_pca)

        step_args = ["otbcli_LSMSSegmentation",
                     "-in", tile_smoothed,
                     "-inpos", tile_smoothed_spatial,
                     "-spatialr", self.args.spatialr,
                     "-ranger", self.args.ranger,
                     "-minsize", 0,
                     "-tilesizex", 1024,
                     "-tilesizey", 1024,
                     "-tmpdir", self.args.tmpfolder,
                     "-out", format_otb_filename(tile_segmentation, compression='DEFLATE'), "uint32"]
        run_step(Step("Segmentation " + tile.id, step_args))

        step_args = ["otbcli_LSMSSmallRegionsMerging",
                     "-in", tile_smoothed,
                     "-inseg", tile_segmentation,
                     "-minsize", self.args.minsize,
                     "-tilesizex", 1024,
                     "-tilesizey", 1024,
                     "-out", format_otb_filename(tile_segmentation_merged, compression='DEFLATE'), "uint32"]
        run_step(Step("Small region merging " + tile.id, step_args))

        if not self.args.keepfiles:
            os.remove(tile_smoothed)
            os.remove(tile_smoothed_spatial)
            os.remove(tile_segmentation)

        step_args = ["otbcli", "MajorityVoting", self.args.buildfolder,
                     "-nodatasegvalue", 0,
                     "-nodataclassifvalue", "-10000",
                     "-minarea", self.args.minarea,
                     "-inclass", tile_crop_mask,
                     "-inseg", tile_segmentation_merged,
                     "-rout", format_otb_filename(tile_segmented, compression='DEFLATE')]
        run_step(Step("Majority voting " + tile.id, step_args))

    def get_tile_crop_mask(self, tile):
        if self.args.skip_segmentation:
            return self.get_tile_classification_output(tile)
        else:
            return self.get_output_path("crop-mask-segmented-{}.tif", tile.id)

    def validate(self, context):
        files = []
        for tile in self.tiles:
            if self.args.skip_segmentation:
                tile_crop_mask = self.get_tile_classification_output(tile)
                files.append(tile_crop_mask)
            else:
                tile_segmented = self.get_output_path("crop-mask-segmented-{}.tif", tile.id)
                files.append(tile_segmented)

        for stratum in self.strata:
            area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)
            area_statistics = self.get_output_path("confusion-matrix-validation-{}.csv", stratum.id)
            area_quality_metrics = self.get_output_path("quality-metrics-{}.txt", stratum.id)
            area_validation_metrics_xml = self.get_output_path("validation-metrics-{}.xml", stratum.id)

            step_args = ["otbcli", "ComputeConfusionMatrixMulti", self.args.buildfolder,
                            "-ref", "vector",
                            "-ref.vector.in", area_validation_polygons,
                            "-ref.vector.field", "CROP",
                            "-out", area_statistics,
                            "-nodatalabel", -10000,
                            "-il"]
            step_args += files
            run_step(Step("ComputeConfusionMatrix_" + str(stratum.id),
                                step_args, out_file=area_quality_metrics))

            step_args = ["otbcli", "XMLStatistics", self.args.buildfolder,
                            "-root", "CropMask",
                            "-confmat", area_statistics,
                            "-quality", area_quality_metrics,
                            "-out", area_validation_metrics_xml]
            run_step(Step("XMLStatistics_" + str(stratum.id), step_args))

        if not self.single_stratum:
            global_validation_polygons = self.get_output_path("validation_polygons_global.shp")
            global_prj_file = self.get_output_path("validation_polygons_global.prj")
            global_statistics = self.get_output_path("confusion-matrix-validation-global.csv")
            global_quality_metrics = self.get_output_path("quality-metrics-global.txt")
            global_validation_metrics_xml = self.get_output_path("validation-metrics-global.xml")

            files = []
            for stratum in self.strata:
                area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)

                files.append(area_validation_polygons)

            step_args = ["otbcli_ConcatenateVectorData",
                            "-out", global_validation_polygons,
                            "-vd"] + files
            run_step(Step("ConcatenateVectorData", step_args))

            first_prj_file = self.get_output_path("validation_polygons-{}.prj", self.strata[0].id)
            shutil.copyfile(first_prj_file, global_prj_file)

            step_args = ["otbcli", "ComputeConfusionMatrixMulti", self.args.buildfolder,
                            "-ref", "vector",
                            "-ref.vector.in", global_validation_polygons,
                            "-ref.vector.field", "CODE",
                            "-out", global_statistics,
                            "-nodatalabel", -10000,
                            "-il"]
            for tile in self.tiles:
                step_args.append(self.get_tile_crop_mask(tile))

            run_step(Step("ComputeConfusionMatrix_Global",
                                step_args, out_file=global_quality_metrics))

            step_args = ["otbcli", "XMLStatistics", self.args.buildfolder,
                            "-root", "CropMask",
                            "-confmat", global_statistics,
                            "-quality", global_quality_metrics,
                            "-out", global_validation_metrics_xml]
            run_step(Step("XMLStatistics_Global", step_args))

        step_args = ["otbcli", "ProductFormatter", self.args.buildfolder,
                        "-destroot", self.args.targetfolder,
                        "-fileclass", "SVT1",
                        "-level", "L4A",
                        "-baseline", "01.00",
                        "-siteid", self.args.siteid,
                        "-processor", "cropmask"]

        step_args.append("-processor.cropmask.file")
        for tile in self.tiles:
            tile_crop_mask = self.get_tile_crop_mask(tile)

            step_args.append("TILE_" + tile.id)
            step_args.append(tile_crop_mask)

        if not self.args.skip_segmentation:
            step_args.append("-processor.cropmask.rawfile")
            for tile in self.tiles:
                tile_crop_mask = self.get_tile_classification_output(tile)

                step_args.append("TILE_" + tile.id)
                step_args.append(tile_crop_mask)

        step_args.append("-processor.cropmask.flags")
        for tile in self.tiles:
            tile_quality_flags = self.get_output_path("status_flags_{}.tif", tile.id)

            step_args.append("TILE_" + tile.id)
            step_args.append(tile_quality_flags)

        step_args.append("-processor.cropmask.quality")
        if not self.single_stratum:
            global_validation_metrics_xml = self.get_output_path("validation-metrics-global.xml")

            step_args.append(global_validation_metrics_xml)

        for stratum in self.strata:
            area_validation_metrics_xml = self.get_output_path("validation-metrics-{}.xml", stratum.id)

            if not self.single_stratum:
                step_args.append("REGION_" + str(stratum.id))

            step_args.append(area_validation_metrics_xml)

        step_args.append("-il")
        step_args += self.args.input

        run_step(Step("ProductFormatter", step_args))

    def get_stratum_tile_classification_output(self, stratum, tile):
        return self.get_output_path("crop_mask_map_uncut_{}_{}.tif", stratum.id, tile.id)

    def get_tile_classification_output(self, tile):
        return self.get_output_path("crop_mask_map_{}.tif", tile.id)

processor = CropMaskProcessor()
processor.execute()
