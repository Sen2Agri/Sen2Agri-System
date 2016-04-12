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
import re
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

    
def generateFileList(inputDir, sat, instrument, orbitDay):

    if sat=="SPOT4":
        patternSPOT4="SPOT4_INST_XS_" 
        patternSPOT4withINSTR=patternSPOT4.replace("INST", instrument)
        patternOrbitDaySPOT4="-DAYD"
        #SPOT4_HRVIR_XS_20130131_N2A_NMaricopa-J1D0000B0000
    elif sat=="SPOT5":
        print("not defined")

    listFile = os.listdir(inputDir);
    listFile.sort()

    listFileOut=[]
    for file in listFile:
        #print file
        if re.search(patternSPOT4withINSTR,file):
            #print '->', file
            if (orbitDay != ""):
                patternOrbitDaySPOT4withDAY=patternOrbitDaySPOT4.replace("DAY", orbitDay)
                if re.search(patternOrbitDaySPOT4withDAY,file):
                    listFileOut.append(inputDir + file + '/' + file + '.xml ')
            else:
                dt = datetime.datetime.strptime((re.search('2013[0-1][1-9][0-3][0-9]',file).group(0)),"%Y%m%d")
                #print dt
                listFileOut.append(inputDir + file + '/' + file + '.xml ')
                
    print(listFileOut)
    return listFileOut




if __name__ == '__main__': 
             
    parser = argparse.ArgumentParser(description='LAI retrieval processor')

    parser.add_argument('--applocation', help='The path where the sen2agri is built', required=True)
    parser.add_argument('--inputdir', help='The input dir', required=True)
    parser.add_argument('--outdir', help="Output directory", required=True)
    parser.add_argument('--backwardwindow', help='Size of the backward window for profile reprocessing', required=True)
    parser.add_argument('--forwardwindow', help='Size of the forward window for profile reprocessing', required=True)
    parser.add_argument('--sat', help='Satellite', required=True)
    parser.add_argument('--inst', help='Instrument', required=True)
    parser.add_argument('--siteid', help='The site ID', required=False)

    args = parser.parse_args()

    appLocation = args.applocation
    outDir = args.outdir
    inDir = args.inputdir
    sat= args.sat
    inst = args.inst
    siteId = "nn"
    if args.siteid:
        siteId = args.siteid
        
    #ProfileReprocessing parameters
    #ALGO_LOCAL_BWR="2"
    #ALGO_LOCAL_FWR="0"
    bwr=args.backwardwindow
    fwr=args.forwardwindow

    listFile = generateFileList(inDir, sat, inst, "")
    tileID="TILE_none"

    if os.path.exists(outDir):
        if not os.path.isdir(outDir):
            print("Can't create the output directory because there is a file with the same name")
            print("Remove: " + outDir)
            exit(1)
    else:
        os.makedirs(outDir)

   
    print("Processing started: " + str(datetime.datetime.now()))
    start = time.time()

    nbFile=len(listFile)
    
    for i in range(0,nbFile):
        # 0 1 2 3 4 5
        print(i, i-int(bwr),i-int(bwr),i)
        xmlList=[]
        if i-int(bwr) < 0 :
            continue
        else:
            xmlList=listFile[i-int(bwr):i+1]
        
        print("current list: ", xmlList)
        
        
        dt = datetime.datetime.strptime((re.search('201[3-5][0-1][1-9][0-3][0-9]',xmlList[-1]).group(0)),"%Y%m%d")
        t0=dt.strftime('%Y%m%d')
        tend=t0
        print(t0)  
        #continue
                
        allXmlParam=[]
        allNdviFilesList=[]
        allLaiParam=[]
        allErrParam=[]
        allMskFlagsParam=[]

        for xml in xmlList:

            # inputpath/SPOT4_HRVIR1_XS_20130203_N2A_EArgentinaD0000B0000.xml
            allXmlParam.append(xml)
            # outputpath/SPOT4_HRVIR1_XS_20130203_N2A_EArgentinaD0000B0000_NDVI_RVI.tif
            #print(xml)
            filename_tmp = os.path.splitext(os.path.basename(xml))[0]
            #print(filename_tmp)
            curOutNDVIImg = outDir + '/' + filename_tmp + '_NDVI_RVI.tif'
            #print("curOutNDVIImg= " + curOutNDVIImg)
            allNdviFilesList.append(curOutNDVIImg)
            # outputpath/SPOT4_HRVIR1_XS_20130203_N2A_EArgentinaD0000B0000_LAI_img.tif
            curOutLaiImg = outDir + '/' + filename_tmp + '_LAI_img.tif'
            allLaiParam.append(curOutLaiImg)
            # outputpath/SPOT4_HRVIR1_XS_20130203_N2A_EArgentinaD0000B0000_LAI_err_img.tif
            curOutLaiErrImg = outDir + '/' + filename_tmp + '_LAI_err_img.tif'
            allErrParam.append(curOutLaiErrImg)
            # outputpath/SPOT4_HRVIR1_XS_20130203_N2A_EArgentinaD0000B0000_LAI_mono_date_mask_flags_img.tif
            curOutLaiMonoMskFlgsImg = outDir + '/' + filename_tmp + '_LAI_mono_date_mask_flags_img.tif'
            allMskFlagsParam.append(curOutLaiMonoMskFlgsImg)
                
        # Write parameters n xml file
        paramsLaiRetrFilenameXML = "{}/lai_retrieval_params.xml".format(outDir)    
        with open(paramsLaiRetrFilenameXML, 'w') as paramsFileXML:
            root = ET.Element('metadata')
            pr= ET.SubElement(root, "ProfileReprocessing_parameters")
            ET.SubElement(pr, "bwr_for_algo_local_online_retrieval").text = bwr
            ET.SubElement(pr, "fwr_for_algo_local_online_retrieval").text = fwr
            usedXMLs = ET.SubElement(root, "XML_files")
            i = 0
            for xml in xmlList:
                ET.SubElement(usedXMLs, "XML_" + str(i)).text = xml
                i += 1
            paramsFileXML.write(prettify(root))

        # Write parameters in txt file
        paramsFilename = "{}/lai_retrieval_params.txt".format(outDir)
        with open(paramsFilename, 'w') as paramsFile:
            paramsFile.write("ProfileReprocessing parameters\n")
            paramsFile.write("    bwr for algo local (online retrieval) = {}\n".format(bwr))
            paramsFile.write("    fwr for algo local (online retrieval) = {}\n".format(fwr))
            paramsFile.write("Used XML files\n")
            for xml in xmlList:
                paramsFile.write("  " + xml + "\n")        

        # Create the output file
        outLaiTimeSeries = "{}/LAI_time_series.tif".format(outDir)
        outErrTimeSeries = "{}/Err_time_series.tif".format(outDir)
        outMaksFlagsTimeSeries = "{}/Mask_Flags_time_series.tif".format(outDir)

        outReprocessedTimeSeries = "{}/ReprocessedTimeSeries.tif".format(outDir)
        outFittedTimeSeries = "{}/FittedTimeSeries.tif".format(outDir)

        fittedRastersListFile = "{}/FittedRastersFilesList.txt".format(outDir)
        fittedFlagsListFile = "{}/FittedFlagsFilesList.txt".format(outDir)
        reprocessedRastersListFile = "{}/ReprocessedRastersFilesist.txt".format(outDir)
        reprocessedFlagsListFile = "{}/ReprocessedFlagsFilesist.txt".format(outDir)

        
        # Create the LAI and Error time series
        #runCmd(["otbcli", "TimeSeriesBuilder", appLocation, "-il"] + allLaiParam + ["-out", outLaiTimeSeries])
        runCmd(["otbcli", "ConcatenateImages", "-il"] + allLaiParam + ["-out", outLaiTimeSeries])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
        runCmd(["otbcli", "ConcatenateImages", "-il"] + allErrParam + ["-out", outErrTimeSeries])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))
        runCmd(["otbcli", "ConcatenateImages", "-il"] + allMskFlagsParam + ["-out", outMaksFlagsTimeSeries])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))


        # Compute the reprocessed time series (On-line Retrieval)
        runCmd(["otbcli", "ProfileReprocessing", appLocation, "-lai", outLaiTimeSeries, "-err", outErrTimeSeries, "-msks", outMaksFlagsTimeSeries, "-ilxml"] + allXmlParam + ["-opf", outReprocessedTimeSeries, "-genall", "0", "-algo", "local", "-algo.local.bwr", str(bwr), "-algo.local.fwr", str(fwr)])
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

        #split the Reprocessed time series to a number of images
        runCmd(["otbcli", "ReprocessedProfileSplitter2", appLocation, "-in", outReprocessedTimeSeries, "-outrlist", reprocessedRastersListFile, "-outflist", reprocessedFlagsListFile, "-compress", "1", "-ilxml"] + allXmlParam)
        print("Exec time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

        runCmd(["otbcli", "ProductFormatter", appLocation,
                "-destroot", outDir,
                "-fileclass", "OPER", "-level", "L3C", "-timeperiod", t0 + '_' + tend, "-baseline", "01.00", "-siteid", siteId,
                "-processor", "vegetation",
                "-processor.vegetation.filelaireproc", tileID, reprocessedRastersListFile,
                "-processor.vegetation.filelaireprocflgs", tileID, reprocessedFlagsListFile,
                "-il"] + allXmlParam[-1], 
                "-gipp", paramsLaiRetrFilenameXML])
        
        runCmd(cmd)

    print("Total execution time: {}".format(datetime.timedelta(seconds=(time.time() - start))))

    '''
    ''' and None
