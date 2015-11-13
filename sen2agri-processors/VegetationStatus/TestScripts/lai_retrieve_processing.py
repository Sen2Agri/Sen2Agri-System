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
import math
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

parser = argparse.ArgumentParser(description='LAI retrieval processor')

parser.add_argument('--applocation', help='The path where the sen2agri is built', required=True)
parser.add_argument('--input', help='The list of products xml descriptors', required=True, nargs='+')
parser.add_argument('--res', help='The requested resolution in meters', required=True)
parser.add_argument('--t0', help='The start date for the temporal resampling interval (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--tend', help='The end date for the temporal resampling interval (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--outdir', help="Output directory", required=True)
parser.add_argument('--rsrfile', help='The RSR file (/path/filename)', required=True)
parser.add_argument('--solarzenith', help='The value for solar zenith angle', required=True)
parser.add_argument('--sensorzenith', help='The value for sensor zenith angle', required=True)
parser.add_argument('--relativeazimuth', help='The value for relative azimuth angle', required=True)

args = parser.parse_args()

appLocation = args.applocation
resolution = args.res
t0 = args.t0
tend = args.tend
outDir = args.outdir

vegetationStatusLocation = "{}/VegetationStatus".format(appLocation)
productFormatterLocation = "{}/MACCSMetadata/src".format(appLocation)

if os.path.exists(outDir):
    if not os.path.isdir(outDir):
        print("Can't create the output directory because there is a file with the same name")
        print("Remove: " + outDir)
        exit(1)
else:
    os.makedirs(outDir)

paramsLaiModelFilenameXML = "{}/lai_model_params.xml".format(outDir)
paramsLaiRetrFilenameXML = "{}/lai_retrieval_params.xml".format(outDir)

runCmd(["./lai_model.py", "--applocation", appLocation, "--rsrfile", args.rsrfile, "--solarzenith", args.solarzenith, "--sensorzenith", args.sensorzenith, "--relativeazimuth", args.relativeazimuth, "--outdir", outDir, "--outxml", paramsLaiModelFilenameXML])

if resolution != 10 and resolution != 20:
    print("The resolution is : {}".format(resolution))
    print("The resolution should be either 10 or 20.")
    print("The product will be created with the original resolution without resampling.")
    resolution=0

#IMG_INV_OTB_LIBS_ROOT="$VEG_STATUS_OTB_LIBS_ROOT/otb-bv/src/applications"
imgInvOtbLibsLocation = vegetationStatusLocation + '/otb-bv/src/applications'

#OUT_NDVI_RVI="$OUT_FOLDER/ndvi_rvi.tif"
outNdviRvi = "{}/ndvi_rvi.tif".format(outDir)
#OUT_LAI_IMG="$OUT_FOLDER/LAI_img_#.tif"
outLaiImg = "{}/LAI_img_#.tif".format(outDir)
#OUT_LAI_ERR_IMG="$OUT_FOLDER/LAI_err_img_#.tif"
outLaiErrImg = "{}/LAI_err_img_#.tif".format(outDir) 

#OUT_LAI_TIME_SERIES="$OUT_FOLDER/LAI_time_series.tif"
outLaiTimeSeries = "{}/LAI_time_series.tif".format(outDir)
#OUT_ERR_TIME_SERIES="$OUT_FOLDER/Err_time_series.tif"
outErrTimeSeries = "{}/Err_time_series.tif".format(outDir)

#OUT_REPROCESSED_TIME_SERIES="$OUT_FOLDER/ReprocessedTimeSeries.tif"
outReprocessedTimeSeries = "{}/ReprocessedTimeSeries.tif".format(outDir)
#OUT_FITTED_TIME_SERIES="$OUT_FOLDER/FittedTimeSeries.tif"
outFittedTimeSeries = "{}/FittedTimeSeries.tif".format(outDir)

#echo "Executing from $MY_PWD"
#echo "Models folder: $MODELS_FOLDER"

#MODELS_INPUT_LIST=""
modelsInputList=[]
print(outDir)
for file in glob.glob("{}/Model_*.txt".format(outDir)):
    modelsInputList.append(file)

#ERR_MODELS_INPUT_LIST=""
errModelsInputList=[]
for file in glob.glob("{}/Err_Est_Model_*.txt".format(outDir)):
    errModelsInputList.append(file)
    

#MODEL_FILE="$OUT_FOLDER/model_file.txt"
modelFile = "{}/model_file.txt".format(outDir)
#ERR_MODEL_FILE="$OUT_FOLDER/err_model_file.txt"
errModelFile = "{}/err_model_file.txt".format(outDir)

#FITTED_LIST_FILE="$OUT_FOLDER/FittedFilesList.txt"
fittedListFile = "{}/FittedFilesList.txt".format(outDir)
#REPROCESSED_LIST_FILE="$OUT_FOLDER/ReprocessedFilesList.txt"
reprocessedListFile = "{}/ReprocessedFilesist.txt".format(outDir)

#ProfileReprocessing parameters
ALGO_LOCAL_BWR="2"
ALGO_LOCAL_FWR="0"

with open(paramsLaiRetrFilenameXML, 'w') as paramsFileXML:
    root = ET.Element('metadata')
    pr= ET.SubElement(root, "ProfileReprocessing_parameters")
    ET.SubElement(pr, "bwr_for_algo_local_online_retrieval").text = ALGO_LOCAL_BWR
    ET.SubElement(pr, "fwr_for_algo_local_online_retrieval").text = ALGO_LOCAL_FWR
    usedXMLs = ET.SubElement(root, "XML_files")
    i = 0
    for xml in args.input:
        ET.SubElement(usedXMLs, "XML_" + str(i)).text = xml
        i += 1
    paramsFileXML.write(prettify(root))


paramsFilename = "{}/lai_retrieval_params.txt".format(outDir)
with open(paramsFilename, 'w') as paramsFile:
    paramsFile.write("ProfileReprocessing parameters\n")
    paramsFile.write("    bwr for algo local (online retrieval) = {}\n".format(ALGO_LOCAL_BWR))
    paramsFile.write("    fwr for algo local (online retrieval) = {}\n".format(ALGO_LOCAL_FWR))
    paramsFile.write("Used XML files\n")
    for xml in args.input:
        paramsFile.write("  " + xml + "\n")        

cnt=int(0)
print("Processing started: " + str(datetime.datetime.now()))
start = time.time()
for xml in args.input:
    
    #timed_exec "try otbcli NdviRviExtraction $IMG_INV_OTB_LIBS_ROOT -xml $xml $RESOLUTION_OPTION -fts $OUT_NDVI_RVI"
    if resolution == 0:
        runCmd(["otbcli", "NdviRviExtraction", imgInvOtbLibsLocation, "-xml", xml, "-fts", outNdviRvi])
    else:
        runCmd(["otbcli", "NdviRviExtraction", imgInvOtbLibsLocation, "-xml", xml, "-outres", resolution, "-fts", outNdviRvi])

    #timed_exec "try otbcli GetLaiRetrievalModel $IMG_INV_OTB_LIBS_ROOT -xml $xml -ilmodels $MODELS_INPUT_LIST -ilerrmodels $ERR_MODELS_INPUT_LIST -outm $MODEL_FILE -outerr $ERR_MODEL_FILE"
    runCmd(["otbcli", "GetLaiRetrievalModel", imgInvOtbLibsLocation, "-xml", xml, "-ilmodels"] + modelsInputList + ["-ilerrmodels"] + errModelsInputList + ["-outm", modelFile, "-outerr", errModelFile])
    
    #CUR_OUT_LAI_IMG=${OUT_LAI_IMG//[#]/$cnt}
    counterString = str(cnt)
    curOutLaiImg = outLaiImg.replace("#", counterString)
    #CUR_OUT_LAI_ERR_IMG=${OUT_LAI_ERR_IMG//[#]/$cnt}
    curOutLaiErrImg = outLaiErrImg.replace("#", counterString)
    
    #timed_exec "try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -modelfile $MODEL_FILE -out $CUR_OUT_LAI_IMG"
    runCmd(["otbcli", "BVImageInversion", imgInvOtbLibsLocation, "-in", outNdviRvi, "-modelfile", modelFile, "-out", curOutLaiImg])
    #timed_exec "try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -modelfile $ERR_MODEL_FILE -out $CUR_OUT_LAI_ERR_IMG"
    runCmd(["otbcli", "BVImageInversion", imgInvOtbLibsLocation, "-in", outNdviRvi, "-modelfile", errModelFile, "-out", curOutLaiErrImg])
    
    cnt += 1


i = int(0)

#ALL_XML_PARAM=""
allXmlParam=[]
#ALL_LAI_PARAM=""
allLaiParam=[]
#ALL_ERR_PARAM=""
allErrParam=[]
#build the parameters -ilxml for -illai
while i < cnt:
    #CUR_LAI_IMG=${OUT_LAI_IMG//[#]/$i}
    counterString = str(i)
    #curLaiImg = outLaiImg.replace("#", counterString)
    #CUR_ERR_IMG=${OUT_LAI_ERR_IMG//[#]/$i}
    #curErrImg = outLaiErrImg.replace("#", counterString)
    #ALL_XML_PARAM=$ALL_XML_PARAM" "${inputXML[$i]}
    allXmlParam.append(args.input[i])
    #ALL_LAI_PARAM=$ALL_LAI_PARAM" "$CUR_LAI_IMG
    allLaiParam.append(outLaiImg.replace("#", counterString))
    #ALL_ERR_PARAM=$ALL_ERR_PARAM" "$CUR_ERR_IMG
    allErrParam.append(outLaiErrImg.replace("#", counterString))
    
    i += 1

# Create the LAI and Error time series
#timed_exec "try otbcli TimeSeriesBuilder $IMG_INV_OTB_LIBS_ROOT -il $ALL_LAI_PARAM -out $OUT_LAI_TIME_SERIES"
runCmd(["otbcli", "TimeSeriesBuilder", imgInvOtbLibsLocation, "-il"] + allLaiParam + ["-out", outLaiTimeSeries])
#timed_exec "try otbcli TimeSeriesBuilder $IMG_INV_OTB_LIBS_ROOT -il $ALL_ERR_PARAM -out $OUT_ERR_TIME_SERIES"
runCmd(["otbcli", "TimeSeriesBuilder", imgInvOtbLibsLocation, "-il"] + allErrParam + ["-out", outErrTimeSeries])

# Compute the reprocessed time series (On-line Retrieval)
#timed_exec "try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_REPROCESSED_TIME_SERIES  -algo local -algo.local.bwr $ALGO_LOCAL_BWR -algo.local.fwr $ALGO_LOCAL_FWR"
runCmd(["otbcli", "ProfileReprocessing", imgInvOtbLibsLocation, "-lai", outLaiTimeSeries, "-err", outErrTimeSeries, "-ilxml"] + allXmlParam + ["-opf", outReprocessedTimeSeries, "-algo", "local", "-algo.local.bwr", str(ALGO_LOCAL_BWR), "-algo.local.fwr", str(ALGO_LOCAL_FWR)])

#split the Reprocessed time series to a number of images
#timed_exec "try otbcli ReprocessedProfileSplitter $IMG_INV_OTB_LIBS_ROOT -in $OUT_REPROCESSED_TIME_SERIES -outlist $REPROCESSED_LIST_FILE -compress 1"
runCmd(["otbcli", "ReprocessedProfileSplitter", imgInvOtbLibsLocation, "-in", outReprocessedTimeSeries, "-outlist", reprocessedListFile, "-compress", "1"])

# Compute the fitted time series (CSDM Fitting)
#timed_exec "try otbcli ProfileReprocessing $IMG_INV_OTB_LIBS_ROOT -lai $OUT_LAI_TIME_SERIES -err $OUT_ERR_TIME_SERIES -ilxml $ALL_XML_PARAM -opf $OUT_FITTED_TIME_SERIES -algo fit"
runCmd(["otbcli", "ProfileReprocessing", imgInvOtbLibsLocation, "-lai", outLaiTimeSeries, "-err", outErrTimeSeries, "-ilxml"] + allXmlParam + ["-opf", outFittedTimeSeries, "-algo", "fit"])

#try otbcli ProductFormatter "$PRODUCT_FORMATER_OTB_LIBS_ROOT" -destroot "$OUT_FOLDER" -fileclass SVT1 -level L3B -timeperiod 20130228_20130615 -baseline 01.00 -processor vegetation -processor.vegetation.lairepr "$OUT_REPROCESSED_TIME_SERIES" -processor.vegetation.laifit "$OUT_FITTED_TIME_SERIES" -il "${inputXML[0]}" -gipp "$PARAMS_TXT"
runCmd(["otbcli", "ProductFormatter", productFormatterLocation, "-destroot", outDir, "-fileclass", "SVT1", "-level", "L3B", "-timeperiod", t0 + '_' + tend, "-baseline", "01.00", "-processor", "vegetation", "-processor.vegetation.lairepr", outReprocessedTimeSeries, "-processor.vegetation.laifit", outFittedTimeSeries, "-il", args.input[0], "-gipp", paramsLaiModelFilenameXML, paramsLaiRetrFilenameXML])

#split the Fitted time series to a number of images
#timed_exec "try otbcli ReprocessedProfileSplitter $IMG_INV_OTB_LIBS_ROOT -in $OUT_FITTED_TIME_SERIES -outlist $FITTED_LIST_FILE -compress 1"
runCmd(["otbcli", "ReprocessedProfileSplitter", imgInvOtbLibsLocation, "-in", outFittedTimeSeries, "-outlist", fittedListFile, "-compress", "1"])

print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

'''
''' and None
