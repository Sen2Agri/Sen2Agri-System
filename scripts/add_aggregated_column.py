#!/usr/bin/env python
from __future__ import print_function

import argparse
import sys
from osgeo import ogr
from gdal import gdalconst


def main():
    parser = argparse.ArgumentParser(description="Create a new column with aggredated values from other columns.")
    parser.add_argument('-f', '--field', help="field name to be created", default="agg_id")
    parser.add_argument('-s', '--sources', help="fields name to be used in aggregation")
    parser.add_argument('--force', help="overwrite field", action='store_true')
    parser.add_argument('input', help="the input dataset")

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

    layer.CreateField(ogr.FieldDefn(args.field, ogr.OFTString))
    field_idx = layer.FindFieldIndex(args.field, False)

    sourceFields = args.sources.split(' ')
    print("Source fields to be used{}".format(sourceFields))
    for feature in layer:
        values_list = [feature.GetField(j) for j in sourceFields]
        # newValue = '-'.join(values_list)
        newValue = '-'.join(str(x) for x in values_list)
        feature.SetField(field_idx, newValue)
        layer.SetFeature(feature)
    dataset.CommitTransaction()

    return 0


if __name__ == '__main__':
    sys.exit(main())
