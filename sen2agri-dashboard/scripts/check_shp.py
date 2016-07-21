#!/usr/bin/env python
from __future__ import print_function

import argparse
import itertools
import sys
from osgeo import ogr
from osgeo import osr


def main():
    parser = argparse.ArgumentParser(
        description="Checks the validity of vector data.")
    parser.add_argument('-b', '--bounds', help="display input bounds", action='store_true')
    parser.add_argument('input', help="the input data source")

    args = parser.parse_args()

    dataSource = ogr.Open(args.input, 0)

    if dataSource is None:
        print("Could not open {}".format(args.input))
        return 1

    layer = dataSource.GetLayer()
    featureCount = layer.GetFeatureCount()
    print("{} feature(s) found".format(featureCount))

    id_field_idx = layer.FindFieldIndex("ID", False)
    if id_field_idx == -1:
        id_field_idx = None

    code_field_idx = layer.FindFieldIndex("CODE", False)
    if code_field_idx == -1:
        code_field_idx = None

    spatialRef = osr.SpatialReference()
    spatialRef.ImportFromEPSG(4326)
    transform = osr.CoordinateTransformation(layer.GetSpatialRef(), spatialRef)

    if args.bounds and featureCount != 1:
        union = ogr.Geometry(ogr.wkbMultiPolygon)

    features = []
    for feature in layer:
        geometry = feature.GetGeometryRef()
        if not geometry.IsValid():
            if id_field_idx is not None:
                feature_id = feature.GetField(id_field_idx)
                print("Invalid feature (ID={}): {}".format(feature_id, geometry.ExportToWkt()))
            else:
                print("Invalid feature: {}".format(geometry.ExportToWkt()))

            return 2

        features.append(feature)

        geometry_clone = geometry.Clone()
        geometry_clone.Transform(transform)

        if args.bounds:
            if featureCount == 1:
                union = geometry_clone
            else:
                union.AddGeometry(geometry_clone)

    if not args.bounds:
        for f0 in features:
            g0 = f0.GetGeometryRef()
            layer.SetSpatialFilter(g0)
            for f1 in layer:
                g1 = f1.GetGeometryRef()
                if g0.Overlaps(g1):
                    if code_field_idx is not None:
                        c0, c1 = f0.GetField(code_field_idx), f1.GetField(code_field_idx)
                    else:
                        c0, c1 = None, None

                    if c0 != c1:
                        if id_field_idx is not None:
                            i0 = f0.GetField(id_field_idx)
                            i1 = f1.GetField(id_field_idx)
                            print("Overlapping features (ID={} and ID={}): {} and {}".format(i0, i1, g0, g1))
                        else:
                            print("Overlapping features: {} and {}".format(g0, g1))
                        return 3

    if args.bounds:
        print("Union: {}".format(union.ExportToWkt()))
    return 0

if __name__ == '__main__':
    sys.exit(main())
