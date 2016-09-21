#!/usr/bin/python

from __future__ import print_function

from osgeo import ogr, osr
import gdal
import os
import os.path
import glob
import argparse
import csv
import re
import sys
from sys import argv
import traceback
import datetime
from sen2agri_common import *


class CropTypeProcessor(ProcessorBase):
    def create_context(self):
        parser = argparse.ArgumentParser(description='Crop Type Processor')

        parser.add_argument('-mission', help='The main mission for the series',
                            required=False, default='SPOT')
        parser.add_argument('-ref', help='The reference polygons',
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
        parser.add_argument('-mask', help='The crop mask for each tile',
                            required=False, nargs='+', default=None)
        parser.add_argument('-maskprod', help='A crop mask product for the same tiles', required=False, default=None)
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

        parser.add_argument('-keepfiles', help="Keep all intermediate files (default false)",
                            default=False, action='store_true')
        parser.add_argument('-siteid', help='The site ID', required=False, default='nn')
        parser.add_argument(
            '-strata', help='Shapefiles with polygons for the strata')
        parser.add_argument('-mode', help='The execution mode',
                            required=False, choices=['train', 'classify', 'validate'], default=None)
        parser.add_argument('-stratum-filter', help='The list of strata to use in training and classification',
                            required=False, type=int, nargs='+', default=None)
        parser.add_argument('-tile-filter', help='The list of tiles to apply the classification to',
                            required=False, nargs='+', default=None)
        self.args = parser.parse_args()

        self.crop_features = load_features(self.args.ref)

        if self.args.mask is not None and self.args.maskprod is not None:
            raise("The -mask and -maskprod arguments are exclusive")

    def after_prepare_tile(self, tile):
        for tile in self.tiles:
            tile.crop_mask = None

        if self.args.mask is not None:
            for idx, tile in self.tiles:
                if self.args.mask[idx] != 'NONE':
                    tile.crop_mask = self.args.mask[idx]
        elif self.args.maskprod is not None:
            mask_dict = {}
            tile_path = os.path.join(self.args.maskprod, 'TILES')
            tile_id_re = re.compile('_T([a-zA-Z0-9]+)$')

            for tile_dir in os.listdir(tile_path):
                m = tile_id_re.search(tile_dir)
                if m:
                    tile_id = m.group(1)
                    segmented_mask = None
                    raw_mask = None

                    img_data_path = os.path.join(os.path.join(tile_path, tile_dir), 'IMG_DATA')
                    for file in os.listdir(img_data_path):
                        if file.startswith('S2AGRI_L4A_CM'):
                            segmented_mask = os.path.join(img_data_path, file)
                        elif file.startswith('S2AGRI_L4A_RAW'):
                            raw_mask = os.path.join(img_data_path, file)

                    if segmented_mask is not None:
                        mask_dict[tile_id] = segmented_mask
                    elif raw_mask is not None:
                        mask_dict[tile_id] = raw_mask

            for tile in self.tiles:
                mask = mask_dict.get(tile.id)
                if mask is not None:
                    tile.crop_mask = mask

        for tile in self.tiles:
            if tile.crop_mask is not None:
                print("Crop mask for tile {}: {}".format(tile.id, tile.crop_mask))


    def tile_has_features(self, stratum, tile):
        if stratum.extent is None:
            geom = tile.footprint
        else:
            geom = stratum.extent.GetGeometryRef().Intersection(tile.footprint)

        for feature in self.crop_features:
            if geom.Intersection(feature.GetGeometryRef()).Area() > 0:
                return True

        return False

    def train_stratum(self, stratum):
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
                                        "-ref", stratum.shapefile,
                                        "-ratio", self.args.ratio,
                                        "-seed", self.args.rseed,
                                        "-tp", area_training_polygons,
                                        "-vp", area_validation_polygons]))
        step_args = ["otbcli", "CropTypeTrainImagesClassifier", self.args.buildfolder,
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
                        "-sample.mt", -1,
                        "-sample.mv", 1000,
                        "-sample.vfn", "CODE",
                        "-sample.vtr", 0.01,
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

        tile_crop_type_map_uncompressed = self.get_output_path("crop_type_map_uncut_{}_{}_uncompressed.tif", stratum.id, tile.id)
        stratum_tile_mask = self.get_stratum_tile_mask(stratum, tile)

        step_args = ["otbcli", "CropTypeImageClassifier", self.args.buildfolder,
                        "-mission", self.args.mission,
                        "-pixsize", self.args.pixsize,
                        "-indays", area_days,
                        "-singletile", "true" if len(stratum.tiles) == 1 else "false",
                        "-bv", -10000,
                        "-model", area_model,
                        "-out", tile_crop_type_map_uncompressed]
        step_args += ["-il"] + tile.descriptors
        step_args += ["-sp"] + self.args.sp
        if self.args.classifier == "svm" or self.args.normalize:
            step_args += ["-outstat", area_statistics]

        step_args += ["-mask", stratum_tile_mask]

        run_step(Step("ImageClassifier_{}_{}".format(stratum.id, tile.id), step_args, retry=True))

        tile_crop_type_map_uncut = self.get_stratum_tile_classification_output(stratum, tile)
        step_args = ["otbcli_Convert",
                        "-in", tile_crop_type_map_uncompressed,
                        "-out", format_otb_filename(tile_crop_type_map_uncut, compression='DEFLATE'), "int16"]
        run_step(Step("Compression_{}_{}".format(stratum.id, tile.id), step_args))

        if not self.args.keepfiles:
            os.remove(tile_crop_type_map_uncompressed)

    def postprocess_tile(self, tile):
        if tile.crop_mask is not None:
            tile_crop_map = self.get_output_path("crop_type_map_{}.tif", tile.id)
            tile_crop_map_masked = self.get_output_path("crop_type_map_masked_{}.tif", tile.id)

            step_args = ["otbcli_BandMath",
                         "-exp", "im2b1 == 0 ? 0 : im1b1",
                         "-il", tile_crop_map, tile.crop_mask,
                         "-out", format_otb_filename(tile_crop_map_masked, compression='DEFLATE'), "int16"]

            run_step(Step("Mask by crop mask " + tile.id, step_args))

    def validate(self, context):
        files = []
        for tile in self.tiles:
            if tile.crop_mask is not None:
                tile_crop_map_masked = self.get_output_path("crop_type_map_masked_{}.tif", tile.id)

                files.append(tile_crop_map_masked)
            else:
                tile_crop_map = self.get_output_path("crop_type_map_{}.tif", tile.id)

                files.append(tile_crop_map)

        for stratum in self.strata:
            area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)
            area_statistics = self.get_output_path("confusion-matrix-validation-{}.csv", stratum.id)
            area_quality_metrics = self.get_output_path("quality-metrics-{}.txt", stratum.id)
            area_validation_metrics_xml = self.get_output_path("validation-metrics-{}.xml", stratum.id)

            step_args = ["otbcli", "ComputeConfusionMatrixMulti", self.args.buildfolder,
                            "-ref", "vector",
                            "-ref.vector.in", area_validation_polygons,
                            "-ref.vector.field", "CODE",
                            "-out", area_statistics,
                            "-nodatalabel", -10000,
                            "-il"]
            step_args += files
            run_step(Step("ComputeConfusionMatrix_" + str(stratum.id),
                                step_args, out_file=area_quality_metrics))

            step_args = ["otbcli", "XMLStatistics", self.args.buildfolder,
                            "-root", "CropType",
                            "-confmat", area_statistics,
                            "-quality", area_quality_metrics,
                            "-out", area_validation_metrics_xml]
            run_step(Step("XMLStatistics_" + str(stratum.id), step_args))

        step_args = ["otbcli", "ProductFormatter", self.args.buildfolder,
                        "-destroot", self.args.targetfolder,
                        "-fileclass", "SVT1",
                        "-level", "L4B",
                        "-baseline", "01.00",
                        "-siteid", self.args.siteid,
                        "-processor", "croptype"]

        has_mask = False
        for tile in self.tiles:
            if tile.crop_mask is not None:
                has_mask = True
                break

        if not has_mask:
            step_args.append("-processor.croptype.file")
            for tile in self.tiles:
                tile_crop_map = self.get_output_path("crop_type_map_{}.tif", tile.id)

                step_args.append("TILE_" + tile.id)
                step_args.append(tile_crop_map)
        else:
            step_args.append("-processor.croptype.file")
            for tile in self.tiles:
                tile_crop_map = self.get_output_path("crop_type_map_{}.tif", tile.id)
                tile_crop_map_masked = self.get_output_path("crop_type_map_masked_{}.tif", tile.id)

                step_args.append("TILE_" + tile.id)

                if tile.crop_mask is not None:
                    step_args.append(tile_crop_map_masked)
                else:
                    step_args.append(tile_crop_map)

            step_args.append("-processor.croptype.rawfile")
            for tile in self.tiles:
                if tile.crop_mask is not None:
                    tile_crop_map = self.get_output_path("crop_type_map_{}.tif", tile.id)

                    step_args.append("TILE_" + tile.id)
                    step_args.append(tile_crop_map)

        step_args.append("-processor.croptype.flags")
        for tile in self.tiles:
            tile_quality_flags = self.get_output_path("status_flags_{}.tif", tile.id)

            step_args.append("TILE_" + tile.id)
            step_args.append(tile_quality_flags)

        step_args.append("-processor.croptype.quality")
        for stratum in self.strata:
            area_validation_metrics_xml = self.get_output_path("validation-metrics-{}.xml", stratum.id)

            if not self.single_stratum:
                step_args.append("REGION_" + str(stratum.id))
            step_args.append(area_validation_metrics_xml)

        step_args.append("-il")
        step_args += self.args.input

        run_step(Step("ProductFormatter", step_args))

    def get_stratum_tile_classification_output(self, stratum, tile):
        return self.get_output_path("crop_type_map_uncut_{}_{}.tif", stratum.id, tile.id)

    def get_tile_classification_output(self, tile):
        return self.get_output_path("crop_type_map_{}.tif", tile.id)

processor = CropTypeProcessor()
processor.execute()
