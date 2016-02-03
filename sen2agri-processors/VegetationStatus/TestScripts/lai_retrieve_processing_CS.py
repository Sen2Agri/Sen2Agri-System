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
parser.add_argument('--t0', help='The start date for the multi-date LAI retrieval pocedure (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--tend', help='The end date for the multi-date LAI retrieval pocedure (in format YYYYMMDD)',
                    required=True, metavar='YYYYMMDD')
parser.add_argument('--outdir', help="Output directory", required=True)
parser.add_argument('--rsrfile', help='The RSR file (/path/filename)', required=True)
#parser.add_argument('--solarzenith', help='The value for solar zenith angle', type=float, required=True)
#parser.add_argument('--sensorzenith', help='The value for sensor zenith angle', type=float, required=True)
#parser.add_argument('--relativeazimuth', help='The value for relative azimuth angle', type=float, required=True)
parser.add_argument('--tileid', help="Tile id", required=False)
parser.add_argument('--modelsfolder', help='The folder where the models are located. If not specified, is considered the outdir', required=False)
parser.add_argument('--generatemodel', help='Generate the model (YES/NO)', required=False)

args = parser.parse_args()

appLocation = args.applocation
resolution = args.res
t0 = args.t0
tend = args.tend
outDir = args.outdir
rsrFile = args.rsrfile

# Compute angles from metadata file


tree = ET.parse(args.input[0])
root=tree.getroot()

for elt in root.findall("./RADIOMETRY/ANGLES"):
	phiS=float(elt.find("PHI_S").text)
	phiV=float(elt.find("PHI_V").text)
	thetaS=float(elt.find("THETA_S").text)
	thetaV=float(elt.find("THETA_V").text)

print(phiS, phiV, thetaS, thetaV)

relAz=phiV-180-phiS
if (relAz < -180):
	relAz=relAz+360.0
if (relAz > 180):
	relAz=relAz-360.0

print(relAz, thetaS, thetaV)

solarZenithAngle = thetaS
sensorZenithAngle = thetaV
relativeAzimuthAngle = relAz

generateModel = args.generatemodel

if (generateModel == "YES"):
	GENERATE_MODEL = True
else:
	GENERATE_MODEL = False
	
	
print(GENERATE_MODEL)

vegetationStatusLocation = "{}/VegetationStatus".format(appLocation)
productFormatterLocation = "{}/MACCSMetadata/src".format(appLocation)
imgInvOtbLibsLocation = vegetationStatusLocation + '/otb-bv/src/applications'

tileID="TILE_none"
if args.tileid:
    tileID = "TILE_{}".format(args.tileid)

# By default, if not specified, models folder is the given out dir.
modelsFolder = outDir
if args.modelsfolder:
    if os.path.exists(args.modelsfolder):
        if not os.path.isdir(args.modelsfolder):
            print("Error: The specified models folder is not a folder but a file.")
            exit(1)
        else:
            modelsFolder = args.modelsfolder
    else:
        if GENERATE_MODEL:
            os.makedirs(args.modelsfolder)
            modelsFolder = args.modelsfolder
        else:
            print("Error: The specified models folder does not exist.")
            exit(1)

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

    def generateModel(self, curXml):
        outGeneratedSampleFile = outDir + '/out_bv_dist_samples.txt'
        # THESE ARE DEFAULT PARAMETERS - THEY WILL BE OVERWRITTEN BY THE PARAMETERS IN
        # THE INCLUDE FILE

        #parameters for BVInputVariableGeneration
        GENERATED_SAMPLES_NO="40000"

        #parameters for TrainingDataGenerator
        BV_IDX="0"
        ADD_REFLS="1"
        RED_INDEX="1"
        NIR_INDEX="2"

        #parameters for generating model
        REGRESSION_TYPE="nn"
        BEST_OF="1"

        # Variables for Prosail Simulator
        NOISE_VAR="0.01"

        outSimuReflsFile = outDir + '/out_simu_refls.txt'
        outTrainingFile = outDir + '/out_training.txt'
        outAnglesFile = outDir + '/out_angles.txt'

        #generating Input BV Distribution file
        print("Generating Input BV Distribution file ...")
        runCmd(["otbcli", "BVInputVariableGeneration", imgInvOtbLibsLocation, "-samples", str(GENERATED_SAMPLES_NO), "-out",  outGeneratedSampleFile])

        # Generating simulation reflectances
        print("Generating simulation reflectances ...")
        #runCmd(["otbcli", "ProSailSimulator", imgInvOtbLibsLocation, "-xml", args.input[0], "-bvfile", outGeneratedSampleFile, "-rsrfile", rsrFile, "-out", outSimuReflsFile, "-solarzenith", str(solarZenithAngle), "-sensorzenith", str(sensorZenithAngle), "-azimuth", str(relativeAzimuthAngle), "-outangles", outAnglesFile])
        runCmd(["otbcli", "ProSailSimulator", imgInvOtbLibsLocation, "-xml", curXml, "-bvfile", outGeneratedSampleFile, "-rsrfile", rsrFile, "-out", outSimuReflsFile, "-outangles", outAnglesFile, "-noisevar", str(NOISE_VAR)])

        # Generating training file
        print("Generating training file ...")
        runCmd(["otbcli", "TrainingDataGenerator", imgInvOtbLibsLocation, "-biovarsfile", outGeneratedSampleFile, "-simureflsfile", outSimuReflsFile, "-outtrainfile", outTrainingFile, "-bvidx", str(BV_IDX), "-addrefls", str(ADD_REFLS), "-redidx", str(RED_INDEX), "-niridx", str(NIR_INDEX)])

        # Reading the used angles from the file and build the out model file name and the out err model file name
        with open(outAnglesFile) as f:
            content = f.readlines()
            solarZenithAngle = float(content[0])
            sensorZenithAngle = float(content[1])
            relativeAzimuthAngle = float(content[2])
            print("Read solar ZENITH ANGLE {}".format(solarZenithAngle))
            print("Read sensor ZENITH ANGLE {}".format(sensorZenithAngle))
            print("Read Rel Azimuth ANGLE {}".format(relativeAzimuthAngle))

        solarZenithReduced = self.getReducedAngle(solarZenithAngle)
        sensorZenithReduced = self.getReducedAngle(sensorZenithAngle)
        relativeAzimuthReduced = self.getReducedAngle(relativeAzimuthAngle)

        print("SOLAR ANGLE reduced from {} to {}".format(solarZenithAngle, solarZenithReduced))
        print("SENSOR ANGLE reduced from {} to {}".format(sensorZenithAngle, sensorZenithReduced))
        print("AZIMUTH ANGLE reduced from {} to {}".format(relativeAzimuthAngle, relativeAzimuthReduced))

        outModelFile = "{0}/Model_THETA_S_{1}_THETA_V_{2}_REL_PHI_{3}.txt".format(modelsFolder, solarZenithReduced, sensorZenithReduced, relativeAzimuthReduced)
        outErrEstFile = "{0}/Err_Est_Model_THETA_S_{1}_THETA_V_{2}_REL_PHI_{3}.txt".format(modelsFolder, solarZenithReduced, sensorZenithReduced, relativeAzimuthReduced)

        # Generating model
        print("Generating model ...")
        runCmd(["otbcli", "InverseModelLearning", imgInvOtbLibsLocation, "-training", outTrainingFile, "-out", outModelFile, "-errest", outErrEstFile, "-regression", str(REGRESSION_TYPE), "-bestof", str(BEST_OF)])

        with open(paramsLaiModelFilenameXML, 'w') as paramsFileXML:
            root = ET.Element('metadata')
            bv = ET.SubElement(root, "BVInputVariableGeneration")
            ET.SubElement(bv, "Number_of_generated_samples").text = GENERATED_SAMPLES_NO
            proSail = ET.SubElement(root, "ProSailSimulator")
            ET.SubElement(proSail, "RSR_file").text = rsrFile
            ET.SubElement(proSail, "solar_zenith_angle").text = str(solarZenithAngle)
            ET.SubElement(proSail, "sensor_zenith_angle").text = str(sensorZenithAngle)
            ET.SubElement(proSail, "relative_azimuth_angle").text = str(relativeAzimuthAngle)
            ET.SubElement(proSail, "noisevar").text = str(NOISE_VAR)
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
            paramsFile.write("    Noise var                     = {}\n".format(NOISE_VAR))
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

if GENERATE_MODEL:
    for xml1 in args.input:
        laiModel = LaiModel()
        laiModel.generateModel(xml1)

if resolution != 10 and resolution != 20:
    print("The resolution is : {}".format(resolution))
    print("The resolution should be either 10 or 20.")
    print("The product will be created with the original resolution without resampling.")
    resolution=0

outNdviRvi = "{}/#_NDVI_RVI.tif".format(outDir)
outLaiImg = "{}/#_LAI_img.tif".format(outDir)
outLaiMonoMskFlgsImg = "{}/#_LAI_mono_date_mask_flags_img.tif".format(outDir)
outLaiErrImg = "{}/#_LAI_err_img.tif".format(outDir)

outLaiTimeSeries = "{}/LAI_time_series.tif".format(outDir)
outErrTimeSeries = "{}/Err_time_series.tif".format(outDir)
outMaksFlagsTimeSeries = "{}/Mask_Flags_time_series.tif".format(outDir)

outReprocessedTimeSeries = "{}/ReprocessedTimeSeries.tif".format(outDir)
outFittedTimeSeries = "{}/FittedTimeSeries.tif".format(outDir)

modelsInputList=[]
print(modelsFolder)
for file in glob.glob("{}/Model_*.txt".format(modelsFolder)):
    modelsInputList.append(file)

errModelsInputList=[]
for file in glob.glob("{}/Err_Est_Model_*.txt".format(modelsFolder)):
    errModelsInputList.append(file)


modelFile = "{}/model_file.txt".format(outDir)
errModelFile = "{}/err_model_file.txt".format(outDir)

fittedRastersListFile = "{}/FittedRastersFilesList.txt".format(outDir)
fittedFlagsListFile = "{}/FittedFlagsFilesList.txt".format(outDir)
reprocessedRastersListFile = "{}/ReprocessedRastersFilesist.txt".format(outDir)
reprocessedFlagsListFile = "{}/ReprocessedFlagsFilesist.txt".format(outDir)

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

allXmlParam=[]
allNdviFilesList=[]
allLaiParam=[]
allErrParam=[]
allMskFlagsParam=[]

for xml in args.input:
    counterString = str(cnt)
    lastPoint = xml.rfind('.')
    lastSlash = xml.rfind('/')
    if lastPoint != -1 and lastSlash != -1 and lastSlash + 1 < lastPoint:
        counterString = xml[lastSlash + 1:lastPoint]

    curOutNDVIImg = outNdviRvi.replace("#", counterString)
    curOutLaiImg = outLaiImg.replace("#", counterString)
    curOutLaiErrImg = outLaiErrImg.replace("#", counterString)
    curOutLaiMonoMskFlgsImg = outLaiMonoMskFlgsImg.replace("#", counterString)

    if resolution == 0:
        runCmd(["otbcli", "NdviRviExtraction2", imgInvOtbLibsLocation, "-xml", xml, "-fts", curOutNDVIImg])
    else:
        runCmd(["otbcli", "NdviRviExtraction2", imgInvOtbLibsLocation, "-xml", xml, "-outres", resolution, "-fts", curOutNDVIImg])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    runCmd(["otbcli", "GetLaiRetrievalModel", imgInvOtbLibsLocation, "-xml", xml, "-ilmodels"] + modelsInputList + ["-ilerrmodels"] + errModelsInputList + ["-outm", modelFile, "-outerr", errModelFile])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

    runCmd(["otbcli", "BVImageInversion", imgInvOtbLibsLocation, "-in", curOutNDVIImg, "-modelfile", modelFile, "-out", curOutLaiImg])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    runCmd(["otbcli", "BVImageInversion", imgInvOtbLibsLocation, "-in", curOutNDVIImg, "-modelfile", errModelFile, "-out", curOutLaiErrImg])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    runCmd(["otbcli", "GenerateLaiMonoDateMaskFlags", imgInvOtbLibsLocation, "-inxml", xml, "-out", curOutLaiMonoMskFlgsImg])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

    allXmlParam.append(xml)
    allNdviFilesList.append(curOutNDVIImg)
    allLaiParam.append(curOutLaiImg)
    allErrParam.append(curOutLaiErrImg)
    allMskFlagsParam.append(curOutLaiMonoMskFlgsImg)

    cnt += 1

timeSeries = False

reprocessedFlagsFilesList = []
reprocessedRastersFilesList = []
fittedFlagsFilesList = []
fittedRastersFilesList = []
	
if timeSeries:
	# Create the LAI and Error time series
	runCmd(["otbcli", "TimeSeriesBuilder", imgInvOtbLibsLocation, "-il"] + allLaiParam + ["-out", outLaiTimeSeries])
	print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
	runCmd(["otbcli", "TimeSeriesBuilder", imgInvOtbLibsLocation, "-il"] + allErrParam + ["-out", outErrTimeSeries])
	print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
	runCmd(["otbcli", "TimeSeriesBuilder", imgInvOtbLibsLocation, "-il"] + allMskFlagsParam + ["-out", outMaksFlagsTimeSeries])
	print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))


	# Compute the reprocessed time series (On-line Retrieval)
	runCmd(["otbcli", "ProfileReprocessing", imgInvOtbLibsLocation, "-lai", outLaiTimeSeries, "-err", outErrTimeSeries, "-msks", outMaksFlagsTimeSeries, "-ilxml"] + allXmlParam + ["-opf", outReprocessedTimeSeries, "-genall", "1", "-algo", "local", "-algo.local.bwr", str(ALGO_LOCAL_BWR), "-algo.local.fwr", str(ALGO_LOCAL_FWR)])
	print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

	#split the Reprocessed time series to a number of images
	runCmd(["otbcli", "ReprocessedProfileSplitter2", imgInvOtbLibsLocation, "-in", outReprocessedTimeSeries, "-outrlist", reprocessedRastersListFile, "-outflist", reprocessedFlagsListFile, "-compress", "1", "-ilxml"] + allXmlParam)
	print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

	# Compute the fitted time series (CSDM Fitting)
	runCmd(["otbcli", "ProfileReprocessing", imgInvOtbLibsLocation, "-lai", outLaiTimeSeries, "-err", outErrTimeSeries, "-msks", outMaksFlagsTimeSeries, "-ilxml"] + allXmlParam + ["-opf", outFittedTimeSeries, "-genall", "1", "-algo", "fit"])
	print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

	#split the Fitted time series to a number of images
	runCmd(["otbcli", "ReprocessedProfileSplitter2", imgInvOtbLibsLocation, "-in", outFittedTimeSeries, "-outrlist", fittedRastersListFile, "-outflist", fittedFlagsListFile, "-compress", "1", "-ilxml"] + allXmlParam)
	print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))


	with open(fittedRastersListFile) as f:
		fittedRastersFilesList = f.readlines()
		fittedRastersFilesList = [x.strip('\n') for x in fittedRastersFilesList]


	with open(fittedFlagsListFile) as f:
		fittedFlagsFilesList = f.readlines()
		fittedFlagsFilesList = [x.strip('\n') for x in fittedFlagsFilesList]


	with open(reprocessedRastersListFile) as f:
		reprocessedRastersFilesList = f.readlines()
		reprocessedRastersFilesList = [x.strip('\n') for x in reprocessedRastersFilesList]


	with open(reprocessedFlagsListFile) as f:
		reprocessedFlagsFilesList = f.readlines()
		reprocessedFlagsFilesList = [x.strip('\n') for x in reprocessedFlagsFilesList]

#cmd = ["otbcli", "ProductFormatter", productFormatterLocation, "-destroot", outDir, "-fileclass", "OPER", "-level", "L3B", "-timeperiod", t0 + '_' + tend, "-baseline", "01.00", "-processor", "vegetation", "-processor.vegetation.laindvi", tileID] + allNdviFilesList + ["-processor.vegetation.laimonodate", tileID] + allLaiParam + ["-processor.vegetation.laimonodateerr", tileID] + allErrParam + ["-processor.vegetation.laimdateflgs", tileID] + allMskFlagsParam + ["-processor.vegetation.laireproc", tileID] + reprocessedRastersFilesList + ["-processor.vegetation.laireprocflgs", tileID] + reprocessedFlagsFilesList + ["-processor.vegetation.laifit", tileID] + fittedRastersFilesList + ["-processor.vegetation.laifitflgs", tileID] + fittedFlagsFilesList + ["-il"] + allXmlParam + ["-gipp", paramsLaiRetrFilenameXML]

if GENERATE_MODEL:
    cmd = ["otbcli", "ProductFormatter", productFormatterLocation, "-destroot", outDir, "-fileclass", "OPER", "-level", "L3B", "-timeperiod", t0 + '_' + tend, "-baseline", "01.00", "-processor", "vegetation", "-processor.vegetation.laindvi", tileID] + allNdviFilesList + ["-processor.vegetation.laimonodate", tileID] + allLaiParam + ["-processor.vegetation.laimonodateerr", tileID] + allErrParam + ["-processor.vegetation.laimdateflgs", tileID] + allMskFlagsParam + ["-processor.vegetation.laireproc", tileID] + reprocessedRastersFilesList + ["-processor.vegetation.laireprocflgs", tileID] + reprocessedFlagsFilesList + ["-processor.vegetation.laifit", tileID] + fittedRastersFilesList + ["-processor.vegetation.laifitflgs", tileID] + fittedFlagsFilesList + ["-il"] + allXmlParam + ["-gipp", paramsLaiModelFilenameXML, paramsLaiRetrFilenameXML]

runCmd(cmd)

print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

'''
''' and None
