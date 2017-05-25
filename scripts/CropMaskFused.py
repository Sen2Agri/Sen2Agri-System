#!/usr/bin/python

from __future__ import print_function

import os
import os.path
import shutil
import argparse
from lxml import etree
from lxml.builder import E
from sen2agri_common import ProcessorBase, Step, split_features, run_step, format_otb_filename, prepare_lut, save_lut


class CropMaskProcessor(ProcessorBase):

    def create_context(self):
        parser = argparse.ArgumentParser(description='Crop Mask Processor')

        group = parser.add_mutually_exclusive_group()
        group.add_argument('-refp', help='The reference polygons',
                           required=False, metavar='reference_polygons')
        group.add_argument('-refr', help='The reference raster',
                           required=False, metavar='reference_raster')
        parser.add_argument('-ratio', help='The ratio between the validation and training polygons (default 0.75)',
                            required=False, metavar='sample_ratio', default=0.75)
        parser.add_argument('-input', help='The list of products descriptors',
                            required=True, metavar='product_descriptor', nargs='+')
        parser.add_argument('-trm', help='The temporal resampling mode (default resample)',
                            choices=['resample', 'gapfill'], required=False, default='resample')
        parser.add_argument('-classifier', help='The classifier (rf or svm) used for training (default rf)',
                            required=False, metavar='classifier', choices=['rf', 'svm'], default='rf')
        parser.add_argument('-nbtrsample', help='The number of samples included in the training set (default 40000)',
                            required=False, metavar='nbtrsample', default=40000)
        parser.add_argument('-rseed', help='The random seed used for training (default 0)',
                            required=False, metavar='random_seed', default=0)
        parser.add_argument('-pixsize', help='The size, in meters, of a pixel (default 10)',
                            required=False, metavar='pixsize', default=10)
        parser.add_argument('-red-edge', help='Include Sentinel-2 vegetation red edge bands',
                            required=False, dest='red_edge', action='store_true')
        parser.add_argument('-no-red-edge', help='Don\'t include Sentinel-2 vegetation red edge bands',
                            required=False, dest='red_edge', action='store_false')
        parser.set_defaults(red_edge=True)
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
        parser.add_argument('-lmbd', help='The lambda parameter used in data smoothing (default 2)', required=False, metavar='lmbd', default=2)
        parser.add_argument('-eroderad', help='The radius used for erosion (default 1)', required=False, metavar='erode_radius', default='1')
        parser.add_argument('-alpha', help='The parameter alpha used by the mahalanobis function (default 0.01)', required=False, metavar='alpha', default='0.01')

        parser.add_argument('-nbcomp', help='The number of components used for dimensionality reduction (default 6)', required=False, type=int, default=6)
        parser.add_argument('-spatialr', help='The spatial radius of the neighborhood used for segmentation (default 10)', required=False, default=10)
        parser.add_argument('-ranger', help='The range radius defining the radius (expressed in radiometry unit) in the multispectral space (default 0.65)', required=False, default=0.65)
        parser.add_argument('-minsize', help='Minimum size of a region (in pixel unit) for segmentation (default 10)', required=False, default=10)
        parser.add_argument('-minarea', help="The minium number of pixel in an area where, for an equal number of crop and nocrop samples, the crop decision is taken (default 20)", required=False, default=20)
        parser.add_argument('-main-mission-segmentation', help='Only use main mission products for the segmentation',
                            required=False, type=bool, default=True)

        parser.add_argument('-keepfiles', help="Keep all intermediate files (default false)",
                            default=False, action='store_true')
        parser.add_argument('-siteid', help='The site ID', required=False, default='nn')
        parser.add_argument('-lut', help='Color LUT for previews (see /usr/share/sen2agri/crop-mask.lut)', required=False)
        parser.add_argument('-outprops', help='Output properties file', required=False)

        parser.add_argument(
            '-strata', help='Shapefiles with polygons for the strata')
        parser.add_argument('-mode', help='The execution mode',
                            required=False, choices=['prepare-site', 'prepare-tiles', 'train', 'classify', 'postprocess-tiles', 'compute-quality-flags', 'validate'], default=None)
        parser.add_argument('-stratum-filter', help='The list of strata to use in training and classification',
                            required=False, type=int, nargs='+', default=None)
        parser.add_argument('-tile-filter', help='The list of tiles to apply the classification to',
                            required=False, nargs='+', default=None)
        parser.add_argument('-skip-segmentation', help="Skip the segmentation step, creating the product with just the raw mask (default false)", default=False, action='store_true')
        parser.add_argument('-reuse-segmentation', help="Reuse the segmentation output if present on disk (default false)", default=False, action='store_true')
        parser.add_argument('-skip-quality-flags', help="Skip quality flags extraction, (default false)", default=False, action='store_true')
        parser.add_argument('-max-parallelism', help="Number of tiles to process in parallel", required=False, type=int)
        parser.add_argument('-tile-threads-hint', help="Number of threads to use for a single tile (default 2)", required=False, type=int, default=2)
        self.args = parser.parse_args()

        if self.args.refp is None and self.args.refr is None:
            raise Exception("One of refp and refr must be set")
        if self.args.refp is not None and self.args.refr is not None:
            raise Exception("Only one of refp and refr must be set")

        self.args.lut = self.get_lut_path()

        self.args.tmpfolder = self.args.outdir

    def prepare_site(self):
        if self.args.lut is not None:
            qgis_lut = self.get_output_path("qgis-color-map.txt")

            lut = prepare_lut(None, self.args.lut)
            save_lut(lut, qgis_lut)

    def prepare_tile_high_par(self, tile):
        if self.args.refr is not None:
            tile_reference = self.get_output_path("reference-{}.tif", tile.id)
            tile_reference_eroded = self.get_output_path("reference-eroded-{}.tif", tile.id)
            tile_spectral_features = self.get_output_path("spectral-features-{}.tif", tile.id)

            ring = tile.footprint.GetGeometryRef(0)
            ll, ur = ring.GetPoint(3), ring.GetPoint(1)

            run_step(Step("PrepareReference_" + tile.id, ["gdalwarp",
                                                          "-dstnodata", 0,
                                                          "-t_srs", tile.projection,
                                                          "-te", ll[0], ll[1], ur[0], ur[1],
                                                          "-tr", self.args.pixsize, self.args.pixsize,
                                                          "-overwrite",
                                                          self.args.refr,
                                                          tile_reference]))
            run_step(Step("Erosion_" + tile.id, ["otbcli", "Erosion", self.args.buildfolder,
                                                 "-radius", self.args.eroderad,
                                                 "-in", tile_reference,
                                                 "-out", tile_reference_eroded]))

            run_step(Step("SpectralFeatures_" + tile.id, ["otbcli", "SpectralFeaturesExtraction", self.args.buildfolder,
                                                          "-mission", self.args.mission.name,
                                                          "-pixsize", self.args.pixsize,
                                                          "-lambda", self.args.lmbd,
                                                          "-out", tile_spectral_features,
                                                          "-il"] + tile.get_descriptor_paths()))

    def prepare_tile_low_par(self, tile):
        if self.args.refr is not None:
            tile_reference_eroded = self.get_output_path("reference-eroded-{}.tif", tile.id)
            tile_spectral_features = self.get_output_path("spectral-features-{}.tif", tile.id)
            tile_reference_trimmed = self.get_output_path("reference-trimmed-{}.tif", tile.id)

            run_step(Step("Trimming_" + tile.id, ["otbcli", "Trimming", self.args.buildfolder,
                                                  "-alpha", self.args.alpha,
                                                  "-nbsamples", 0,
                                                  "-seed", self.args.rseed,
                                                  "-feat", tile_spectral_features,
                                                  "-ref", tile_reference_eroded,
                                                  "-out", tile_reference_trimmed]))

    def train_stratum(self, stratum):
        area_model = self.get_output_path("model-{}.txt", stratum.id)
        area_confmatout = self.get_output_path("confusion-matrix-training-{}.csv", stratum.id)
        if self.args.refp is not None:
            features_shapefile = self.get_output_path("features-{}.shp", stratum.id)

            split_features(stratum, self.args.refp, self.args.outdir)

            area_training_polygons = self.get_output_path("training_polygons-{}.shp", stratum.id)
            area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)
            area_statistics = self.get_output_path("statistics-{}.xml", stratum.id)
            area_days = self.get_output_path("days-{}.txt", stratum.id)

            area_descriptors = []
            area_prodpertile = []
            for tile in stratum.tiles:
                area_descriptors += tile.get_descriptor_paths()
                area_prodpertile.append(len(tile.descriptors))

            run_step(Step("SampleSelection", ["otbcli", "SampleSelection", self.args.buildfolder,
                                              "-ref", features_shapefile,
                                              "-ratio", self.args.ratio,
                                              "-seed", self.args.rseed,
                                              "-nofilter", "true",
                                              "-tp", area_training_polygons,
                                              "-vp", area_validation_polygons]))
            step_args = ["otbcli", "CropMaskTrainImagesClassifier", self.args.buildfolder,
                         "-mission", self.args.mission.name,
                         "-nodatalabel", -10000,
                         "-pixsize", self.args.pixsize,
                         "-outdays", area_days,
                         "-mode", self.args.trm,
                         "-io.vd", area_training_polygons,
                         "-rand", self.args.rseed,
                         "-sample.bm", 0,
                         "-io.confmatout", area_confmatout,
                         "-io.out", area_model,
                         "-sample.mt", self.args.nbtrsample,
                         "-sample.mv", 10,
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
            else:
                step_args += ["-classifier.svm.k", "rbf",
                              "-classifier.svm.opt", 1,
                              "-imstat", area_statistics]

            run_step(Step("TrainImagesClassifier", step_args))
        else:
            for tile in stratum.tiles:
                tile_reference_trimmed = self.get_output_path("reference-trimmed-{}.tif", tile.id)
                tile_stratum_reference_trimmed = self.get_output_path("reference-trimmed-{}-{}.tif", stratum.id, tile.id)

                self.rasterize_tile_mask(stratum, tile)

                stratum_tile_mask = self.get_stratum_tile_mask(stratum, tile)

                step_args = ["otbcli_BandMath",
                             "-exp", "im1b1 > 0 ? im2b1 : -10000",
                             "-il", stratum_tile_mask, tile_reference_trimmed,
                             "-out", format_otb_filename(tile_stratum_reference_trimmed, compression='DEFLATE'), "int16"]

                run_step(Step("BandMath_" + str(tile.id), step_args))

            area_model = self.get_output_path("model-{}.txt", stratum.id)
            area_confmatout = self.get_output_path("confusion-matrix-training-{}.csv", stratum.id)

            if self.args.classifier == "svm":
                files = []
                for tile in stratum.tiles:
                    tile_spectral_features = self.get_output_path("spectral-features-{}.tif", tile.id)
                    tile_stratum_spectral_features = self.get_output_path("spectral-features-{}-{}.tif", stratum.id, tile.id)

                    stratum_tile_mask = self.get_stratum_tile_mask(stratum, tile)

                    step_args = ["otbcli_BandMath",
                                 "-exp", "im1b1 > 0 ? im2b1 : -10000",
                                 "-il", stratum_tile_mask, tile_spectral_features,
                                 "-out", format_otb_filename(tile_stratum_spectral_features, compression='DEFLATE'), "int16"]

                    run_step(Step("BandMath_" + str(tile.id), step_args))

                    files.append(tile_stratum_spectral_features)

                step_args = ["otbcli_ComputeImagesStatistics",
                             "-bv", -10000,
                             "-out", area_statistics,
                             "-il"] + files

            step_args = ["otbcli", "TrainImagesClassifierNew", self.args.buildfolder,
                         "-nodatalabel", -10000,
                         "-rand", self.args.rseed,
                         "-sample.bm", 0,
                         "-io.confmatout", area_confmatout,
                         "-io.out", area_model,
                         "-sample.mt", self.args.nbtrsample,
                         "-sample.mv", 1000,
                         "-sample.vfn", "CROP",
                         "-sample.vtr", 0.01,
                         "-classifier", self.args.classifier]
            if self.args.classifier == "rf":
                step_args += ["-classifier.rf.nbtrees", self.args.rfnbtrees,
                              "-classifier.rf.min", self.args.rfmin,
                              "-classifier.rf.max", self.args.rfmax]
            else:
                step_args += ["-classifier.svm.k", "rbf",
                              "-classifier.svm.opt", 1,
                              "-imstat", area_statistics]

            step_args.append("-io.rs")
            for tile in stratum.tiles:
                tile_stratum_reference_trimmed = self.get_output_path("reference-trimmed-{}-{}.tif", stratum.id, tile.id)

                step_args.append(tile_stratum_reference_trimmed)

            step_args.append("-io.il")
            for tile in stratum.tiles:
                tile_spectral_features = self.get_output_path("spectral-features-{}.tif", tile.id)

                step_args.append(tile_spectral_features)

            run_step(Step("TrainImagesClassifier", step_args))

    def classify_tile(self, tile):
        models = []
        model_ids = []
        days = []
        statistics = []
        for stratum in tile.strata:
            area_model = self.get_output_path("model-{}.txt", stratum.id)
            area_days = self.get_output_path("days-{}.txt", stratum.id)
            area_statistics = self.get_output_path("statistics-{}.xml", stratum.id)

            models.append(area_model)
            model_ids.append(stratum.id)
            days.append(area_days)
            statistics.append(area_statistics)

        if len(models) == 0:
            print("Skipping classification for tile {} due to stratum filter".format(tile.id))
            return

        if not self.single_stratum:
            tile_model_mask = self.get_output_path("model-mask-{}.tif", tile.id)

            run_step(Step("Rasterize model mask",
                          ["otbcli_Rasterization",
                           "-mode", "attribute",
                           "-mode.attribute.field", "ID",
                           "-in", self.args.filtered_strata,
                           "-im", tile.reference_raster,
                           "-out", format_otb_filename(tile_model_mask, compression='DEFLATE'), "uint8"]))

        tile_crop_mask_uncompressed = self.get_output_path("crop_mask_map_{}_uncompressed.tif", tile.id)

        if self.args.refp is not None:
            step_args = ["otbcli", "CropMaskImageClassifier", self.args.buildfolder,
                         "-mission", self.args.mission.name,
                         "-pixsize", self.args.pixsize,
                         "-bv", -10000,
                         "-nodatalabel", -10000,
                         "-bm", "true" if self.args.bm else "false",
                         "-out", tile_crop_mask_uncompressed,
                         "-indays"] + days
            step_args += ["-model"] + models
            step_args += ["-il"] + tile.get_descriptor_paths()
            if self.args.classifier == "svm":
                step_args += ["-imstat"] + statistics
            if not self.single_stratum:
                step_args += ["-mask", tile_model_mask]
                step_args += ["-modelid"] + model_ids
        else:
            tile_spectral_features = self.get_output_path("spectral-features-{}.tif", tile.id)

            step_args = ["otbcli", "MultiModelImageClassifier", self.args.buildfolder,
                         "-in", tile_spectral_features,
                         "-out", tile_crop_mask_uncompressed]
            step_args += ["-model"] + models
            if not self.single_stratum:
                step_args += ["-mask", tile_model_mask]
                step_args += ["-modelid"] + model_ids

        run_step(Step("ImageClassifier_{}".format(tile.id), step_args, retry=True))

        if not self.args.keepfiles:
            if not self.single_stratum:
                os.remove(tile_model_mask)

            if self.args.refp is None:
                tile_spectral_features = self.get_output_path("spectral-features-{}.tif", tile.id)

                os.remove(tile_spectral_features)

        tile_crop_mask_map = self.get_tile_classification_output(tile)
        step_args = ["otbcli_Convert",
                     "-in", tile_crop_mask_uncompressed,
                     "-out", format_otb_filename(tile_crop_mask_map, compression='DEFLATE'), "int16"]
        run_step(Step("Compression_{}".format(tile.id), step_args))

        if not self.args.keepfiles:
            os.remove(tile_crop_mask_uncompressed)

    def postprocess_tile(self, tile):
        if self.args.skip_segmentation:
            return

        tile_crop_mask = self.get_tile_classification_output(tile)
        if not os.path.exists(tile_crop_mask):
            print("Skipping post-processing for tile {} due to missing raw mask".format(tile.id))
            return

        tile_ndvi = self.get_output_path("ndvi-{}.tif", tile.id)
        tile_ndvi_filled = self.get_output_path("ndvi-filled-{}.tif", tile.id)
        tile_pca = self.get_output_path("pca-{}.tif", tile.id)
        tile_smoothed = self.get_output_path("smoothed-{}.tif", tile.id)
        tile_smoothed_spatial = self.get_output_path("smoothed-spatial-{}.tif", tile.id)
        tile_segmentation = self.get_output_path("segmentation-{}.tif", tile.id)
        tile_segmentation_merged = self.get_output_path("segmentation-merged-{}.tif", tile.id)
        tile_segmented = self.get_output_path("crop-mask-segmented-{}.tif", tile.id)

        if not self.args.reuse_segmentation:
            needs_segmentation_merged = True
            needs_segmentation = True
            needs_tile_smoothed = True
            needs_tile_smoothed_spatial = True
            needs_pca = True
            needs_ndvi_filled = True
            needs_ndvi = True
        else:
            needs_segmentation_merged = not os.path.exists(tile_segmentation_merged)
            needs_segmentation = needs_segmentation_merged and not os.path.exists(tile_segmentation)
            needs_tile_smoothed = needs_segmentation and not os.path.exists(tile_smoothed)
            needs_tile_smoothed_spatial = needs_segmentation and not os.path.exists(tile_smoothed_spatial)
            needs_pca = (needs_tile_smoothed or needs_tile_smoothed_spatial) and not os.path.exists(tile_pca)
            needs_ndvi_filled = needs_pca and not os.path.exists(tile_ndvi_filled)
            needs_ndvi = needs_ndvi_filled and not os.path.exists(tile_ndvi)

        if self.args.main_mission_segmentation:
            tile_descriptors = tile.get_mission_descriptor_paths(self.args.mission)
        else:
            tile_descriptors = tile.get_descriptor_paths()

        step_args = ["otbcli", "NDVISeries", self.args.buildfolder,
                     "-mission", self.args.mission.name,
                     "-pixsize", self.args.pixsize,
                     "-mode", "gapfill",
                     "-out", tile_ndvi]
        step_args += ["-il"] + tile_descriptors
        step_args += ["-sp"] + self.args.sp

        if not needs_ndvi:
            print("Skipping NDVI extraction for tile {}".format(tile.id))
        else:
            run_step(Step("NDVI Series " + tile.id, step_args))

        step_args = ["otbcli", "FillNoData", self.args.buildfolder,
                     "-in", tile_ndvi,
                     "-bv", -10000,
                     "-out", tile_ndvi_filled]

        if not needs_ndvi_filled:
            print("Skipping NDVI no data filling for tile {}".format(tile.id))
        else:
            run_step(Step("FillNoData " + tile.id, step_args))

            if not self.args.keepfiles:
                os.remove(tile_ndvi)

        step_args = ["otbcli_DimensionalityReduction",
                     "-method", "pca",
                     "-nbcomp", self.args.nbcomp,
                     "-in", tile_ndvi_filled,
                     "-out", tile_pca]

        if not needs_pca:
            print("Skipping PCA for tile {}".format(tile.id))
        else:
            run_step(Step("NDVI PCA " + tile.id, step_args))

            if not self.args.keepfiles:
                os.remove(tile_ndvi_filled)

        step_args = ["otbcli_MeanShiftSmoothing",
                     "-in", tile_pca,
                     "-modesearch", 0,
                     "-spatialr", self.args.spatialr,
                     "-ranger", self.args.ranger,
                     "-maxiter", 20,
                     "-fout", tile_smoothed,
                     "-foutpos", tile_smoothed_spatial]

        if not needs_tile_smoothed and not needs_tile_smoothed_spatial:
            print("Skipping mean-shift smoothing for tile {}".format(tile.id))
        else:
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

        if not needs_segmentation:
            print("Skipping segmentation for tile {}".format(tile.id))
        else:
            run_step(Step("Segmentation " + tile.id, step_args))

        step_args = ["otbcli_LSMSSmallRegionsMerging",
                     "-in", tile_smoothed,
                     "-inseg", tile_segmentation,
                     "-minsize", self.args.minsize,
                     "-tilesizex", 1024,
                     "-tilesizey", 1024,
                     "-out", format_otb_filename(tile_segmentation_merged, compression='DEFLATE'), "uint32"]

        if not needs_segmentation_merged:
            print("Skipping small regions merging for tile {}".format(tile.id))
        else:
            run_step(Step("Small regions merging " + tile.id, step_args))

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

        if not self.args.keepfiles and not self.args.reuse_segmentation:
            os.remove(tile_segmentation_merged)

    def get_tile_crop_mask(self, tile):
        if self.args.skip_segmentation:
            return self.get_tile_classification_output(tile)
        else:
            return self.get_output_path("crop-mask-segmented-{}.tif", tile.id)

    def validate(self, context):
        for stratum in self.strata:
            area_statistics = self.get_output_path("confusion-matrix-validation-{}.csv", stratum.id)
            area_quality_metrics = self.get_output_path("quality-metrics-{}.txt", stratum.id)
            area_validation_metrics_xml = self.get_output_path("validation-metrics-{}.xml", stratum.id)

            step_args = ["otbcli", "ComputeConfusionMatrixMulti", self.args.buildfolder,
                         "-out", area_statistics,
                         "-nodatalabel", -10000,
                         "-il"]
            for tile in stratum.tiles:
                step_args.append(self.get_tile_crop_mask(tile))

            if self.args.refp is not None:
                area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)

                step_args += ["-ref", "vector",
                              "-ref.vector.in", area_validation_polygons,
                              "-ref.vector.field", "CROP"]
            else:
                step_args += ["-ref", "raster",
                              "-ref.raster.in"]
                for tile in stratum.tiles:
                    tile_stratum_reference_trimmed = self.get_output_path("reference-trimmed-{}-{}.tif", stratum.id, tile.id)

                    step_args.append(tile_stratum_reference_trimmed)

            run_step(Step("ComputeConfusionMatrix_" + str(stratum.id),
                          step_args, out_file=area_quality_metrics))

            step_args = ["otbcli", "XMLStatistics", self.args.buildfolder,
                         "-root", "CropMask",
                         "-confmat", area_statistics,
                         "-quality", area_quality_metrics,
                         "-out", area_validation_metrics_xml]
            run_step(Step("XMLStatistics_" + str(stratum.id), step_args))

        if not self.single_stratum:
            global_validation_metrics_xml = self.get_output_path("validation-metrics-global.xml")

            if len(self.strata) > 1:
                global_statistics = self.get_output_path("confusion-matrix-validation-global.csv")
                global_quality_metrics = self.get_output_path("quality-metrics-global.txt")

                step_args = ["otbcli", "ComputeConfusionMatrixMulti", self.args.buildfolder,
                             "-out", global_statistics,
                             "-nodatalabel", -10000,
                             "-il"]
                for tile in self.tiles:
                    step_args.append(self.get_tile_crop_mask(tile))

                if self.args.refp is not None:
                    global_validation_polygons = self.get_output_path("validation_polygons_global.shp")
                    global_prj_file = self.get_output_path("validation_polygons_global.prj")

                    files = []
                    for stratum in self.strata:
                        area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)

                        files.append(area_validation_polygons)

                    run_step(Step("ConcatenateVectorData",
                                  ["otbcli_ConcatenateVectorData",
                                   "-out", global_validation_polygons,
                                   "-vd"] + files))

                    first_prj_file = self.get_output_path("validation_polygons-{}.prj", self.strata[0].id)
                    shutil.copyfile(first_prj_file, global_prj_file)

                    step_args += ["-ref", "vector",
                                  "-ref.vector.in", global_validation_polygons,
                                  "-ref.vector.field", "CROP"]
                else:
                    step_args += ["-ref", "raster",
                                  "-ref.raster.in"]
                    for tile in self.tiles:
                        tile_reference_trimmed = self.get_output_path("reference-trimmed-{}.tif", tile.id)

                        step_args.append(tile_reference_trimmed)

                run_step(Step("ComputeConfusionMatrix_Global",
                              step_args, out_file=global_quality_metrics))

                step_args = ["otbcli", "XMLStatistics", self.args.buildfolder,
                             "-root", "CropMask",
                             "-confmat", global_statistics,
                             "-quality", global_quality_metrics,
                             "-out", global_validation_metrics_xml]
                run_step(Step("XMLStatistics_Global", step_args))
            else:
                area_validation_metrics_xml = self.get_output_path("validation-metrics-{}.xml", self.strata[0].id)

                shutil.copyfile(area_validation_metrics_xml, global_validation_metrics_xml)

        step_args = ["otbcli", "ProductFormatter", self.args.buildfolder,
                     "-destroot", self.args.targetfolder,
                     "-fileclass", "SVT1",
                     "-level", "L4A",
                     "-baseline", "01.00",
                     "-siteid", self.args.siteid,
                     "-gipp", self.get_metadata_file(),
                     "-processor", "cropmask"]

        if self.args.refp is not None:
            step_args += ["-isd", self.get_in_situ_data_file()]

        if self.args.lut is not None:
            qgis_lut = self.get_output_path("qgis-color-map.txt")

            step_args += ["-lut", self.args.lut,
                          "-lutqgis", qgis_lut]

        if self.args.outprops is not None:
            step_args += ["-outprops", self.args.outprops]

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

    def get_tile_classification_output(self, tile):
        return self.get_output_path("crop_mask_map_{}.tif", tile.id)

    def get_lut_path(self):
        if self.args.lut is not None:
            lut_path = self.args.lut
            if os.path.isfile(lut_path):
                return lut_path
            else:
                print("Warning: The LUT file {} does not exist, using the default one".format(self.args.lut))

        script_dir = os.path.dirname(__file__)
        lut_path = os.path.join(script_dir, "../sen2agri-processors/CropMask/crop-mask.lut")
        if not os.path.isfile(lut_path):
            lut_path = os.path.join(script_dir, "../share/sen2agri/crop-mask.lut")
        if not os.path.isfile(lut_path):
            lut_path = os.path.join(script_dir, "crop-mask.lut")
        if not os.path.isfile(lut_path):
            lut_path = os.path.join(script_dir, "/usr/share/sen2agri/crop-mask.lut")
        if not os.path.isfile(lut_path):
            lut_path = None

        return lut_path

    def build_metadata(self):
        tiles = E.Tiles()
        for tile in self.tiles:
            inputs = E.Inputs()
            for descriptor in tile.get_descriptor_paths():
                inputs.append(
                    E.Input(os.path.splitext(os.path.basename(descriptor))[0])
                )

            tiles.append(
                E.Tile(
                    E.Id(tile.id),
                    inputs
                )
            )

        metadata = E.Metadata(
            E.ProductType("Crop Mask"),
            E.Level("L4A"),
            E.SiteId(self.args.siteid)
        )

        if self.args.refp is not None:
            metadata.append(
                E.ReferencePolygons(os.path.basename(self.args.refp))
            )
        else:
            metadata.append(
                E.ReferenceData(os.path.basename(self.args.refr))
            )

        if self.args.strata is not None:
            metadata.append(
                E.Strata(os.path.basename(self.args.strata))
            )

        metadata.append(tiles)

        parameters = E.Parameters(
            E.MainMission(self.args.mission.name),
            E.PixelSize(str(self.args.pixsize)),
            E.SampleRatio(str(self.args.ratio)),
            E.Classifier(self.args.classifier),
            E.Seed(str(self.args.rseed)),
            E.IncludeRedEdge(str(self.args.red_edge))
        )

        if self.args.lut is not None:
            parameters.append(
                E.LUT(os.path.basename(self.args.lut))
            )

        metadata.append(parameters)

        if self.args.refr is not None:
            parameters.append(
                E.Smoothing(
                    E.Window(str(self.args.window)),
                    E.Lambda(str(self.args.lmbd))
                )
            )
            parameters.append(
                E.ErosionRadius(str(self.args.eroderad))
            )
            parameters.append(
                E.TrimmingAlpha(str(self.args.alpha))
            )

        classifier = E.Classifier(
            E.TrainingSamplesPerTile(str(self.args.nbtrsample))
        )

        if self.args.classifier == 'rf':
            classifier.append(
                E.RF(
                    E.NbTrees(str(self.args.rfnbtrees)),
                    E.Min(str(self.args.rfmin)),
                    E.Max(str(self.args.rfmax))
                )
            )
        else:
            classifier.append(
                E.SVM()
            )

        parameters.append(
            classifier
        )

        parameters.append(
            E.Segmentation(
                E.PCAComponents(str(self.args.nbcomp)),
                E.SpatialRadius(str(self.args.spatialr)),
                E.RangeRadius(str(self.args.ranger)),
                E.MinSize(str(self.args.minsize)),
                E.MinCropArea(str(self.args.minarea)),
            )
        )

        return etree.ElementTree(metadata)

processor = CropMaskProcessor()
processor.execute()
