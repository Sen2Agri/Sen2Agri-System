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
import pipes
import subprocess
from sys import argv
import traceback
import datetime

#name:      The name of the step as it will be dispayed on screen
#args:      The external process that must be invoked and it arguments
#kwargs:    The list containing the arguments with default values.
#           Possible values are:
#               outf - the file where the output is redirected to (default: "")
#               skip - if True then this step is not executed (default: False)
# rmfiles - the list of files to remove after the execution of the process
# ends (default: [])


def executeStep(name, *args, **kwargs):
    # Check if the output should be redirected to a file
    outf = kwargs.get("outf", "")
    skip = kwargs.get("skip", False)
    rmfiles = kwargs.get("rmfiles", [])
    retry = kwargs.get("retry", False)

    # Get the start date
    startTime = datetime.datetime.now()

    #Check if the step should be skiped
    if skip:
        print("Skipping " + name + " at " + str(startTime))
        return

    retries = 5 if retry else 1
    while retries > 0:
        retries -= 1

        # Print start message
        print("Executing " + name + " at " + str(startTime))

        # Build the command line and print it to the output
        args = map(str, args)
        cmdLine = " ".join(map(pipes.quote, args))
        if len(outf):
            cmdLine = cmdLine + " > " + pipes.quote(outf)
        print(cmdLine)

        #invoke the external process
        if len(outf):
            fil = open(outf, "w")
            result = subprocess.call(args, stdout=fil)
        else:
            result = subprocess.call(args)

        #Get the end time
        endTime = datetime.datetime.now()

        # Check for errors
        if result != 0:
             print("Error running " + name + " at " + str(endTime) + ". The call returned " + str(result))
             if retries == 0:
                raise Exception("Error running " + name, result)

        # Remove intermediate files if needed
        for fil in rmfiles:
            os.remove(fil)

        # Print end message
        if result == 0:
            print(name + " done at " + str(endTime) + ". Duration: " + str(endTime - startTime))
            break
#end executeStep

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
    def __init__(self, id, extent):
        self.id = id
        self.extent = extent
        self.tiles = []
        self.training_tiles = []

class Tile(object):
    def __init__(self, id, descriptors):
        self.id = id
        self.descriptors = descriptors
        self.footprint = None
        self.area = None
        self.strata = []
        self.main_stratum = None
        self.main_mask = None


def split_features(stratum, data, out_folder):
    driver = ogr.GetDriverByName('ESRI Shapefile')

    data_ds = ogr.Open(data, 0)
    if data_ds is None:
        raise Exception("Could not open feature dataset", data)

    data_layer = data_ds.GetLayer()
    data_layer_def = data_layer.GetLayerDefn()
    data_srs = data_layer.GetSpatialRef()

    out_name = os.path.join(out_folder, "features-{}.shp".format(stratum.id))

    if os.path.exists(out_name):
        driver.DeleteDataSource(out_name)

    print("Writing {}".format(out_name))

    out_ds = driver.CreateDataSource(out_name)
    if out_ds is None:
        raise Exception("Could not create output dataset", out_name)

    out_layer = out_ds.CreateLayer('features', srs=data_srs,
                                    geom_type=ogr.wkbMultiPolygon)
    out_layer_def = out_layer.GetLayerDefn()

    for i in xrange(data_layer_def.GetFieldCount()):
        out_layer.CreateField(data_layer_def.GetFieldDefn(i))

    wgs84_srs = osr.SpatialReference()
    wgs84_srs.ImportFromEPSG(4326)

    area_transform = osr.CoordinateTransformation(wgs84_srs, data_srs)
    area_geom_reprojected = stratum.extent.Clone()
    area_geom_reprojected.Transform(area_transform)

    out_features = 0
    data_layer.ResetReading()
    data_layer.SetSpatialFilter(area_geom_reprojected)
    for region in data_layer:
        region_geom = region.GetGeometryRef()

        out_feature = ogr.Feature(out_layer_def)
        out_features += 1

        for i in xrange(out_layer_def.GetFieldCount()):
            out_feature.SetField(out_layer_def.GetFieldDefn(
                i).GetNameRef(), region.GetField(i))

        out_feature.SetGeometry(region_geom)
        out_layer.CreateFeature(out_feature)

    return out_features

def load_strata(areas, site_footprint_wgs84):
    wgs84_srs = osr.SpatialReference()
    wgs84_srs.ImportFromEPSG(4326)

    area_ds = ogr.Open(areas, 0)
    if area_ds is None:
        raise Exception("Could not open stratum dataset", areas)

    area_layer = area_ds.GetLayer()
    area_layer_def = area_layer.GetLayerDefn()
    area_srs = area_layer.GetSpatialRef()
    area_wgs84_transform = osr.CoordinateTransformation(area_srs, wgs84_srs)

    result = []
    for area in area_layer:
        area_id = area.GetField('ID')
        area_geom_wgs84 = area.GetGeometryRef().Clone()
        area_geom_wgs84.Transform(area_wgs84_transform)
        area_geom_wgs84 = area_geom_wgs84.Intersection(site_footprint_wgs84)
        if area_geom_wgs84.GetArea() == 0:
            print("Stratum {} does not intersect the site extent".format(area_id))
            continue

        area_feature_wgs84 = ogr.Feature(area_layer_def)
        area_feature_wgs84.SetGeometry(area_geom_wgs84)

        result.append(Stratum(area_id, area_feature_wgs84.GetGeometryRef().Clone()))

    # driver = ogr.GetDriverByName('ESRI Shapefile')

    # if os.path.exists(relabelled_areas):
    #     driver.DeleteDataSource(relabelled_areas)

    # out_ds = driver.CreateDataSource(relabelled_areas)
    # if out_ds is None:
    #     raise Exception(
    #         "Could not create output shapefile", relabelled_areas)

    # out_layer = out_ds.CreateLayer('strata', srs=area_srs, geom_type=area_layer_def.GetGeomType())
    # field_index = ogr.FieldDefn("Index", ogr.OFTInteger)
    # field_index.SetWidth(3)
    # out_layer.CreateField(field_index)
    # field_id = ogr.FieldDefn("Id", ogr.OFTInteger)
    # field_id.SetWidth(3)
    # out_layer.CreateField(field_id)
    # out_layer_def = out_layer.GetLayerDefn()

    # feature = ogr.Feature(out_layer_def)
    # feature.SetGeometry(geom)
    # out_layer.CreateFeature(feature)

    # result.sort(key=lambda stratum: stratum.id)

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
            exit_status = 0
            exit_requested = False
            context = self.create_context()

            self.load_tiles()

            metadata_file = self.get_metadata_file()

            metadata = self.build_metadata()
            metadata.write(metadata_file, xml_declaration=True, encoding='UTF-8', pretty_print=True)

            if self.args.mode is None or self.args.mode == 'prepare-site':
                self.prepare_site()

            if self.args.mode is None or self.args.mode == 'prepare-tiles':
                for tile in self.tiles:
                    if self.args.tile_filter and tile.id not in self.args.tile_filter:
                        print("Skipping pre-processing for tile {} due to tile filter".format(tile.id))
                        continue

                    self.prepare_tile(tile)

            if self.args.mode is None or self.args.mode == 'train':
                for stratum in self.strata:
                    if self.args.stratum_filter and stratum.id not in self.args.stratum_filter:
                        print("Skipping training for stratum {} due to stratum filter".format(stratum.id))
                        continue

                    print("Building model for stratum:", stratum.id)
                    self.train_stratum(stratum)

            if self.args.mode is None or self.args.mode == 'classify':
                for stratum in self.strata:
                    if self.args.stratum_filter and stratum.id not in self.args.stratum_filter:
                        print("Skipping classification for stratum {} due to stratum filter".format(stratum.id))
                        continue

                    print("Applying model for stratum:", stratum.id)

                    for tile in stratum.tiles:
                        if self.args.tile_filter and tile.id not in self.args.tile_filter:
                            print("Skipping classification for tile {} due to tile filter".format(tile.id))
                            continue

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

            if self.args.mode is None or self.args.mode == 'merge':
                self.merge_classification_outputs()

            if self.args.mode is None or self.args.mode == 'postprocess-tiles':
                for tile in self.tiles:
                    if self.args.tile_filter and tile.id not in self.args.tile_filter:
                        print("Skipping post-processing for tile {} due to tile filter".format(tile.id))
                        continue

                    self.postprocess_tile(tile)

                if self.args.skip_quality_flags:
                    print("Skipping quality flags extraction")
                else:
                    for tile in self.tiles:
                        if self.args.tile_filter and tile.id not in self.args.tile_filter:
                            print("Skipping quality flags extraction for tile {} due to tile filter".format(tile.id))
                            continue

                        self.compute_quality_flags(tile)

            if self.args.mode is None or self.args.mode == 'validate':
                self.validate(context)
        except SystemExit:
            exit_requested = True
            pass
        except:
            traceback.print_exc()
            exit_status = 1
        finally:
            if not exit_requested:
                end_time = datetime.datetime.now()
                print("Processor finished in", str(end_time - start_time))

            sys.exit(exit_status)

    def compute_quality_flags(self, tile):
        tile_quality_flags = self.get_output_path("status_flags_{}.tif", tile.id)

        step_args = ["otbcli", "QualityFlagsExtractor", self.args.buildfolder,
                        "-mission", self.args.mission,
                        "-out", format_otb_filename(tile_quality_flags, compression='DEFLATE'),
                        "-pixsize", self.args.pixsize]
        step_args += ["-il"] + tile.descriptors

        run_step(Step("QualityFlags_" + str(tile.id), step_args))

    def prepare_site(self):
        pass

    def prepare_tile(self, tile):
        pass

    def postprocess_tile(self, tile):
        pass

    def load_tiles(self):
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
            self.strata = load_strata(self.args.strata, self.site_footprint_wgs84)
            if len(self.strata) == 0:
                print("No training data found inside any of the strata, exiting")
                sys.exit(1)

            strata_relabelled = self.get_output_path("strata_relabelled.shp")

            step_args = ["ogr2ogr",
                            "-overwrite",
                            strata_relabelled,
                            self.args.strata]
            run_step(Step("Duplicate stratum list", step_args))

            step_args = ["ogrinfo",
                            "-sql", "alter table strata_relabelled add column index integer",
                            strata_relabelled]
            run_step(Step("Add index column", step_args))

            step_args = ["ogrinfo",
                            "-dialect", "sqlite",
                            "-sql", "update strata_relabelled set `index` = (select count(*) from strata_relabelled sr where sr.id < strata_relabelled.id)",
                            strata_relabelled]
            run_step(Step("Re-number strata", step_args))

            tile_best_weights = [0.0] * len(self.tiles)

            for tile in self.tiles:
                tile_best_weight = 0.0
                tile_strata = []

                for stratum in self.strata:
                    weight = stratum.extent.Intersection(tile.footprint).Area() / tile.area

                    if weight > tile_best_weight:
                        tile_best_weight = weight
                        tile.main_stratum = stratum

                    tile_strata.append((stratum, weight))

                tile_strata.sort(key=lambda x: -x[1])

                print(tile.id, [(ts[0].id, ts[1]) for ts in tile_strata])
                if len(tile_strata) == 0:
                    pass
                elif tile_strata[0][1] <= self.args.min_coverage:
                    tile_training_strata = [tile_strata[0]]
                else:
                    tile_training_strata = [x for x in tile_strata if x[1] > self.args.min_coverage]
                print(tile.id, [(ts[0].id, ts[1]) for ts in tile_strata])

                print("Strata for tile {}: {}".format(tile.id, map(lambda x: x[0].id, tile_strata)))
                print("Training strata for tile {}: {}".format(tile.id, map(lambda x: x[0].id, tile_training_strata)))

                if len(tile_training_strata) == 0:
                    pass

                for (stratum, _) in tile_strata:
                    stratum.tiles.append(tile)
                    tile.strata.append(stratum)

                for (stratum, _) in tile_training_strata:
                    stratum.training_tiles.append(tile)

            for tile in self.tiles:
                geom = tile.footprint.Clone()

                for stratum in tile.strata:
                    if stratum != tile.main_stratum:
                        geom = geom.Difference(stratum.extent)

                tile.main_mask = geom
        else:
            stratum = Stratum(0, self.site_footprint_wgs84)
            stratum.tiles = self.tiles
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
            classification_mask = stratum.extent.Intersection(tile.footprint)

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

    def get_metadata_file(self):
        return self.get_output_path("metadata.xml")
