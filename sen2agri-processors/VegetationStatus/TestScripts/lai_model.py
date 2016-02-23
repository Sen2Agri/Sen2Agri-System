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
parser.add_argument('--outdir', help="Output directory", required=True)
parser.add_argument('--rsrfile', help='The RSR file (/path/filename)', required=True)
parser.add_argument('--solarzenith', help='The value for solar zenith angle', type=float, required=True)
parser.add_argument('--sensorzenith', help='The value for sensor zenith angle', type=float, required=True)
parser.add_argument('--relativeazimuth', help='The value for relative azimuth angle', type=float, required=True)

args = parser.parse_args()

appLocation = args.applocation
outDir = args.outdir
rsrFile = args.rsrfile
solarZenithAngle = args.solarzenith
sensorZenithAngle = args.sensorzenith
relativeAzimuthAngle = args.relativeazimuth

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
        os.makedirs(args.modelsfolder)
        modelsFolder = args.modelsfolder

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

        #OUT_SIMU_REFLS_FILE="$OUT_FOLDER/out_simu_refls.txt"
        outSimuReflsFile = outDir + '/out_simu_refls.txt'
        #OUT_TRAINING_FILE="$OUT_FOLDER/out_training.txt"
        outTrainingFile = outDir + '/out_training.txt'
        outAnglesFile = outDir + '/out_angles.txt'

        #generating Input BV Distribution file
        print("Generating Input BV Distribution file ...")
        #try otbcli BVInputVariableGeneration $IMG_INV_OTB_LIBS_ROOT -samples $GENERATED_SAMPLES_NO -out $OUT_GENERATED_SAMPLE_FILE
        runCmd(["otbcli", "BVInputVariableGeneration", appLocation, "-samples", str(GENERATED_SAMPLES_NO), "-out",  outGeneratedSampleFile])

        # Generating simulation reflectances
        print("Generating simulation reflectances ...")
        #try otbcli ProSailSimulator $IMG_INV_OTB_LIBS_ROOT -bvfile $OUT_GENERATED_SAMPLE_FILE -rsrfile $RSR_FILE -out $OUT_SIMU_REFLS_FILE -solarzenith $SOLAR_ZENITH_ANGLE -sensorzenith $SENSOR_ZENITH_ANGLE -azimuth $RELATIVE_AZIMUTH_ANGLE
        #runCmd(["otbcli", "ProSailSimulator", imgInvOtbLibsLocation, "-xml", args.input[0], "-bvfile", outGeneratedSampleFile, "-rsrfile", rsrFile, "-out", outSimuReflsFile, "-solarzenith", str(solarZenithAngle), "-sensorzenith", str(sensorZenithAngle), "-azimuth", str(relativeAzimuthAngle), "-outangles", outAnglesFile])
        runCmd(["otbcli", "ProSailSimulator", appLocation, "-xml", curXml, "-bvfile", outGeneratedSampleFile, "-rsrfile", rsrFile, "-out", outSimuReflsFile, "-outangles", outAnglesFile])

        # Generating training file
        print("Generating training file ...")
        #try otbcli TrainingDataGenerator $IMG_INV_OTB_LIBS_ROOT -biovarsfile $OUT_GENERATED_SAMPLE_FILE -simureflsfile $OUT_SIMU_REFLS_FILE -outtrainfile $OUT_TRAINING_FILE -bvidx $BV_IDX -addrefls $ADD_REFLS -redidx $RED_INDEX -niridx $NIR_INDEX
        runCmd(["otbcli", "TrainingDataGenerator", appLocation, "-biovarsfile", outGeneratedSampleFile, "-simureflsfile", outSimuReflsFile, "-outtrainfile", outTrainingFile, "-bvidx", str(BV_IDX), "-addrefls", str(ADD_REFLS), "-redidx", str(RED_INDEX), "-niridx", str(NIR_INDEX)])

        # Reading the used angles from the file and build the out model file name and the out err model file name
        with open(outAnglesFile) as f:
            content = f.readlines()
            solarZenithAngle = float(content[0])
            sensorZenithAngle = float(content[1])
            relativeAzimuthAngle = float(content[2])
            print("Read solar ZENITH ANGLE {}".format(solarZenithAngle))
            print("Read sensor ZENITH ANGLE {}".format(sensorZenithAngle))
            print("Read Rel Azimuth ANGLE {}".format(relativeAzimuthAngle))

        #SOLAR_ZENITH_REDUCED=$( getReducedAngle $SOLAR_ZENITH_ANGLE)
        solarZenithReduced = self.getReducedAngle(solarZenithAngle)
        #SENSOR_ZENITH_REDUCED=$( getReducedAngle $SENSOR_ZENITH_ANGLE)
        sensorZenithReduced = self.getReducedAngle(sensorZenithAngle)
        #REL_AZIMUTH_REDUCED=$( getReducedAngle $RELATIVE_AZIMUTH_ANGLE)
        relativeAzimuthReduced = self.getReducedAngle(relativeAzimuthAngle)

        print("SOLAR ANGLE reduced from {} to {}".format(solarZenithAngle, solarZenithReduced))
        print("SENSOR ANGLE reduced from {} to {}".format(sensorZenithAngle, sensorZenithReduced))
        print("AZIMUTH ANGLE reduced from {} to {}".format(relativeAzimuthAngle, relativeAzimuthReduced))

        outModelFile = "{0}/Model_THETA_S_{1}_THETA_V_{2}_REL_PHI_{3}.txt".format(modelsFolder, solarZenithReduced, sensorZenithReduced, relativeAzimuthReduced)
        outErrEstFile = "{0}/Err_Est_Model_THETA_S_{1}_THETA_V_{2}_REL_PHI_{3}.txt".format(modelsFolder, solarZenithReduced, sensorZenithReduced, relativeAzimuthReduced)

        # Generating model
        print("Generating model ...")
        #try otbcli InverseModelLearning $IMG_INV_OTB_LIBS_ROOT -training $OUT_TRAINING_FILE -out $OUT_MODEL_FILE -errest $OUT_ERR_EST_FILE -regression $REGRESSION_TYPE -bestof $BEST_OF
        runCmd(["otbcli", "InverseModelLearning", appLocation, "-training", outTrainingFile, "-out", outModelFile, "-errest", outErrEstFile, "-regression", str(REGRESSION_TYPE), "-bestof", str(BEST_OF)])

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

for xml1 in args.input:
    laiModel = LaiModel()
    laiModel.generateModel(xml1)
