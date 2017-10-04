#!/usr/bin/env python
from __future__ import print_function

import argparse
import gdal
from gdalconst import GA_Update
import re
import osr
import sys


# adapted from http://gis.stackexchange.com/a/20306
def wkt2epsg(wkt, epsg='/usr/share/proj/epsg', forceProj4=False):
    ''' Transform a WKT string to an EPSG code

    Arguments
    ---------

    wkt: WKT definition
    epsg: the proj.4 epsg file (defaults to '/usr/share/proj/epsg')
    forceProj4: whether to perform brute force proj4 epsg file check (last resort)

    Returns: EPSG code

    '''
    code = None
    p_in = osr.SpatialReference()
    s = p_in.ImportFromWkt(wkt)
    if s == 5:  # invalid WKT
        return None
    if p_in.IsLocal() == 1:  # this is a local definition
        return p_in.ExportToWkt()
    if p_in.IsGeographic() == 1:  # this is a geographic srs
        cstype = 'GEOGCS'
    else:  # this is a projected srs
        cstype = 'PROJCS'
    an = p_in.GetAuthorityName(cstype)
    ac = p_in.GetAuthorityCode(cstype)
    if an is not None and ac is not None:  # return the EPSG code
        return '{}:{}'.format(
            p_in.GetAuthorityName(cstype), p_in.GetAuthorityCode(cstype))
    else:  # try brute force approach by grokking proj epsg definition file
        p_out = p_in.ExportToProj4()
        if p_out:
            if forceProj4 is True:
                return p_out
            f = open(epsg)
            for line in f:
                if line.find(p_out) != -1:
                    m = re.search('<(\\d+)>', line)
                    if m:
                        code = m.group(1)
                        break
            if code:  # match
                return 'EPSG:%s' % code
            else:  # no match
                return None
        else:
            return None


def fix_file(input, args):
    print("Processing {}".format(input))
    ds = gdal.Open(input, GA_Update)
    if ds is None:
        raise Exception("Could not open dataset", input)

    prj = ds.GetProjection()
    if args.verbose:
        print("Input projection: {}".format(prj))
    srs = osr.SpatialReference(wkt=prj)

    proj_code = wkt2epsg(prj)
    if proj_code is None:
        if args.verbose:
            print("Unable to identify SRS, nothing to do")
        return
    if not proj_code.startswith('EPSG:'):
        if args.verbose:
            print("Projection does not look like an EPSG one, nothing to do")
        return

    epsg_code = int(proj_code[5:])
    if args.verbose:
        print("Found projection EPSG:{}".format(epsg_code))

    if not(epsg_code >= 32601 and epsg_code <= 32660 or epsg_code >= 32701 and epsg_code <= 32760):
        if args.verbose:
            print("Projection is not WGS 84 / UTM zone, nothing to do")
        return

    gt = ds.GetGeoTransform()
    if args.verbose:
        print("Input GeoTransform: {}".format(gt))
    if gt[3] >= 0:
        if args.verbose:
            print("Y coordinate is positive, nothing to do")
        return

    if epsg_code >= 32601 and epsg_code <= 32660:
        epsg_code = epsg_code + 100
    else:
        epsg_code = epsg_code - 100
    if args.verbose:
        print("Changing input SRS to EPSG:{}".format(epsg_code))

    srs.ImportFromEPSG(epsg_code)
    ds.SetProjection(srs.ExportToWkt())
    gt = (gt[0], gt[1], gt[2], 10000000 + gt[3], gt[4], gt[5])
    if args.verbose:
        print("Changing GeoTransform to {}".format(gt))
    ds.SetGeoTransform(gt)


def main():
    parser = argparse.ArgumentParser(description="Fixes rasters with negative coordinates in WGS 84 / UTM zones")
    parser.add_argument('-v', '--verbose', action='store_true', help="verbose output")
    parser.add_argument('input', help='the input rasters', nargs='+')

    args = parser.parse_args()

    for input in args.input:
        fix_file(input, args)
    return 0


if __name__ == '__main__':
    sys.exit(main())
