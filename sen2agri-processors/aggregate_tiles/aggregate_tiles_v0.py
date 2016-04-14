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

This script allows to create an aggreagte file product from the tiles inside IMG_DATA and QI_DATA folders.
The output generated is in LEGACY_DATA folder of the product
Following steps are performed in order to obtain the mosaic from all tiles of the product:
       - Create running context: init all the necessary variables in order to start the process
       - Create folder tree inside LEGACY_DATA folder: productFolder/LEGACY_DATA/IMG_DATA and productFolder/LEGACY_DATA/QI_DATA
       - Reproject and rescale all tiles found in productFolder/TILES/*/IMG_DATA and productFolder/TILES/*/QI_DATA to desired rescaleFactor
         Reprojection is performed to the majority Coordinate_Reference_System found in all the tiles from productFolder/TILES/*/IMG_DATA
         and respectively to the majority Coordinate_Reference_System found in all the masks from productFolder/TILES/*/QI_DATA
       - Build mosaic files from TILES/*/IMG_DATA and TILES/*/QI_DATA into corresponding LEGACY_DATA folder
         Mosaic/aggregation is performed keeping into account that one distinct mosaic file is generated for each type of tile: 
         ex: one distinct mosaic for each biospherical suffix or mask suffix
       - Post process operation for files found into folder LEGACY_DATA/IMG_DATA. This is performed in order to concatenate all mosaic files produced
         on previous step (all files having or not biosferical suffix are concatenated into a single final aggragate/mosaic image)
       - Generation of the metadata xml file containing valuable information about the generated aggragate/mosaic image
       - Generation of the quicklook preview file of the generated aggragate/mosaic image

Usage :
    python aggregate_tiles.py -prodfolder path/to/my/product/directory -rescaleval 60
"""

from __future__ import print_function
import argparse
import re
import glob
import gdal
import osr
import subprocess
import math
import os
import shutil
import sys
import pipes
import itertools
import operator
from collections import defaultdict
from xml.dom.minidom import parse, parseString
import datetime

#---------------------------------------------------------------
#max number of Channels in Channel List for otb Quicklook application
CONST_NB_BANDS_QUIKLOOL_LIMIT = "4"
#---------------------------------------------------------------
def run_command(args):
    print(" ".join(map(pipes.quote, args)))
    subprocess.call(args)
#----------------------------------------------------------------
def quicklook_mosaic(inpFileName, outFileName, channelList):
    run_command(["otbcli_Quicklook",
                 "-in",inpFileName,
                 "-cl"] + channelList + [
                 "-out", outFileName])
#----------------------------------------------------------------
def process_mosaic_images(interpolName, listOfImages, imgAggregatedName):
    run_command(["otbcli",
                 "Mosaic",
                 "-il"] + listOfImages + [
                 "-out", imgAggregatedName,
                 "-comp.feather", "none",
                 "-interpolator", interpolName ])
#----------------------------------------------------------------
def image_reproject_and_rescale(reprojIsNecesary, targetSrs, rescaleIsNecesary, targetSizeX, targetSizeY, resampleMethod, inpFile, outFile ):

   if reprojIsNecesary and rescaleIsNecesary:
      #perform both reprojection and rescale
      run_command(["gdalwarp",
                 "-of", "GTiff",
                 "-t_srs", "ESRI::" + str(targetSrs),
                 "-ts", str(targetSizeX), str(targetSizeY),
                 "-r", resampleMethod,
                 inpFile,
                 outFile])
      print("[.....]->Both reprojection and rescale on:" + inpFile)
   elif reprojIsNecesary:
      #perform just reprojection
      run_command(["gdalwarp",
                 "-of", "GTiff",
                 "-t_srs", "ESRI::" + str(targetSrs),
                 inpFile,
                 outFile])
      print("[.....]->Just reprojection on:" + inpFile)
   elif rescaleIsNecesary:
      #perform just  rescale
      run_command(["gdalwarp",
                 "-of", "GTiff",
                 "-t_srs", "ESRI::" + str(targetSrs),
                 "-ts", str(targetSizeX), str(targetSizeY),
                 inpFile,
                 outFile])
      print("[.....]->Just rescale on:" + inpFile)
   else:
      print("[.....]->No reprojection nor rescale needed on:" + inpFile)
      #copy input file name to output destination as a renamed file
      shutil.copyfile (inpFile, outFile)

#----------------------------------------------------------------
class Context(object):

    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)
#----------------------------------------------------------------
def print_dict_entry(dictionary):
   for key, listFiles in dictionary.iteritems():
      print (key, ', '.join(listFiles))
#----------------------------------------------------------------
def print_post_processing_list(context):
   for item in context.post_process_list:
         print(item)
#----------------------------------------------------------------
def create_context(args):
    #colect all raster images into a specific list
    retrieved_img_data_list= get_list_img_file_names(args.prodfolder, "TILES", "IMG_DATA")
    retrieved_qi_data_list= get_list_img_file_names(args.prodfolder, "TILES", "QI_DATA")

    #retrieve Field option Time Period :_V or Aquisition Period Field:_A from the Product Folder
    productFolderName = (args.prodfolder.split('/'))[-1]
    pattern = re.compile("(_[AV][\d]{8})")
    #extract part _AYYYYMMDD or _VYYYYMMDD where YYYYMMDD= YearMonthDay
    fieldOptionList = pattern.findall(productFolderName)
    if not fieldOptionList:
       sys.exit("ERROR:Raster images from TILES folder should have Time Period or Aquisition Period Field ")
    else:
       #extract Field Character: _V or _A
       fieldAsStr = ''.join(fieldOptionList)
       characterField = fieldAsStr[0] + fieldAsStr[1]

    ret_val_img = "", "", ""
    if len(retrieved_img_data_list):
       #extract CS_NAME, CS_CODE and PROJCS from all the input tiles (IMG_DATA)
       ret_val_img = getMajorityWGS_PROJCS(retrieved_img_data_list)

    ret_val_qi = "", "", ""
    if len(retrieved_qi_data_list):
       #extract CS_NAME, CS_CODE and PROJCS from all the input masks (QI_DATA)
       ret_val_qi = getMajorityWGS_PROJCS(retrieved_qi_data_list)

    #create a dictionary holding all necessary information for processing 
    d = dict(#reprojection and rescale to desired rescale_output
             img_data_inp_list = retrieved_img_data_list,
             qi_data_inp_list = retrieved_qi_data_list,
             img_data_out_list = list(),
             qi_data_out_list = list(),
             #init dict used for aggregation of tiles: dictionnary with files containing or not suffixes (SLAIF,SLAIR,SRFL,etc)
             img_data_dic = defaultdict(list),
             #init dict used used for aggregation of masks : dictionnary with files contain or not suffixes (MFLG, MDAT and SWGT,etc)
             qi_data_dic = defaultdict(list),
             #optional field separator for Acquisition Date: _A or Time Period: _V
             field_separator = str(characterField),
             #name of output folder where aggregate img are found : IMG_DATA and QI_DATA
             out_dir_name_file_img_data = "none",
             out_dir_name_file_qi_data = "none",
             #files name output according to PDS doc v1.2 pag41
             out_filename_aggreg_img_data_dict = defaultdict(lambda: "empty"),
             out_filename_aggreg_qi_data_dict = defaultdict(lambda: "empty"),
             #CS reprojection coord majority in masks
             projcs_reproj_coord_qi = ret_val_qi[0],
             #CS name majority in tiles
             projcs_reproj_coord_img = ret_val_img[0],
             cs_name_majority_img = ret_val_img[1],
             cs_code_majority_img = ret_val_img[2],
             #post processing afer mosaic
             post_process_list = list(),
             post_process_out_filename = "none",
             #xml metadata information
             out_metadata_file_name = "none",
             xml_info_from_mosaic_dict = defaultdict(lambda: "empty"),
             #quicklook generation
             quicklook_out_filename = "none",
             #other useful information -- arguments values
             destRootFolder = "LEGACY_DATA",
             prodFolderName = args.prodfolder,
             rescale_output = args.rescaleval)

    return Context(**d)
#----------------------------------------------------------------
def create_processing_list_upon_sufix(initialList):
   images_by_patterns = defaultdict(list)

   for fullFileName in initialList:  
      #extract the file name
      fileName = os.path.basename(fullFileName)
      
      #try split file name before DATE Time Period:_Vyyyymmdd_YYYYMMDD    
      file_name_parts = fileName.split(context.field_separator)

      #search if biosferical status/mask suffix (SRFL_10, MDAT,etc) exists in second part of
      #filename ex: 20150728_74586544_SRFL_10.TIF and extract it
      #This suffix may start with M or S letters : _Mxxx or _Sxxx acording to PSD doc
      pattern = re.compile("_([MS](?:[A-Z_0-9]+))\.\w+$")
      result=pattern.search(file_name_parts[1])
      if result:
         #add entry in dictionnary with key result.group(1)
         images_by_patterns[result.group(1)].append(fullFileName)
      else:
         #add entry in dictionnary with key result.group(1)
         images_by_patterns[""].append(fullFileName)
      
   return images_by_patterns
#----------------------------------------------------------------
def build_file_name_output(prodFolderName, patternName, extensionFileName, outFolderName):

   #set the legacy file name as product name
   file_name = prodFolderName.split('/')[-1]

   #extract needed parts to build the output name for the file
   pattern = re.compile("([A-Z_0-9]*_[A-Z]*_L[0-9][A-Z])\w+$")
   #extract part S2AGRI_SVT1_PRD_L4A for example
   result_first=pattern.search(file_name)

   pattern = re.compile("([A-Z]*_[0-9]{6,}\w+$)")
   #extract part CSRO_20151221T164437_V20130206_20130616 for example
   result_sec=pattern.search(file_name)
   
   #build name as doc states for legacy file  #insert LY tag after product name
   legacy_file_tmp = str(result_first.group(1)) + "_LY_" + str(result_sec.group(1))

   if patternName == "":
      legacy_file_name = legacy_file_tmp + extensionFileName
   else:
      legacy_file_name = legacy_file_tmp + "_" + patternName + extensionFileName

   if outFolderName == "IMG_DATA":
      #replace PRD by DAT as documentation PSD1.2 states for legacy file for IMG_DATA
      file_name = legacy_file_name.replace("_PRD_","_DAT_")

      #create full path file name to be generated
      full_path_file = os.path.join(context.out_dir_name_file_img_data, file_name)

      #update context dictionary holding output file name for corresponding pattern
      context.out_filename_aggreg_img_data_dict[patternName] = full_path_file

   else:
      #replace PRD by MSK as documentation PSD1.2 states for legacy file for IMG_DATA
      file_name = legacy_file_name.replace("_PRD_","_MSK_")

      #create full path file name to be generated
      full_path_file = os.path.join(context.out_dir_name_file_qi_data, file_name)

      #update context dictionary holding output file name for corresponding pattern
      context.out_filename_aggreg_qi_data_dict[patternName] = full_path_file

   #For IMG_DATA  -naming rules for L4 product
   #out name to be set:  S2AGRI_OPER_DAT_L4A_LY_C001_20140201T134012_V20140908_20141015.jp2

   #For QI_DATA
   #out name to be set S2AGRI_OPER_MSK_L4A_LY_C001_20140201T134012_V20140908_20141015_MNODATA.gml
#----------------------------------------------------------------
def get_list_img_file_names(prodDir, srcDirProcess, imgDataFolder):
    """
    This function will generate the file names in a directory 
    """
    list_file_paths = []  # List which will store all of the full filepaths.
     	
    #imgFolder : IMG_DATA or QI_DATA  srcDirProcess : TILES
    full_folder_path=os.path.join(prodDir, srcDirProcess, '*', imgDataFolder, '*.TIF')

    #retrieve all TIF files on that path
    list_file_paths=glob.glob(full_folder_path)  # Add discovered files to the list.

    return list_file_paths  # Self-explanatory.
#----------------------------------------------------------------
def crete_dirs_tree(context, outFolderName):
    #check if LEGACY_DATA exists - if not --create it
    path_to_check=os.path.join(context.prodFolderName, context.destRootFolder)
    if not os.path.isdir(path_to_check):
        os.makedirs(path_to_check)

    #create subdir IMG_DATA or QI_DATA in LEGACY_DATA folder
    path_to_check=os.path.join(context.prodFolderName, context.destRootFolder, outFolderName) 
    if not os.path.isdir(path_to_check):
      os.makedirs(path_to_check)

    #update contextinformation for this field
    if outFolderName == 'IMG_DATA':
       context.out_dir_name_file_img_data=path_to_check
    else:
       context.out_dir_name_file_qi_data=path_to_check
#----------------------------------------------------------------
def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Creates Agregate Images for LEGACY_DATA folder")
    parser.add_argument('-prodfolder', required=True, help="input dir path. Ex: /FullProductNamePath")
    parser.add_argument('-rescaleval', required=True, help="pixel scale on output mosaic . Ex: 60")

    args = parser.parse_args()
    return create_context(args)
#----------------------------------------------------------------
def generate_names_for_output_files(context, dataFolder):
   if dataFolder == "IMG_DATA":
      for keyIMGSuffix in context.img_data_dic:
         #build_file_name_output(context.img_data_dic[keyIMGSuffix], keyIMGSuffix, ".TIF", dataFolder)
         build_file_name_output(context.prodFolderName, keyIMGSuffix, ".TIF", dataFolder)
   else:
      for keyQISuffix in context.qi_data_dic:
         #build_file_name_output(context.qi_data_dic[keyQISuffix], keyQISuffix, ".TIF", dataFolder)
         build_file_name_output(context.prodFolderName, keyQISuffix, ".TIF", dataFolder)
#----------------------------------------------------------------
def collect_image_resolution_name(fullFileName):
    #use gdal to get resolution
    dataset = gdal.Open(fullFileName, gdal.gdalconst.GA_ReadOnly)

    size_x = dataset.RasterXSize
    size_y = dataset.RasterYSize

    #create tuple as : resolution pixels, x_size, y_size, file_name without full path
    tuple_entry = size_x*size_y, size_x, size_y, os.path.basename(fullFileName)
   
    return tuple_entry
#----------------------------------------------------------------
def resolution_mismatch_found(context):
   isMismatch = False
   for a, b in itertools.combinations(context.post_process_list, 2):
      if cmp(a[0], b[0]):
         isMismatch = True
         break
   return isMismatch
#----------------------------------------------------------------
def resize_resolution_to_referece_val(context, width, height, dataFolder):
   #inspect the processing list
   for item in context.post_process_list: 
      if cmp(item[1], width) or cmp(item[2], height):
         #compute input file name to be resized
         inpFileName = os.path.join(dataFolder, item[3])

         #compute output file name (ex: inpFileName_resized.TIF)
         compFileName = (item[3].split('.'))[0] + "_resized" + ".TIF"
         outFileName = os.path.join(dataFolder, compFileName)
         
         #perform resize opeartion with GDAL
         run_command(["gdalwarp",
                       "-of", "GTiff",
                       "-ts",  str(width), str(height),
                       inpFileName,
                       outFileName])

         #remove initial file that was input for resize
         os.remove(inpFileName)

         #rename output file as the input file before resize
         os.rename(outFileName, inpFileName)
#----------------------------------------------------------------
def concatenate_mosaic_files(context, listOfImages):
   #perform concatenation
   run_command(["otbcli",
                "ConcatenateImages",
                "-il"] + listOfImages + [
                "-out", context.post_process_out_filename])
#----------------------------------------------------------------
def perform_images_concatenation(context, listOfFiles, dataFolder):
   #build intermediary dictionnary
   tmp_data_dictionary = create_processing_list_upon_sufix(listOfFiles)

   #create a list of tupples to store files upon scale resolution: 10,20,.. 
   list_img_by_scale = list()
   for key, fileList in tmp_data_dictionary.iteritems():
      if key:
         if ("_") in key:
            #split key upon "_" (the key is "bisofericalStatusIndicator"_"scaleProjection" -> key: SLAIR_20)
            scale_projection = (key.split("_"))[1]
         else:
            #no split needed (the key is "bisofericalStatusIndicator" -> key: SLAIR)
            scale_projection = ""

         #add entry to list as a tupple : ("20","SLAIR_20","fileList") or ("","SLAIR","fileList")
         tuple_entry = str(scale_projection), str(key), ''.join(fileList)
         list_img_by_scale.append(tuple_entry)

   #sort the list in order to provide concatenation files in right sequence (stating with lowest scale , then upon same suffix)
   list_img_by_scale.sort()

   #create final  output name of legacy file containing all concatenated files from LEGACY_DATA/IMG_DATA
   #get first file from the provided list: at this stage of processing all the files from the list
   #have the correct format naming as per PSD but still have biosferical suffix when product type is L3A or L3B
   file_name = os.path.basename( (list_img_by_scale[0])[2] )

   #build here file name by stripping unwanted suffix from the file name
   file_tmp = format_file_name_output(file_name) + ".TIF"

   #store full file name into context variable
   context.post_process_out_filename = os.path.join(dataFolder, file_tmp)
   
   #concatenate files into one single mosaic image
   concatenate_mosaic_files(context, [f[2] for f in list_img_by_scale])
   
   #remove files which were inputfor concatenation - keeping the final output name
   for fileName in list_img_by_scale:
      os.remove(fileName[2])
#----------------------------------------------------------------
def format_file_name_output(fileName):

   #split file name before DATE Time Period:_Vyyyymmdd_YYYYMMDD
   file_name_parts = fileName.split(context.field_separator)

   #extract needed date parts to build the output name for the file
   #from 20150728_none_SRFL_10.TIF it extracts 20150728
   #from 20150728_20150602_SRFL_20.TIF it extracts [20150728, 20150602]
   pattern = re.compile("([\d]{8}(?:[A-Z])?)")
   #list of matches
   resultList=pattern.findall(file_name_parts[1])
   
   datePart = ""
   #concatenate list elements
   if len(resultList) == 1:
      #append the separator _ after getting the only elem in the list
      datePart = datePart + resultList[-1]
   else:
      #concatenate elem list with the separator _
      datePart = '_'.join(resultList)

   #build here full file name without extension
   file_tmp = file_name_parts[0] + context.field_separator + datePart

   #return the computed name of the file without extension
   return file_tmp
#----------------------------------------------------------------
def post_process_mosaic_images(context, dataFolder):

   #obtain mosaic files
   list_file_paths=glob.glob(os.path.join(dataFolder, '*.TIF'))

   #when none or one mosaic file produced -- nothing to do furher 
   if len(list_file_paths) == 0:
      print("No post processing needed ")
   elif len(list_file_paths) == 1:
      print("Format file name to match documentation naming")
      file_name = os.path.basename(list_file_paths[0])
      
      #format file name
      legacy_final_out_name = format_file_name_output(file_name)
      new_file_name = os.path.join(dataFolder, legacy_final_out_name + ".TIF")
    
      #update field in dict for final out name for mosaic file
      context.post_process_out_filename = new_file_name

      #rename the only one file found in directory with the computed new name (biosferical suffix discarded)
      os.rename(''.join(list_file_paths), new_file_name)
   else:
      #perform resolution adjust after creating mosaic files in order to 
      #bring all mosaic files to same resolution for being able to concatenate them
      #get resolution of each file and save it
      for idx, fullFileName in enumerate(list_file_paths):
         context.post_process_list.append(collect_image_resolution_name(fullFileName))

      #sort the list in order to obtain the smallest resolution  wich will be referece for
      #all further files resize operations
      context.post_process_list.sort()

      #detemine if resolution mismatch exists in aggregate files from LEGACY_DATA/IMG_DATA
      if resolution_mismatch_found(context):
         #get first element from the sorted list and extract resolution
         #this resolution will be reference resolution for further resize operations
         xSize = (context.post_process_list[0])[1]
         ySize = (context.post_process_list[0])[2]

         #uniformize the resolutions of the post procesed files to the smallest resolution
         resize_resolution_to_referece_val(context, xSize, ySize, dataFolder)

      #concatenate files in order to obtain final aggregation file
      perform_images_concatenation(context, list_file_paths, dataFolder)
#----------------------------------------------------------------
def perform_tiles_aggreagtion(context):

    #build dictionnary with files containing or not patterns at the end of the file (SLAIF,SLAIR,SRFL,etc)
    context.img_data_dic = create_processing_list_upon_sufix(context.img_data_out_list)
    #build dictionnary with files contain or not patterns the end of the file (MFLG, MDAT and SWGT,etc)
    context.qi_data_dic = create_processing_list_upon_sufix(context.qi_data_out_list)

    #create output file name for aggregate images
    if len(context.img_data_dic):
       #generate file name of the IMG_DATA legacy mosaic file
       generate_names_for_output_files(context, "IMG_DATA")

       #call mosaic function for each processing list from IMG_DATA
       for keyIMGSuffix in context.img_data_dic:
          process_mosaic_images("linear",  #use interpolator Linear for TILES
                                context.img_data_dic[keyIMGSuffix],
                                context.out_filename_aggreg_img_data_dict[keyIMGSuffix])

       #remove previous generated files during reprojection and rescale - no more needed
       for fileName in context.img_data_out_list:
          os.remove(fileName)

       #clear list
       del context.img_data_out_list[:]
      
    else:
       print("List of files from IMG_DATA is empty")


    if len(context.qi_data_dic):
       #generate file name of the QI_DATA legacy mosaic file
       generate_names_for_output_files(context, "QI_DATA")

       #call mosaic function for each processing list from IMG_DATA
       for keyQISuffix in context.qi_data_dic:
          process_mosaic_images("nn",  #use interpolator Nearest Neighborhood for MASKS
                                context.qi_data_dic[keyQISuffix],
                                context.out_filename_aggreg_qi_data_dict[keyQISuffix])

       #remove previous generated files during reprojection and rescale - no more needed
       for fileName in context.qi_data_out_list:
          os.remove(fileName)

       #clear list
       del context.qi_data_out_list[:]
    else:
       print("List of files from QI_DATA is empty")

     
#----------------------------------------------------------------
def get_resolution_and_projection(fileName):
   #current projection system
   dataset = gdal.Open(fileName, gdal.gdalconst.GA_ReadOnly)
   source_srs = osr.SpatialReference()
   source_srs.ImportFromWkt(dataset.GetProjection())
   
   #current scale
   geo_transform = dataset.GetGeoTransform()
   spacing = geo_transform[1]

   ret_val = str(source_srs), dataset.RasterXSize, dataset.RasterYSize, spacing
   return ret_val
#----------------------------------------------------------------
def compute_target_resolution(curScale, targetScale, curX, curY):
   multiply_factor = float(curScale)/float(targetScale)
   targetX = int ( float(curX) * multiply_factor )
   targetY = int ( float(curY) * multiply_factor )
   target_coord = targetX, targetY
   return target_coord
#----------------------------------------------------------------
def raster_reprojection_and_rescale(listOfFiles, targetSrs, resamplAlg, destFolder):
   for fileName in listOfFiles:
      #get current resolution and projection coordinates of the image
      ret_val = get_resolution_and_projection(fileName)
      #when projection of the current image is different from the one computed as
      #being the reference projection for all the images
      reprojIsNecesary = False
      if ret_val[0] != str(targetSrs):
         reprojIsNecesary = True

      # compute target resolution and call function performing reprojection and rescale if necessary
      rescaleIsNecesary = False
      targetX = ret_val[1]
      targetY = ret_val[2]
      #when scale of the current image is different from the parameter rescale_output
      if int(ret_val[3]) != int(context.rescale_output):
         rescaleIsNecesary = True
         ret_target_coord = compute_target_resolution(ret_val[3], context.rescale_output, ret_val[1], ret_val[2])
         targetX = ret_target_coord[0]
         targetY = ret_target_coord[1]

      #compute output file name (ex: same as inpFileName.TIF) - The reprojected file will reside to LEGACY_DATA/IMG_DATA or LEGACY_DATA/QI_DATA
      shortFilename = os.path.basename(fileName)
      outFullPathFilename = os.path.join(context.prodFolderName,context.destRootFolder, destFolder, shortFilename)
      
      #update list with reprojected files
      if destFolder == "IMG_DATA":
         context.img_data_out_list.append(outFullPathFilename)
      else:
         context.qi_data_out_list.append(outFullPathFilename)

      #call the function performing that tasks of reprojection and rescale
      image_reproject_and_rescale(reprojIsNecesary, targetSrs, rescaleIsNecesary, targetX, targetY, resamplAlg, fileName, outFullPathFilename )
#----------------------------------------------------------------
def perform_reprojection_and_rescale(context):

   #if files in TILES/*/IMG_DATA
   if len(context.img_data_inp_list):
      raster_reprojection_and_rescale (context.img_data_inp_list, context.projcs_reproj_coord_img, "bilinear", "IMG_DATA")

   #if files in TILES/*/QI_DATA
   if len(context.qi_data_inp_list):
      raster_reprojection_and_rescale (context.qi_data_inp_list, context.projcs_reproj_coord_qi, "near", "QI_DATA")
#----------------------------------------------------------------
def getExtent(gt, cols, rows):
    ext = []
    xarr = [0, cols]
    yarr = [0, rows]

    for px in xarr:
        for py in yarr:
            x = gt[0] + px * gt[1] + py * gt[2]
            y = gt[3] + px * gt[4] + py * gt[5]
            ext.append([x, y])
        yarr.reverse()
    return ext
#----------------------------------------------------------------
def reprojectCoords(coords, src_srs, tgt_srs):
    trans_coords = []
    transform = osr.CoordinateTransformation(src_srs, tgt_srs)
    for x, y in coords:
        x, y, z = transform.TransformPoint(x, y)
        trans_coords.append([x, y])
    return trans_coords
#----------------------------------------------------------------
def getReprojectedDatasetGdal(dataset):

    size_x = dataset.RasterXSize
    size_y = dataset.RasterYSize

    geo_transform = dataset.GetGeoTransform()

    spacing_x = geo_transform[1]
    spacing_y = geo_transform[5]

    extent = getExtent(geo_transform, size_x, size_y)

    source_srs = osr.SpatialReference()
    source_srs.ImportFromWkt(dataset.GetProjection())
    epsg_code = source_srs.GetAttrValue("AUTHORITY", 1)
    
    target_srs = osr.SpatialReference()
    target_srs.ImportFromEPSG(4326)

    wgs84_extent = reprojectCoords(extent, source_srs, target_srs)

    return wgs84_extent
#----------------------------------------------------------------
def granule_img_build_dict(listOfInitialInputFiles):
   granule_dict = defaultdict(list)
   
   for itemFullPath in listOfInitialInputFiles:
      #extract tilenamefolder and list of tiles inside ../tilenamefolder/IMG_DATA/
      split_list = itemFullPath.split('/')
      #last elem is TileName, the 3-rd to last is Tile Folder name
      granule_dict[split_list[-3]].append(split_list[-1])
      #build dict as d["Folder_tile_1"]=["file1, "file2"], d["Folder_tile_2"]=["file2, "file3"]

   #convert dict to list
   result = granule_dict.items()
   #converted list obtained[("Folder_tile_1", ["file1, "file2"]), ("Folder_tile_2", ["file3, "file4"])]
   result.sort()
   #sort again to have list of files sorted
   for directory in result:
      directory[1].sort()

   #return the list
   return result

#----------------------------------------------------------------
def getMajorityWGS_PROJCS(listFiles):
   proj_CS_dict = defaultdict(int)
   for itemFullPath in listFiles:
      dataset = gdal.Open(itemFullPath, gdal.gdalconst.GA_ReadOnly)
      source_srs = osr.SpatialReference()
      source_srs.ImportFromWkt(dataset.GetProjection())

      if not source_srs in proj_CS_dict:
         proj_CS_dict[source_srs] = 1
      else:
         proj_CS_dict[source_srs] += 1

   max_srs = max(proj_CS_dict.iteritems(), key=operator.itemgetter(1))[0]

   #prepare the output as a tuple
   proj_CS_name_computed = max_srs.GetAttrValue("PROJCS", 0)
   proj_CS_code_computed = max_srs.GetAttrValue("AUTHORITY", 1)

   ret_val = str(max_srs), proj_CS_name_computed, proj_CS_code_computed

   return ret_val
#----------------------------------------------------------------
def build_info_from_mosaic(mosaicFileName):

   mosaic_dict = defaultdict(lambda: "empty")

   #open file in Gdal
   dataset = gdal.Open(mosaicFileName, gdal.gdalconst.GA_ReadOnly)

   #Corner coordinates transformed as "EPSG:4236"  from mosaic file for LOWER_CORNER/UPPER_CORNER xml node
   all_coord_points = getReprojectedDatasetGdal(dataset)

   lower_corner_val = str(all_coord_points[1][0]) + " " + str(all_coord_points[1][1])
   mosaic_dict["lower_corner_val"] = lower_corner_val
   upper_corner_val = str(all_coord_points[3][0]) + " " + str(all_coord_points[3][1])
   mosaic_dict["upper_corner_val"] = upper_corner_val

   #size resolution from mosaic file for Geoposition/ subnooddes ULX/ULY/XDIM/YDIM xml node
   src_x_size = dataset.RasterXSize
   src_y_size = dataset.RasterYSize
   src_geo_transform = dataset.GetGeoTransform()
   (ulx, uly) = (src_geo_transform[0], src_geo_transform[3])
   (pixRes, pixResNeg) = (src_geo_transform[1], src_geo_transform[5])

   #size resolution from mosaic file for ULX/ULY/XDIM/YDIM xml node
   mosaic_dict["ulx"] = str(int(src_geo_transform[0])) #top left x
   mosaic_dict["uly"] = str(int(src_geo_transform[3])) #top left y

   mosaic_dict["xdim"] = str(src_geo_transform[1]) # w-e pixel resolution
   mosaic_dict["ydim"] = str(src_geo_transform[5]) # n-s pixel resolution (negative value)

   #extent position extraction
   extent_list_val = str(format(all_coord_points[0][0], '.6f')) + " " + str(format(all_coord_points[0][1], '.6f')) + " " + \
   str(format(all_coord_points[1][0], '.6f')) + " " + str(format(all_coord_points[1][1], '.6f')) + " " + \
   str(format(all_coord_points[2][0], '.6f')) + " " + str(format(all_coord_points[2][1], '.6f')) + " " + \
   str(format(all_coord_points[3][0], '.6f')) + " " + str(format(all_coord_points[3][1], '.6f')) + " " + \
   str(format(all_coord_points[0][0], '.6f')) + " " + str(format(all_coord_points[0][1], '.6f'))

   mosaic_dict["extent_list_val"] = extent_list_val

   #size of mosaic file for node Size and subnodes NROWS/NCOLS xml node
   mosaic_dict["nrows"] = str(dataset.RasterXSize)
   mosaic_dict["ncols"] = str(dataset.RasterYSize)

   #resolution atr value for node Geometric_Info and subnodes Size and Geoposition
   mosaic_dict["resolution"] = str(int(src_geo_transform[1]))

   #Number of bands from mosaic file for BAND_NAME xml node
   mosaic_dict["nb_mosaic_bands"] = dataset.RasterCount
   
   #granule List from all input TILES files for Granules/IMAGE_ID xml node
   mosaic_dict["granule_list"] = granule_img_build_dict(context.img_data_inp_list)

   #mask List from all input MASK files for MASK_FILENAME xml node
   mosaic_dict["mask_list"] = granule_img_build_dict(context.qi_data_inp_list)

   #wgs_projcs  - information already computed in context: projcs_reproj_coord ,cs_name_majority and cs_code_majority

   return mosaic_dict
#----------------------------------------------------------------
#----------------------------------------------------------------
#     XML UPDATE HELPER
#----------------------------------------------------------------
#----------------------------------------------------------------
def get_node_element_upon_name(dom, nodeName):
   listNodes = dom.getElementsByTagName(nodeName)
   if len(listNodes):
      return listNodes[-1]
   else:
      return []
#----------------------------------------------------------------
def update_text_xml_node(nodeElem, newTextValue):
   nodeElem.firstChild.nodeValue = newTextValue
#----------------------------------------------------------------
def remove_xml_node(nodeElem, childNodeName):
   #if node has a child named childNodeName
   if nodeElem.hasChildNodes():
      childList =  nodeElem.childNodes
      for child in childList:
         if child.nodeName == childNodeName:
            #remove that child
            nodeElem.removeChild(child)
#----------------------------------------------------------------
def update_xml_node_atr(dom, nodeElem, attrName, attrVal):
   attr = dom.createAttribute(attrName)
   attr.nodeValue = attrVal
   nodeElem.setAttributeNode(attr)
#----------------------------------------------------------------
def create_xml_node(dom, nodeElem, newNodeName, attrNameValList, textNodeVal):
   newNode = dom.createElement(newNodeName)
   if attrNameValList:
      for idx, (atr, val) in enumerate(attrNameValList):
         attr = dom.createAttribute((attrNameValList[idx])[0])
         attr.nodeValue =  (attrNameValList[idx])[1]
         newNode.setAttributeNode(attr)
      if textNodeVal:
         nodeText = dom.createTextNode(textNodeVal)
         newNode.appendChild(nodeText)
   else:
      if textNodeVal:
         nodeText = dom.createTextNode(textNodeVal)
         newNode.appendChild(nodeText)

   nodeElem.appendChild(newNode)
#----------------------------------------------------------------
def remove_whilespace_nodes(node):
    """ Removes all of the whitespace-only text decendants of a DOM node. """
    #prepare the list of text nodes to remove (and recurse when needed)
    remove_list = [  ]
    for child in node.childNodes:
        if child.nodeType == child.TEXT_NODE and not child.data.strip( ):
            #add this text node to the to-be-removed list
            remove_list.append(child)
        elif child.hasChildNodes( ):
            #recurse, it's the simplest way to deal with the subtree
            remove_whilespace_nodes(child)
    #perform the removals
    for node in remove_list:
        node.parentNode.removeChild(node)
        node.unlink( )
#----------------------------------------------------------------
def create_xml_mosaic_metadata(context):

   #prepare necessary information for xml metadata file creation for LEGACY produced images
   #information is obtained from mosaic file produced on earlier stages of processing
   context.xml_info_from_mosaic_dict = build_info_from_mosaic(context.post_process_out_filename)

   #get metatdata xml product file residing in PRODUCT folder
   list_file_paths=glob.glob(os.path.join(context.prodFolderName,'*'))
   input_xml_file = "none"
   for file in list_file_paths:
        if file.endswith(".xml"):
           input_xml_file = file

   #set output name for metadata file of the LEGACY_DATA folder : create the name
   #starting from metadata file of the product
   file_tmp = os.path.basename(input_xml_file)
   fileOut = file_tmp.replace("_PR_","_LY_")

   #compute full path name and save it
   context.out_metadata_file_name = os.path.join(context.prodFolderName, context.destRootFolder, fileOut)

   #open xml file from product folder
   dom = parse(input_xml_file)
   root = dom.documentElement

   #update text for <PRODUCT_TYPE> node
   node = get_node_element_upon_name(dom, 'PRODUCT_TYPE')
   update_text_xml_node(node, 'MOSAIC')

   #update text for <GENERATION_TIME> node
   node = get_node_element_upon_name(dom, 'GENERATION_TIME')
   update_text_xml_node(node, datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%dT%H:%M:%S'))

   #remove node <PREVIEW_IMAGE_URL> -- which is subnode for Product_Info
   node = get_node_element_upon_name(dom, 'Product_Info')
   if node :
      remove_xml_node(node, 'PREVIEW_IMAGE_URL')

   #update node <PREVIEW_IMAGE> -- which is subnode for Query_Options
   node = get_node_element_upon_name(dom, 'PREVIEW_IMAGE')
   update_text_xml_node(node, "true")

   #update text for <Bbox> coordinates <LOWER_CORNER> and <UPPER_CORNER>
   node = get_node_element_upon_name(dom, 'LOWER_CORNER')
   update_text_xml_node(node , context.xml_info_from_mosaic_dict["lower_corner_val"])
   node = get_node_element_upon_name(dom, 'UPPER_CORNER')
   update_text_xml_node(node, context.xml_info_from_mosaic_dict["upper_corner_val"])

   #update <Band_List> node
   #get nodes <Band_List> if these nodes exists
   node = get_node_element_upon_name(dom, 'Band_List')
   if node :
      remove_xml_node(node, 'BAND_NAME')
   else :
      #get parent node on which Band_List should reside
      node = get_node_element_upon_name(dom, 'Query_Options')
      #create subnode Band_List as child of Query_Options
      create_xml_node(dom, node, 'Band_List', [], '')

   #add nodes BAND_NAME, as subnode for Band_List for each band extracted
   node = get_node_element_upon_name(dom, 'Band_List')

   for bandId  in range(context.xml_info_from_mosaic_dict["nb_mosaic_bands"]):
      textNode = "B" + str(bandId + 1)
      create_xml_node(dom, node, 'BAND_NAME', [], textNode)

   #update <Granule_List> node
   #first delete all nodes <Granules>
   node = get_node_element_upon_name(dom, 'Granule_List')
   if node :
      remove_xml_node(node, 'Granules')

   #create nodes <Granules> starting from entry in xml_info_from_mosaic_dict["granule_list"]
   node = get_node_element_upon_name(dom, 'Granule_List')
   for item in context.xml_info_from_mosaic_dict["granule_list"]:
      create_xml_node(dom, node, 'Granules', [('granuleIdentifier',item[0]), ('imageFormat','GEOTIFF')], '')
      nodeSub = get_node_element_upon_name(dom, 'Granules')
      for listElem in item[1]:
         create_xml_node(dom, nodeSub, 'IMAGE_ID', [], listElem)

   #update node <EXT_POS_LIST>  -- child of Global_Footprint
   node = get_node_element_upon_name(dom, 'EXT_POS_LIST')
   if node :
      update_text_xml_node(node, context.xml_info_from_mosaic_dict["extent_list_val"])

   #remove node <Coordinate_Reference_System>
   node = get_node_element_upon_name(dom, 'Geometric_Info')
   if node :
      remove_xml_node(node, 'Coordinate_Reference_System')

   #create node <Tile_Geocoding> as subsode of Geometric_Info
   node = get_node_element_upon_name(dom, 'Geometric_Info')
   create_xml_node(dom, node, 'Tile_Geocoding', [('metadataLevel', 'Brief')], '')

   #create node <HORIZONTAL_CS_NAME> as subsode of Tile_Geocoding
   node = get_node_element_upon_name(dom, 'Tile_Geocoding')
   create_xml_node(dom, node, 'HORIZONTAL_CS_NAME', [], context.cs_name_majority_img)

   #create node <HORIZONTAL_CS_CODE> as subsode of Tile_Geocoding
   node = get_node_element_upon_name(dom, 'Tile_Geocoding')
   create_xml_node(dom, node, 'HORIZONTAL_CS_CODE', [], "EPSG:" + context.cs_code_majority_img)

   #create node <Size> as subsode of Tile_Geocoding
   node = get_node_element_upon_name(dom, 'Tile_Geocoding')
   create_xml_node(dom, node, 'Size', [ ('resolution', context.xml_info_from_mosaic_dict["resolution"]) ],'')

   #create node <NROWS> as subsode of Size
   node = get_node_element_upon_name(dom, 'Size')
   create_xml_node(dom, node, 'NROWS', [], context.xml_info_from_mosaic_dict["nrows"])

   #create node <NCOLS> as subsode of Size
   node = get_node_element_upon_name(dom, 'Size')
   create_xml_node(dom, node, 'NCOLS', [], context.xml_info_from_mosaic_dict["ncols"])

   #create node <Geoposition> as subsode of Tile_Geocoding
   node = get_node_element_upon_name(dom, 'Tile_Geocoding')
   create_xml_node(dom, node, 'Geoposition', [('resolution', context.xml_info_from_mosaic_dict["resolution"]) ],'')

   #create node <ULX> as subsode of Geoposition
   node = get_node_element_upon_name(dom, 'Geoposition')
   create_xml_node(dom, node, 'ULX', [], context.xml_info_from_mosaic_dict["ulx"])

   #create node <ULY> as subsode of Geoposition
   node = get_node_element_upon_name(dom, 'Geoposition')
   create_xml_node(dom, node, 'ULY', [], context.xml_info_from_mosaic_dict["uly"])

   #create node <XDIM> as subsode of Geoposition
   node = get_node_element_upon_name(dom, 'Geoposition')
   create_xml_node(dom, node, 'XDIM', [], context.xml_info_from_mosaic_dict["xdim"])

   #create node <YDIM> as subsode of Geoposition
   node = get_node_element_upon_name(dom, 'Geoposition')
   create_xml_node(dom, node, 'YDIM', [], context.xml_info_from_mosaic_dict["ydim"])

   #remove node <Quality_Control_Checks> 
   node = get_node_element_upon_name(dom, 'Quality_Indicators_Info')
   if node :
      remove_xml_node(node, 'Quality_Control_Checks')

   #update node Quality_Indicators_Info
   update_xml_node_atr(dom, node, 'metadataLevel', 'Standard')

   #create node Pixel_Level_QI as subsode of Quality_Indicators_Info
   node = get_node_element_upon_name(dom, 'Quality_Indicators_Info')
   create_xml_node(dom, node, 'Pixel_Level_QI', [], '')

   #create node MASK_FILENAME as subsode of Pixel_Level_QI
   node = get_node_element_upon_name(dom, 'Pixel_Level_QI')
   for item in context.xml_info_from_mosaic_dict["mask_list"]:
      for listElem in item[1]:
         create_xml_node(dom, node, 'MASK_FILENAME', [('type',''), ('geometry',context.xml_info_from_mosaic_dict["resolution"] + "m")], listElem)

   #------------------------------------------------------------------
   #remove node <Auxiliary_Data_Info> - specific Node for L3B product under ROOT node
   nodeEl = get_node_element_upon_name(dom, 'Auxiliary_Data_Info')
   if nodeEl :
      remove_xml_node(nodeEl.parentNode, 'Auxiliary_Data_Info')

      #update node <GIPP> - specific Node for L3B product under Aux_Lis/aux node
      node = get_node_element_upon_name(dom, 'GIPP')
      if node :
         update_text_xml_node(node, "NO")
   #------------------------------------------------------------------

   #preaty format the document Nodes by removing whitespaces
   remove_whilespace_nodes(root)

   #write the file
   dom.writexml( open(context.out_metadata_file_name , 'w'),
               indent="",
               addindent="  ",
               newl='\n')

   #discard nodes references
   dom.unlink()
#------------------------------------------------------------------
def create_mosaic_quicklook(context):

   #set output name for quicklook file of the LEGACY_DATA folder : create the name
   #starting from mosaic file name of the product; strip TIF extension and replace by JPG
   compFileName = (os.path.basename(context.post_process_out_filename).split('.'))[0] + ".jpg"
   #replace MTD by PVI
   fileOut = compFileName.replace("_DAT_","_PVI_")

   #compute full path name and save it - this file reside into LEGACY_DATA
   context.quicklook_out_filename = os.path.join(context.prodFolderName, context.destRootFolder, fileOut)

   #call otb application QuickLook  with a max no of bands defined by CONST_NB_BANDS_QUIKLOOL_LIMIT
   #red, green, blue and grey
   channelList = []
   nb_bands_in_mosaic = int(context.xml_info_from_mosaic_dict["nb_mosaic_bands"])
   if nb_bands_in_mosaic == 1 :
      ##there is one channel
      channelName = "Channel1"
      #add entry to the list
      channelList.append(channelName)
   elif nb_bands_in_mosaic == 2 :
      ## use only the first channel since JPEG driver doesn't support 2 bands.  Must be 1 (grey), 3 (RGB) or 4 bands.
      channelName = "Channel1"
      #add entry to the list
      channelList.append(channelName)
   else:
      for bandId  in range(context.xml_info_from_mosaic_dict["nb_mosaic_bands"]):
         channelName = "Channel" + str(bandId + 1)
         #check the number of bands of the mosaic file: quicklook has a limit over 4 bands
         if bandId < int(CONST_NB_BANDS_QUIKLOOL_LIMIT):
            #add entry to the list
            channelList.append(channelName)
   #call otb application doing quicklook with the apropriate channel list
   quicklook_mosaic(context.post_process_out_filename, context.quicklook_out_filename, channelList)

   #remove rezidual files generated by otb application QuickLook (aux)
   list_file_paths=glob.glob(os.path.join(context.prodFolderName, context.destRootFolder,'*.aux*'))
   for file in list_file_paths:
        os.remove(file)

###################################################################################################
#################################  MAIN ###########################################################
###################################################################################################
#create context
startTime = datetime.datetime.now()
print("--------->START: Context creation")
context = parse_arguments()

#create folder tree inside LEGACY_DATA folder
print("--------->Folders creation")
crete_dirs_tree(context, "IMG_DATA")
crete_dirs_tree(context, "QI_DATA")

#reproject and rescale all images to desired rescaleFactor
print("--------->Reproject and Resize")
perform_reprojection_and_rescale(context)

#build mosaic files from TILES/*/IMG_DATA and TILES/*/QI_DATA into corresponding LEGACY_DATA folder
print("--------->Mosaic files -- Aggregation")
perform_tiles_aggreagtion(context)

#call post process function for folder LEGACY_DATA/IMG_DATA
#to concatenate produced mosaic files having biosferical suffix into a single final aggragate image
print("--------->Post Process Mosaic files")
post_process_mosaic_images(context,os.path.join(context.prodFolderName, context.destRootFolder, "IMG_DATA"))

#call metadata create file in order to generate the XML
print("--------->XML metadata creation")
create_xml_mosaic_metadata(context)

#call generate quicklook file in order to generate preview of the mosaic file
print("--------->QUICKLOOK generation")
create_mosaic_quicklook(context)
print("--------->END...")

#execution time
execTimeSec = datetime.datetime.now() - startTime
hours, remainder = divmod(execTimeSec.seconds, 3600)
minutes, seconds = divmod(remainder, 60)
print("Script Execution took ({}H-{}M-{}S)".format(hours, minutes, seconds))
print("###################################################################################################")

