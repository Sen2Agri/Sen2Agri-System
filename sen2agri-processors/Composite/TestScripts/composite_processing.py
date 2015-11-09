#!/usr/bin/env python
from __future__ import print_function

import os
import shutil
import glob
import argparse
import csv
from sys import argv
import datetime
import subprocess
import pipes
import time
import xml.etree.ElementTree as ET
from xml.dom import minidom

def runCmd(cmdArray):
    start = time.time()
    print(" ".join(map(pipes.quote, cmdArray)))    
    res = subprocess.call(cmdArray)
    print("OTB app finished in: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    if res != 0:
        print("OTB application error")
        exit(1)
    return res

def prettify(elem):
    """Return a pretty-printed XML string for the Element.
    """
    rough_string = ET.tostring(elem, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="  ")


parser = argparse.ArgumentParser(description='Composite Python processor')

parser.add_argument(
    '--applocation', help='The path where the sen2agri is built', required=True)
parser.add_argument('--syntdate', help='Synthesis period', required=True)
parser.add_argument('--synthalf', help='Half synthesis', required=True)
parser.add_argument(
    '--input', help='The list of products xml descriptors', required=True, nargs='+')
parser.add_argument(
    '--res', help='The requested resolution in meters', required=True)
parser.add_argument('--t0', help='The start date for the temporal resampling interval (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--tend', help='The end date for the temporal resampling interval (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--outdir', help="Output directory", required=True)
parser.add_argument(
    '--bandsmap', help="Bands mapping file location", required=True)
parser.add_argument('--scatteringcoef',
                    help="Scattering coefficient file. This file is requested only in S2 case", required=False)

USE_COMPRESSION=True
REMOVE_TEMP=True

args = parser.parse_args()

t0 = args.t0
tend = args.tend
syntDate = args.syntdate
syntHalf = args.synthalf
resolution = args.res
bandsMap = args.bandsmap
appLocation = args.applocation
outDir = args.outdir

#filesToBeDeleted = outDir + "/L["rm", filesToBeDele3AResult*"
#runCmd(["rm", filesToBeDeleted])
#exit(1)

compositeLocation = appLocation + '/Composite'

print("Composite location:{}".format(compositeLocation))
productFormatterLocation = appLocation + '/MACCSMetadata/src'
print("Product formater location:{}".format(productFormatterLocation))

#defintion for filenames
weightOtbLibsRoot = compositeLocation + '/WeightCalculation'
#WEIGHT_OTB_LIBS_ROOT="$COMPOSITE_OTB_LIBS_ROOT/WeightCalculation"
outSpotMasks = outDir + '/spot_masks.tif'
#OUT_SPOT_MASKS="$OUT_FOLDER/spot_masks.tif"
outImgBands = outDir + '/res' + resolution + '.tif'
#OUT_IMG_BANDS="$OUT_FOLDER/res$RESOLUTION.tif"
outImgBandsAll = outDir + '/res' + resolution + '_all.tif'
#OUT_IMG_BANDS_ALL="$OUT_FOLDER/res"$RESOLUTION"_all.tif"
outCld = outDir + '/cld' + resolution + '.tif'
#OUT_CLD="$OUT_FOLDER/cld$RESOLUTION.tif"
outWat = outDir + '/wat' + resolution + '.tif'
#OUT_WAT="$OUT_FOLDER/wat$RESOLUTION.tif"
outSnow = outDir + '/snow' + resolution + '.tif'
#OUT_SNOW="$OUT_FOLDER/snow$RESOLUTION.tif"
outAot = outDir + '/aot' + resolution + '.tif'
#OUT_AOT="$OUT_FOLDER/aot$RESOLUTION.tif"

outWeightAotFile = outDir + '/WeightAot.tif'
#OUT_WEIGHT_AOT_FILE="$OUT_FOLDER/WeightAot.tif"
outWeightCloudFile = outDir + '/WeightCloud.tif'
#OUT_WEIGHT_CLOUD_FILE="$OUT_FOLDER/WeightCloud.tif"
outTotalWeightFile = outDir + '/WeightTotal.tif'
#OUT_TOTAL_WEIGHT_FILE="$OUT_FOLDER/WeightTotal.tif"
outL3AFile = outDir + '/L3AResult#_' + resolution + 'M.tif'
#OUT_L3A_FILE="$OUT_FOLDER/L3AResult#_"$RESOLUTION"M.tif"

WEIGHT_AOT_MIN="0.33"
WEIGHT_AOT_MAX="1"
AOT_MAX="50"

COARSE_RES="240"
SIGMA_SMALL_CLD="10"
SIGMA_LARGE_CLD="50"

WEIGHT_SENSOR="0.33"
WEIGHT_DATE_MIN="0.10"

outWeights = outDir + '/L3AResult#_weights.tif'
#OUT_WEIGHTS="$OUT_FOLDER/L3AResult#_weights.tif"
outDates = outDir + '/L3AResult#_dates.tif'
#OUT_DATES="$OUT_FOLDER/L3AResult#_dates.tif
outRefls = outDir + '/L3AResult#_refls.tif'
#OUT_REFLS="$OUT_FOLDER/L3AResult#_refls.tif"
outFlags = outDir + '/L3AResult#_flags.tif'
#OUT_FLAGS="$OUT_FOLDER/L3AResult#_flags.tif
outRGB = outDir + '/L3AResult#_rgb.tif'
#OUT_RGB="$OUT_FOLDER/L3AResult#_rgb.tif"

fullScatCoeffs=""
#FULL_SCAT_COEFFS=""

if os.path.exists(outDir):
    if not os.path.isdir(outDir):
        print("Can't create the output directory because there is a file with the same name")
        print("Remove: " + outDir)
        exit(1)
else:
    os.makedirs(outDir)

shutil.copy(bandsMap, outDir)

if args.scatteringcoef:
    shutil.copy(args.scatteringcoef, outDir)
    scatteringCoefFilename = args.scatteringcoef[args.scatteringcoef.rfind('/'):]
    fullScatCoeffs = outDir + scatteringCoefFilename
    print(fullScatCoeffs)
#    FULL_SCAT_COEFFS= "-scatcoef $OUT_FOLDER/$SCAT_COEFFS"

paramsFilenameXML= outDir + '/params.xml'
with open(paramsFilenameXML, 'w') as paramsFileXML:
    root = ET.Element('metadata')
    wAOT = ET.SubElement(root, "Weight_AOT")
    ET.SubElement(wAOT, "weight_aot_min").text = WEIGHT_AOT_MIN
    ET.SubElement(wAOT, "weight_aot_max").text = WEIGHT_AOT_MAX
    ET.SubElement(wAOT, "aot_max").text = AOT_MAX
    wCLD = ET.SubElement(root, "Weight_On_Clouds")
    ET.SubElement(wCLD, "coarse_res").text = COARSE_RES
    ET.SubElement(wCLD, "sigma_small_cloud").text = SIGMA_SMALL_CLD
    ET.SubElement(wCLD, "sigma_large_cloud").text = SIGMA_LARGE_CLD
    wDATE = ET.SubElement(root, "Weight_On_Date")
    ET.SubElement(wDATE, "weight_date_min").text = WEIGHT_DATE_MIN
    ET.SubElement(wDATE, "l3a_product_date").text = syntDate
    ET.SubElement(wDATE, "half_synthesis").text = syntHalf
    wON = ET.SubElement(root, "Weight_ON")
    ET.SubElement(wON, "weight_sensor").text = WEIGHT_SENSOR
    usedXMLs = ET.SubElement(root, "XML_files")
    i = 0
    for xml in args.input:
        ET.SubElement(usedXMLs, "XML_" + str(i)).text = xml
        i += 1
    paramsFileXML.write(prettify(root))

paramsFilename= outDir + '/params.txt'
with open(paramsFilename, 'w') as paramsFile:
    paramsFile.write("Weight AOT\n")
    paramsFile.write("    weight aot min    = " + WEIGHT_AOT_MIN + "\n")
    paramsFile.write("    weight aot max    = " + WEIGHT_AOT_MAX + "\n")
    paramsFile.write("    aot max           = " + AOT_MAX + "\n")
    paramsFile.write("Weight on clouds\n")
    paramsFile.write("    coarse res        = " + COARSE_RES + "\n")
    paramsFile.write("    sigma small cloud = " + SIGMA_SMALL_CLD + "\n")
    paramsFile.write("    sigma large cloud = " + SIGMA_LARGE_CLD + "\n")
    paramsFile.write("Weight on Date\n")
    paramsFile.write("    weight date min   = " + WEIGHT_DATE_MIN + "\n")
    paramsFile.write("    l3a product date  = " + syntDate + "\n")
    paramsFile.write("    half synthesis    = " + syntHalf + "\n")
    paramsFile.write("Weight on\n")
    paramsFile.write("    weight sensor     = " + WEIGHT_SENSOR + "\n")
    paramsFile.write(" ")
    paramsFile.write("Used XML files\n")
    for xml in args.input:
        paramsFile.write("  " + xml + "\n")        

prevL3A=[]
i = 0

print("Processing started: " + str(datetime.datetime.now()))
start = time.time()
for xml in args.input:
    
# otbcli MaskHandler "$COMPOSITE_OTB_LIBS_ROOT/MaskHandler/" -xml "$xml" -out "$OUT_SPOT_MASKS" -sentinelres $RESOLUTION
    runCmd(["otbcli", "MaskHandler", compositeLocation + '/MaskHandler', "-xml", xml, "-out", outSpotMasks, "-sentinelres", resolution])
    
    counterString = str(i)
    mod=outL3AFile.replace("#", counterString)
    out_w=outWeights.replace("#", counterString)
    out_d=outDates.replace("#", counterString)
    out_r=outRefls.replace("#", counterString)
    out_f=outFlags.replace("#", counterString)
    out_rgb=outRGB.replace("#", counterString)


#    try otbcli CompositePreprocessing2 "$COMPOSITE_OTB_LIBS_ROOT/CompositePreprocessing/" -xml "$xml" -bmap "$FULL_BANDS_MAPPING" -res "$RESOLUTION" "$FULL_SCAT_COEFFS" -msk "$OUT_SPOT_MASKS" -outres "$OUT_IMG_BANDS" -outcmres "$OUT_CLD" -outwmres "$OUT_WAT" -outsmres "$OUT_SNOW" -outaotres "$OUT_AOT"
    if fullScatCoeffs:
        cmd = ["otbcli", "CompositePreprocessing2", compositeLocation + '/CompositePreprocessing', "-xml", xml, "-bmap", bandsMap, "-res", resolution, "-scatcoef", fullScatCoeffs, "-msk", outSpotMasks, "-outres", outImgBands, "-outcmres", outCld, "-outwmres", outWat, "-outsmres", outSnow, "-outaotres", outAot]
    else:
        cmd = ["otbcli", "CompositePreprocessing2", compositeLocation + '/CompositePreprocessing', "-xml", xml, "-bmap", bandsMap, "-res", resolution, "-msk", outSpotMasks, "-outres", outImgBands, "-outcmres", outCld, "-outwmres", outWat, "-outsmres", outSnow, "-outaotres", outAot]
    
    runCmd(cmd)
        
#    try otbcli WeightAOT "$WEIGHT_OTB_LIBS_ROOT/WeightAOT/" -in "$OUT_AOT" -xml "$xml" -waotmin "$WEIGHT_AOT_MIN" -waotmax "$WEIGHT_AOT_MAX" -aotmax "$AOT_MAX" -out "$OUT_WEIGHT_AOT_FILE"
    
    runCmd(["otbcli", "WeightAOT", weightOtbLibsRoot + '/WeightAOT', "-xml", xml, "-in", outAot, "-waotmin", WEIGHT_AOT_MIN, "-waotmax", WEIGHT_AOT_MAX, "-aotmax", AOT_MAX, "-out", outWeightAotFile])

#    try otbcli WeightOnClouds "$WEIGHT_OTB_LIBS_ROOT/WeightOnClouds/" -incldmsk "$OUT_CLD" -coarseres "$COARSE_RES" -sigmasmallcld "$SIGMA_SMALL_CLD" -sigmalargecld "$SIGMA_LARGE_CLD" -out "$OUT_WEIGHT_CLOUD_FILE"
    
    runCmd(["otbcli", "WeightOnClouds", weightOtbLibsRoot + '/WeightOnClouds', "-incldmsk", outCld, "-coarseres", COARSE_RES, "-sigmasmallcld", SIGMA_SMALL_CLD, "-sigmalargecld", SIGMA_LARGE_CLD, "-out", outWeightCloudFile])

#    try otbcli TotalWeight "$WEIGHT_OTB_LIBS_ROOT/TotalWeight/" -xml "$xml" -waotfile "$OUT_WEIGHT_AOT_FILE" -wcldfile "$OUT_WEIGHT_CLOUD_FILE" -wsensor "$WEIGHT_SENSOR" -l3adate "$L3A_DATE" -halfsynthesis "$HALF_SYNTHESIS" -wdatemin "$WEIGHT_DATE_MIN" -out "$OUT_TOTAL_WEIGHT_FILE"
    runCmd(["otbcli", "TotalWeight", weightOtbLibsRoot + '/TotalWeight', "-xml", xml, "-waotfile", outWeightAotFile, "-wcldfile", outWeightCloudFile, "-wsensor", WEIGHT_SENSOR, "-l3adate", syntDate, "-halfsynthesis", syntHalf, "-wdatemin", WEIGHT_DATE_MIN, "-out", outTotalWeightFile])
    #todo... search for previous L3A produc?

#    try otbcli UpdateSynthesis2 "$COMPOSITE_OTB_LIBS_ROOT/UpdateSynthesis/" -in "$OUT_IMG_BANDS" -bmap "$FULL_BANDS_MAPPING" -xml "$xml" $PREV_L3A -csm "$OUT_CLD" -wm "$OUT_WAT" -sm "$OUT_SNOW" -wl2a "$OUT_TOTAL_WEIGHT_FILE" -out "$mod"
    runCmd(["otbcli", "UpdateSynthesis2", compositeLocation + '/UpdateSynthesis', "-in", outImgBands, "-bmap", bandsMap, "-xml", xml, "-csm", outCld, "-wm", outWat, "-sm", outSnow, "-wl2a", outTotalWeightFile, "-out", mod] + prevL3A)

    tmpOut_w = out_w
    tmpOut_d = out_d
    tmpOut_r = out_r
    tmpOut_f = out_f
    tmpOut_rgb = out_rgb

    if USE_COMPRESSION:         
        tmpOut_w += '?gdal:co:COMPRESS=DEFLATE' 
        tmpOut_d += '?gdal:co:COMPRESS=DEFLATE'
        tmpOut_r += '?gdal:co:COMPRESS=DEFLATE'
        tmpOut_f += '?gdal:co:COMPRESS=DEFLATE' 
        tmpOut_rgb += '?gdal:co:COMPRESS=DEFLATE'

    runCmd(["otbcli", "CompositeSplitter2", compositeLocation + '/CompositeSplitter', "-in", mod, "-xml", xml, "-bmap", bandsMap, "-outweights", tmpOut_w, "-outdates", tmpOut_d, "-outrefls", tmpOut_r, "-outflags", tmpOut_f, "-outrgb", tmpOut_rgb])

    prevL3A = ["-prevl3aw", out_w, "-prevl3ad", out_d, "-prevl3ar", out_r, "-prevl3af", out_f]

    runCmd(["otbcli", "ProductFormatter", productFormatterLocation, "-destroot", outDir, "-fileclass", "SVT1", "-level", "L3A", "-timeperiod", t0 + '_' + tend, "-baseline", "01.00", "-processor", "composite", "-processor.composite.refls", out_r, "-processor.composite.weights", out_w, "-processor.composite.flags", out_f, "-processor.composite.dates", out_d, "-processor.composite.rgb", out_rgb, "-il", xml, "-gipp", paramsFilename])
    
    runCmd(["rm", "-fr", mod, outSpotMasks, outImgBands, outCld, outWat, outSnow, outAot, outWeightAotFile, outWeightCloudFile, outTotalWeightFile])
    if REMOVE_TEMP and i > 0:
        counterString = str(i - 1)
        print("!!!!!!!! The following files will be deleted: !!!!!!!!")
        os.remove(outWeights.replace("#", counterString))
        print(outWeights.replace("#", counterString))
        os.remove(outDates.replace("#", counterString))
        print(outDates.replace("#", counterString))
        os.remove(outRefls.replace("#", counterString))
        print(outRefls.replace("#", counterString))
        os.remove(outFlags.replace("#", counterString))
        print(outFlags.replace("#", counterString))
        #os.remove(outRGB.replace("#", counterString))
    i += 1
if REMOVE_TEMP and i > 0:
    counterString = str(i - 1)
    print("!!!!!!!! The following files will be deleted: !!!!!!!!")
    os.remove(outWeights.replace("#", counterString))
    print(outWeights.replace("#", counterString))
    os.remove(outDates.replace("#", counterString))
    print(outDates.replace("#", counterString))
    os.remove(outRefls.replace("#", counterString))
    print(outRefls.replace("#", counterString))
    os.remove(outFlags.replace("#", counterString))
    print(outFlags.replace("#", counterString))
    
if REMOVE_TEMP:
    filesToBeDeleted = outDir + '/L3AResult*'
    runCmd(["rm", "-fr", filesToBeDeleted])

print("Processing finnished: " + str(datetime.datetime.now()))
print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

'''
''' and None
