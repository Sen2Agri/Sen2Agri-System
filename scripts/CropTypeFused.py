#!/usr/bin/python

from __future__ import print_function

from osgeo import ogr, osr
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
from sen2agri_common import executeStep


class Step(object):

    def __init__(self, name, arguments, out_file=None):
        self.name = name
        self.arguments = arguments
        self.out_file = out_file


def run_step(step):
    if step.out_file:
        executeStep(step.name, *step.arguments, outf=step.out_file)
    else:
        executeStep(step.name, *step.arguments)


def run_steps(steps):
    for step in steps:
        run_step(step)


def get_reference_raster(product):
    file = os.path.basename(product)
    parts = os.path.splitext(file)
    extension = parts[1].lower()
    if extension == ".tif":
        return product

    directory = os.path.dirname(product)
    if extension == ".xml":
        files = glob.glob(os.path.join(directory, "*PENTE*"))
        if files:
            return files[0]
    if extension == ".hdr":
        dir = os.path.join(directory, os.path.join(parts[0], ".DBL.DIR"))
        files = glob.glob(os.path.join(dir, "*.FRE_R1.DBL.TIF"))
        if files:
            return files[0]
        files = glob.glob(os.path.join(dir, "*.FRE.DBL.TIF"))
        if files:
            return files[0]


def get_tileid(product):
    file = os.path.basename(product)
    m = re.match("SPOT.+_([a-z0-9]+)\\.xml", file, re.IGNORECASE)
    if m:
        return m.group(1)
    m = re.match(".+_L8_(\d{3})_(\d{3}).hdr", file, re.IGNORECASE)
    if m:
        return m.group(1) + m.group(2)
    m = re.match("L8_.+_(\d{6})_\d{8}.HDR", file, re.IGNORECASE)
    if m:
        return m.group(1)
    m = re.match(
        "S2[a-z0-9]_[a-z0-9]+_[a-z0-9]+_[a-z0-9]+_([a-z0-9]+)_.+\\.HDR", file, re.IGNORECASE)
    if m:
        return m.group(1)


class Stratum(object):
    def __init__(self, id, shapefile, extent, crop_features):
        self.id = id
        self.shapefile = shapefile
        self.extent = extent
        self.crop_features = crop_features
        self.tiles = []

class Tile(object):
    def __init__(self, id, descriptors):
        self.id = id
        self.descriptors = descriptors
        self.footprint = None
        self.num_strata = 0
        self.main_stratum = None
        self.main_mask = None


def split_ecoareas(areas, data, out_folder):
    area_ds = ogr.Open(areas, 0)
    if area_ds is None:
        raise Exception("Could not open eco areas dataset", areas)

    out_srs = osr.SpatialReference()
    out_srs.ImportFromEPSG(4326)

    data_ds = ogr.Open(data, 0)
    in_layer = area_ds.GetLayer()
    in_layer_def = in_layer.GetLayerDefn()
    in_srs = in_layer.GetSpatialRef()
    area_transform = osr.CoordinateTransformation(in_srs, out_srs)

    data_layer = data_ds.GetLayer()
    data_layer_def = data_layer.GetLayerDefn()
    data_srs = data_layer.GetSpatialRef()
    data_transform = osr.CoordinateTransformation(data_srs, out_srs)

    driver = ogr.GetDriverByName('ESRI Shapefile')

    result = []
    for area in in_layer:
        area_id = area.GetField('ID')
        area_geom = area.GetGeometryRef().Clone()
        area_geom.Transform(area_transform)
        area_feature = ogr.Feature(in_layer_def)
        area_feature.SetGeometry(area_geom)

        name = os.path.splitext(os.path.basename(data))[0]
        out_name = os.path.join(out_folder, "{}-{}.shp".format(name, area_id))

        if os.path.exists(out_name):
            driver.DeleteDataSource(out_name)

        print("Writing {}".format(out_name))

        out_ds = driver.CreateDataSource(out_name)
        if out_ds is None:
            raise Exception("Could not create output shapefile", out_name)

        out_layer = out_ds.CreateLayer('features', srs=in_srs,
                                       geom_type=ogr.wkbMultiPolygon)
        out_layer_def = out_layer.GetLayerDefn()

        for i in range(data_layer_def.GetFieldCount()):
            out_layer.CreateField(data_layer_def.GetFieldDefn(i))

        reprojected_features = []
        data_layer.ResetReading()
        for region in data_layer:
            geom = region.GetGeometryRef().Intersection(area.GetGeometryRef())
            if geom.GetArea() > 0:
                out_feature = ogr.Feature(out_layer_def)

                for i in range(out_layer_def.GetFieldCount()):
                    out_feature.SetField(out_layer_def.GetFieldDefn(
                        i).GetNameRef(), region.GetField(i))

                out_feature.SetGeometry(geom)
                out_layer.CreateFeature(out_feature)

                geom.Transform(data_transform)
                reprojected_feature = ogr.Feature(out_layer_def)
                reprojected_feature.SetGeometry(geom)

                reprojected_features.append(reprojected_feature)
        result.append(Stratum(area_id, out_name, area_feature, reprojected_features))

    return result


def load_from_shp(file_name, out_srs=None):
    if not out_srs:
        out_srs = osr.SpatialReference()
        out_srs.ImportFromEPSG(4326)

    ds = ogr.Open(file_name, 0)
    if ds is None:
        raise Exception("Could not open dataset", file_name)

    layer = ds.GetLayer()
    layer_def = layer.GetLayerDefn()

    in_srs = layer.GetSpatialRef()
    transform = osr.CoordinateTransformation(in_srs, out_srs)

    reprojected_features = []
    for feature in layer:
        geom = feature.GetGeometryRef().Clone()
        geom.Transform(transform)

        new_feature = ogr.Feature(layer_def)
        new_feature.SetGeometry(geom)

        reprojected_features.append(new_feature)

    return reprojected_features


def load_footprints(footprints):
    result = []

    out_srs = osr.SpatialReference()
    out_srs.ImportFromEPSG(4326)

    for file in footprints:
        ds = ogr.Open(file, 0)
        if ds is None:
            raise Exception("Could not open eco areas dataset", file)

        layer = ds.GetLayer()
        layer_def = layer.GetLayerDefn()

        in_srs = layer.GetSpatialRef()
        transform = osr.CoordinateTransformation(in_srs, out_srs)

        feature = layer.GetNextFeature()
        geom = feature.GetGeometryRef()
        geom.Transform(transform)

        reprojected_feature = ogr.Feature(layer_def)
        reprojected_feature.SetGeometry(geom)

        result.append(reprojected_feature)
    return result


def save_to_shp(file_name, geom, out_srs=None):
    if not out_srs:
        out_srs = osr.SpatialReference()
        out_srs.ImportFromEPSG(4326)

    driver = ogr.GetDriverByName('ESRI Shapefile')

    if os.path.exists(file_name):
        driver.DeleteDataSource(file_name)

    out_ds = driver.CreateDataSource(file_name)
    if out_ds is None:
        raise Exception(
            "Could not create output shapefile", file_name)

    out_layer = out_ds.CreateLayer('area', srs=out_srs,
                                   geom_type=geom.GetGeometryType())
    out_layer_def = out_layer.GetLayerDefn()

    feature = ogr.Feature(out_layer_def)
    feature.SetGeometry(geom)
    out_layer.CreateFeature(feature)


def get_tileid_for_descriptors(descriptors):
    for d in descriptors:
        tileid = get_tileid(d)
        if tileid:
            return tileid


def build_merge_expression(n):
    if n > 0:
        return "im{}b1 > 0 ? im{}b1 : ({})".format(2 * n, 2 * n - 1, build_merge_expression(n - 1))
    else:
        return "-10000"

parser = argparse.ArgumentParser(description='CropType Python processor')

parser.add_argument('-mission', help='The main mission for the series',
                    required=False, default='SPOT')
parser.add_argument('-ref', help='The reference polygons',
                    required=True, metavar='reference_polygons')
parser.add_argument('-ratio', help='The ratio between the validation and training polygons (default 0.75)',
                    required=False, metavar='sample_ratio', default=0.75)
parser.add_argument('-input', help='The list of products descriptors',
                    required=True, metavar='product_descriptor', nargs='+')
parser.add_argument(
    '-prodpertile', help='Number of products for each tile', nargs='+')
parser.add_argument('-trm', help='The temporal resampling mode (default gapfill)',
                    choices=['resample', 'gapfill'], required=False, default='gapfill')
parser.add_argument('-rate', help='The sampling rate for the temporal series, in days (default 5)',
                    required=False, metavar='sampling_rate', default=5)
parser.add_argument('-classifier', help='The classifier (rf or svm) used for training (default rf)',
                    required=False, metavar='classifier', choices=['rf', 'svm'], default='rf')
parser.add_argument('-normalize', help='Normalize the input before classification', default=False,
                    required=False, action='store_true')
parser.add_argument('-rseed', help='The random seed used for training (default 0)',
                    required=False, metavar='random_seed', default=0)
parser.add_argument('-mask', help='The crop mask',
                    required=False, metavar='crop_mask', default='')
parser.add_argument('-pixsize', help='The size, in meters, of a pixel (default 10)',
                    required=False, metavar='pixsize', default=10)
parser.add_argument('-tilename', help="The name of the tile", default="T0000")
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
parser.add_argument(
    '-fromstep', help="Run from the selected step (default 1)", type=int, default=1)
parser.add_argument('-siteid', help='The site ID', required=False)
parser.add_argument(
    '-ecoareas', help='Shapefiles with polygons for the eco areas')
parser.add_argument('-mode', help='The execution mode',
                    required=True, choices=['train', 'classify', 'validate'])
parser.add_argument('-crop', help='Border cropping mode',
                    required=False, choices=['extent', 'data'], default='extent')
args = parser.parse_args()

if not args.prodpertile:
    args.prodpertile = [len(args.input)]
else:
    psum = sum(args.prodspertile)
    if psum != len(args.input):
        print("The number of input products ({}) doesn't match the total tile count ({})".format(
            len(args.input), psum))

ntiles = len(args.prodpertile)
print("Tiles:", ntiles)
single_ecoarea = args.ecoareas is None
print("Single ecoarea:", single_ecoarea)

indesc = args.input

start = 0
tiles = []
for t in args.prodpertile:
    end = start + t
    descriptors = indesc[start:end]
    id = get_tileid_for_descriptors(descriptors)
    tiles.append(Tile(id, descriptors))
    start += t

mission = args.mission

reference_polygons = args.ref
sample_ratio = str(args.ratio)

sp = args.rate
trm = args.trm
classifier = args.classifier
random_seed = args.rseed
crop_mask = args.mask
pixsize = args.pixsize
rfnbtrees = str(args.rfnbtrees)
rfmax = str(args.rfmax)
rfmin = str(args.rfmin)
siteId = "nn"
if args.siteid:
    siteId = str(args.siteid)

buildFolder = args.buildfolder

targetFolder = args.targetfolder if args.targetfolder != "" else args.outdir
tilename = args.tilename

reference_polygons_reproject = os.path.join(
    args.outdir, "reference_polygons_reproject.shp")
reference_polygons_clip = os.path.join(args.outdir, "reference_clip.shp")
training_polygons = os.path.join(args.outdir, "training_polygons.shp")
validation_polygons = os.path.join(args.outdir, "validation_polygons.shp")
rawtocr = os.path.join(args.outdir, "rawtocr.tif")
tocr = os.path.join(args.outdir, "tocr.tif")
rawmask = os.path.join(args.outdir, "rawmask.tif")

statusFlags = os.path.join(args.outdir, "status_flags.tif")

mask = os.path.join(args.outdir, "mask.tif")
dates = os.path.join(args.outdir, "dates.txt")
shape = os.path.join(args.outdir, "shape.shp")
shape_proj = os.path.join(args.outdir, "shape.prj")
rtocr = os.path.join(args.outdir, "rtocr.tif")
fts = os.path.join(args.outdir, "feature-time-series.tif")
lut = os.path.join(args.outdir, "lut.txt")

reprojected_crop_mask = os.path.join(args.outdir, "reprojected_crop_mask.tif")
cropped_crop_mask = os.path.join(args.outdir, "cropped_crop_mask.tif")

crop_type_map_uncompressed = os.path.join(
    args.outdir, "crop_type_map_uncompressed.tif")
crop_type_map = os.path.join(args.outdir, "crop_type_map.tif")
color_crop_type_map = os.path.join(args.outdir, "color_crop_type_map.tif")

confusion_matrix_validation = os.path.join(
    args.outdir, "confusion-matrix-validation.csv")
quality_metrics = os.path.join(args.outdir, "quality-metrics.txt")
xml_validation_metrics = os.path.join(args.outdir, "validation-metrics.xml")

keepfiles = args.keepfiles
fromstep = args.fromstep

if not os.path.exists(args.outdir):
    os.makedirs(args.outdir)

globalStart = datetime.datetime.now()

try:
    for tile in tiles:
        footprint_shp = os.path.join(
            args.outdir, "footprint_{}.shp".format(tile.id))

        steps = []
        steps.append(
            Step("CreateFootprint_" + tile.id,
                 ["otbcli", "CreateFootprint", buildFolder,
                  "-in", tile.descriptors[0],
                  "-out", footprint_shp,
                  "-mode", "metadata" if args.crop == 'extent' else 'raster']))

        run_steps(steps)
        tile.footprint = load_from_shp(footprint_shp)[0]
        os.unlink(footprint_shp)

    if not single_ecoarea:
        strata = split_ecoareas(args.ecoareas, args.ref, args.outdir)
        tile_best_weights = [0.0] * ntiles

        for tile in tiles:
            tile_best_weight = 0.0
            for stratum in strata:
                footprint_geom = tile.footprint.GetGeometryRef()
                footprint_area = footprint_geom.Area()

                weight = stratum.extent.GetGeometryRef().Intersection(
                    footprint_geom).Area() / footprint_area
                if weight > tile_best_weight:
                    tile_best_weight = weight
                    tile.main_stratum = stratum

                for area_feature in stratum.crop_features:
                    if area_feature.GetGeometryRef().Intersection(footprint_geom).Area() > 0:
                        stratum.tiles.append(tile)
                        tile.num_strata += 1
                        break

        if args.mode == 'classify':
            for tile in tiles:
                geom = tile.footprint.GetGeometryRef().Clone()

                for stratum in strata:
                    if stratum != tile.main_stratum:
                        geom = geom.Difference(stratum.extent.GetGeometryRef())

                tile.main_mask = geom
    else:
        stratum = Stratum(0, reference_polygons, None, None);
        stratum.tiles = tiles;
        strata = [stratum]
        for tile in tiles:
            tile.num_strata = 1
            tile.main_stratum = stratum

        if args.mode == 'classify':
            for tile in tiles:
                tile.main_mask = tile.footprint.GetGeometryRef().Clone()

    #print("Tiles for areas: {}".format(tiles_for_area))
    #print("Areas per tile: {}".format(areas_per_tile))
    #print("Tile main areas: {}".format(tile_main_area))

    sp = ["SENTINEL", "10", "SPOT", "5", "LANDSAT", "16"]
    if args.mode == 'train':
        for stratum in strata:
            print("Building model for area:", stratum.id)
            area_training_polygons = os.path.join(
                args.outdir, "training_polygons-{}.shp".format(stratum.id))
            area_validation_polygons = os.path.join(
                args.outdir, "validation_polygons-{}.shp".format(stratum.id))
            area_statistics = os.path.join(
                args.outdir, "statistics-{}.xml".format(stratum.id))
            area_model = os.path.join(
                args.outdir, "model-{}.txt".format(stratum.id))
            area_confmatout = os.path.join(
                args.outdir, "confusion-matrix-training-{}.csv".format(stratum.id))

            area_descriptors = []
            area_prodpertile = []
            for tile in stratum.tiles:
                area_descriptors += tile.descriptors
                area_prodpertile.append(len(tile.descriptors))

            steps = [Step("SampleSelection", ["otbcli", "SampleSelection", buildFolder,
                                         "-ref", stratum.shapefile,
                                         "-ratio", sample_ratio,
                                         "-seed", random_seed,
                                         "-tp", area_training_polygons,
                                         "-vp", area_validation_polygons])]
            step_args = ["otbcli", "CropTypeTrainImagesClassifier", buildFolder,
                            "-nodatalabel", -10000,
                            "-pixsize", pixsize,
                            "-mode", trm,
                            "-io.vd", area_training_polygons,
                            "-rand", random_seed,
                            "-sample.bm", 0,
                            "-io.confmatout", area_confmatout,
                            "-io.out", area_model,
                            "-sample.mt", -1,
                            "-sample.mv", -1,
                            "-sample.vfn", "CODE",
                            "-sample.vtr", 0.1,
                            "-classifier", classifier]
            step_args += ["-sp"] + sp
            step_args += ["-prodpertile"] + area_prodpertile
            step_args += ["-il"] + area_descriptors
            if classifier == "rf":
                step_args += ["-classifier.rf.nbtrees", rfnbtrees,
                                "-classifier.rf.min", rfmin,
                                "-classifier.rf.max", rfmax]
                if args.normalize:
                    step_args += ["-outstat", area_statistics]
            else:
                step_args += ["-classifier.svm.k", "rbf",
                              "-classifier.svm.opt", 1,
                              "-outstat", area_statistics]
            steps.append(Step("TrainImagesClassifier", step_args))

            run_steps(steps)
    elif args.mode == 'classify':
        for stratum in strata:
            print("Applying model for area:", stratum.id)
            area_model = os.path.join(
                args.outdir, "model-{}.txt".format(stratum.id))
            area_statistics = os.path.join(
                args.outdir, "statistics-{}.xml".format(stratum.id))
            area_mask_shape = os.path.join(
                args.outdir, "classification-mask-{}.shp".format(stratum.id))

            area_prodpertile = []
            single_tile = len(stratum.tiles) == 1
            for tile in stratum.tiles:
                print("Processing tile:", tile.id)
                area_prodpertile.append(len(tile.descriptors))

                tile_crop_type_map_uncut = os.path.join(args.outdir,
                                                        "crop_type_map_uncut_{}_{}.tif".format(stratum.id, tile.id))
                area_mask_raster = None
                tile_reference_raster = get_reference_raster(
                    tile.descriptors[0])
                print("Reference raster for tile:",
                      tile_reference_raster)

                area_mask_raster = os.path.join(
                    args.outdir, "classification-mask-{}-{}.tif".format(stratum.id, tile.id))
                if not single_ecoarea:
                    if tile.num_strata == 0:
                        print(
                            "Warning: no eco-area found for tile {}. Unable to perform classification.".format(tile.id))
                        continue
                    elif tile.num_strata == 1:
                        print(
                            "Tile {} is covered by a singe eco-area".format(tile.id))

                steps = []
                tile_mask = os.path.join(
                    args.outdir, "tile_mask_{}_{}.shp".format(stratum.id, tile.id))

                if stratum == tile.main_stratum:
                    classification_mask = tile.main_mask
                else:
                    classification_mask = stratum.extent.GetGeometryRef().Intersection(tile.footprint.GetGeometryRef())

                save_to_shp(tile_mask, classification_mask)
                steps.append(Step("Rasterize mask",
                                  ["otbcli_Rasterization",
                                   "-in", tile_mask,
                                   "-im", tile_reference_raster,
                                   "-out", area_mask_raster, "uint8",
                                   "-mode", "binary"]))

                step_args = ["otbcli", "CropTypeImageClassifier", buildFolder,
                             "-pixsize", pixsize,
                             "-mode", trm,
                             "-singletile", "true" if single_tile else "false",
                             "-bv", -10000,
                             "-model", area_model,
                             "-out", tile_crop_type_map_uncut]
                step_args += ["-il"] + tile.descriptors
                step_args += ["-sp"] + sp
                if classifier == "svm" or args.normalize:
                    step_args += ["-outstat", area_statistics]

                if area_mask_raster:
                    step_args += ["-mask", area_mask_raster]

                steps.append(Step("ImageClassifier_{}_{}".format(
                    stratum.id, tile.id), step_args))

                run_steps(steps)

        for tile in tiles:
            print("Merging classification results for tile:", tile.id)
            files = []
            for stratum in strata:
                if tile in stratum.tiles:
                    tile_crop_type_map_uncut = os.path.join(args.outdir,
                                                            "crop_type_map_uncut_{}_{}.tif".format(stratum.id, tile.id))
                    area_mask_raster = os.path.join(
                        args.outdir, "classification-mask-{}-{}.tif".format(stratum.id, tile.id))

                    files.append(tile_crop_type_map_uncut)
                    files.append(area_mask_raster)

            steps = []

            tile_crop_map = os.path.join(
                args.outdir, "crop_type_map_{}.tif".format(tile.id))
            tile_quality_flags = os.path.join(
                args.outdir, "status_flags_{}.tif".format(tile.id))

            step_args = ["otbcli_BandMath",
                         "-exp", build_merge_expression(len(strata)),
                         "-out", tile_crop_map]
            step_args += ["-il"] + files

            steps.append(Step("BandMath_" + str(tile.id), step_args))

            steps.append(Step("Nodata_" + str(tile.id),
                              ["gdal_edit.py",
                                  "-a_nodata", -10000,
                                  tile_crop_map]))

            step_args = ["otbcli", "QualityFlagsExtractor", buildFolder,
                         "-mission", mission,
                         "-out", tile_quality_flags,
                         "-pixsize", pixsize]
            step_args += ["-il"] + tile.descriptors

            steps.append(Step("QualityFlags_" + str(tile.id), step_args))
            run_steps(steps)
    else:
        files = []
        for tile in tiles:
            tile_crop_map = os.path.join(
                args.outdir, "crop_type_map_{}.tif".format(tile.id))
            files.append(tile_crop_map)

        steps = []
        for stratum in strata:
            area_validation_polygons = os.path.join(
                args.outdir, "validation_polygons-{}.shp".format(stratum.id))
            area_statistics = os.path.join(
                args.outdir, "confusion-matrix-validation-{}.csv".format(stratum.id))
            area_quality_metrics = os.path.join(
                args.outdir, "quality-metrics-{}.txt".format(stratum.id))

            step_args = ["otbcli", "ComputeConfusionMatrixMulti", buildFolder,
                         "-ref", "vector",
                         "-ref.vector.in", area_validation_polygons,
                         "-ref.vector.field", "CODE",
                         "-out", area_statistics,
                         "-nodatalabel", -10000,
                         "-il"]
            step_args += files
            steps.append(Step("ComputeConfusionMatrix_" + str(stratum.id),
                              step_args, out_file=area_quality_metrics))

        for stratum in strata:
            area_statistics = os.path.join(
                args.outdir, "confusion-matrix-validation-{}.csv".format(stratum.id))
            area_quality_metrics = os.path.join(
                args.outdir, "quality-metrics-{}.txt".format(stratum.id))
            area_validation_metrics_xml = os.path.join(
                args.outdir, "validation-metrics-{}.xml".format(stratum.id))

            step_args = ["otbcli", "XMLStatistics", buildFolder,
                         "-root", "CropType",
                         "-confmat", area_statistics,
                         "-quality", area_quality_metrics,
                         "-out", area_validation_metrics_xml]
            steps.append(Step("XMLStatistics_" + str(stratum.id), step_args))

        step_args = ["otbcli", "ProductFormatter", buildFolder,
                     "-destroot", targetFolder,
                     "-fileclass", "SVT1",
                     "-level", "L4B",
                     "-baseline", "01.00",
                     "-siteid", siteId,
                     "-processor", "croptype",
                     "-processor.croptype.file"]

        for tile in tiles:
            tile_crop_map = os.path.join(
                args.outdir, "crop_type_map_{}.tif".format(tile.id))

            step_args.append("TILE_" + tile.id)
            step_args.append(tile_crop_map)

        step_args.append("-processor.croptype.flags")
        for tile in tiles:
            tile_quality_flags = os.path.join(
                args.outdir, "status_flags_{}.tif".format(tile.id))

            step_args.append("TILE_" + tile.id)
            step_args.append(tile_quality_flags)

        step_args.append("-processor.croptype.quality")
        for stratum in strata:
            area_validation_metrics_xml = os.path.join(
                args.outdir, "validation-metrics-{}.xml".format(stratum.id))

            if not single_ecoarea:
                step_args.append("REGION_" + str(stratum.id))
            step_args.append(area_validation_metrics_xml)

        step_args.append("-il")
        step_args += indesc

        steps.append(Step("ProductFormatter", step_args))

        run_steps(steps)
except:
    traceback.print_exc()
finally:
    globalEnd = datetime.datetime.now()
    print("Processor CropType finished in", str(globalEnd - globalStart))
