#!/usr/bin/env python
from __future__ import print_function
import argparse
import re
import os
from os.path import isfile, isdir, join
import glob
import sys
import time, datetime
import gdal
from osgeo import ogr
from multiprocessing import Pool
from sen2agri_common_db import *
from subprocess import Popen, PIPE
import math

NOT_A_VALUE = -10000

parser = argparse.ArgumentParser(
    description="Products validity checker. Works for the following products: Composite (L3A), LAI (L3B), LAI reprocessed (L3C), LAI reprocessed over the season (L3D), Crop mask (L4A) ")

parser.add_argument('input', help="Input file")
parser.add_argument('--number-of-bands', required=True, help="Value to compare")

args = parser.parse_args()

return_value = int(0)
try:
    #output = subprocess.check_output(["otbcli_ComputeImagesStatistics", "-il", args.input])

    mean = re.search(r"\(INFO\) Mean: \[([\w., -]+)\]", output)
    deviation = re.search(r"\(INFO\) Standard Deviation: \[([\w., ]+)\]", output)
    if mean is not None and deviation is not None:
        print(output)
        print("{} | {}".format(mean.group(1), deviation.group(1)))
        mean_values = mean.group(1).strip(" \n\r\t").split(",")
        deviation_values = deviation.group(1).strip(" \n\r\t").split(",")
        print("{} - {}".format(mean_values, deviation_values))        
        if len(mean_values) == int(args.number_of_bands):
            for mean_value in mean_values:
                if math.fabs(float(mean_value) - (NOT_A_VALUE)) <= 0.001:
                    # if a band average is almost NOT_A_VALUES, it means all the band is not a values resulting in error
                    return_value = int(-1)
                    break
            for deviation_value in deviation_values:
                if math.fabs(float(deviation_value)) <= 0.1:
                    # if a band standard deviation value is near 0, means that all the pixels from that band are the same, resulting in a blank picture
                    return_value = int(-2)
                    break
        else:
            # the number of expected bands is different
            return_value = int(-3)

except subprocess.CalledProcessError, e:
    print ("otbcli_ComputeImageStatistic exception: {}".format(e.output))
    return_value = int(-4)

print("{}".format(return_value))
sys.exit(return_value)
