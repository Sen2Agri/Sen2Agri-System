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


GENERATE_MODEL = True

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
parser.add_argument('--t0', help='The start date for the multi-date LAI retrieval pocedure (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--tend', help='The end date for the multi-date LAI retrieval pocedure (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--outdir', help="Output directory", required=True)
parser.add_argument('--rsrfile', help='The RSR file (/path/filename)', required=True)
parser.add_argument('--solarzenith', help='The value for solar zenith angle', type=float, required=True)
parser.add_argument('--sensorzenith', help='The value for sensor zenith angle', type=float, required=True)
parser.add_argument('--relativeazimuth', help='The value for relative azimuth angle', type=float, required=True)
parser.add_argument('--tileid', help="Tile id", required=False)

args = parser.parse_args()

appLocation = args.applocation
resolution = args.res
t0 = args.t0
tend = args.tend
outDir = args.outdir
rsrFile = args.rsrfile
solarZenithAngle = args.solarzenith
sensorZenithAngle = args.sensorzenith
relativeAzimuthAngle = args.relativeazimuth

vegetationStatusLocation = "{}/VegetationStatus".format(appLocation)
productFormatterLocation = "{}/MACCSMetadata/src".format(appLocation)
imgInvOtbLibsLocation = vegetationStatusLocation + '/otb-bv/src/applications'

tileID="TILE_none"
if args.tileid:
    tileID = "TILE_{}".format(args.tileid)

if os.path.exists(outDir):
    if not os.path.isdir(outDir):
        print("Can't create the output directory because there is a file with the same name")
        print("Remove: " + outDir)
        exit(1)
else:
    os.makedirs(outDir)

paramsLaiModelFilenameXML = "{}/lai_model_params.xml".format(outDir)
paramsLaiRetrFilenameXML = "{}/lai_retrieval_params.xml".format(outDir)

class LaiModel(object):
    def __init__(self):
        self.init = 1

    def getReducedAngle(self, decimal):
        dec, int = math.modf(decimal * 10)
        return (int / 10)
    
    def generateModel(self):
        outGeneratedSampleFile = outDir + '/out_bv_dist_samples.txt'
        # THESE ARE DEFAULT PARAMETERS - THEY WILL BE OVERWRITTEN BY THE PARAMETERS IN
        # THE INCLUDE FILE

        #parameters for BVInputVariableGeneration
        GENERATED_SAMPLES_NO="40000"

        #parameters for TrainingDataGenerator
        BV_IDX="0"
        ADD_REFLS="0"
        RED_INDEX="0"
        NIR_INDEX="2"

        #parameters for generating model
        REGRESSION_TYPE="nn"
        BEST_OF="1"

        # Variables for Prosail Simulator

        #OUT_SIMU_REFLS_FILE="$OUT_FOLDER/out_simu_refls.txt"
        outSimuReflsFile = outDir + '/out_simu_refls.txt'
        #OUT_TRAINING_FILE="$OUT_FOLDER/out_training.txt"
        outTrainingFile = outDir + '/out_training.txt'

        #SOLAR_ZENITH_REDUCED=$( getReducedAngle $SOLAR_ZENITH_ANGLE)
        solarZenithReduced = self.getReducedAngle(solarZenithAngle)
        #SENSOR_ZENITH_REDUCED=$( getReducedAngle $SENSOR_ZENITH_ANGLE)
        sensorZenithReduced = self.getReducedAngle(sensorZenithAngle)
        #REL_AZIMUTH_REDUCED=$( getReducedAngle $RELATIVE_AZIMUTH_ANGLE)
        relativeAzimuthReduced = self.getReducedAngle(relativeAzimuthAngle)

        print("SOLAR ANGLE reduced from {} to {}".format(solarZenithAngle, solarZenithReduced))
        print("SENSOR ANGLE reduced from {} to {}".format(sensorZenithAngle, sensorZenithReduced))
        print("AZIMUTH ANGLE reduced from {} to {}".format(relativeAzimuthAngle, relativeAzimuthReduced))

        # Variables for InverseModelLearning
        #OUT_MODEL_FILE="$OUT_FOLDER/Model_THETA_S_"$SOLAR_ZENITH_REDUCED"_THETA_V_"$SENSOR_ZENITH_REDUCED"_REL_PHI_"$REL_AZIMUTH_REDUCED".txt"
        outModelFile = "{0}/Model_THETA_S_{1}_THETA_V_{2}_REL_PHI_{3}.txt".format(outDir, solarZenithReduced, sensorZenithReduced, relativeAzimuthReduced)
        #outModelFile = outDir + '/Model_THETA_S_' + str(solarZenithReduced) + '_THETA_V_' + str(sensorZenithReduced) + '_REL_PHI_' + relativeAzimuthReduced + '.txt'
        #OUT_ERR_EST_FILE="$OUT_FOLDER/Err_Est_Model_THETA_S_"$SOLAR_ZENITH_REDUCED"_THETA_V_"$SENSOR_ZENITH_REDUCED"_REL_PHI_"$REL_AZIMUTH_REDUCED".txt"
        outErrEstFile = "{0}/Err_Est_Model_THETA_S_{1}_THETA_V_{2}_REL_PHI_{3}.txt".format(outDir, solarZenithReduced, sensorZenithReduced, relativeAzimuthReduced)
        #outErrEstFile = outDir + '/Err_Est_Model_THETA_S_' + solarZenithReduced + '_THETA_V_' + sensorZenithReduced + '_REL_PHI_' + relativeAzimuthReduced + '.txt'
        with open(paramsLaiModelFilenameXML, 'w') as paramsFileXML:
            root = ET.Element('metadata')
            bv = ET.SubElement(root, "BVInputVariableGeneration")
            ET.SubElement(bv, "Number_of_generated_samples").text = GENERATED_SAMPLES_NO
            proSail = ET.SubElement(root, "ProSailSimulator")
            ET.SubElement(proSail, "RSR_file").text = rsrFile
            ET.SubElement(proSail, "solar_zenith_angle").text = str(solarZenithAngle)
            ET.SubElement(proSail, "sensor_zenith_angle").text = str(sensorZenithAngle)
            ET.SubElement(proSail, "relative_azimuth_angle").text = str(relativeAzimuthAngle)
            tr = ET.SubElement(root, "TrainingDataGenerator")
            ET.SubElement(tr, "BV_index").text = BV_IDX
            ET.SubElement(tr, "add_refectances").text = ADD_REFLS
            ET.SubElement(tr, "RED_band_index").text = RED_INDEX
            ET.SubElement(tr, "NIR_band_index").text = NIR_INDEX
            iv = ET.SubElement(root, "Weight_ON")
            ET.SubElement(iv, "regression_type").text = REGRESSION_TYPE
            ET.SubElement(iv, "best_of").text = BEST_OF
            ET.SubElement(iv, "generated_model_filename").text = outModelFile
            ET.SubElement(iv, "generated_error_estimation_model_file_name").text = outErrEstFile
            #   usedXMLs = ET.SubElement(root, "XML_files")
            #   i = 0
            #   for xml in args.input:    
            #   ET.SubElement(usedXMLs, "XML_" + str(i)).text = xml   
            #   i += 1
            paramsFileXML.write(prettify(root))

        paramsFilename= "{}/generate_lai_model_params.txt".format(outDir)
        with open(paramsFilename, 'w') as paramsFile:
            paramsFile.write("BVInputVariableGeneration\n")
            paramsFile.write("    Number of generated samples    = {}\n".format(GENERATED_SAMPLES_NO))
            paramsFile.write("ProSailSimulator\n")
            paramsFile.write("    RSR file                      = {}\n".format(rsrFile))
            paramsFile.write("    Solar zenith angle            = {}\n".format(solarZenithAngle))
            paramsFile.write("    Sensor zenith angle           = {}\n".format(sensorZenithAngle))
            paramsFile.write("    Relative azimuth angle        = {}\n".format(relativeAzimuthAngle))
            paramsFile.write("TrainingDataGenerator" + "\n")
            paramsFile.write("    BV Index                      = {}\n".format(BV_IDX))
            paramsFile.write("    Add reflectances              = {}\n".format(ADD_REFLS))
            paramsFile.write("    RED Band Index                = {}\n".format(RED_INDEX))
            paramsFile.write("    NIR Band Index                = {}\n".format(NIR_INDEX))
            paramsFile.write("Inverse model generation (InverseModelLearning)\n")
            paramsFile.write("    Regression type               = {}\n".format(REGRESSION_TYPE))
            paramsFile.write("    Best of                       = {}\n".format(BEST_OF))
            paramsFile.write("    Generated model file name     = {}\n".format(outModelFile))
            paramsFile.write("    Generated error estimation model file name = {}\n".format(outErrEstFile))

        #generating Input BV Distribution file
        print("Generating Input BV Distribution file ...")
        #try otbcli BVInputVariableGeneration $IMG_INV_OTB_LIBS_ROOT -samples $GENERATED_SAMPLES_NO -out $OUT_GENERATED_SAMPLE_FILE
        runCmd(["otbcli", "BVInputVariableGeneration", imgInvOtbLibsLocation, "-samples", str(GENERATED_SAMPLES_NO), "-out",  outGeneratedSampleFile])

        # Generating simulation reflectances
        print("Generating simulation reflectances ...")
        #try otbcli ProSailSimulator $IMG_INV_OTB_LIBS_ROOT -bvfile $OUT_GENERATED_SAMPLE_FILE -rsrfile $RSR_FILE -out $OUT_SIMU_REFLS_FILE -solarzenith $SOLAR_ZENITH_ANGLE -sensorzenith $SENSOR_ZENITH_ANGLE -azimuth $RELATIVE_AZIMUTH_ANGLE
        runCmd(["otbcli", "ProSailSimulator", imgInvOtbLibsLocation, "-xml", args.input[0], "-bvfile", outGeneratedSampleFile, "-rsrfile", rsrFile, "-out", outSimuReflsFile, "-solarzenith", str(solarZenithAngle), "-sensorzenith", str(sensorZenithAngle), "-azimuth", str(relativeAzimuthAngle)])

        # Generating training file
        print("Generating training file ...")
        #try otbcli TrainingDataGenerator $IMG_INV_OTB_LIBS_ROOT -biovarsfile $OUT_GENERATED_SAMPLE_FILE -simureflsfile $OUT_SIMU_REFLS_FILE -outtrainfile $OUT_TRAINING_FILE -bvidx $BV_IDX -addrefls $ADD_REFLS -redidx $RED_INDEX -niridx $NIR_INDEX
        runCmd(["otbcli", "TrainingDataGenerator", imgInvOtbLibsLocation, "-biovarsfile", outGeneratedSampleFile, "-simureflsfile", outSimuReflsFile, "-outtrainfile", outTrainingFile, "-bvidx", str(BV_IDX), "-addrefls", str(ADD_REFLS), "-redidx", str(RED_INDEX), "-niridx", str(NIR_INDEX)])

        # Generating model
        print("Generating model ...")
        #try otbcli InverseModelLearning $IMG_INV_OTB_LIBS_ROOT -training $OUT_TRAINING_FILE -out $OUT_MODEL_FILE -errest $OUT_ERR_EST_FILE -regression $REGRESSION_TYPE -bestof $BEST_OF
        runCmd(["otbcli", "InverseModelLearning", imgInvOtbLibsLocation, "-training", outTrainingFile, "-out", outModelFile, "-errest", outErrEstFile, "-regression", str(REGRESSION_TYPE), "-bestof", str(BEST_OF)])


if GENERATE_MODEL:
    laiModel = LaiModel()
    laiModel.generateModel()

if resolution != 10 and resolution != 20:
    print("The resolution is : {}".format(resolution))
    print("The resolution should be either 10 or 20.")
    print("The product will be created with the original resolution without resampling.")
    resolution=0

#OUT_NDVI_RVI="$OUT_FOLDER/ndvi_rvi.tif"
outNdviRvi = "{}/ndvi_rvi.tif".format(outDir)
#OUT_LAI_IMG="$OUT_FOLDER/LAI_img_#.tif"
outLaiImg = "{}/#_LAI_img.tif".format(outDir)
#OUT_LAI_ERR_IMG="$OUT_FOLDER/LAI_err_img_#.tif"
outLaiErrImg = "{}/#_LAI_err_img.tif".format(outDir) 

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
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    #timed_exec "try otbcli GetLaiRetrievalModel $IMG_INV_OTB_LIBS_ROOT -xml $xml -ilmodels $MODELS_INPUT_LIST -ilerrmodels $ERR_MODELS_INPUT_LIST -outm $MODEL_FILE -outerr $ERR_MODEL_FILE"
    runCmd(["otbcli", "GetLaiRetrievalModel", imgInvOtbLibsLocation, "-xml", xml, "-ilmodels"] + modelsInputList + ["-ilerrmodels"] + errModelsInputList + ["-outm", modelFile, "-outerr", errModelFile])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    
    #CUR_OUT_LAI_IMG=${OUT_LAI_IMG//[#]/$cnt}
    counterString = str(cnt)
    lastPoint = xml.rfind('.')
    lastSlash = xml.rfind('/')
    if lastPoint != -1 and lastSlash != -1 and lastSlash + 1 < lastPoint:
        counterString = xml[lastSlash + 1:lastPoint]
    print("counterString = {}".format(counterString))
    curOutLaiImg = outLaiImg.replace("#", counterString)
    print("curOutLaiImg = {}".format(curOutLaiImg))
    #CUR_OUT_LAI_ERR_IMG=${OUT_LAI_ERR_IMG//[#]/$cnt}
    curOutLaiErrImg = outLaiErrImg.replace("#", counterString)
    print("curOutLaiErrImg = {}".format(curOutLaiErrImg))
    continue
    #timed_exec "try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -modelfile $MODEL_FILE -out $CUR_OUT_LAI_IMG"
    runCmd(["otbcli", "BVImageInversion", imgInvOtbLibsLocation, "-in", outNdviRvi, "-modelfile", modelFile, "-out", curOutLaiImg])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    #timed_exec "try otbcli BVImageInversion $IMG_INV_OTB_LIBS_ROOT -in $OUT_NDVI_RVI -modelfile $ERR_MODEL_FILE -out $CUR_OUT_LAI_ERR_IMG"
    runCmd(["otbcli", "BVImageInversion", imgInvOtbLibsLocation, "-in", outNdviRvi, "-modelfile", errModelFile, "-out", curOutLaiErrImg])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    
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
runCmd(["otbcli", "TimeSeriesBuilder", imgInvOtbLibsLocation, "-il"] + allLaiParam + ["-out", outLaiTimeSeries])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
runCmd(["otbcli", "TimeSeriesBuilder", imgInvOtbLibsLocation, "-il"] + allErrParam + ["-out", outErrTimeSeries])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

# Compute the reprocessed time series (On-line Retrieval)
runCmd(["otbcli", "ProfileReprocessing", imgInvOtbLibsLocation, "-lai", outLaiTimeSeries, "-err", outErrTimeSeries, "-ilxml"] + allXmlParam + ["-opf", outReprocessedTimeSeries, "-algo", "local", "-algo.local.bwr", str(ALGO_LOCAL_BWR), "-algo.local.fwr", str(ALGO_LOCAL_FWR)])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

#split the Reprocessed time series to a number of images
runCmd(["otbcli", "ReprocessedProfileSplitter", imgInvOtbLibsLocation, "-in", outReprocessedTimeSeries, "-outlist", reprocessedListFile, "-compress", "1"])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

# Compute the fitted time series (CSDM Fitting)
runCmd(["otbcli", "ProfileReprocessing", imgInvOtbLibsLocation, "-lai", outLaiTimeSeries, "-err", outErrTimeSeries, "-ilxml"] + allXmlParam + ["-opf", outFittedTimeSeries, "-algo", "fit"])
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

cmd = ["otbcli", "ProductFormatter", productFormatterLocation, "-destroot", outDir, "-fileclass", "SVT1", "-level", "L3B", "-timeperiod", t0 + '_' + tend, "-baseline", "01.00", "-processor", "vegetation", "-processor.vegetation.lairepr", tileID, outReprocessedTimeSeries, "-processor.vegetation.laifit", tileID, outFittedTimeSeries, "-il", args.input[0], "-gipp", paramsLaiRetrFilenameXML]
if GENERATE_MODEL:
    cmd = ["otbcli", "ProductFormatter", productFormatterLocation, "-destroot", outDir, "-fileclass", "SVT1", "-level", "L3B", "-timeperiod", t0 + '_' + tend, "-baseline", "01.00", "-processor", "vegetation", "-processor.vegetation.lairepr", tileID, outReprocessedTimeSeries, "-processor.vegetation.laifit", tileID, outFittedTimeSeries, "-il", args.input[0], "-gipp", paramsLaiModelFilenameXML, paramsLaiRetrFilenameXML]

runCmd(cmd)
print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
#split the Fitted time series to a number of images
#timed_exec "try otbcli ReprocessedProfileSplitter $IMG_INV_OTB_LIBS_ROOT -in $OUT_FITTED_TIME_SERIES -outlist $FITTED_LIST_FILE -compress 1"
runCmd(["otbcli", "ReprocessedProfileSplitter", imgInvOtbLibsLocation, "-in", outFittedTimeSeries, "-outlist", fittedListFile, "-compress", "1"])

print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

'''
''' and None
