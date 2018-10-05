#!/usr/bin/env python
from __future__ import print_function

import argparse
import sys
from osgeo import ogr
from gdal import gdalconst


def main():
    parser = argparse.ArgumentParser(description="Renumbers the features in a dataset.")
    parser.add_argument('-f', '--field', help="field name", default="seq_id")
    parser.add_argument('--force', help="overwrite field", action='store_true')
    parser.add_argument('input', help="the input data source")

    args = parser.parse_args()

    dataset = ogr.Open(args.input, gdalconst.GA_Update)

    layer = dataset.GetLayer()
    feature_count = layer.GetFeatureCount()
    print("{} feature(s) found".format(feature_count))

    dataset.StartTransaction()
    field_idx = layer.FindFieldIndex(args.field, False)
    if field_idx != -1:
        if args.force:
            print("Field {} already exists, removing it".format(args.field))
            layer.DeleteField(field_idx)
        else:
            print("Field {} already exists, exiting".format(args.field))
            return 0

    layer.CreateField(ogr.FieldDefn(args.field, ogr.OFTInteger))
    field_idx = layer.FindFieldIndex(args.field, False)

    id = 0
    for feature in layer:
        id += 1
        feature.SetField(field_idx, id)
        layer.SetFeature(feature)
    dataset.CommitTransaction()

    return 0


if __name__ == '__main__':
    sys.exit(main())
