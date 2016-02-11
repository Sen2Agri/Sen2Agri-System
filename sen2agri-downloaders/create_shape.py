#!/usr/bin/env python
import argparse
import osgeo.ogr as ogr
import osgeo.osr as osr
from common import *

class AOI(object):
    def __init__(self, aoiName, lonMin, latMin, lonMax, latMax):        
        self.aoiName = aoiName
        self.latMin = float(latMin)
        self.latMax = float(latMax)
        self.lonMin = float(lonMin)
        self.lonMax = float(lonMax)

parser = argparse.ArgumentParser(description='Creates a shapefile with AOI')
'''
parser.add_argument('--latMin', help='Minimum latitude', required=True)
parser.add_argument('--latMax', help='Maximum latitude', required=True)
parser.add_argument('--lonMin', help='Minimum longitude', required=True)
parser.add_argument('--lonMax', help='Maximum longitude', required=True)
'''
parser.add_argument('--inFile', help='Input file. This file should have the following format on each line: aoi:lonMin:latMin:lonMax:latMax, where aoi is the name of the area of interest', required=False)
parser.add_argument('--outDir', help='Output directory where the shape files will be created', required=False)

args = parser.parse_args()
inFile = args.inFile

outDirname = "./"

if args.outDir:
    outDirname = args.outDir
    if not createRecursiveDirs(outDirname):
        print("Could not create the output directory")
        sys.exit(-1)
aoiArray = []
print("Loading information from input file {}".format(inFile))
with open(inFile, 'r') as inputFile:
    for line in inputFile:
        lineStripped = line.strip(" \n\t\r")
        if len(lineStripped) <= 0:
            continue
        elements = lineStripped.split(':')
        if len(elements) != 5:
            print("Can't load information from this line: {}".format(line))
            continue
        aoiArray.append(AOI(elements[0], elements[1], elements[2], elements[3], elements[4]))

# set up the shapefile driver
driver = ogr.GetDriverByName("ESRI Shapefile")

# create the data source
data_source = driver.CreateDataSource(outDirname)

# create the spatial reference, WGS84
srs = osr.SpatialReference()
if not srs:
    print ("Could not create the spatial reference")
    sys.exit(-1)
srs.ImportFromEPSG(4326)

# create the layer
for aoi in aoiArray:
    layer = data_source.CreateLayer(aoi.aoiName, srs, ogr.wkbPolygon)

    if not layer:
        print("Could not create the layer for {}. Maybe the file already exists?".format(aoi.aoiName))
        continue
    # Process the text file and add the attributes and features to the shapefile
    reader = [[0 for x in range(2)] for x in range(4)]

    reader[0] = [aoi.lonMin, aoi.latMin]
    reader[1] = [aoi.lonMax, aoi.latMin]
    reader[2] = [aoi.lonMax, aoi.latMax]
    reader[3] = [aoi.lonMin, aoi.latMax]

    print reader
    ring = ogr.Geometry(ogr.wkbLinearRing)
    for row in reader:
        # create the feature
    #    feature = ogr.Feature(layer.GetLayerDefn())

        # create the WKT for the feature using Python string formatting
    #    wkt = "POINT(%f %f)" %  (float(row[0]) , float(row[1]))

        # Create the point from the Well Known Txt
    #    point = ogr.CreateGeometryFromWkt(wkt)

        ring.AddPoint(row[0], row[1])
        # Set the feature geometry using the point
    #    feature.SetGeometry(point)
        # Create the feature in the layer (shapefile)
    #    layer.CreateFeature(feature)
        # Destroy the feature to free resources
    #    feature.Destroy()

    ring.AddPoint(reader[0][0], reader[0][1])

    poly = ogr.Geometry(ogr.wkbPolygon)
    if not poly:
        print("Could not create the geometry for {}".format(aoi.aoiName))
        sys.exit(-1)

    poly.AddGeometry(ring)

    feature = ogr.Feature(layer.GetLayerDefn())
    if not feature:
        print("Could not create the feature for {}".format(aoi.aoiName))
        sys.exit(-1)
    feature.SetGeometry(poly)
    layer.CreateFeature(feature)

#LAYER.AddGeometry(poly)
# Destroy the data source to free resources
data_source.Destroy()
