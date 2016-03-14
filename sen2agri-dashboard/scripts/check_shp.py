#!/usr/bin/env python
from __future__ import print_function

import argparse
import sys
from osgeo import ogr
from osgeo import osr


def main():
    parser = argparse.ArgumentParser(
        description="Checks the validity of vector data.")
    parser.add_argument('input', help="the input data source")

    args = parser.parse_args()

    dataSource = ogr.Open(args.input, 0)

    if dataSource is None:
        print("Could not open {}".format(args.input))
        return 1

    layer = dataSource.GetLayer()
    featureCount = layer.GetFeatureCount()
    print("{} feature(s) found".format(featureCount))

    spatialRef = osr.SpatialReference()
    spatialRef.ImportFromEPSG(4326)
    transform = osr.CoordinateTransformation(layer.GetSpatialRef(), spatialRef)

    if featureCount != 1:
        union = ogr.Geometry(ogr.wkbMultiPolygon)

    for feature in layer:
        geometry = feature.GetGeometryRef()
        if not geometry.IsValid():
            print("Invalid feature: {}".format(geometry.ExportToWkt()))
            return 2

        geometry.Transform(transform)
        if featureCount == 1:
            union = geometry
        else:
            union.AddGeometry(geometry)

    print("Union: {}".format(union.ExportToWkt()))
    return 0

if __name__ == '__main__':
    sys.exit(main())
