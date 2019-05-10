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


class LaiModel(object):
    def __init__(self):
        """ Constructor """
        self.init = 1
        self.modelFile=""
        self.modelErrFile=""

    def getReducedAngle(self, decimal):
        dec, int = math.modf(decimal * 10)
        return (int / 10)

    def generateModel(self, curXml, outDir, paramsLaiModelFilenameXML):
        outGeneratedSampleFile = outDir + '/out_bv_dist_samples.txt'

        #parameters for BVInputVariableGeneration
        GENERATED_SAMPLES_NO="40000"

        #parameters for TrainingDataGenerator
        #BV_IDX="0"
        ADD_REFLS="1"
        #RED_INDEX="1"
        #NIR_INDEX="2"

        #parameters for generating model
        REGRESSION_TYPE="nn"
        BEST_OF="1"

        # Variables for Prosail Simulator
        NOISE_VAR="0.01"

        outSimuReflsFile = outDir + '/out_simu_refls.txt'
        outTrainingFile = outDir + '/out_training.txt'
        outAnglesFile = outDir + '/out_angles.txt'
        imgModelFileName = outDir + '/img_model.txt'
        errEstModelFileName = outDir + '/err_est_model.txt'
        newModelsNamesFile = outDir + '/new_models_names.txt'

        #generating Input BV Distribution file
        print("Generating Input BV Distribution file ...")
        runCmd(["otbcli", "BVInputVariableGeneration", appLocation,
                "-samples", str(GENERATED_SAMPLES_NO),
                "-out",  outGeneratedSampleFile])

        # Generating simulation reflectances
        print("Generating simulation reflectances ...")
        if not rsrCfg:
            if not rsrFile:
                print("Please provide the rsrcfg or rsrfile!")
                exit(1)
            else:
                runCmd(["otbcli", "ProSailSimulator", appLocation,
                        "-xml", curXml,
                        "-bvfile", outGeneratedSampleFile,
                        "-rsrfile", rsrFile,
                        "-out", outSimuReflsFile,
                        "-outangles", outAnglesFile,
                        "-noisevar", str(NOISE_VAR)])
        else:
            runCmd(["otbcli", "ProSailSimulator", appLocation,
                    "-xml", curXml,
                    "-bvfile", outGeneratedSampleFile,
                    "-rsrcfg", rsrCfg,
                    "-out", outSimuReflsFile,
                    "-outangles", outAnglesFile,
                    "-noisevar", str(NOISE_VAR)])

        # Generating training file
        print("Generating training file ...")
        runCmd(["otbcli", "TrainingDataGenerator", appLocation,
                "-xml", curXml,
                "-biovarsfile", outGeneratedSampleFile,
                "-simureflsfile", outSimuReflsFile,
                "-outtrainfile", outTrainingFile,
                "-addrefls", str(ADD_REFLS)])

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

        # Generating model
        print("Generating model ...")
        runCmd(["otbcli", "InverseModelLearning", appLocation,
                "-training", outTrainingFile,
                "-out", imgModelFileName,
                "-errest", errEstModelFileName,
                "-regression", str(REGRESSION_TYPE),
                "-bestof", str(BEST_OF),
                "-xml", curXml,
                "-computedmodelnames", newModelsNamesFile,
                "-newnamesoutfolder", modelsFolder])
        with open(newModelsNamesFile) as f:
            content = f.readlines()
            self.modelFile = content[0]
            self.modelErrFile = content[1]

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
            ET.SubElement(tr, "BV_index").text = "0"
            ET.SubElement(tr, "add_refectances").text = ADD_REFLS
            #ET.SubElement(tr, "RED_band_index").text = RED_INDEX
            #ET.SubElement(tr, "NIR_band_index").text = NIR_INDEX
            iv = ET.SubElement(root, "Weight_ON")
            ET.SubElement(iv, "regression_type").text = REGRESSION_TYPE
            ET.SubElement(iv, "best_of").text = BEST_OF
            ET.SubElement(iv, "generated_model_filename").text = self.modelFile
            ET.SubElement(iv, "generated_error_estimation_model_file_name").text = self.modelErrFile

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
            paramsFile.write("    BV Index                      = {}\n".format(0))
            paramsFile.write("    Add reflectances              = {}\n".format(ADD_REFLS))
            #paramsFile.write("    RED Band Index                = {}\n".format(RED_INDEX))
            #paramsFile.write("    NIR Band Index                = {}\n".format(NIR_INDEX))
            paramsFile.write("Inverse model generation (InverseModelLearning)\n")
            paramsFile.write("    Regression type               = {}\n".format(REGRESSION_TYPE))
            paramsFile.write("    Best of                       = {}\n".format(BEST_OF))
            paramsFile.write("    Generated model file name     = {}\n".format(self.modelFile))
            paramsFile.write("    Generated error estimation model file name = {}\n".format(self.modelErrFile))


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='LAI retrieval processor')

    parser.add_argument('--applocation', help='The path where the sen2agri is built', required=True)
    parser.add_argument('--input', help='The list of products xml descriptors', required=True, nargs='+')
    parser.add_argument('--res', help='The requested resolution in meters', required=True)
    parser.add_argument('--outdir', help="Output directory", required=True)
    parser.add_argument('--rsrfile', help='The RSR file (/path/filename)', required=False)
    parser.add_argument('--rsrcfg', help='The RSR configuration file each mission (/path/filename)', required=False)
    parser.add_argument('--tileid', help="Tile id", required=False)
    parser.add_argument('--modelsfolder', help='The folder where the models are located. If not specified, is considered the outdir', required=False)
    parser.add_argument('--generatemodel', help='Generate the model (YES/NO)', required=False)
    parser.add_argument('--siteid', help='The site ID', required=False)

    args = parser.parse_args()

    appLocation = args.applocation
    resolution = args.res
    outDir = args.outdir
    rsrFile = args.rsrfile
    rsrCfg = args.rsrcfg
    generateModel = args.generatemodel
    siteId = "nn"
    if args.siteid:
        siteId = args.siteid

    if (generateModel == "YES"):
        GENERATE_MODEL = True
    else:
        GENERATE_MODEL = False

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

    if resolution != 10 and resolution != 20:
        print("The resolution is : {}".format(resolution))
        print("The resolution should be either 10 or 20.")
        print("The product will be created with the original resolution without resampling.")
        resolution=0

    outSingleNdvi = "{}/#_Single_NDVI.tif".format(outDir)
    outNdviRvi = "{}/#_NDVI_RVI.tif".format(outDir)
    outLaiImg = "{}/#_LAI_img.tif".format(outDir)
    outLaiErrImg = "{}/#_LAI_err_img.tif".format(outDir)
    outLaiMonoMskFlgsImg = "{}/#_LAI_mono_date_mask_flags_img.tif".format(outDir)
    # LAI images encoded as short. These are used only by ProductFormatter
    outLaiShortImg = "{}/#_LAI_img_16.tif".format(outDir)
    outLaiErrShortImg = "{}/#_LAI_err_img_16.tif".format(outDir)

    cnt=int(0)
    print("Processing started: " + str(datetime.datetime.now()))
    start = time.time()

    allXmlParam=[]
    allLaiParam=[]
    allErrParam=[]
    allMskFlagsParam=[]

    for idx, xml in enumerate(args.input):
        counterString = str(cnt)

        # Compute the LAI model
        laiModel = LaiModel()
        laiModel.generateModel(xml,outDir,paramsLaiModelFilenameXML)

        lastPoint = xml.rfind('.')
        lastSlash = xml.rfind('/')
        if lastPoint != -1 and lastSlash != -1 and lastSlash + 1 < lastPoint:
            counterString = xml[lastSlash + 1:lastPoint]

        curOutSingleNDVIImg = outSingleNdvi.replace("#", counterString)
        curOutNDVIRVIImg = outNdviRvi.replace("#", counterString)
        curOutLaiImg = outLaiImg.replace("#", counterString)
        curOutLaiErrImg = outLaiErrImg.replace("#", counterString)
        curOutLaiMonoMskFlgsImg = outLaiMonoMskFlgsImg.replace("#", counterString)
        # LAI images encoded as short. These are used only by ProductFormatter
        curOutLaiShortImg = outLaiShortImg.replace("#", counterString)
        curOutLaiErrShortImg = outLaiErrShortImg.replace("#", counterString)

        runCmd(["otbcli", "GenerateLaiMonoDateMaskFlags", appLocation,
                "-inxml", xml,
                "-out", curOutLaiMonoMskFlgsImg])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

        #Compute NDVI and RVI
        if resolution == 0:
            runCmd(["otbcli", "NdviRviExtractionNew", appLocation,
            "-xml", xml,
            "-ndvi", curOutSingleNDVIImg,
            "-msks", curOutLaiMonoMskFlgsImg,
            "-fts", curOutNDVIRVIImg,
            "-laicfgs", "/usr/share/sen2agri/Lai_Bands_Cfgs.cfg"])
        else:
            runCmd(["otbcli", "NdviRviExtractionNew", appLocation,
            "-xml", xml,
            "-outres", resolution,
            "-ndvi", curOutSingleNDVIImg,
            "-msks", curOutLaiMonoMskFlgsImg,
            "-fts", curOutNDVIRVIImg,
            "-laicfgs", "/usr/share/sen2agri/Lai_Bands_Cfgs.cfg"])

        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

        # Retrieve the LAI model
        runCmd(["otbcli", "BVImageInversion", appLocation,
                "-in", curOutNDVIRVIImg,
                "-out", curOutLaiImg,
                "-xml", xml,
                "-modelsfolder", modelsFolder,
                "-modelprefix", "Model_"])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

        runCmd(["otbcli", "BVImageInversion", appLocation,
                "-in", curOutNDVIRVIImg,
                "-out", curOutLaiErrImg,
                "-xml", xml,
                "-modelsfolder", modelsFolder,
                "-modelprefix", "Err_Est_Model_"])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

        runCmd(["otbcli", "QuantifyImage", appLocation,
                "-in", curOutLaiImg,
                "-out", curOutLaiShortImg])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
        runCmd(["otbcli", "QuantifyImage", appLocation,
                "-in", curOutLaiErrImg,
                "-out", curOutLaiErrShortImg])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
        
        allXmlParam.append(xml)
        allLaiParam.append(curOutLaiImg)
        allErrParam.append(curOutLaiErrImg)
        allMskFlagsParam.append(curOutLaiMonoMskFlgsImg)

        cnt += 1
        
        # Generate the product for the monodate
        runCmd(["otbcli", "ProductFormatter", appLocation,
                "-destroot", outDir,
                "-fileclass", "SVT1", "-level", "L3B", "-baseline", "01.00", "-siteid", siteId,
                "-processor", "vegetation",
                "-processor.vegetation.laindvi", tileID, curOutSingleNDVIImg,
                "-processor.vegetation.laimonodate", tileID, curOutLaiShortImg,
                "-processor.vegetation.laimonodateerr", tileID, curOutLaiErrShortImg,
                "-processor.vegetation.laistatusflgs", tileID, allMskFlagsParam[idx],
                "-il", xml, 
                "-gipp", paramsLaiModelFilenameXML])

#    fittedRastersListFile = "{}/FittedRastersFilesList.txt".format(outDir)
#    fittedFlagsListFile = "{}/FittedFlagsFilesList.txt".format(outDir)
#    reprocessedRastersListFile = "{}/ReprocessedRastersFilesist.txt".format(outDir)
#    reprocessedFlagsListFile = "{}/ReprocessedFlagsFilesist.txt".format(outDir)
#
#    runCmd(["otbcli", "ProductFormatter", appLocation,
#           "-destroot", outDir,
#           "-fileclass", "OPER",
#           "-level", "L3B",
#           "-baseline", "01.00",
#           "-siteid", "S01",           
#           "-processor", "vegetation",
#           "-processor.vegetation.laindvi", tileID] + allSingleNdviFilesList + [
#           "-processor.vegetation.laimonodate", tileID] + allLaiShortParam + [
#           "-processor.vegetation.laimonodateerr", tileID] + allErrShortParam + [
#           "-processor.vegetation.laistatusflgs", tileID] + allMskFlagsParam + [
#           "-processor.vegetation.filelaireproc", tileID, reprocessedRastersListFile,
#           "-processor.vegetation.filelaireprocflgs", tileID, reprocessedFlagsListFile,
#           "-processor.vegetation.filelaifit", tileID, fittedRastersListFile,
#           "-processor.vegetation.filelaifitflgs", tileID, fittedFlagsListFile] + [
#           "-il"] + allXmlParam + [
#           "-gipp",  paramsLaiModelFilenameXML, paramsLaiRetrFilenameXML])

    print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

    '''
    ''' and None
