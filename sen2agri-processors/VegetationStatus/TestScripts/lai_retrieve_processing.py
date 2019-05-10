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
import ntpath


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


def getMonoDateProductFiles(productMeta, siteId, tileId, inLaiMonoDir, useIntermediateFiles):
    fileName = os.path.splitext(os.path.basename(productMeta))[0]
    print("Filename: {}".format(fileName))
    
    #fileName = productMeta
    if fileName.startswith('S2') :
        dateIdx = 8
    if fileName.startswith('L8') :
        dateIdx = 5
    if fileName.startswith('SPOT') :
        dateIdx = 3
    words = fileName.split('_')
    productDate =  words[dateIdx]
    print("Product date: {}".format(productDate))
    
    if useIntermediateFiles :
        laiImgPattern = inLaiMonoDir + '/*_' + productDate + "_*_LAI_img.tif"
        #print("Intermediate LAI file: {}".format(laiImgPattern))
        laiImgsFiles = glob.glob(laiImgPattern)
        laiErrImgPattern = inLaiMonoDir + '/*_' + productDate + "_*_LAI_err_img.tif"
        laiErrImgsFiles = glob.glob(laiErrImgPattern)
        msksPattern = inLaiMonoDir + '/*_' + productDate + "_*_LAI_mono_date_mask_flags_img.tif"
        msksFiles = glob.glob(msksPattern)
        
        if (len(laiImgsFiles) == 0) or (len(laiErrImgsFiles) == 0) or (len(msksFiles) == 0) :
            return ("", "", "")
        laiMonoDateImg = laiImgsFiles[-1]
        laiErrDateImg = laiErrImgsFiles[-1]
        laiMsksDateImg = msksFiles[-1]
    else :
        monodateFolderPattern = inLaiMonoDir + '/S2AGRI_L3B_PRD_S' + siteId + "_*_A" + productDate + "*"
        print("monodateFolderPattern: {}".format(monodateFolderPattern))
        
        monodateFolders = glob.glob(monodateFolderPattern)
        if (len(monodateFolders) == 0):
            print ("No monodate products found!!!")
            return ("", "", "")
        
        #for monodateFolder in monodateFolders:
        #    print("Existing monodates folders: {}".format(monodateFolder))            

        monodateFolder = monodateFolders[-1]
        # Get exactly the product date as it can be either _AyyyyMMdd or _AyyyyMMddTHHmmSS
        productDate  = monodateFolder.split("_A",1)[1]
        print("Product date: {}".format(productDate))
        laiMonoDateImg = monodateFolder + "/TILES/S2AGRI_L3B_A" + productDate + "_T" + tileId + \
                        "/IMG_DATA/S2AGRI_L3B_SLAIMONO_A" +productDate+"_T" + tileId + ".TIF"
        laiErrDateImg = monodateFolder + "/TILES/S2AGRI_L3B_A" + productDate + "_T" + tileId + \
                        "/QI_DATA/S2AGRI_L3B_MLAIERR_A" +productDate+"_T" + tileId + ".TIF"
        laiMsksDateImg = monodateFolder + "/TILES/S2AGRI_L3B_A" + productDate + "_T" + tileId + \
                        "/QI_DATA/S2AGRI_L3B_MMONODFLG_A" +productDate+"_T" + tileId + ".TIF"
        
        #print("LAI : {}".format(laiMonoDateImg))
        #print("ERR : {}".format(laiErrDateImg))
        #print("MSK : {}".format(laiMsksDateImg))
    if os.path.isfile(laiMonoDateImg) and os.path.isfile(laiErrDateImg) and os.path.isfile(laiMsksDateImg) :
        return (laiMonoDateImg, laiErrDateImg, laiMsksDateImg)
    return ("", "", "")

def buildReprocessedTimeSeries(xmlList, siteId, simpleTileId, inLaiMonoDir, laiTimeSeriesFile, errTimeSeriesFile, mskTimeSeriesFile, useIntermediateFiles):
    allLaiParam=[]
    allErrParam=[]
    allMskFlagsParam=[]

    for idx, xml in enumerate(xmlList):
        laiImg, errImg, flgsImg = getMonoDateProductFiles(xml, siteId, simpleTileId, inLaiMonoDir, useIntermediateFiles)
        allLaiParam.append(laiImg)
        allErrParam.append(errImg)
        allMskFlagsParam.append(flgsImg)
   
    deqString = ["-deqval", "1000"]
    if useIntermediateFiles :
        deqString = []
    
    # Create the LAI and Error time series
    runCmd(["otbcli", "TimeSeriesBuilder", appLocation, "-il"] + allLaiParam + ["-out", laiTimeSeriesFile] + deqString)
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    runCmd(["otbcli", "TimeSeriesBuilder", appLocation, "-il"] + allErrParam + ["-out", errTimeSeriesFile] + deqString)
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    runCmd(["otbcli", "TimeSeriesBuilder", appLocation, "-il"] + allMskFlagsParam + ["-out", mskTimeSeriesFile])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

def generateReprocessedLAI(tileID, siteId, allXmlParam, laiTimeSeriesFile, errTimeSeriesFile,\
                            mskTimeSeriesFile, outDir) :
    #ProfileReprocessing parameters
    ALGO_LOCAL_BWR="2"
    ALGO_LOCAL_FWR="0"
    paramsLaiRetrFilenameXML = "{}/lai_retrieval_params.xml".format(outDir)
    reprocessedRastersListFile = "{}/ReprocessedRastersFilesist.txt".format(outDir)
    reprocessedFlagsListFile = "{}/ReprocessedFlagsFilesist.txt".format(outDir)
    outReprocessedTimeSeries = "{}/ReprocessedTimeSeries.tif".format(outDir)
    
    with open(paramsLaiRetrFilenameXML, 'w') as paramsFileXML:
        root = ET.Element('metadata')
        pr= ET.SubElement(root, "ProfileReprocessing_parameters")
        ET.SubElement(pr, "bwr_for_algo_local_online_retrieval").text = ALGO_LOCAL_BWR
        ET.SubElement(pr, "fwr_for_algo_local_online_retrieval").text = ALGO_LOCAL_FWR
        usedXMLs = ET.SubElement(root, "XML_files")
        i = 0
        for xml in allXmlParam:
            ET.SubElement(usedXMLs, "XML_" + str(i)).text = xml
            i += 1
        paramsFileXML.write(prettify(root))

    # Compute the reprocessed time series (On-line Retrieval)
    runCmd(["otbcli", "ProfileReprocessing", appLocation, "-lai", laiTimeSeriesFile, "-err", errTimeSeriesFile, "-msks", mskTimeSeriesFile, "-ilxml"] + allXmlParam + ["-opf", outReprocessedTimeSeries, "-genall", "1", "-algo", "local", "-algo.local.bwr", str(ALGO_LOCAL_BWR), "-algo.local.fwr", str(ALGO_LOCAL_FWR)])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

    #split the Reprocessed time series to a number of images
    runCmd(["otbcli", "ReprocessedProfileSplitter2", appLocation, "-in", outReprocessedTimeSeries, "-outrlist", reprocessedRastersListFile, "-outflist", reprocessedFlagsListFile, "-compress", "1", "-ilxml"] + allXmlParam)
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    # Generate the product for the Reprocessed LAI
    runCmd(["otbcli", "ProductFormatter", appLocation,
            "-destroot", outDir,
            "-fileclass", "SVT1", "-level", "L3C", "-baseline", "01.00", "-siteid", siteId,
            "-processor", "vegetation",
            "-processor.vegetation.filelaireproc", tileID, reprocessedRastersListFile,
            "-processor.vegetation.filelaireprocflgs", tileID, reprocessedFlagsListFile,
            "-il"] + allXmlParam + [
            "-gipp", paramsLaiRetrFilenameXML])
                
def generateFittedLAI(tileID, siteId, allXmlParam, laiTimeSeriesFile, errTimeSeriesFile, mskTimeSeriesFile, outDir) :
    fittedRastersListFile = "{}/FittedRastersFilesList.txt".format(outDir)
    fittedFlagsListFile = "{}/FittedFlagsFilesList.txt".format(outDir)
    outFittedTimeSeries = "{}/FittedTimeSeries.tif".format(outDir)

    # Compute the fitted time series (CSDM Fitting)
    runCmd(["otbcli", "ProfileReprocessing", appLocation, "-lai", laiTimeSeriesFile, "-err", errTimeSeriesFile, "-msks", mskTimeSeriesFile, "-ilxml"] + allXmlParam + ["-opf", outFittedTimeSeries, "-genall", "1", "-algo", "fit"])
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

    #split the Fitted time series to a number of images
    runCmd(["otbcli", "ReprocessedProfileSplitter2", appLocation, "-in", outFittedTimeSeries, "-outrlist", fittedRastersListFile, "-outflist", fittedFlagsListFile, "-compress", "1", "-ilxml"] + allXmlParam)
    print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    # Generate the product for the Fitted LAI
    runCmd(["otbcli", "ProductFormatter", appLocation,
            "-destroot", outDir,
            "-fileclass", "SVT1", "-level", "L3D", "-baseline", "01.00", "-siteid", siteId,
            "-processor", "vegetation",
            "-processor.vegetation.filelaifit", tileID, fittedRastersListFile,
            "-processor.vegetation.filelaifitflgs", tileID, fittedFlagsListFile,
            "-il"] + allXmlParam)
    
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
        print("Generating Input BV Distribution file with parameters :")
        print("\tminlai: {}".format(args.minlai))
        print("\tmaxlai: {}".format(args.maxlai))
        print("\tmodlai: {}".format(args.modlai))
        print("\tstdlai: {}".format(args.stdlai))
        print("\tminala: {}".format(args.minala))
        print("\tmaxala: {}".format(args.maxala))
        print("\tmodala: {}".format(args.modala))
        print("\tstdala: {}".format(args.stdala))
        
        runCmd(["otbcli", "BVInputVariableGeneration", appLocation,
                "-samples", str(GENERATED_SAMPLES_NO),
                "-out",  outGeneratedSampleFile,
                "-minlai", args.minlai,
                "-maxlai", args.maxlai,
                "-modlai", args.modlai,
                "-stdlai", args.stdlai,
                "-minala", args.minala,
                "-maxala", args.maxala,
                "-modala", args.modala,
                "-stdala", args.stdala])

        if args.useSystemBVDistFile :
            outGeneratedSampleFile = "/usr/share/sen2agri/LaiCommonBVDistributionSamples.txt"
            print ("##########################################")
            print("Using the SYSTEM BV Distribution file {}!".format(outGeneratedSampleFile))
            print ("##########################################")
            
        # Generating simulation reflectances
        print("Generating simulation reflectances ...")
        if not rsrCfg:
            if not rsrFile:
                print("Please provide the rsrcfg or rsrfile!")
                exit(1)
            else:
                runCmd(["otbcli", "ProSailSimulatorNew", appLocation,
                        "-xml", curXml,
                        "-bvfile", outGeneratedSampleFile,
                        "-rsrfile", rsrFile,
                        "-out", outSimuReflsFile,
                        "-outangles", outAnglesFile,
                        "-laicfgs", args.laiBandsCfg])
        else:
            runCmd(["otbcli", "ProSailSimulatorNew", appLocation,
                    "-xml", curXml,
                    "-bvfile", outGeneratedSampleFile,
                    "-rsrcfg", rsrCfg,
                    "-out", outSimuReflsFile,
                    "-outangles", outAnglesFile,
                    "-laicfgs", args.laiBandsCfg])

        # Generating training file
        print("Generating training file ...")
        runCmd(["otbcli", "TrainingDataGeneratorNew", appLocation,
                "-xml", curXml,
                "-biovarsfile", outGeneratedSampleFile,
                "-simureflsfile", outSimuReflsFile,
                "-outtrainfile", outTrainingFile,
                "-laicfgs", args.laiBandsCfg])

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

class LaiMonoDate(object):
    def __init__(self):
        """ Constructor """
        self.init = 1
    
    def setUseInraVersion(self, useInraVersion) :
        self.useInraVersion = useInraVersion
        
    def setGenFapar(self, genFapar) :
        self.genFapar = genFapar
        
    def setGenFCover(self, genFCover) :
        self.genFCover = genFapar
    
    def generateLaiMonoDates(self, xml, index, resolution, paramsLaiModelFilenameXML, outDir):  
        counterString = str(idx)
        
        outSingleNdvi = "{}/#_Single_NDVI.tif".format(outDir)        
        outNdviRvi = "{}/#_NDVI_RVI.tif".format(outDir)
        outLaiImg = "{}/#_LAI_img.tif".format(outDir)
        outFaparImg = "{}/#_FAPAR_img.tif".format(outDir)
        outFCoverImg = "{}/#_FCOVER_img.tif".format(outDir)
        
        outLaiErrImg = "{}/#_LAI_err_img.tif".format(outDir)
        outLaiMonoMskFlgsImg = "{}/#_LAI_mono_date_mask_flags_img.tif".format(outDir)
        # LAI images encoded as short. These are used only by ProductFormatter
        outLaiShortImg = "{}/#_LAI_img_16.tif".format(outDir)
        outFaparShortImg = "{}/#_FAPAR_img_16.tif".format(outDir)
        outFCoverShortImg = "{}/#_FCOVER_img_16.tif".format(outDir)
        outLaiErrShortImg = "{}/#_LAI_err_img_16.tif".format(outDir)
        
        lastPoint = xml.rfind('.')
        lastSlash = xml.rfind('/')
        if lastPoint != -1 and lastSlash != -1 and lastSlash + 1 < lastPoint:
            counterString = xml[lastSlash + 1:lastPoint]

        curOutSingleNDVIImg = outSingleNdvi.replace("#", counterString)
        curOutNDVIRVIImg = outNdviRvi.replace("#", counterString)
        curOutLaiImg = outLaiImg.replace("#", counterString)
        
        curOutFaparImg = outFaparImg.replace("#", counterString)
        curOutFCoverImg = outFCoverImg.replace("#", counterString)
        
        curOutLaiErrImg = outLaiErrImg.replace("#", counterString)
        curOutLaiMonoMskFlgsImg = outLaiMonoMskFlgsImg.replace("#", counterString)
        # LAI images encoded as short. These are used only by ProductFormatter
        curOutLaiShortImg = outLaiShortImg.replace("#", counterString)
        curOutFaparShortImg = outFaparShortImg.replace("#", counterString)
        curOutFCoverShortImg = outFCoverShortImg.replace("#", counterString)
        curOutLaiErrShortImg = outLaiErrShortImg.replace("#", counterString)

        runCmd(["otbcli", "GenerateLaiMonoDateMaskFlags", appLocation,
                "-inxml", xml,
                "-out", curOutLaiMonoMskFlgsImg])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
        
        if self.useInraVersion == True :
            runCmd(["otbcli", "NdviRviExtractionNew", appLocation,
            "-xml", xml,
            "-msks", curOutLaiMonoMskFlgsImg,
            "-ndvi", curOutSingleNDVIImg,
            "-laicfgs", args.laiBandsCfg])
        
            # Execute LAI processor
            runCmd(["otbcli", "BVLaiNewProcessor", appLocation,
                    "-xml", xml,
                    "-outlai", curOutLaiImg,
                    "-laicfgs", args.laiBandsCfg])
            print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
            if self.genFapar == True :
                # Execute fAPAR processor
                runCmd(["otbcli", "BVLaiNewProcessor", appLocation,
                        "-xml", xml,
                        "-outfapar", curOutFaparImg,
                        "-laicfgs", args.laiBandsCfg])
                print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
            if self.genFCover == True :
                # Execute fAPAR processor
                runCmd(["otbcli", "BVLaiNewProcessor", appLocation,
                        "-xml", xml,
                        "-outfcover", curOutFCoverImg,
                        "-laicfgs", args.laiBandsCfg])
                print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

            runCmd(["otbcli", "QuantifyImage", appLocation,
                    "-in", curOutLaiImg,
                    "-out", curOutLaiShortImg])
            print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
            runCmd(["otbcli", "QuantifyImage", appLocation,
                    "-in", curOutFaparImg,
                    "-out", curOutFaparShortImg])
            print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
            runCmd(["otbcli", "QuantifyImage", appLocation,
                    "-in", curOutFCoverImg,
                    "-out", curOutFCoverShortImg])
            print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
                
            # Generate the product for the monodate
            runCmd(["otbcli", "ProductFormatter", appLocation,
                    "-destroot", outDir,
                    "-fileclass", "SVT1", "-level", "L3B", "-baseline", "01.00", "-siteid", siteId,
                    "-processor", "vegetation",
                    "-processor.vegetation.laindvi", tileID, curOutSingleNDVIImg,
                    "-processor.vegetation.laimonodate", tileID, curOutLaiShortImg,
                    "-processor.vegetation.faparmonodate", tileID, curOutFaparShortImg,
                    "-processor.vegetation.fcovermonodate", tileID, curOutFCoverShortImg,
                    "-processor.vegetation.laistatusflgs", tileID, curOutLaiMonoMskFlgsImg,
                    "-il", xml, 
                    "-gipp", paramsLaiModelFilenameXML])        
                
        else :
            #Compute NDVI and RVI
            if resolution == 0:
                runCmd(["otbcli", "NdviRviExtractionNew", appLocation,
                "-xml", xml,
                "-msks", curOutLaiMonoMskFlgsImg,
                "-ndvi", curOutSingleNDVIImg,
                "-fts", curOutNDVIRVIImg,
                "-laicfgs", args.laiBandsCfg])
            else:
                runCmd(["otbcli", "NdviRviExtractionNew", appLocation,
                "-xml", xml,
                "-msks", curOutLaiMonoMskFlgsImg,
                "-ndvi", curOutSingleNDVIImg,
                "-fts", curOutNDVIRVIImg,
                "-outres", resolution,
                "-laicfgs", args.laiBandsCfg])
            print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
        
            # Retrieve the LAI model
            runCmd(["otbcli", "BVImageInversion", appLocation,
                    "-in", curOutNDVIRVIImg,
                    "-msks", curOutLaiMonoMskFlgsImg,
                    "-out", curOutLaiImg,
                    "-xml", xml,
                    "-modelsfolder", modelsFolder,
                    "-modelprefix", "Model_"])
            print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
            runCmd(["otbcli", "BVImageInversion", appLocation,
                    "-in", curOutNDVIRVIImg,
                    "-msks", curOutLaiMonoMskFlgsImg,
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

            # Generate the product for the monodate
            runCmd(["otbcli", "ProductFormatter", appLocation,
                    "-destroot", outDir,
                    "-fileclass", "SVT1", "-level", "L3B", "-baseline", "01.00", "-siteid", siteId,
                    "-processor", "vegetation",
                    "-processor.vegetation.laindvi", tileID, curOutSingleNDVIImg,
                    "-processor.vegetation.laimonodate", tileID, curOutLaiShortImg,
                    "-processor.vegetation.laimonodateerr", tileID, curOutLaiErrShortImg,
                    "-processor.vegetation.laistatusflgs", tileID, curOutLaiMonoMskFlgsImg,
                    "-il", xml, 
                    "-gipp", paramsLaiModelFilenameXML])        
        
if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='LAI retrieval processor')

    parser.add_argument('--applocation', help='The path where the sen2agri is built', default="")
    parser.add_argument('--input', help='The list of products xml descriptors', required=True, nargs='+')
    parser.add_argument('--res', help='The requested resolution in meters', required=True)
    parser.add_argument('--outdir', help="Output directory", required=True)
    parser.add_argument('--inlaimonodir', help="Directory where input mono-date LAI products are located (only for reprocessing)", required=False)
    parser.add_argument('--rsrfile', help='The RSR file (/path/filename)', required=False)
    parser.add_argument('--rsrcfg', help='The RSR configuration file each mission (default /usr/share/sen2agri/rsr_cfg.txt)',required=False)
    parser.add_argument('--tileid', help="Tile id", required=False)
    parser.add_argument('--modelsfolder', help='The folder where the models are located. If not specified, is considered the outdir', required=False)
    parser.add_argument('--generatemodel', help='Generate the model (YES/NO)', required=False)
    parser.add_argument('--generatemonodate', help='Generate the mono-date LAI (YES/NO)', required=False)
    parser.add_argument('--genreprocessedlai', help='Generate the reprocessed N-Days LAI (YES/NO)', required=False)
    parser.add_argument('--genfittedlai', help='Generate the Fitted LAI (YES/NO)', required=False)
    parser.add_argument('--siteid', help='The site ID', required=False)
    parser.add_argument('--useintermlaifiles', help='Specify if intermediate files or the final product files should be used', required=False)
    
    parser.add_argument('--minlai', help='Minimum value for LAI', required=False, default="0.0")
    parser.add_argument('--maxlai', help='Maximum value for LAI', required=False, default="5.0")
    parser.add_argument('--modlai', help='Mode value for LAI', required=False, default="0.5")
    parser.add_argument('--stdlai', help='Standard deviation value for LAI', required=False, default="1.0")
    parser.add_argument('--minala', help='Minimum value for ALA', required=False, default="5.0")
    parser.add_argument('--maxala', help='Maximum value for ALA', required=False, default="80.0")
    parser.add_argument('--modala', help='Mode value for ALA', required=False, default="40.0")
    parser.add_argument('--stdala', help='Standard deviation for ALA', required=False, default="20.0")
    
    parser.add_argument('--useinra', help='Generate LAI using INRA version (YES/NO)', required=False, default="NO")
    parser.add_argument('--fcover', help='Generate fCover (YES/NO)', required=False, default="YES")
    parser.add_argument('--fapar', help='Generate fAPAR (YES/NO)', required=False, default="YES")
    
    parser.add_argument('--laiBandsCfg', help='LAI bands to be used configuration file', required=False, default="/usr/share/sen2agri/Lai_Bands_Cfgs.cfg")
    parser.add_argument('--useSystemBVDistFile', help='Specifies if the system BV distribution file should be used or it should be generated at each execution', required=False, default=False)
    
    args = parser.parse_args()
    
    appLocation = args.applocation
    resolution = args.res
    outDir = args.outdir
    inlaimonodir = outDir
    if(args.inlaimonodir) :
        inlaimonodir = args.inlaimonodir
    rsrFile = args.rsrfile
    rsrCfg = args.rsrcfg
    if not rsrFile and not rsrCfg :
        rsrCfg = "/usr/share/sen2agri/rsr_cfg.txt"
        
    generatemonodate = True
    if (args.generatemonodate == "YES"):
        generatemonodate = True
    genreprocessedlai = False
    if (args.genreprocessedlai == "YES"):
        genreprocessedlai = True
    genfittedlai = False
    if (args.genfittedlai == "YES"):
        genfittedlai = True
    useintermlaifiles = False
    if (args.useintermlaifiles == "YES") :
        useintermlaifiles = True
    
    siteId = "nn"
    if args.siteid:
        siteId = args.siteid
        
    GENERATE_MODEL = False
    if (args.generatemodel == "YES"):
        GENERATE_MODEL = True

    useInraVersion = False
    if (args.useinra == "YES"):
        useInraVersion = True
    genFCover = False
    if (args.fcover == "YES"):
        genFCover = True
    genFapar = False
    if (args.fapar == "YES"):
        genFapar = True
        
    simpleTileId = "none"
    tileID="TILE_none"
    if args.tileid:
        simpleTileId = args.tileid
    tileID = "TILE_{}".format(simpleTileId)

    # #######################################
    #for idx, xml in enumerate(args.input):
    #    inlaimonodir="/mnt/output/L3B/SPOT4-T5/Ukraine/test_lai_2016-03-17/"
    #    laiImg, errImg, flgsImg = getMonoDateProductFiles(xml, siteId, simpleTileId, inlaimonodir, True)
    #    print("Selected IMG: {}".format(laiImg))
    #    print("Selected ERR: {}".format(errImg))
    #    print("Selected MSK: {}".format(flgsImg))
    #    
    #exit(1)
    ## #######################################
    
    # By default, if not specified, models folder is the given out dir.
    modelsFolder = outDir
    if useInraVersion == False :
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

    if resolution != 10 and resolution != 20:
        print("The resolution is : {}".format(resolution))
        print("The resolution should be either 10 or 20.")
        print("The product will be created with the original resolution without resampling.")
        resolution=0

    print("Processing started: " + str(datetime.datetime.now()))
    start = time.time()
        
    for idx, xml in enumerate(args.input):
        if GENERATE_MODEL:
            laiModel = LaiModel()
            laiModel.generateModel(xml,outDir,paramsLaiModelFilenameXML)
        
        if  generatemonodate:
            laiMonoDate = LaiMonoDate()
            laiMonoDate.setUseInraVersion(useInraVersion)
            laiMonoDate.setGenFapar(genFapar)
            laiMonoDate.setGenFCover(genFCover)
            laiMonoDate.generateLaiMonoDates(xml, idx, resolution, paramsLaiModelFilenameXML, outDir)
            
    laiTimeSeriesFile = "{}/LAI_time_series.tif".format(outDir)
    errTimeSeriesFile = "{}/Err_time_series.tif".format(outDir)
    mskTimeSeriesFile = "{}/Mask_Flags_time_series.tif".format(outDir)

    if genreprocessedlai or genfittedlai:
        allXmlParam = args.input
        buildReprocessedTimeSeries(args.input, siteId, simpleTileId, outDir, \
                        laiTimeSeriesFile, errTimeSeriesFile, mskTimeSeriesFile, useintermlaifiles)

    if genreprocessedlai:
        generateReprocessedLAI(tileID, siteId, allXmlParam, laiTimeSeriesFile, errTimeSeriesFile, \
                                mskTimeSeriesFile, outDir)

    if genfittedlai:
        generateFittedLAI(tileID, siteId, allXmlParam, laiTimeSeriesFile, errTimeSeriesFile, mskTimeSeriesFile, outDir)
        
    print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

    '''
    ''' and None
