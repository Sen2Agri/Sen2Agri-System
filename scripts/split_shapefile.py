#!/usr/bin/env python
from __future__ import print_function

import argparse
from osgeo import ogr
import os.path


def main():
    parser = argparse.ArgumentParser(
        description="Splits vector data by eco-areas")
    parser.add_argument('-o', '--output', required=True,
                        help="the output location")
    parser.add_argument('areas', help="the regions shapefile (ID field)")
    parser.add_argument('data', help="the vector data")

    args = parser.parse_args()

    area_ds = ogr.Open(args.areas, 0)
    if area_ds is None:
        print("Could not open {}".format(area_ds))
        return 1

    data_ds = ogr.Open(args.data, 0)
    in_layer = area_ds.GetLayer()
    in_layer_def = in_layer.GetLayerDefn()

    data_layer = data_ds.GetLayer()

    driver = ogr.GetDriverByName('ESRI Shapefile')

    for area in in_layer:
        area_id = area.GetField('ID')

        name = os.path.splitext(os.path.basename(args.data))[0]
        out_name = os.path.join(args.output, "{}-{}.shp".format(name, area_id))

        if os.path.exists(out_name):
            driver.DeleteDataSource(out_name)

        print("Writing {}".format(out_name))

        out_ds = driver.CreateDataSource(out_name)
        if out_ds is None:
            print("Could not create {}".format(out_name))
            return 1

        out_layer = out_ds.CreateLayer(
            'features', srs=in_layer.GetSpatialRef(),
            geom_type=ogr.wkbMultiPolygon)
        out_layer_def = out_layer.GetLayerDefn()

        for i in range(in_layer_def.GetFieldCount()):
            out_layer.CreateField(in_layer_def.GetFieldDefn(i))

        data_layer.ResetReading()
        for region in data_layer:
            geom = region.GetGeometryRef().Intersection(area.GetGeometryRef())
            if geom.GetArea() > 0:
                out_feature = ogr.Feature(out_layer_def)

                for i in range(out_layer_def.GetFieldCount()):
                    out_feature.SetField(out_layer_def.GetFieldDefn(
                        i).GetNameRef(), region.GetField(i))

                out_feature.SetGeometry(geom)
                out_layer.CreateFeature(out_feature)

        out_ds.Destroy()

    data_ds.Destroy()
    area_ds.Destroy()


if __name__ == '__main__':
    main()
