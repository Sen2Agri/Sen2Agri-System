# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:         DEM&WaterBodyModule

   Author:          CS SI, (Alexia Mondot alexia.mondot@c-s.fr)
   Copyright:       CS SI
   Licence:         See Licence.txt

   Language:        Python
  _____________________________________________________________________________

  HISTORY

  VERSION: 01.00.00 | DATE: <13/04/2015> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________

"""

# system libraries
import os
import sys
import math

# import gdal
import gdal
from gdalconst import *
import osr
import gdalinfo


class gdalinfoO(object):
    def __init__(self, filename, getStats=False):
        self.imageFilename = filename

        if not os.path.exists(filename) and not os.path.isfile(filename):
            print "WARNING : " + filename + " does not exists !"
            sys.exit(1)

        self._size = []
        self._pixelSize = []
        self._origin = []
        self._numberOfBands = -1
        self._driver = []
        self._projection = []
        self._intEpsgCode = -1
        self._projectionName = None
        self._extent = None
        self._histogram = None
        self._geotransform = None
        self._dataType = []
        self._noData = []

        self._getinfo(getStats)

    def _getinfo(self, getStats):
        dataset = gdal.Open(str(self.imageFilename), GA_ReadOnly)
        if dataset is not None:
            self._driver = [dataset.GetDriver().ShortName, dataset.GetDriver().LongName]
            self._size = [dataset.RasterXSize, dataset.RasterYSize]
            self._numberOfBands = dataset.RasterCount
            self._projection = dataset.GetProjection()
            geotransform = dataset.GetGeoTransform()
            if geotransform is not None:
                self._origin = [geotransform[0], geotransform[3]]
                self._pixelSize = [geotransform[1], geotransform[5]]
                self._geotransform = geotransform

            spatialReference = osr.SpatialReference()
            spatialReference.ImportFromWkt(dataset.GetProjectionRef())
            self._intEpsgCode = str(spatialReference.GetAuthorityCode("PROJCS"))
            self._projectionName = spatialReference.GetAttrValue("PROJCS")

            self._setExtent(dataset)
            if getStats:
                self._setHistogram(dataset)
        else:
            print "ERROR"

    def __str__(self):
        message = ["Driver: " + str(self.getDriver()),
                   "Nb bands: " + str(self.getNumberOfBands()),
                   # "Projection: " + str(self.getProjection()),
                   "Origin: " + str(self.getOrigin()),
                   "Pixel Size: " + str(self.getPixelSize()),
                   "EPSG Code: " + str(self.getIntEpsgCode()),
                   "Projection: " + str(self.getProjectionName())]
        return "\n".join(message)

    # TODO check corners (begins to 1 ???)
    def _setExtent(self, dataset):
        extent = []
        # ## Code from GDALINFO.py
        pszProjection = dataset.GetGCPProjection()
        # /* -------------------------------------------------------------------- */
        # /*      Setup projected to lat/long transform if appropriate.           */
        # /* -------------------------------------------------------------------- */
        if pszProjection is not None and len(pszProjection) > 0:
            hProj = osr.SpatialReference(pszProjection)
            if hProj is not None:
                hLatLong = hProj.CloneGeogCS()

            if hLatLong is not None:
                gdal.PushErrorHandler('CPLQuietErrorHandler')
                hTransform = osr.CoordinateTransformation(hProj, hLatLong)
                gdal.PopErrorHandler()
                if gdal.GetLastErrorMsg().find('Unable to load PROJ.4 library') != -1:
                    hTransform = None

                coord = [[1, 1], [1, int(dataset.RasterYSize)], [int(dataset.RasterXSize), 1],
                         [int(dataset.RasterXSize), int(dataset.RasterYSize)],
                         [int(dataset.RasterXSize / 2.0), int(dataset.RasterYSize / 2.0)]]
                for item in coord:
                    a, b = item
                    line = gdalinfo.GDALInfoReportCorner(dataset, hTransform, x=a, y=b)
                    if len(line) == 4:
                        newline = line[2:] + line[:2] + [int(math.ceil(a))] + [int(math.ceil(b))]
                        extent.append(newline)

            self._extent = extent

    def _setHistogram(self, dataset):
        histogramBand = []
        for band_number in range(1, dataset.RasterCount + 1):
            band = dataset.GetRasterBand(band_number)

            # get raster and overview if available
            overview = 2
            if overview < band.GetOverviewCount():
                band_overview = band.GetOverview(overview)
            else:
                band_overview = band

            # get overview statistics
            rasterMin, rasterMax, rasterMean, rasterStddev = band_overview.ComputeStatistics(False)
            histogramBand.append(["B" + str(band_number - 1), int(rasterMin), int(rasterMax),
                                  rasterMean, rasterStddev])
            self._dataType.append(gdal.GetDataTypeName(band.DataType))
            self._noData.append(band.GetNoDataValue())
        self._histogram = histogramBand
        print "self._dataType", self._dataType

    def getSize(self):
        return self._size

    def getPixelSize(self):
        return self._pixelSize

    def getOrigin(self):
        return self._origin

    def getNumberOfBands(self):
        return self._numberOfBands

    def getDriver(self):
        return self._driver

    def getProjection(self):
        return self._projection

    def getIntEpsgCode(self):
        return self._intEpsgCode

    def getProjectionName(self):
        return self._projectionName

    def getExtent(self):
        # originX, originY = self.getOrigin()
        # pixelSizeX, pixelSizeY = self.getPixelSize()
        # sizeX, sizeY = self.getSize()
        # a = [-1, -1, originX, originY, 1, 1]
        # b = [-1, -1, originX, originY+pixelSizeY*sizeY, 1, sizeY+1]
        # c = [-1, -1, originX+pixelSizeX*sizeX, originY+pixelSizeY*sizeY, sizeY+1, sizeY+1]
        # d = [-1, -1, originX+pixelSizeX*sizeX, originY, sizeY+1, 1]
        # center = [-1, -1, math.ceil((originX+pixelSizeX*sizeX)/2),
        #           math.ceil((originY+pixelSizeY*sizeY)/2),
        #           math.ceil((sizeY+1)/2), math.ceil((sizeY+1)/2)]
        # return [a, b, c, d, center]
        return self._extent

    def getHistogram(self):
        return self._histogram

    def getGeotransform(self):
        return self._geotransform

    def getGeotransformStr(self):
        geoTransformList = self.getGeotransform()
        geoTransformListStr = [str(x) for x in geoTransformList]
        return ", ".join(geoTransformListStr)

    def getDataType(self):
        return self._dataType

    def getNoDataValue(self):
        return self._noData