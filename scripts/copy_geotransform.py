#!/usr/bin/env python
import argparse
import gdal

parser = argparse.ArgumentParser(description="Copies the geotransform from one file to another")
parser.add_argument('source', help="Source dataset")
parser.add_argument('target', help="Target dataset")

args = parser.parse_args()

source_ds = gdal.Open(args.source, gdal.gdalconst.GA_ReadOnly)
target_ds = gdal.Open(args.target, gdal.gdalconst.GA_Update)

if not source_ds:
    raise Exception("Could not open source dataset", args.source)
if not target_ds:
    raise Exception("Could not open target dataset", args.target)

srs = source_ds.GetProjectionRef()
if srs == "":
    srs = source_ds.GetMetadata("GEOLOCATION").get("SRS")

if srs is not None and srs != "":
    target_ds.SetProjection(srs)

target_ds.SetGeoTransform(source_ds.GetGeoTransform())
