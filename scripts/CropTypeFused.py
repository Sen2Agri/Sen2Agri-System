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
from sen2agri_common import executeStep


class Step(object):

    def __init__(self, name, arguments, out_file=None, retry=False):
        self.name = name
        self.arguments = arguments
        self.out_file = out_file
        self.retry = retry


def run_step(step):
    if step.out_file:
        executeStep(step.name, *step.arguments, outf=step.out_file, retry=step.retry)
    else:
        executeStep(step.name, *step.arguments, retry=step.retry)

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
        dir = os.path.join(directory, parts[0] + ".DBL.DIR")
        files = glob.glob(os.path.join(dir, "*_FRE_R1.DBL.TIF"))
        if files:
            return files[0]
        files = glob.glob(os.path.join(dir, "*_FRE.DBL.TIF"))
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
    def __init__(self, id, shapefile, extent):
        self.id = id
        self.shapefile = shapefile
        self.extent = extent
        self.tiles = []

class Tile(object):
    def __init__(self, id, descriptors):
        self.id = id
        self.descriptors = descriptors
        self.footprint = None
        self.area = None
        self.strata = []
        self.main_stratum = None
        self.main_mask = None


def filter_polygons(crop_features, site_footprint_wgs84, out_folder):
    features_ds = ogr.Open(crop_features, 0)
    if features_ds is None:
        raise Exception("Could not open reference dataset", crop_features)

    wgs84_srs = osr.SpatialReference()
    wgs84_srs.ImportFromEPSG(4326)

    in_layer = features_ds.GetLayer()
    in_layer_def = in_layer.GetLayerDefn()
    in_srs = in_layer.GetSpatialRef()

    wgs84_transform = osr.CoordinateTransformation(in_srs, wgs84_srs)

    driver = ogr.GetDriverByName('ESRI Shapefile')
    out_name = os.path.join(out_folder, "filtered-features.shp")

    if os.path.exists(out_name):
        driver.DeleteDataSource(out_name)

    print("Writing {}".format(out_name))

    out_ds = driver.CreateDataSource(out_name)
    if out_ds is None:
        raise Exception("Could not create output dataset", out_name)

    out_layer = out_ds.CreateLayer('features', srs=in_srs,
                                   geom_type=ogr.wkbMultiPolygon)
    out_layer_def = out_layer.GetLayerDefn()

    for i in xrange(in_layer_def.GetFieldCount()):
        out_layer.CreateField(in_layer_def.GetFieldDefn(i))

    for region in in_layer:
        region_geom = region.GetGeometryRef()
        region_geom_wgs84 = region_geom.Clone()
        region_geom_wgs84.Transform(wgs84_transform)

        region_geom_wgs84 = region_geom_wgs84.Intersection(site_footprint_wgs84)
        if region_geom_wgs84.GetArea() > 0:
            out_feature = ogr.Feature(out_layer_def)

            for i in xrange(out_layer_def.GetFieldCount()):
                out_feature.SetField(out_layer_def.GetFieldDefn(
                    i).GetNameRef(), region.GetField(i))

            out_feature.SetGeometry(region_geom)
            out_layer.CreateFeature(out_feature)

    return out_name

def load_features(data):
    out_srs = osr.SpatialReference()
    out_srs.ImportFromEPSG(4326)

    data_ds = ogr.Open(data, 0)
    data_layer = data_ds.GetLayer()
    data_layer_def = data_layer.GetLayerDefn()
    data_srs = data_layer.GetSpatialRef()

    transform = osr.CoordinateTransformation(data_srs, out_srs)

    result = []
    for region in data_layer:
        region_geom = region.GetGeometryRef()
        region_geom_wgs84 = region_geom.Clone()
        region_geom_wgs84.Transform(transform)

        reprojected_feature = ogr.Feature(data_layer_def)
        reprojected_feature.SetGeometry(region_geom_wgs84)

        result.append(reprojected_feature)

    return result

def split_strata(areas, data, site_footprint_wgs84, out_folder):
    area_ds = ogr.Open(areas, 0)
    if area_ds is None:
        raise Exception("Could not open stratum dataset", areas)

    out_srs = osr.SpatialReference()
    out_srs.ImportFromEPSG(4326)

    in_layer = area_ds.GetLayer()
    in_layer_def = in_layer.GetLayerDefn()
    in_srs = in_layer.GetSpatialRef()
    area_transform = osr.CoordinateTransformation(in_srs, out_srs)

    data_ds = ogr.Open(data, 0)
    data_layer = data_ds.GetLayer()
    data_layer_def = data_layer.GetLayerDefn()
    data_srs = data_layer.GetSpatialRef()
    data_transform = osr.CoordinateTransformation(data_srs, out_srs)

    driver = ogr.GetDriverByName('ESRI Shapefile')
    result = []
    for area in in_layer:
        area_id = area.GetField('ID')
        area_geom_wgs84 = area.GetGeometryRef().Clone()
        area_geom_wgs84.Transform(area_transform)
        area_geom_wgs84 = area_geom_wgs84.Intersection(site_footprint_wgs84)
        if area_geom_wgs84.GetArea() == 0:
            continue

        area_feature = ogr.Feature(in_layer_def)
        area_feature.SetGeometry(area_geom_wgs84)

        name = os.path.splitext(os.path.basename(data))[0]
        out_name = os.path.join(out_folder, "{}-{}.shp".format(name, area_id))

        if os.path.exists(out_name):
            driver.DeleteDataSource(out_name)

        print("Writing {}".format(out_name))

        out_ds = driver.CreateDataSource(out_name)
        if out_ds is None:
            raise Exception("Could not create output dataset", out_name)

        print("output srs: {}".format(in_srs))
        out_layer = out_ds.CreateLayer('features', srs=out_srs,
                                       geom_type=ogr.wkbMultiPolygon)
        out_layer_def = out_layer.GetLayerDefn()

        for i in xrange(data_layer_def.GetFieldCount()):
            out_layer.CreateField(data_layer_def.GetFieldDefn(i))

        out_features = 0
        data_layer.ResetReading()
        for region in data_layer:
            region_geom = region.GetGeometryRef()
            region_geom_wgs84 = region_geom.Clone()
            region_geom_wgs84.Transform(data_transform)

            region_geom_wgs84 = region_geom_wgs84.Intersection(area_geom_wgs84)
            if region_geom_wgs84.GetArea() > 0:
                out_feature = ogr.Feature(out_layer_def)
                out_features += 1

                for i in xrange(out_layer_def.GetFieldCount()):
                    out_feature.SetField(out_layer_def.GetFieldDefn(
                        i).GetNameRef(), region.GetField(i))

                out_feature.SetGeometry(region_geom_wgs84)
                out_layer.CreateFeature(out_feature)

        if out_features > 0:
            print("Stratum {}: {} features".format(area_id, out_features))
            result.append(Stratum(area_id, out_name, area_feature))
        else:
            print("Ignoring stratum {}: no features inside of site extent".format(area_id))

    return result

def GetExtent(gt, cols, rows):
    ext = []
    xarr = [0, cols]
    yarr = [0, rows]

    for px in xarr:
        for py in yarr:
            x = gt[0] + px * gt[1] + py * gt[2]
            y = gt[3] + px * gt[4] + py * gt[5]
            ext.append([x, y])
        yarr.reverse()
    return ext


def ReprojectCoords(coords, src_srs, tgt_srs):
    trans_coords = []
    transform = osr.CoordinateTransformation(src_srs, tgt_srs)
    for x, y in coords:
        x, y, z = transform.TransformPoint(x, y)
        trans_coords.append([x, y])
    return trans_coords

def get_raster_footprint(image_filename):
    dataset = gdal.Open(image_filename, gdal.gdalconst.GA_ReadOnly)

    size_x = dataset.RasterXSize
    size_y = dataset.RasterYSize

    geo_transform = dataset.GetGeoTransform()

    spacing_x = geo_transform[1]
    spacing_y = geo_transform[5]

    extent = GetExtent(geo_transform, size_x, size_y)

    source_srs = osr.SpatialReference()
    source_srs.ImportFromWkt(dataset.GetProjection())
    epsg_code = source_srs.GetAttrValue("AUTHORITY", 1)
    target_srs = osr.SpatialReference()
    target_srs.ImportFromEPSG(4326)

    wgs84_extent = ReprojectCoords(extent, source_srs, target_srs)

    ring = ogr.Geometry(ogr.wkbLinearRing)
    ring.AddPoint_2D(wgs84_extent[0][0], wgs84_extent[0][1])
    ring.AddPoint_2D(wgs84_extent[3][0], wgs84_extent[3][1])
    ring.AddPoint_2D(wgs84_extent[2][0], wgs84_extent[2][1])
    ring.AddPoint_2D(wgs84_extent[1][0], wgs84_extent[1][1])
    ring.AddPoint_2D(wgs84_extent[0][0], wgs84_extent[0][1])

    geom = ogr.Geometry(ogr.wkbPolygon)
    geom.AddGeometry(ring)

    return geom

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

def format_otb_filename(file, compression=None):
    if compression is not None:
        file += "?gdal:co:COMPRESS=" + compression
    return file

class ProcessorBase(object):
    def execute(self):
        start_time = datetime.datetime.now()
        try:
            context = self.create_context()

            self.prepare_tiles()

            for tile in self.tiles:
                self.after_prepare_tile(tile)

            if self.args.mode is None or self.args.mode == 'train':
                for stratum in self.strata:
                    print("Building model for stratum:", stratum.id)
                    self.train_stratum(stratum)

            if self.args.mode is None or self.args.mode == 'classify':
                for stratum in self.strata:
                    print("Applying model for stratum:", stratum.id)

                    for tile in stratum.tiles:
                        print("Processing tile:", tile.id)
                        if len(tile.strata) == 0:
                            print(
                                "Warning: no stratum found for tile {}. Classification will not be performed.".format(tile.id))
                            continue
                        elif len(tile.strata) == 1:
                            print(
                                "Tile {} is covered by a single stratum".format(tile.id))

                        self.rasterize_tile_mask(stratum, tile)
                        self.classify_tile(stratum, tile)

                self.merge_classification_outputs()

                for tile in self.tiles:
                    self.postprocess_tile(tile)

                self.compute_quality_flags()

            if self.args.mode is None or self.args.mode == 'validate':
                self.validate(context)
        except:
            traceback.print_exc()
        finally:
            end_time = datetime.datetime.now()
            print("Processor finished in", str(end_time - start_time))

    def compute_quality_flags(self):
        for tile in self.tiles:
            tile_quality_flags = self.get_output_path("status_flags_{}.tif", tile.id)

            step_args = ["otbcli", "QualityFlagsExtractor", self.args.buildfolder,
                            "-mission", self.args.mission,
                            "-out", format_otb_filename(tile_quality_flags, compression='DEFLATE'),
                            "-pixsize", self.args.pixsize]
            step_args += ["-il"] + tile.descriptors

            run_step(Step("QualityFlags_" + str(tile.id), step_args))

    def prepare_tiles(self):
        if not self.args.prodspertile:
            self.args.prodspertile = [len(self.args.input)]
        else:
            psum = sum(self.args.prodspertile)
            if psum != len(self.args.input):
                print("The number of input products ({}) doesn't match the total tile count ({})".format(
                    len(self.args.input), psum))

        start = 0
        self.tiles = []
        for idx, t in enumerate(self.args.prodspertile):
            end = start + t
            descriptors = self.args.input[start:end]
            id = get_tileid_for_descriptors(descriptors)
            self.tiles.append(Tile(id, descriptors))
            start += t

        print("Tiles:", len(self.tiles))

        for tile in self.tiles:
            print(tile.id, len(tile.descriptors))

        self.single_stratum = self.args.strata is None
        print("Single stratum:", self.single_stratum)

        if not self.args.targetfolder:
            self.args.targetfolder = self.args.outdir

        if not os.path.exists(self.args.outdir):
            os.makedirs(self.args.outdir)

        self.site_footprint_wgs84 = ogr.Geometry(ogr.wkbPolygon)
        for tile in self.tiles:
            tile.footprint = get_raster_footprint(get_reference_raster(tile.descriptors[0]))
            tile.area = tile.footprint.Area()
            self.site_footprint_wgs84 = self.site_footprint_wgs84.Union(tile.footprint)

        if not self.single_stratum:
            self.strata = split_strata(self.args.strata, self.args.ref, self.site_footprint_wgs84, self.args.outdir)
            tile_best_weights = [0.0] * len(self.tiles)

            for tile in self.tiles:
                tile_best_weight = 0.0
                tile_strata = []

                for stratum in self.strata:
                    weight = stratum.extent.GetGeometryRef().Intersection(
                        tile.footprint).Area() / tile.area

                    if weight > tile_best_weight:
                        tile_best_weight = weight
                        tile.main_stratum = stratum

                    tile_strata.append((stratum, weight))

                tile_strata.sort(key=lambda x: -x[1])

                print(tile.id, [(ts[0].id, ts[1]) for ts in tile_strata])
                min_coverage = 0
                if len(tile_strata) == 0:
                    pass
                elif tile_strata[0][1] <= min_coverage:
                    tile_strata = [tile_strata[0]]
                else:
                    tile_strata = [x for x in tile_strata if x[1] > min_coverage]
                print(tile.id, [(ts[0].id, ts[1]) for ts in tile_strata])

                print("Strata for tile {}: {}".format(tile.id, map(lambda x: x[0].id, tile_strata)))

                for (stratum, _) in tile_strata:
                    stratum.tiles.append(tile)
                    tile.strata.append(stratum)

            for tile in self.tiles:
                geom = tile.footprint.Clone()

                for stratum in tile.strata:
                    if stratum != tile.main_stratum:
                        geom = geom.Difference(stratum.extent.GetGeometryRef())

                tile.main_mask = geom
        else:
            filtered_polygons = filter_polygons(self.args.ref, self.site_footprint_wgs84, self.args.outdir)
            stratum = Stratum(0, filtered_polygons, None)
            stratum.tiles = self.tiles;
            self.strata = [stratum]
            for tile in self.tiles:
                tile.strata = [stratum]
                tile.main_stratum = stratum

            for tile in self.tiles:
                tile.main_mask = tile.footprint

    def merge_strata_for_tile(self, tile, inputs, output, compression=None):
        files = []
        for idx, stratum in enumerate(tile.strata):
            input = inputs[idx]
            area_mask_raster = self.get_output_path("classification-mask-{}-{}.tif", stratum.id, tile.id)

            files.append(input)
            files.append(area_mask_raster)

        step_args = ["otbcli_BandMath",
                        "-exp", build_merge_expression(len(tile.strata)),
                        "-out", format_otb_filename(output, compression='DEFLATE'), "int16"]
        step_args += ["-il"] + files

        run_step(Step("BandMath_" + str(tile.id), step_args))

        run_step(Step("Nodata_" + str(tile.id),
                            ["gdal_edit.py",
                                "-a_nodata", -10000,
                                output]))

    def merge_classification_outputs(self):
        for tile in self.tiles:
            print("Merging classification results for tile:", tile.id)

            tile_crop_map = self.get_tile_classification_output(tile)
            inputs = [self.get_stratum_tile_classification_output(stratum, tile) for stratum in tile.strata]

            self.merge_strata_for_tile(tile, inputs, tile_crop_map, compression='DEFLATE')

    def rasterize_tile_mask(self, stratum, tile):
        tile_mask = self.get_output_path("tile-mask-{}-{}.shp", stratum.id, tile.id)
        stratum_tile_mask = self.get_stratum_tile_mask(stratum, tile)

        if stratum == tile.main_stratum:
            classification_mask = tile.main_mask
        else:
            classification_mask = stratum.extent.GetGeometryRef().Intersection(tile.footprint)

        save_to_shp(tile_mask, classification_mask)

        tile_reference_raster = get_reference_raster(tile.descriptors[0])
        print("Reference raster for tile:", tile_reference_raster)

        run_step(Step("Rasterize mask",
                            ["otbcli_Rasterization",
                            "-in", tile_mask,
                            "-im", tile_reference_raster,
                            "-out", format_otb_filename(stratum_tile_mask, compression='DEFLATE'), "uint8",
                            "-mode", "binary"]))

    def get_output_path(self, fmt, *args):
        return os.path.join(self.args.outdir, fmt.format(*args))

    def get_stratum_tile_mask(self, stratum, tile):
        return self.get_output_path("classification-mask-{}-{}.tif", stratum.id, tile.id)


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

        tile_crop_type_map_uncut = self.get_stratum_tile_classification_output(stratum, tile)
        stratum_tile_mask = self.get_stratum_tile_mask(stratum, tile)

        step_args = ["otbcli", "CropTypeImageClassifier", self.args.buildfolder,
                        "-mission", self.args.mission,
                        "-pixsize", self.args.pixsize,
                        "-indays", area_days,
                        "-singletile", "true" if len(stratum.tiles) == 1 else "false",
                        "-bv", -10000,
                        "-model", area_model,
                        "-out", format_otb_filename(tile_crop_type_map_uncut, compression='DEFLATE')]
        step_args += ["-il"] + tile.descriptors
        step_args += ["-sp"] + self.args.sp
        if self.args.classifier == "svm" or self.args.normalize:
            step_args += ["-outstat", area_statistics]

        step_args += ["-mask", stratum_tile_mask]

        run_step(Step("ImageClassifier_{}_{}".format(stratum.id, tile.id), step_args, retry=True))

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


class CropMaskProcessor(ProcessorBase):
    def create_context(self):
        parser = argparse.ArgumentParser(description='Crop Mask Processor')

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
        parser.add_argument('-mode', help='The execution mode',
                            required=False, choices=['train', 'classify', 'validate'], default=None)
        self.args = parser.parse_args()

        self.args.tmpfolder = self.args.outdir
        self.crop_features = load_features(self.args.ref)

    def after_prepare_tile(self, tile):
        pass

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

        tile_crop_type_map_uncut = self.get_stratum_tile_classification_output(stratum, tile)
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
                        "-out", tile_crop_type_map_uncut]
        step_args += ["-il"] + tile.descriptors
        step_args += ["-sp"] + self.args.sp
        if self.args.classifier == "svm" or self.args.normalize:
            step_args += ["-outstat", area_statistics]

        step_args += ["-mask", stratum_tile_mask]

        run_step(Step("ImageClassifier_{}_{}".format(stratum.id, tile.id), step_args, retry=True))

    def postprocess_tile(self, tile):
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

        step_args = ["otbcli", "MajorityVoting", self.args.buildfolder,
                     "-nodatasegvalue", 0,
                     "-nodataclassifvalue", "-10000",
                     "-minarea", self.args.minarea,
                     "-inclass", tile_crop_mask,
                     "-inseg", tile_segmentation_merged,
                     "-rout", format_otb_filename(tile_segmented, compression='DEFLATE')]
        run_step(Step("Majority voting " + tile.id, step_args))

    def validate(self, context):
        files = []
        for tile in self.tiles:
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
                            "-root", "CropType",
                            "-confmat", area_statistics,
                            "-quality", area_quality_metrics,
                            "-out", area_validation_metrics_xml]
            run_step(Step("XMLStatistics_" + str(stratum.id), step_args))

        step_args = ["otbcli", "ProductFormatter", self.args.buildfolder,
                        "-destroot", self.args.targetfolder,
                        "-fileclass", "SVT1",
                        "-level", "L4A",
                        "-baseline", "01.00",
                        "-siteid", self.args.siteid,
                        "-processor", "cropmask"]

        step_args.append("-processor.cropmask.file")
        for tile in self.tiles:
            tile_segmented = self.get_output_path("crop-mask-segmented-{}.tif", tile.id)

            step_args.append("TILE_" + tile.id)
            step_args.append(tile_segmented)

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

processor = CropTypeProcessor()
# processor = CropMaskProcessor()
processor.execute()
