#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:      Sen2Agri-Processors
   Language:     Python
   Copyright:    2015-2016, CS Romania, office@c-s.ro
   See COPYRIGHT file for details.

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
_____________________________________________________________________________

"""
from __future__ import print_function
import argparse
import re
import os
import sys
import subprocess
import math


def get_otb_launcher():
    if os.getenv("ITK_AUTOLOAD_PATH") is None:
        return "otbcli"
    else:
        return "otbApplicationLauncherCommandLine"

NOT_A_VALUE = -10000

parser = argparse.ArgumentParser(
    description="Products validity checker. Works for the following products: Composite (L3A), LAI (L3B), LAI reprocessed (L3C), LAI reprocessed over the season (L3D), Crop mask (L4A) ")

parser.add_argument('input', help="Input file")
parser.add_argument('--number-of-bands', required=True, help="Value to compare")

args = parser.parse_args()

return_value = int(0)
try:
    if not os.path.exists(args.input) or not os.path.isfile(args.input):
        return_value = int(-1)
    else:
        output = subprocess.check_output([get_otb_launcher(), "ComputeImagesStatistics",
                                          "-progress", "false",
                                          "-il", args.input])

        mean = re.search(r"\(INFO\) Mean: \[([\w., -]+)\]", output)
        deviation = re.search(r"\(INFO\) Standard Deviation: \[([\w., ]+)\]", output)
        if mean is not None and deviation is not None:
            print(output)
            mean_values = mean.group(1).strip(" \n\r\t").split(",")
            deviation_values = deviation.group(1).strip(" \n\r\t").split(",")
            if len(mean_values) >= int(args.number_of_bands):
                for mean_value in mean_values:
                    if math.fabs(float(mean_value) - (NOT_A_VALUE)) <= 0.001:
                        # if a band average is almost NOT_A_VALUES, it means all the band is not a values resulting in error
                        return_value = int(-2)
                        break
                for deviation_value in deviation_values:
                    if math.fabs(float(deviation_value)) <= 0.1:
                        # if a band standard deviation value is near 0, means that all the pixels from that band are the same, resulting in a blank picture
                        return_value = int(-3)
                        break
            else:
                # the number of expected bands is different
                return_value = int(-4)

except subprocess.CalledProcessError, e:
    print ("Running ComputeImageStatistics failed: {}".format(e.output))
    return_value = int(-5)

print("return value: {}".format(return_value))
sys.exit(return_value)
