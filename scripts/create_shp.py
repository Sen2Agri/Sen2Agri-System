#!/usr/bin/env python
from __future__ import print_function

import argparse
from osgeo import osr
from osgeo import ogr
import os.path
import sys


def main():
    parser = argparse.ArgumentParser(
        description="Creates a vector data file from WKT geometry")
    parser.add_argument('-l', '--layer', required=True,
                        help="layer name")
    parser.add_argument('-o', '--output', required=True,
                        help="output file name")
    parser.add_argument('-s', '--srs', required=True, help="output SRS")
    parser.add_argument('wkt', help="geometry Well-Known Text")
    args = parser.parse_args()

    driver = ogr.GetDriverByName('ESRI Shapefile')

    if os.path.exists(args.output):
        driver.DeleteDataSource(args.output)

    data_source = driver.CreateDataSource(args.output)
    if data_source is None:
        print("Could not create {}".format(args.output))
        return 1

    srs = osr.SpatialReference()
    srs.SetFromUserInput(args.srs)

    geom = ogr.CreateGeometryFromWkt(args.wkt)
    layer = data_source.CreateLayer(args.layer, srs, geom.GetGeometryType())
    feature = ogr.Feature(layer.GetLayerDefn())
    feature.SetGeometry(geom)
    layer.CreateFeature(feature)

    return 0

if __name__ == '__main__':
    sys.exit(main())
