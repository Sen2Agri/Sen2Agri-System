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
import ntpath
from shutil import copyfile
import tempfile

def GetAcquisitionDate(fileName) :
    words = fileName.split('_')
    if len(words) >= 2:
        return words[1]
    return ""

# find a file in a folder having the given prefix, the given identifier and the given extension 
def findFile(inputFolder, strPrefix, strIdent, ext) :
    filesPattern = inputFolder + "/*"
    allFiles = glob.glob(filesPattern)
    for filePath in allFiles:
        filename = os.path.basename(filePath)
        hasIdent = ((not strIdent) or (strIdent in filename))        
        fileName, fileExt = os.path.splitext(filename)
        
        #print("FileName : {}, fileExt: {}, prefix: {}, ext: {}, ident: {}, hasIdent: {}".format(fileName, fileExt, strPrefix, ext, strIdent, hasIdent))
        #print("X: {}, Y: {}".format((fileExt == ext), fileName.startswith(strPrefix)))
        
        if ((fileExt == ext) and fileName.startswith(strPrefix) and hasIdent) : 
            return filePath

    return ""
    
def GetAngleValue(line, prefix) :
    strVal = ""
    idx = line.find(prefix)
    if (idx != -1) :
        strVal = line[len(prefix):]
        
    if (strVal.find("0 ") != -1):
        strVal = "0.000000000"
    
    strVal = strVal.rstrip('\n')
    #if strVal :
    #    print("Angle: {}, len: {}".format(strVal, len(strVal)))
    
    return strVal
    
def GetAnglesFromSourceFile(inputFolder, fileName) :
    returnValues = ("", "", "", "")
    fullFileName = inputFolder + "/" + fileName + "_USGS_surf_pente_30m.hdr"

    theta_s = ""
    phi_s = ""
    theta_v = ""
    phi_v = ""
    
    with open(fullFileName) as f:
        content = f.readlines()
        for fileLine in content:
            angleVal = GetAngleValue(fileLine, "theta_s       = ")
            if angleVal:
                theta_s = angleVal
                
            angleVal = GetAngleValue(fileLine, "phi_s         = ")
            if angleVal :
                phi_s = angleVal
                
            angleVal = GetAngleValue(fileLine, "theta_v       = ")
            if angleVal :
                theta_v = angleVal
                
            angleVal = GetAngleValue(fileLine, "phi_v         = ")
            if angleVal :
                phi_v = angleVal

    #print("Tupple: {}, {}, {}, {}".format(theta_s, phi_s, theta_v, phi_v))
    return (theta_s, phi_s, theta_v, phi_v)
    
def CopySpecificFile(strInputFolder, strPrefix, strIdent, inExt, strOutFolder, strOutSuffix, outExt, newPrefix):

    fileName = findFile(strInputFolder, strPrefix, strIdent, inExt)
    if (len(newPrefix) > 0) :
        resFileName = strOutFolder + "/" + newPrefix + strOutSuffix + outExt
    else :
        resFileName = strOutFolder + "/" + strPrefix + strOutSuffix + outExt
    try:
        #print("Copying file : {} to {}".format(fileName, resFileName))
        copyfile(fileName, resFileName) 
    except OSError as e:
        print("Error copying file name : {}".format(resFileName))
    
    return resFileName
    
def createMainHdrFile(inputDir, refDir, refName, outProductDirPath, textToReplace, filePrefix) :
    fileCopied = CopySpecificFile(refDir, refName, "", ".HDR", outProductDirPath, "", ".HDR", "")
    strSunZenith, strSunAzimuth, strViewZenith, strViewAzimuth = GetAnglesFromSourceFile(inputDir, filePrefix)
    
    #print("strSunZenith {} : strSunAzimuth: {}, strViewZenith: {}, strViewAzimuth: {}".format(\
    #    strSunZenith, strSunAzimuth, strViewZenith, strViewAzimuth))
    #    
    #print("createMainHdrFile params : inputDir: {}, refDir: {}, refName: {}, outProductDirPath: {}, textToReplace: {}, filePrefix: {}".format(inputDir, \
    #    inputDir, refName, outProductDirPath, textToReplace, filePrefix))
 
    acquisitionDate = GetAcquisitionDate(filePrefix)
    
    fileContent = []
    bMeanSolarAnglesSection = False
    bMeanViewingAnglesSection = False
    
    with open(fileCopied) as f:
        content = f.readlines()
        for fileLine in content :
            if (fileLine.find("<Acquisition_Date>") != -1) and acquisitionDate :
                fileLine = "                <Acquisition_Date>" + acquisitionDate + "</Acquisition_Date>\n"
                
            if (fileLine.find("<Mean_Solar_Angles>") != -1) :
                bMeanSolarAnglesSection = True
            
            if (fileLine.find("</Mean_Solar_Angles>") != -1) :
                bMeanSolarAnglesSection = False
            
            if (fileLine.find("<Mean_Viewing_Angles>") != -1) :
                bMeanViewingAnglesSection = True
            
            if (fileLine.find("</Mean_Viewing_Angles>") != -1) :
                bMeanViewingAnglesSection = False
            
            if (bMeanSolarAnglesSection) :
                if (fileLine.find("<Azimuth unit=\"deg\"") != -1) :
                    fileLine = "                    <Azimuth unit=\"deg\">" + strSunAzimuth + "</Azimuth>\n"
                else : 
                    if (fileLine.find("<Zenith unit=\"deg\"") != -1) :
                        fileLine = "                    <Zenith unit=\"deg\">" + strSunZenith + "</Zenith>\n"

            if (bMeanViewingAnglesSection) :
                if (fileLine.find("<Azimuth unit=\"deg\"") != -1) :
                    fileLine = "                    <Azimuth unit=\"deg\">" + strViewAzimuth + "</Azimuth>\n"
                else :
                    if (fileLine.find("<Zenith unit=\"deg\"") != -1) :
                        fileLine = "                    <Zenith unit=\"deg\">" + strViewZenith + "</Zenith>\n"
            
            if (fileLine.find(textToReplace) != -1): 
                #print("Replacing in: {} the text: {} with: {}".format(fileLine, textToReplace, filePrefix))
                fileLine = fileLine.replace(textToReplace, filePrefix)
            fileContent.append(fileLine)

    targetFile = open(outProductDirPath + "/" + filePrefix+".HDR", 'w')
    for fileLine in fileContent:
        targetFile.write(fileLine)
        #targetFile.write("\n")
    targetFile.close()

    try:
        #print("Removing file: {}".format(fileCopied))
        os.remove(fileCopied)
    except e:
        print("File {} not found, cause : {}".format(fileCopied, e))

def createSpecificHdrFiles(refDir, refName, targetDir, filePrefix) :
    refDblFolder = refDir + "/" + refName + ".DBL.DIR"
    CopySpecificFile(refDblFolder, refName, "_ATB", ".HDR", targetDir, "_ATB", ".HDR", filePrefix)
    CopySpecificFile(refDblFolder, refName, "_MSK", ".HDR", targetDir, "_MSK", ".HDR", filePrefix)
    CopySpecificFile(refDblFolder, refName, "_CLD", ".HDR", targetDir, "_CLD", ".HDR", filePrefix)
    CopySpecificFile(refDblFolder, refName, "_FRE", ".HDR", targetDir, "_FRE", ".HDR", filePrefix)
    
def createImageFiles(inputDir, targetDir, filePrefix) :
    CopySpecificFile(inputDir, filePrefix, ".aot", ".tif",           targetDir, "_ATB", ".DBL.TIF", "")
    CopySpecificFile(inputDir, filePrefix, "bord_eau_neige", ".tif", targetDir, "_MSK", ".DBL.TIF", "")
    CopySpecificFile(inputDir, filePrefix, ".nuages", ".tif",        targetDir, "_CLD", ".DBL.TIF", "")
    CopySpecificFile(inputDir, filePrefix, "_pente_30m.tif", ".tif", targetDir, "_FRE", ".DBL.TIF", "")
    createQltFiles(inputDir, filePrefix, targetDir)

def createFoldersForProduct(inputDir, refDir, refName, filePrefix, filesList, outDir):
    outProductDirPath = outDir + "/" + filePrefix
    if not os.path.exists(outProductDirPath):
        os.makedirs(outProductDirPath)
    dblDirPath = outProductDirPath + "/" + filePrefix + ".DBL.DIR"
    if not os.path.exists(dblDirPath):
        os.makedirs(dblDirPath)

    createMainHdrFile(inputDir, refDir, refName, outProductDirPath, refName, filePrefix)
    createSpecificHdrFiles(refDir, refName, dblDirPath, filePrefix)
    createImageFiles(inputDir, dblDirPath, filePrefix)
        
def handleFolderFiles(inputDir, refDir, refName, outDir) :
    filesPattern = inputDir + "/" + "*_USGS*.tif"
    imgsFiles = glob.glob(filesPattern)
    mapFiles = []
    for imgFile in imgsFiles :
        imgFile = os.path.basename(imgFile)
        usgsIdx = imgFile.find("_USGS")
        filePrefix = imgFile[:usgsIdx]
        bFound = False
        for item in mapFiles:
            if (item[0] == filePrefix) and (not bFound):
                item[1].append(imgFile)
                bFound = True
        if not bFound :
            mapFiles.append((filePrefix, [imgFile]))
    
    for item in mapFiles:
        # print("Prefix: {} ImgFile : {}".format(item[0], item[1]))
        createFoldersForProduct(inputDir, refDir, refName, item[0], item[1], outDir)

def createQltFiles(inputDir, filePrefix, targetDir) :
    tempfolder = tempfile.mkdtemp()
    satfile = findFile(inputDir, filePrefix, "_USGS_surf_pente_30m.saturation", ".tif")
    rastfile = findFile(targetDir, filePrefix, "_FRE", ".TIF")
    # print("satfile: {} ".format(satfile))
    # print("rastfile: {} ".format(rastfile))
    
    #use bandmath to generate the edge mask
    edgemap = os.path.join(tempfolder, "edgemap.tif")
    bmCmdLine = "otbcli_BandMath -il \""+rastfile+"\" -out "+edgemap+" -exp \"im1b1 == -10000 ? 1 : 0\""
    print ("BandMath command {}".format(bmCmdLine))
    result = os.system(bmCmdLine)

    if result != 0 :
        print ("Error running BandMath for product {}".format(filePrefix))
        shutil.rmtree(tempfolder)                
        return
    
    # concatenate the saturation file (twice) and the edge map
    qualitymap = os.path.join(tempfolder, filePrefix+"_QLT.DBL.TIF")
    concatCmdLine = "otbcli_ConcatenateImages -il \""+satfile+"\" \""+satfile+"\" "+edgemap+" -out "+qualitymap    
    print ("Concat command {}".format(concatCmdLine))
    result = os.system(concatCmdLine)

    if result != 0 :
        print ("Error running ConcatenateImage for product {}".format(filePrefix))
        shutil.rmtree(tempfolder)                
        return
    
    #copy to destination
    destQuality=os.path.join(targetDir, filePrefix+"_QLT.DBL.TIF")
    try:
        #print("Copying file : {} to {}".format(fileName, resFileName))
        copyfile(qualitymap, destQuality) 
    except OSError as e:
        print("Error copying file name : {} to {}".format(qualitymap, destQuality))   
        shutil.rmtree(tempfolder)        
        return
    # ... do stuff with dirpath
    shutil.rmtree(tempfolder)

   
if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Convert to MACCS format')

    parser.add_argument('--indir', help='The input folder containing original files', required=True)
    parser.add_argument('--refdir', help='The folder containing reference files', required=True)
    parser.add_argument('--outdir', help='The ouput folder where the new products will be created', required=True)    

    args = parser.parse_args()
    inDir = args.indir
    refDir = args.refdir
    outDir = args.outdir
    
    refName = os.path.basename(refDir)
    print("RefName: {}".format(refName))
    
    handleFolderFiles(inDir, refDir, refName, outDir)

    

