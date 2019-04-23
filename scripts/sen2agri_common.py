#!/usr/bin/python

from __future__ import print_function

from collections import defaultdict
from osgeo import ogr, osr
import gdal
import os
import os.path
import glob
import re
import sys
import zipfile
import pipes
import resource
import subprocess
import traceback
import datetime
import multiprocessing.dummy

# name:      The name of the step as it will be dispayed on screen
# args:      The external process that must be invoked and it arguments
# kwargs:    The list containing the arguments with default values.
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

    # Check if the step should be skiped
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

        # invoke the external process
        if len(outf):
            fil = open(outf, "w")
            result = subprocess.call(args, stdout=fil)
        else:
            result = subprocess.call(args)

        # Get the end time
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
# end executeStep


def increase_rlimits():
    try:
        nofiles = resource.getrlimit(resource.RLIMIT_NOFILE)
        if nofiles[0] < nofiles[1]:
            print("Increasing NOFILE soft limit from {} to {}".format(nofiles[0], nofiles[1]))
            resource.setrlimit(resource.RLIMIT_NOFILE, (nofiles[1], nofiles[1]))
        else:
            print("NOFILE limit is set to {}".format(nofiles[0]))
    except:
        traceback.print_exc()


def expand_file_list(files):
    def expand(acc, file, dir):
        if file.startswith("@"):
            file_name = os.path.join(dir, file[1:])
            file_dir = os.path.dirname(file_name)
            with open(file_name, 'r') as f:
                for line in f:
                    expand(acc, line.rstrip("\n"), file_dir)
        else:
            acc.append(file)

    result = []
    for file in files:
        expand(result, file, ".")
    return result


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
        files = glob.glob(os.path.join(directory, "*_FRE_B4.tif"))
        if files:
            return files[0]
        else:
            files = glob.glob(os.path.join(directory, "*PENTE*"))
            if files:
                return files[0]
            else:
                files = glob.glob(os.path.join(directory, "*"))
                raise Exception("Unable to find a reference raster for MAJA or SPOT product", directory, files)
    if extension == ".hdr":
        dir = os.path.join(directory, parts[0] + ".DBL.DIR")
        files = glob.glob(os.path.join(dir, "*_FRE_R1.DBL.TIF"))
        if files:
            return files[0]
        files = glob.glob(os.path.join(dir, "*_FRE.DBL.TIF"))
        if files:
            return files[0]
        else:
            files = glob.glob(os.path.join(directory, "*"))
            raise Exception("Unable to find a reference raster for MACCS product", dir, files)

    raise Exception("Unable to determine product type", product)


class Mission(object):

    def __init__(self, value, name):
        self.value = value
        self.name = name


class Mission(object):
    SENTINEL = Mission(1, "SENTINEL")
    SPOT = Mission(2, "SPOT")
    LANDSAT = Mission(3, "LANDSAT")


def get_tile_id(product):
    file = os.path.basename(product)
    m = re.match("SPOT.+_([a-z0-9]+)\\.xml", file, re.IGNORECASE)
    if m:
        return (Mission.SPOT, m.group(1))

    m = re.match(".+_L8_(\d{3})_(\d{3}).hdr", file, re.IGNORECASE)
    if m:
        return (Mission.LANDSAT, m.group(1) + m.group(2))
    m = re.match("L8_.+_(\d{6})_\d{8}.HDR", file, re.IGNORECASE)
    if m:
        return (Mission.LANDSAT, m.group(1))

    m = re.match(
        "S2[a-z0-9]_[a-z0-9]+_[a-z0-9]+_[a-z0-9]+_([a-z0-9]+)_.+\\.HDR", file, re.IGNORECASE)
    if m:
        return (Mission.SENTINEL, m.group(1))
    m = re.match(
        "SENTINEL2[A-D]_.+_L2A_T([A-Z0-9]+)_.+_MTD_ALL\.xml", file, re.IGNORECASE)
    if m:
        return (Mission.SENTINEL, m.group(1))

    return None


class Stratum(object):

    def __init__(self, id, extent):
        self.id = id
        self.extent = extent
        self.tiles = []


class Descriptor(object):

    def __init__(self, path, mission):
        self.path = path
        self.mission = mission


class Tile(object):

    def __init__(self, id, footprint, footprint_wgs84, projection, descriptors, reference_raster):
        self.id = id
        self.footprint = footprint
        self.footprint_wgs84 = footprint_wgs84
        self.projection = projection
        self.descriptors = descriptors
        self.reference_raster = reference_raster
        self.strata = []

    def get_descriptor_paths(self):
        return map(lambda d: d.path, self.descriptors)

    def get_mission_descriptor_paths(self, mission):
        return [d.path for d in self.descriptors if d.mission == mission]


def load_lut(lut):
    r = []
    with open(lut, 'r') as f:
        for line in f:
            m = re.match("(-?\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s*#\s*(.*)", line)
            if m:
                value = m.group(1)
                red = m.group(2)
                green = m.group(3)
                blue = m.group(4)
                label = m.group(5)

                r.append((int(value), int(red), int(green), int(blue), label))
    return r


def save_lut(lut, file):
    with open(file, 'w') as f:
        f.write("INTERPOLATION:EXACT\n")
        for entry in lut:
            f.write("{},{},{},{},255,{}\n".format(*entry))


def prepare_lut(data, lut):
    default_lut = load_lut(lut)
    if data is None:
        return default_lut

    default_lut_map = {}
    for entry in default_lut:
        if entry[0] != 0:
            default_lut_map[entry[0]] = entry[1:5]

    driver = ogr.GetDriverByName('ESRI Shapefile')

    data_ds = ogr.Open(data, 0)
    if data_ds is None:
        raise Exception("Could not open feature dataset", data)

    data_layer = data_ds.GetLayer()
    data_layer_def = data_layer.GetLayerDefn()
    data_srs = data_layer.GetSpatialRef()

    in_codes = {}
    for feature in data_layer:
        crop = feature.GetField("CROP")
        code = feature.GetField("CODE")
        lc = feature.GetField("LC")

        if crop == 1:
            in_codes[code] = lc

    lut_entries = []
    e = default_lut_map.pop(-10000, None)
    if e is not None:
        lut_entries.append((-10000,) + e)
    else:
        lut_entries.append((-10000, 255, 255, 255, "No crop/No data"))

    available_colors = []
    for code, lc in in_codes.iteritems():
        e = default_lut_map.pop(code, None)
        if e is not None:
            entry = (code, e[0], e[1], e[2], lc)
            lut_entries.append(entry)
            available_colors.append((entry[0], entry[1:]))

    for entry in lut_entries:
        in_codes.pop(entry[0], None)

    available_colors.sort(key=lambda e: e[0])
    palette_idx = len(available_colors)

    unused_colors = default_lut_map.items()
    unused_colors.sort(key=lambda e: e[0])
    available_colors.extend(unused_colors)

    remaining_codes = in_codes.items()
    remaining_codes.sort(key=lambda e: e[0])

    for code, lc in remaining_codes:
        _, e = available_colors[palette_idx]
        palette_idx += 1
        if palette_idx == len(available_colors):
            palette_idx = 0

        lut_entries.append((code,) + e[0:3] + (lc, ))

    lut_entries.sort(key=lambda e: e[0])
    return lut_entries


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

    result.sort(key=lambda stratum: stratum.id)

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

    extent = GetExtent(geo_transform, size_x, size_y)

    ring = ogr.Geometry(ogr.wkbLinearRing)
    ring.AddPoint_2D(extent[0][0], extent[0][1])
    ring.AddPoint_2D(extent[3][0], extent[3][1])
    ring.AddPoint_2D(extent[2][0], extent[2][1])
    ring.AddPoint_2D(extent[1][0], extent[1][1])
    ring.AddPoint_2D(extent[0][0], extent[0][1])

    geom = ogr.Geometry(ogr.wkbPolygon)
    geom.AddGeometry(ring)

    source_srs = osr.SpatialReference()
    source_srs.ImportFromWkt(dataset.GetProjection())
    target_srs = osr.SpatialReference()
    target_srs.ImportFromEPSG(4326)

    wgs84_extent = ReprojectCoords(extent, source_srs, target_srs)

    ring = ogr.Geometry(ogr.wkbLinearRing)
    ring.AddPoint_2D(wgs84_extent[0][0], wgs84_extent[0][1])
    ring.AddPoint_2D(wgs84_extent[3][0], wgs84_extent[3][1])
    ring.AddPoint_2D(wgs84_extent[2][0], wgs84_extent[2][1])
    ring.AddPoint_2D(wgs84_extent[1][0], wgs84_extent[1][1])
    ring.AddPoint_2D(wgs84_extent[0][0], wgs84_extent[0][1])

    geom_wgs84 = ogr.Geometry(ogr.wkbPolygon)
    geom_wgs84.AddGeometry(ring)

    return (geom, geom_wgs84, source_srs)


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


def format_otb_filename(file, compression=None):
    if compression is not None:
        file += "?gdal:co:COMPRESS=" + compression
    return file


def build_descriptor_list(mission, products):
    return map(lambda p: Descriptor(p, mission), products)


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

            increase_rlimits()

            tile_threads = self.args.tile_threads_hint

            num_cpus = multiprocessing.cpu_count()
            if self.args.max_parallelism is None:
                # note that this doesn't take CPU affinity (/proc/self/cpuset) into account
                parallelism = num_cpus / tile_threads
                if parallelism == 0:
                    parallelism = 1
            else:
                parallelism = self.args.max_parallelism

            if parallelism > len(self.tiles):
                parallelism = len(self.tiles)

            adj_threads = num_cpus / parallelism
            if adj_threads > tile_threads:
                tile_threads = adj_threads

            print("Processing {} tiles at once".format(parallelism))
            print("Using {} threads for each tile".format(tile_threads))
            os.environ["ITK_USE_THREADPOOL"] = str(1)
            os.environ["ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS"] = str(tile_threads)

            if self.args.mode is None or self.args.mode == 'prepare-site':
                self.prepare_site()

            if self.args.mode is None or self.args.mode == 'prepare-tiles':
                pool = multiprocessing.dummy.Pool(parallelism)
                pool.map(self.internal_prepare_tile_high_par, self.tiles)
                pool.close()
                pool.join()

                # HACK: the high/low-parallelism split is only because the trimming
                #       step is not quite memory-friendly
                pool = multiprocessing.dummy.Pool(2 if parallelism > 1 else 1)
                pool.map(self.internal_prepare_tile_low_par, self.tiles)
                pool.close()
                pool.join()

            if self.args.mode is None or self.args.mode == 'train':
                pool = multiprocessing.dummy.Pool(parallelism)
                pool.map(self.internal_train_stratum, self.strata)
                pool.close()
                pool.join()

            if self.args.mode is None or self.args.mode == 'classify':
                pool = multiprocessing.dummy.Pool(parallelism)
                pool.map(self.internal_classify_tile, self.tiles)
                pool.close()
                pool.join()

            if self.args.mode is None or self.args.mode == 'postprocess-tiles':
                pool = multiprocessing.dummy.Pool(parallelism)
                pool.map(self.internal_postprocess_tile, self.tiles)
                pool.close()
                pool.join()

            if self.args.mode is None or self.args.mode == 'compute-quality-flags':
                if self.args.skip_quality_flags:
                    print("Skipping quality flags extraction")
                else:
                    pool = multiprocessing.dummy.Pool(parallelism)
                    pool.map(self.internal_compute_quality_flags, self.tiles)
                    pool.close()
                    pool.join()

            if self.args.mode is None or self.args.mode == 'validate':
                if self.args.refp is not None:
                    self.create_in_situ_archive()

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

    def internal_prepare_tile_high_par(self, tile):
        if self.args.tile_filter and tile.id not in self.args.tile_filter:
            print("Skipping tile preparation (1/2) for tile {} due to tile filter".format(tile.id))
            return

        print("Performing tile preparation (1/2) for tile:", tile.id)
        self.prepare_tile_high_par(tile)

    def internal_prepare_tile_low_par(self, tile):
        if self.args.tile_filter and tile.id not in self.args.tile_filter:
            print("Skipping tile preparation (2/2) for tile {} due to tile filter".format(tile.id))
            return

        print("Performing tile preparation (2/2) for tile:", tile.id)
        self.prepare_tile_low_par(tile)

    def internal_train_stratum(self, stratum):
        if self.args.stratum_filter and stratum.id not in self.args.stratum_filter:
            print("Skipping training for stratum {} due to stratum filter".format(stratum.id))
            return

        print("Building model for stratum:", stratum.id)
        self.train_stratum(stratum)

    def internal_classify_tile(self, tile):
        if self.args.tile_filter and tile.id not in self.args.tile_filter:
            print("Skipping classification for tile {} due to tile filter".format(tile.id))
            return

        print("Performing classification for tile:", tile.id)
        self.classify_tile(tile)

    def internal_postprocess_tile(self, tile):
        if self.args.tile_filter and tile.id not in self.args.tile_filter:
            print("Skipping post-processing for tile {} due to tile filter".format(tile.id))
            return

        print("Performing post-processing for tile:", tile.id)
        self.postprocess_tile(tile)

    def internal_compute_quality_flags(self, tile):
        if self.args.tile_filter and tile.id not in self.args.tile_filter:
            print("Skipping quality flags extraction for tile {} due to tile filter".format(tile.id))
            return

        print("Computing quality flags for tile:", tile.id)
        self.compute_quality_flags(tile)

    def compute_quality_flags(self, tile):
        tile_quality_flags = self.get_output_path("status_flags_{}.tif", tile.id)

        step_args = ["otbcli", "QualityFlagsExtractor", self.args.buildfolder,
                     "-progress", "false",
                     "-mission", self.args.mission.name,
                     "-out", format_otb_filename(tile_quality_flags, compression='DEFLATE'),
                     "-pixsize", self.args.pixsize]
        step_args += ["-il"] + tile.get_descriptor_paths()

        run_step(Step("QualityFlags_" + str(tile.id), step_args))

    def prepare_site(self):
        if self.args.lut is not None:
            qgis_lut = self.get_output_path("qgis-color-map.txt")

            lut = prepare_lut(self.args.refp, self.args.lut)
            save_lut(lut, qgis_lut)

    def prepare_tile_high_par(self, tile):
        pass

    def prepare_tile_low_par(self, tile):
        pass

    def postprocess_tile(self, tile):
        pass

    def load_tiles(self):
        self.args.input = expand_file_list(self.args.input)

        main_mission = None
        mission_products = defaultdict(lambda: defaultdict(list))
        for product in self.args.input:
            (mission, tile_id) = get_tile_id(product)
            if mission is None:
                raise Exception("Unable to determine product type", product)

            mission_products[mission][tile_id].append(product)
            if main_mission is None:
                main_mission = mission
            elif main_mission.value > mission.value:
                main_mission = mission

        self.args.mission = main_mission
        print("Main mission: {}".format(main_mission.name))

        self.tiles = []
        for (tile_id, products) in mission_products[main_mission].iteritems():
            raster = get_reference_raster(products[0])
            (tile_footprint, tile_footprint_wgs84,
             tile_projection) = get_raster_footprint(raster)

            tile = Tile(tile_id, tile_footprint, tile_footprint_wgs84,
                        tile_projection, build_descriptor_list(main_mission, products), raster)

            if self.args.tile_filter is None or tile_id in self.args.tile_filter:
                self.tiles.append(tile)

        for (mission, mission_tiles) in mission_products.iteritems():
            if mission != main_mission:
                for (tile_id, products) in mission_tiles.iteritems():
                    raster = get_reference_raster(products[0])
                    (_, tile_footprint_wgs84, _) = get_raster_footprint(raster)

                    for main_tile in self.tiles:
                        intersection = tile_footprint_wgs84.Intersection(
                            main_tile.footprint_wgs84)
                        if intersection.GetArea() > 0:
                            print("Tile {} intersects main tile {}".format(
                                tile_id, main_tile.id))

                            main_tile.descriptors += build_descriptor_list(mission, products)

        self.tiles.sort(key=lambda t: t.id)

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
            self.site_footprint_wgs84 = self.site_footprint_wgs84.Union(tile.footprint_wgs84)

        if not self.single_stratum:
            if self.args.stratum_filter is not None:
                strata_ids_str = map(str, self.args.stratum_filter)
                filtered_strata = "filtered-strata-{}.shp".format("-".join(strata_ids_str))
                self.args.filtered_strata = self.get_output_path(filtered_strata)

                run_step(Step("Filter strata",
                              ["ogr2ogr",
                               "-where", "id in ({})".format(", ".join(strata_ids_str)),
                               self.args.filtered_strata, self.args.strata]))
            else:
                self.args.filtered_strata = self.args.strata

            self.strata = load_strata(self.args.filtered_strata, self.site_footprint_wgs84)
            if len(self.strata) == 0:
                print("No training data found inside any of the strata, exiting")
                sys.exit(1)

            for tile in self.tiles:
                for stratum in self.strata:
                    if stratum.extent.Intersection(tile.footprint_wgs84).Area() > 0 and (self.args.stratum_filter is None or stratum.id in self.args.stratum_filter):
                        tile.strata.append(stratum)
                        stratum.tiles.append(tile)

                print("Strata for tile {}: {}".format(tile.id, map(lambda s: s.id, tile.strata)))

        else:
            stratum = Stratum(0, self.site_footprint_wgs84)
            stratum.tiles = self.tiles
            self.strata = [stratum]
            for tile in self.tiles:
                tile.strata = [stratum]

    def rasterize_tile_mask(self, stratum, tile):
        tile_mask = self.get_output_path("tile-mask-{}-{}.shp", stratum.id, tile.id)
        stratum_tile_mask = self.get_stratum_tile_mask(stratum, tile)

        classification_mask = stratum.extent.Intersection(tile.footprint_wgs84)

        save_to_shp(tile_mask, classification_mask)

        print("Reference raster for tile:", tile.reference_raster)

        run_step(Step("Rasterize mask",
                      ["otbcli_Rasterization",
                       "-progress", "false",
                       "-in", tile_mask,
                       "-im", tile.reference_raster,
                       "-out", format_otb_filename(stratum_tile_mask, compression='DEFLATE'), "uint8",
                       "-mode", "binary"]))

    def create_in_situ_archive(self):
        with zipfile.ZipFile(self.get_in_situ_data_file(), 'w', zipfile.ZIP_DEFLATED) as archive:
            for stratum in self.strata:
                area_training_polygons = self.get_output_path("training_polygons-{}.shp", stratum.id)
                area_validation_polygons = self.get_output_path("validation_polygons-{}.shp", stratum.id)

                for ext in ['.shp', '.shx', '.dbf', '.prj']:
                    file = os.path.splitext(area_training_polygons)[0] + ext
                    archive.write(file, os.path.basename(file))
                    file = os.path.splitext(area_validation_polygons)[0] + ext
                    archive.write(file, os.path.basename(file))

    def get_output_path(self, fmt, *args):
        return os.path.join(self.args.outdir, fmt.format(*args))

    def get_stratum_tile_mask(self, stratum, tile):
        return self.get_output_path("classification-mask-{}-{}.tif", stratum.id, tile.id)

    def get_metadata_file(self):
        return self.get_output_path("metadata.xml")

    def get_in_situ_data_file(self):
        return self.get_output_path("polygons.zip")
