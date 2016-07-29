#!/usr/bin/env python
import argparse
import gdal
import osr
import ogr


def get_l8_tile(l8_tiles, tile_id):
    ds = ogr.Open(l8_tiles)
    layer = ds.GetLayer()
    layer.SetAttributeFilter("PR={}".format(tile_id))

    for feature in layer:
        return layer.GetSpatialRef(), feature

def get_input_extent(l8_tiles, tile_id, out_srs):
    l8_srs, tile = get_l8_tile(l8_tiles, tile_id)
    geom = tile.GetGeometryRef()
    envelope = geom.GetEnvelope() # minX, maxX, minY, maxY
    envelope = [envelope[0], envelope[2], envelope[1], envelope[3]] # minX, minY, maxX, maxY

    transform = osr.CoordinateTransformation(l8_srs, out_srs)

    envelope_reprojected = [transform.TransformPoint(envelope[0], envelope[1]), transform.TransformPoint(envelope[2], envelope[3])] # ll, ur
    print(l8_srs)
    print(envelope)
    print(envelope_reprojected)
    fudge_extent(envelope_reprojected)

def get_dataset_srs(product):
    ds = gdal.Open(product, gdal.gdalconst.GA_ReadOnly)
    return osr.SpatialReference(wkt=ds.GetProjection())


def fudge_extent(envelope):
    print(envelope)
    A, B = envelope[0][0:2] # llx, lly
    C, D = envelope[1][0:2] # urx, ury

    newA = round(A / 30) * 30 + 15
    newB = round(B / 30) * 30 + 15

    Ctemp = round(C / 30) * 30 + 15
    Dtemp = round(D / 30) * 30 + 15

    newC = Ctemp
    newD = Dtemp

    sizeX = newC - newA
    sizeY = newD - newB

    print("{} {} {} {}".format(A, B, C, D))
    print("{} {} {} {}".format(newA, newB, newC, newD))

    scale_factor = 240
    restX = sizeX % scale_factor
    restY = sizeY % scale_factor

    if restX:
        newC = newC + scale_factor - restX
    if restY:
        newD = newD + scale_factor - restY

    print("{} {} {} {}".format(newA, newB, newC, newD))
    print("{} {} {} {}".format(newA, newD, newC, newB)) # llx, ury, urx, lly

out_srs = get_dataset_srs("/mnt/archive/dwn_def/l8/default/south_africa_small/LC81710782015282LGN00/LC81710782015282LGN00_B1.TIF")
print(out_srs)
get_input_extent("/home/grayshade/sen2agri/orbits/wrs2_descending.shp", 171078, out_srs)