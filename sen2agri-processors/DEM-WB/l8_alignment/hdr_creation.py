# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:         DEM&WaterBodyModule
 
   Author:          CS SI, (Alexia Mondot alexia.mondot@c-s.fr)
   Copyright:       CS SI
   Licence:         See Licence.txt

   Language:        Python
  _____________________________________________________________________________

  HISTORY

  VERSION: 01.00.00 | DATE: <29/05/2015> | COMMENTS: Creation of file.

  END-HISTORY
  _____________________________________________________________________________

  $Id: $
  _____________________________________________________________________________
  
"""

import argparse
import glob
import os
import re
import sys

import lxml.etree as ET

import config
from DEM_common import display_parameters, searchOneFile
from GDAL_Tools.gdalinfoO import gdalinfoO

# import logging for debug messages
import logging
logging.basicConfig()
# create logger
logger = logging.getLogger('HDR creation')
logger.setLevel(config.level_working_modules)
fh = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s %(name)s %(levelname)s - %(message)s')
fh.setFormatter(formatter)
fh.setLevel(logging.DEBUG)
logger.addHandler(fh)


def get_parameters():

    argParser = argparse.ArgumentParser()
    required_arguments = argParser.add_argument_group('required arguments')
    required_arguments.add_argument('-i', '--input_dir', required=True,
                                    help='Path to input image')
    required_arguments.add_argument('-o', '--output_filename', required=True,
                                    help='Path to output HDR filename')
    required_arguments.add_argument('-s', "--sensor", choices=['s2', 'l8'], required=True)

    args = argParser.parse_args(sys.argv[1:])

    input_dir = os.path.realpath(args.input_dir)
    output_filename = os.path.realpath(args.output_filename)
    sensor = args.sensor

    return input_dir, output_filename, sensor


def getRoot():
    #<Earth_Explorer_Header schema_version="1.00"
    #   xsi:schemaLocation="http://eop-cfi.esa.int/CFI ./AUX_REFDE2_ReferenceDemDataLevel2.xsd"
    #   xmlns="http://eop-cfi.esa.int/CFI" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    #   xsi:type="REFDE2_Header_Type">
    xmlns = "http://eop-cfi.esa.int/CFI"
    xsi = "http://www.w3.org/2001/XMLSchema-instance"
    schemaLocation = "http://eop-cfi.esa.int/CFI ./AUX_REFDE2_ReferenceDemDataLevel2.xsd"
    typeXsi = "REFDE2_Header_Type"
    root = ET.Element("Earth_Explorer_Header", attrib={"schema_version":"1.00",
                                                       "{" + xsi + "}schemaLocation": schemaLocation,
                                                       "{" + xsi + "}type": typeXsi},
                      nsmap={'xsi': xsi, None: xmlns})

    #    xmlns = ""
    # xsi = "http://www.w3.org/2001/XMLSchema-instance"
    # schemaLocation = "Dimapv2_kalideos_product.xsd"
    # root = ET.Element("Dimap_Document", attrib={"{" + xsi + "}schemaLocation": schemaLocation},
    #                   nsmap={'xsi': xsi, None: xmlns})
    #<Dimap_Document
    # xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    # xsi:noNamespaceSchemaLocation="Schemas/Dimapv2_phr_system_rectified_product.xsd">

    return root


def extractSiteNicknameFromMntDirname(mntDirName, sensor):
    idMNT = ""
    if sensor=="l8":
        idMNT = re.search("L8_TEST_AUX_REFDE2_(.+?)_0001", os.path.basename(mntDirName)).group(1)
    elif sensor == "s2":
        idMNT = re.search("S2__TEST_AUX_REFDE2_(.+?___)_5001", os.path.basename(mntDirName)).group(1)

    return idMNT





def hdr_creation(input_dir, output_filename, sensor):
    display_parameters(locals(), "hdr_creation")

    idMnt = extractSiteNicknameFromMntDirname(input_dir, sensor)

    imageMnt=""
    if sensor == "l8":
        count = "7"
        files_order = ["SLP", "ASP", "ALT", "ALC", "MSK", "ASC", "SLC"  ]
        mission = "LANDSAT_8"
        applicableSite = "L8_TEST_MPL_SITDEF_S_"
        imageMNT = searchOneFile(input_dir, "*ALT.TIF")
    elif sensor == "s2":
        count = "10"
        files_order = ["SLP_R1", "SLP_R2", "ASP_R1", "ASP_R2", "ALT_R1", "ALT_R2", "ALC", "MSK", "ASC", "SLC"  ]
        mission = "SENTINEL-2_"
        applicableSite = "S2__VM01_MPL_SITDEF_S_" + idMnt
        imageMNT = searchOneFile(input_dir, "*ALT_R1.TIF")
    info = gdalinfoO(imageMNT)
    basename_out = os.path.basename(os.path.splitext(output_filename)[0])
    logging.debug(basename_out)


    xml_file = os.path.join(os.path.dirname(__file__), "xml_template.xml")
    tree = ET.parse(xml_file)

    # ET.register_namespace("", "http://eop-cfi.esa.int/CFI")
    root = getRoot()
        #tree.getroot()

    # root = ET.Element("Earth_Explorer_Header", schema_version = "1.00", xmlns = "http://eop-cfi.esa.int/CFI", xsi = "http://www.w3.org/2001/XMLSchema-instance", type = "REFDE2_Header_Type", schemaLocation = "http://eop-cfi.esa.int/CFI AUX_REFDE2_ReferenceDemDataLevel2.xsd")

    a1 = ET.SubElement(root, "Fixed_Header")
    toto = ET.SubElement(a1, "File_Name")
    toto.text = basename_out
    toto = ET.SubElement(a1, "File_Description")
    toto.text = "ReferenceDemDataLevel2"
    toto = ET.SubElement(a1, "Notes")
    toto.text = "String"
    toto = ET.SubElement(a1, "Mission")
    toto.text = mission
    toto = ET.SubElement(a1, "File_Class")
    toto.text = "TEST"
    toto = ET.SubElement(a1, "File_Type")
    toto.text = "AUX_REFDE2"
    b1 = ET.SubElement(a1, "Validity_Period")
    toto = ET.SubElement(b1, "Validity_Start")
    toto.text = "UTC=2006-07-01T18:11:45"
    toto = ET.SubElement(b1, "Validity_Stop")
    toto.text = "UTC=9999-99-99T99:99:99"
    toto = ET.SubElement(a1, "File_Version")
    toto.text = "0001"
    b2 = ET.SubElement(a1, "Source")
    toto = ET.SubElement(b2, "System")
    toto.text = "MACCS"
    toto = ET.SubElement(b2, "Creator")
    toto.text = "Earth Explorer CFI"
    toto = ET.SubElement(b2, "Creator_Version")
    toto.text = "1.1"
    toto = ET.SubElement(b2, "Creation_Date")
    toto.text = "UTC=2010-02-10T11:00:00"


    a = ET.SubElement(root, "Variable_Header")
    b2 = ET.SubElement(a, "Main_Product_Header")
    ET.SubElement(b2, "List_of_Consumers", count = "0")
    ET.SubElement(b2, "List_of_Extensions", count = "0")
    b3 = ET.SubElement(a, "Specific_Product_Header")
    b4 = ET.SubElement(b3, "Instance_Id")
    toto = ET.SubElement(b4, "Applicable_Site_Nick_Name")
    toto.text = idMnt

    toto = ET.SubElement(b4, "File_Version")
    toto.text = (os.path.basename(input_dir).split("_")[-1]).split(".")[0]

    b5 = ET.SubElement(b3, "List_of_Applicable_SiteDefinition_Ids", count = "1")
    toto = ET.SubElement(b5, "Applicable_SiteDefinition_Id", sn = "1")
    toto.text = applicableSite

    b6 = ET.SubElement(b3, "DEM_Information")
    b7 = ET.SubElement(b6, "Cartographic")
    b8 = ET.SubElement(b7, "Coordinate_Reference_System")
    toto = ET.SubElement(b8, "Code")
    toto.text = "EPSG:{}".format(info.getIntEpsgCode())
    toto = ET.SubElement(b8, "Short_Description")
    toto.text = ""
    b8 = ET.SubElement(b7, "Upper_Left_Corner")
    toto = ET.SubElement(b8, "X", unit = "m")
    toto.text = str(info.getOrigin()[0])
    toto = ET.SubElement(b8, "Y", unit = "m")
    toto.text = str(info.getOrigin()[0])
    b8 = ET.SubElement(b7, "Sampling_Interval")
    toto = ET.SubElement(b8, "By_Line", unit = "m")
    toto.text = str(info.getPixelSize()[0])
    toto = ET.SubElement(b8, "By_Column", unit = "m")
    toto.text = str(info.getPixelSize()[1])
    b8 = ET.SubElement(b7, "Size")
    toto = ET.SubElement(b8, "Lines")
    toto.text = str(info.getSize()[0])
    toto = ET.SubElement(b8, "Columns")
    toto.text = str(info.getSize()[0])

    toto = ET.SubElement(b6, "Mean_Altitude_Over_L2_Coverage", unit = "m")
    toto.text = "10"
    toto = ET.SubElement(b6, "Altitude_Standard_Deviation_Over_L2_Coverage", unit = "m")
    toto.text = "20"
    toto = ET.SubElement(b6, "Nodata_Value")
    toto.text = "NIL=N/A"
    toto = ET.SubElement(b6, "L2_To_DEM_Subsampling_Ratio")
    toto.text = "1"
    toto = ET.SubElement(b6, "Comment")
    toto.text = ""


    # b = ET.SubElement(a, "Specific_Product_Header")
    c = ET.SubElement(b3, "DBL_Organization")

    d = ET.SubElement(c, "List_of_Packaged_DBL_Files", count = count)

    for image in xrange(len(files_order)):
        paths_to_file = glob.glob(os.path.join(input_dir, "*" + files_order[image] + "*"))
        if paths_to_file:
            path_to_file = paths_to_file[0]
            logging.debug(path_to_file)
            e1 = ET.SubElement(d, "Packaged_DBL_File", sn = str(image + 1))
            ET.SubElement(e1, "Relative_File_Path").text = os.path.join(os.path.basename(os.path.normpath(input_dir)), os.path.basename(path_to_file))
            ET.SubElement(e1, "File_Definition")


    # tree = ET.ElementTree(root)
    # output_file = os.sys.argv[1]

    logging.debug("############################")
    logging.debug(output_filename)

    # tree.write(sys.argv[2], encoding = "utf-8", xml_declaration = True, method = "xml")

    # xmlstr = minidom.parseString(ET.tostring(root)).toprettyxml(indent = "   ")
    # with open(output_filename, "w") as f:
    #     f.write(xmlstr.encode('utf-8'))

    tree = ET.ElementTree(root)

    f = open(output_filename, "w")
    f.write(ET.tostring(tree, pretty_print=True, xml_declaration=True,
                        encoding="UTF-8"))
    f.close()


if __name__ == '__main__':
    input_dir, output_filename, sensor = get_parameters()
    hdr_creation(input_dir, output_filename, sensor)


