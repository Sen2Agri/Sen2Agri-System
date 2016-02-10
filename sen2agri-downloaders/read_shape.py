#!/usr/bin/env python
import os
import osgeo.ogr as ogr
import osgeo.osr as osr


# set up the shapefile driver
driver = ogr.GetDriverByName("ESRI Shapefile")

#shapefile = "/mnt/Sen2Agri_DataSets/MACCS/DEM/s2agri-utils/out/shape_tiles_L8.shp"
shapefile = "/mnt/Sen2Agri_DataSets/MACCS/DEM/s2agri-utils/out/shape_tiles_S2.shp"
dataSource = driver.Open(shapefile, 0) # 0 means read-only. 1 means writeable.

# Check to see if shapefile is found.
if dataSource is None:
    print 'Could not open %s' % (shapefile)
else:
    print 'Opened %s' % (shapefile)
    layer = dataSource.GetLayer()
    featureCount = layer.GetFeatureCount()
    print "Number of features in %s: %d" % (os.path.basename(shapefile),featureCount)
#   layer.SetAttributeFilter("SUB_REGION = 'TileID'")
    for feature in layer:
        print feature.GetField("TileID")

