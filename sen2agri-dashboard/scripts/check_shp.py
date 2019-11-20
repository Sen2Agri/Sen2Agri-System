#!/usr/bin/env python
from __future__ import print_function

import argparse
import sys
from osgeo import ogr
from osgeo import osr


ERR_NONE = 0
ERR_IO = 1
ERR_BAD_GEOMETRY = 2
ERR_OVERLAPPING_GEOMETRY = 3
ERR_COMPLEX_GEOMETRY = 4
ERR_BAD_FILE = 5


def get_geometry_size(geom):
    num = geom.GetPointCount()
    for i in range(0, geom.GetGeometryCount()):
        num += get_geometry_size(geom.GetGeometryRef(i))
    return num


def main():
    parser = argparse.ArgumentParser(description="Checks the validity of vector data.")
    parser.add_argument(
        "-b", "--bounds", help="display input bounds", action="store_true"
    )
    parser.add_argument("-f", "--fix", help="try to fix dataset", action="store_true")
    parser.add_argument("input", help="the input data source")

    args = parser.parse_args()

    dataSource = ogr.Open(args.input, 1 if args.fix else 0)

    if dataSource is None:
        print("Could not open {}".format(args.input))
        return ERR_IO

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
    layer_srs = layer.GetSpatialRef()
    if not layer_srs:
        print("Input has no spatial reference (projection)")
        return ERR_BAD_FILE

    transform = osr.CoordinateTransformation(layer_srs, spatialRef)

    if args.bounds and featureCount != 1:
        union = ogr.Geometry(ogr.wkbMultiPolygon)

    status = ERR_NONE
    features = []
    for feature in layer:
        geometry = feature.GetGeometryRef()
        if not geometry:
            if id_field_idx is not None:
                feature_id = feature.GetField(id_field_idx)
                print("Feature with no geometry: ID={}".format(feature_id))
            else:
                print("Feature with no geometry")
            if not args.fix:
                status = ERR_BAD_GEOMETRY
            else:
                layer.DeleteFeature(feature.GetFID())
            continue
        elif not geometry.IsValid():
            if id_field_idx is not None:
                feature_id = feature.GetField(id_field_idx)
                print("Invalid feature: ID={}".format(feature_id))
            else:
                print("Invalid feature: {}".format(geometry.ExportToWkt()))

            bad = True
            if args.fix:
                geometry = geometry.Buffer(0)
                if geometry.IsValid() and not geometry.IsEmpty():
                    feature.SetGeometryDirectly(geometry)
                    layer.SetFeature(feature)

                    if id_field_idx is not None:
                        print("Fixed feature: ID={}".format(feature_id))
                    else:
                        print("Fixed feature")

                    bad = False
                else:
                    print("Unable to fix feature")
                    layer.DeleteFeature(feature.GetFID())

            if bad:
                status = ERR_BAD_GEOMETRY

        features.append(feature)

        geometry_clone = geometry.Clone()
        geometry_clone.Transform(transform)

        if args.bounds:
            if featureCount == 1:
                union = geometry_clone
            else:
                union.AddGeometry(geometry_clone)

    if args.fix:
        layer.SyncToDisk()

    if not args.bounds:
        for f0 in features:
            g0 = f0.GetGeometryRef()
            layer.SetSpatialFilter(g0)
            for f1 in layer:
                g1 = f1.GetGeometryRef()

                if g0.Overlaps(g1):
                    intersection = g0.Intersection(g1)
                else:
                    intersection = None
                if (
                    intersection is not None
                    and not intersection.IsEmpty()
                    and intersection.Area() > 0
                ):
                    if code_field_idx is not None:
                        c0, c1 = (
                            f0.GetField(code_field_idx),
                            f1.GetField(code_field_idx),
                        )
                    else:
                        c0, c1 = None, None

                    if c0 != c1:
                        if id_field_idx is not None:
                            i0 = f0.GetField(id_field_idx)
                            i1 = f1.GetField(id_field_idx)
                            if i1 > i0:
                                continue

                            print(
                                "Overlapping features: ID={} and ID={}".format(i0, i1)
                            )
                        else:
                            print("Overlapping features: {} and {}".format(g0, g1))

                        bad = True
                        if args.fix:
                            g0, g1 = g0.Difference(g1), g1.Difference(g0)
                            f0.SetGeometryDirectly(g0)
                            f1.SetGeometryDirectly(g1)

                            layer.SetFeature(f0)
                            layer.SetFeature(f1)

                            print("Fixed features")
                            bad = False

                        if bad and status == 0:
                            status = ERR_OVERLAPPING_GEOMETRY

    if args.bounds:
        # arbitrary limit, probably too large
        SIZE_LIMIT = 1580
        if get_geometry_size(union) > SIZE_LIMIT:
            return ERR_COMPLEX_GEOMETRY

        print("Union: {}".format(union.ExportToWkt()))

    if args.fix:
        layer.SyncToDisk()

    return status


if __name__ == "__main__":
    sys.exit(main())
