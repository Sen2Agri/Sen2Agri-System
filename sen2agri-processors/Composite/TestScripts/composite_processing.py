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
import os.path
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

def getFileName(fileName, cntStr, res=""):
    fileName = fileName.replace("#", cntStr)
    if res != "":
        fileName = fileName.replace("%", res)
    return fileName
    
def checkInputProducts(inputFiles) :
    i = 0
    for filePath in inputFiles:
        i += 1
    if i == 0:
        print("No L2A products found !")
        exit(1)

def removeFiles(filesToRemove):
    print("The following files will be deleted:")
    for fileToRemove in filesToRemove:
        print(fileToRemove)
        try:
            #runCmd(["rm", "-fr", fileToRemove])
            os.remove(fileToRemove)
        except:
            pass
        
def areMultiMissionFiles(inputFiles) :
    missions = []
    for filePath in inputFiles:
        fileName = os.path.basename(filePath)
        words = fileName.split('_')
        if len(words) >= 1:
            if (words[0] not in missions):
                missions.append(words[0])
    
    print("Missions found: {}".format(missions))
    
    return (len(missions) > 1)
    
def getMasterMissionFilePrefix(bandsMappingFile) :
    masterMissionPrefix = ""
    with open(bandsMappingFile) as f:
        content = f.readlines()
        if len(content) > 0 :
            masterMission = (content[0].split(','))[0]
            if masterMission.find("SENTINEL") != -1 :
                masterMissionPrefix = "S2"
            elif masterMission.find("LANDSAT") != -1 :
                masterMissionPrefix = "L8"
            else :
                masterMissionPrefix = masterMission
        else :
            print("Cannot load master mission from bands mapping {}".format(bandsMappingFile))
            return ""
    
    print("Master mission prefix found: {}".format(masterMissionPrefix))

    return masterMissionPrefix

def getFirstMasterFile(inputFiles, bandsMappingFile) :
    masterMissionPrefix = getMasterMissionFilePrefix(bandsMappingFile)
    for filePath in inputFiles:
        fileName = os.path.basename(filePath)
        words = fileName.split('_')
        if len(words) >= 1:
            if (words[0].find(masterMissionPrefix) != -1):  
                print("First master file found: {}".format(filePath))
                return filePath
    
    print("No master file was found for bands mapping {}".format(bandsMappingFile))
    return ""

def isMasterMissionFile(inputFile, bandsMappingFile) :
    masterMissionPrefix = getMasterMissionFilePrefix(bandsMappingFile)    
    fileName = os.path.basename(inputFile)
    words = fileName.split('_')
    if len(words) >= 1:
        if (words[0].find(masterMissionPrefix) != -1):  
            print("File {} is master mission file".format(inputFile))
            return True
    
    print("File {} is secondary mission file".format(inputFile))
    return False
    
def writeParamsFileXml(paramsFilenameXML) :
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
        dates = ET.SubElement(root, "Dates_information")
        ET.SubElement(dates, "synthesis_date").text = syntDate
        ET.SubElement(dates, "synthesis_half").text = syntHalf
        usedXMLs = ET.SubElement(root, "XML_files")
        i = 0
        for xml in args.input:
            ET.SubElement(usedXMLs, "XML_" + str(i)).text = xml
            i += 1
        paramsFileXML.write(prettify(root))        

parser = argparse.ArgumentParser(description='Composite Python processor')

parser.add_argument('--applocation', help='The path where the sen2agri is built', default="")
parser.add_argument('--syntdate', help='L3A synthesis date', required=True)
parser.add_argument('--synthalf', help='Half synthesis', required=True)
parser.add_argument('--input', help='The list of products xml descriptors', required=True, nargs='+')
parser.add_argument('--res', help='The requested resolution in meters', required=False, default=0)
parser.add_argument('--outdir', help="Output directory", required=True)
parser.add_argument('--bandsmap', help="Bands mapping file location", required=True)
parser.add_argument('--scatteringcoef', help="Scattering coefficient file. This file is requested in S2 case ONLY", required=False)
parser.add_argument('--tileid', help="Tile id", required=False)
parser.add_argument('--siteid', help='The site ID', required=False)
parser.add_argument('--lut', help="Lookup table", required=False)

USE_COMPRESSION=True
REMOVE_TEMP=False

args = parser.parse_args()

syntDate = args.syntdate
syntHalf = args.synthalf
resolutions = [str(args.res)]
if (str(args.res) != "10") and (str(args.res) != "20"):
    resolutions = ["10", "20"]

print("The product will be created at resolutions: ")
print(*resolutions, sep='\n')
    
bandsMap = args.bandsmap
appLocation = args.applocation

outDir = args.outdir
if os.path.exists(outDir):
    if not os.path.isdir(outDir):
        print("Can't create the output directory because there is a file with the same name")
        print("Remove: " + outDir)
        exit(1)
else:
    os.makedirs(outDir)

siteId = "nn"
checkInputProducts(args.input)
multisatelites = areMultiMissionFiles(args.input)
firstMasterFile = ""
if multisatelites :
    firstMasterFile = getFirstMasterFile(args.input, bandsMap)
    print("MULTI mission products found!")
else :
    print("SINGLE mission products found!")

if args.siteid:
    siteId = args.siteid


masterInfoFile = outDir + 'MasterInfo_#.txt'

outWeightAotFile = outDir + '/WeightAot#_%.tif'
outWeightCloudFile = outDir + '/WeightCloud#_%.tif'
outTotalWeightFile = outDir + '/WeightTotal#_%.tif'
outL3AFile = outDir + '/L3AResult#_%M.tif'

WEIGHT_AOT_MIN="0.33"
WEIGHT_AOT_MAX="1"
AOT_MAX="0.8"

COARSE_RES="240"
SIGMA_SMALL_CLD="2"
SIGMA_LARGE_CLD="10"

WEIGHT_DATE_MIN="0.5"

outWeights = outDir + '/L3AResult#_%_weights.tif'
outDates = outDir + '/L3AResult#_%_dates.tif'
outRefls = outDir + '/L3AResult#_%_refls.tif'
outFlags = outDir + '/L3AResult#_%_flags.tif'
outRGB = outDir + '/L3AResult#_%_rgb.tif'

fullScatCoeffs=[]
fullLut=[]
tileID="TILE_none"

shutil.copyfile(bandsMap, os.path.join(outDir, os.path.basename(bandsMap)))

if args.scatteringcoef:
    shutil.copyfile(args.scatteringcoef, os.path.join(outDir, os.path.basename(args.scatteringcoef)))
    scatteringCoefFilename = args.scatteringcoef[args.scatteringcoef.rfind('/'):]
    fullScatCoeffs = ["-scatcoef", outDir + scatteringCoefFilename]
    print(fullScatCoeffs)

if args.lut:
    fullLut = ["-lut", args.lut]
    print(fullLut)
    
if args.tileid:
    tileID = "TILE_{}".format(args.tileid)

paramsFilenameXML= outDir + '/params.xml'
writeParamsFileXml(paramsFilenameXML)

l3aOutRefls=[]
l3aOutWeights=[]
l3aOutFlags=[]
l3aOutDates=[]
l3aOutRgbs=[]

print("Processing started: " + str(datetime.datetime.now()))
start = time.time()

for resolution in resolutions :
    i = 0
    prevL3A=[]
    
    #defintion for filenames
    outExtractedMasks = outDir + '/masks' + resolution + '.tif'
    outImgBands = outDir + '/res' + resolution + '.tif'
    outCld = outDir + '/cld' + resolution + '.tif'
    outWat = outDir + '/wat' + resolution + '.tif'
    outSnow = outDir + '/snow' + resolution + '.tif'
    outAot = outDir + '/aot' + resolution + '.tif'
    tmpFilesToRemove = [outExtractedMasks, outImgBands, outCld, outWat, outSnow, outAot]

    fullScatCoeffs = ["-scatcoef", getFileName("/usr/share/sen2agri/scattering_coeffs_%m.txt", "", resolution)]
    for xml in args.input:
    
        runCmd(["otbcli", "MaskHandler", appLocation, "-xml", xml, "-out", outExtractedMasks, "-sentinelres", resolution])

        counterString = str(i)
        mod=getFileName(outL3AFile, counterString, resolution)
        out_w=getFileName(outWeights, counterString, resolution)
        out_d=getFileName(outDates, counterString, resolution)
        out_r=getFileName(outRefls, counterString, resolution)
        out_f=getFileName(outFlags, counterString, resolution)
        out_rgb=getFileName(outRGB, counterString, resolution)

        out_w_Aot=getFileName(outWeightAotFile, counterString, resolution)
        out_w_Cloud=getFileName(outWeightCloudFile, counterString, resolution)
        out_w_Total=getFileName(outTotalWeightFile, counterString, resolution)
        curMasterInfoFile=getFileName(masterInfoFile, counterString, resolution)

        if (not multisatelites) or isMasterMissionFile(xml, bandsMap):
            runCmd(["otbcli", "CompositePreprocessing", appLocation, "-xml", xml, "-bmap", bandsMap, "-res", resolution] + fullScatCoeffs + ["-msk", outExtractedMasks, "-outres", outImgBands, "-outcmres", outCld, "-outwmres", outWat, "-outsmres", outSnow, "-outaotres", outAot, "-masterinfo", curMasterInfoFile])
        else:
            runCmd(["otbcli", "CompositePreprocessing", appLocation, "-xml", xml, "-bmap", bandsMap, "-res", resolution] + fullScatCoeffs + ["-msk", outExtractedMasks, "-outres", outImgBands, "-outcmres", outCld, "-outwmres", outWat, "-outsmres", outSnow, "-outaotres", outAot, "-masterinfo", curMasterInfoFile, "-pmxml", firstMasterFile])
        
        runCmd(["otbcli", "WeightAOT", appLocation, "-xml", xml, "-in", outAot, "-waotmin", WEIGHT_AOT_MIN, "-waotmax", WEIGHT_AOT_MAX, "-aotmax", AOT_MAX, "-out", out_w_Aot])

        runCmd(["otbcli", "WeightOnClouds", appLocation, "-inxml", xml, "-incldmsk", outCld, "-coarseres", COARSE_RES, "-sigmasmallcld", SIGMA_SMALL_CLD, "-sigmalargecld", SIGMA_LARGE_CLD, "-out", out_w_Cloud])

        runCmd(["otbcli", "TotalWeight", appLocation, "-xml", xml, "-waotfile", out_w_Aot, "-wcldfile", out_w_Cloud, "-l3adate", syntDate, "-halfsynthesis", syntHalf, "-wdatemin", WEIGHT_DATE_MIN, "-out", out_w_Total])

        runCmd(["otbcli", "UpdateSynthesis", appLocation, "-in", outImgBands, "-bmap", bandsMap, "-xml", xml, "-csm", outCld, "-wm", outWat, "-sm", outSnow, "-wl2a", out_w_Total, "-out", mod] + prevL3A)

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

        runCmd(["otbcli", "CompositeSplitter2", appLocation, "-in", mod, "-xml", xml, "-bmap", bandsMap, "-outweights", tmpOut_w, "-outdates", tmpOut_d, "-outrefls", tmpOut_r, "-outflags", tmpOut_f, "-outrgb", tmpOut_rgb])

        prevL3A = ["-prevl3aw", out_w, "-prevl3ad", out_d, "-prevl3ar", out_r, "-prevl3af", out_f]

        if REMOVE_TEMP:
            removeFiles([out_w_Aot, out_w_Cloud, out_w_Total])

        tmpFilesToRemove.append(mod)
        removeFiles(tmpFilesToRemove)
        if REMOVE_TEMP and i > 0:
            counterString = str(i - 1)
            removeFiles([getFileName(outWeights, counterString, resolution), getFileName(outDates, counterString, resolution), getFileName(outRefls, counterString, resolution), getFileName(outFlags, counterString, resolution), getFileName(outRGB, counterString, resolution)])
        i += 1
    l3aOutRefls.append(out_r)
    l3aOutWeights.append(out_w)
    l3aOutFlags.append(out_f)
    l3aOutDates.append(out_d)
    l3aOutRgbs.append(out_rgb)


runCmd(["otbcli", "ProductFormatter", appLocation, 
    "-destroot", outDir, 
    "-fileclass", "SVT1", 
    "-level", "L3A", 
    "-timeperiod", syntDate, 
    "-baseline", "01.00", 
    "-siteid",siteId, 
    "-processor", "composite", 
    "-processor.composite.refls", tileID] + l3aOutRefls +
    ["-processor.composite.weights", tileID] + l3aOutWeights + 
    ["-processor.composite.flags", tileID] + l3aOutFlags + 
    ["-processor.composite.dates", tileID] + l3aOutDates +
    ["-processor.composite.rgb", tileID] + l3aOutRgbs +
    ["-il"] + args.input + [
    "-gipp", paramsFilenameXML] + 
    fullLut)

if REMOVE_TEMP:
    i -= 1
    counterString = str(i)
    for resolution in [10, 20] :
        removeFiles([getFileName(outWeights, counterString, resolution), getFileName(outDates, counterString, resolution), getFileName(outRefls, counterString, resolution), getFileName(outFlags, counterString, resolution)])    


print("Processing finished: " + str(datetime.datetime.now()))
print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

'''
''' and None
