#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:      Sen2Agri-Processors
   Language:     Python
   Copyright:    2015-2020, CS Romania, office@c-s.ro
   See COPYRIGHT file for details.

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
_____________________________________________________________________________

"""
from __future__ import print_function
import argparse
import glob
import re
import os
from os.path import isfile, isdir, join
from os import mkdir
import sys
import time, datetime
import pipes
import shutil
from multiprocessing import Pool
from threading import Thread
import threading
import tempfile
import subprocess
from distutils.dir_util import copy_tree
import lxml
from lxml import etree
import hashlib
from sen2agri_common_db import *

# default values
DEBUG = False
SEN2COR_LOG_DIR_PATH = "/tmp/"
SEN2COR_LOG_FILE_NAME = "sen2cor.log"
default_dem_path = "/mnt/archive/srtm"
docker_dem_path = "/sen2cor/2.8/dem/srtm/"
# default_lc_snow_cond_path = "/mnt/archive/reference_data/ESACCI-LC-L4-Snow-Cond-500m-P13Y7D-2000-2012-v2.0"
docker_lc_snow_cond_path = "/opt/Sen2Cor-02.08.00-Linux64/lib/python2.7/site-packages/sen2cor/aux_data/ESACCI-LC-L4-Snow-Cond-500m-P13Y7D-2000-2012-v2.0"
# default_lc_lccs_map_path = "/mnt/archive/reference_data/ESACCI-LC-L4-LCCS-Map-300m-P1Y-2015-v2.0.7.tif"
docker_lc_lccs_map_path = "/opt/Sen2Cor-02.08.00-Linux64/lib/python2.7/site-packages/sen2cor/aux_data/ESACCI-LC-L4-LCCS-Map-300m-P1Y-2015-v2.0.7.tif"
# default_lc_wb_map_path = "/mnt/archive/reference_data/ESACCI-LC-L4-WB-Map-150m-P13Y-2000-v4.0.tif"
docker_lc_wb_map_path = "/opt/Sen2Cor-02.08.00-Linux64/lib/python2.7/site-packages/sen2cor/aux_data/ESACCI-LC-L4-WB-Map-150m-P13Y-2000-v4.0.tif"
docker_output_path = "/sen2cor/2.8/out"
docker_wrk_path = "/sen2cor/2.8/wrk"
default_sen2cor_image_name = "sen2cor"
default_gdal_image_name = "docker.io/osgeo/gdal:ubuntu-small-3.1.2"
docker_L2A_path = "/sen2cor/2.8/gipp/L2A_GIPP.xml"
docker_L2A_SC_path = "/sen2cor/2.8/gipp/L2A_CAL_SC_GIPP.xml"
docker_L2A_AC_path = "/sen2cor/2.8/gipp/L2A_CAL_AC_GIPP.xml"
docker_L2A_PB_path = "/sen2cor/2.8/gipp/L2A_PB_GIPP.xml"
docker_tile_path = "/sen2cor/2.8/tile"
docker_datastrip_path = "/sen2cor/2.8/datastrip"
docker_img_database_path = "/sen2cor/2.8/img_database"
docker_res_database_path = "/sen2cor/2.8/res_database"
docker_sen2cor_exec_path = "/opt/Sen2Cor-02.08.00-Linux64/bin/L2A_Process"
docker_if_path = "/tmp/if/"  # input files path
docker_of_path = "/tmp/of/"  # output files path


def CheckInput():
    global default_wrk_dir_path
    global docker_input_path
    global default_sen2cor_image_name
    global default_gdal_image_name
    global default_dem_path

    # input_dir checks
    # remove trailing / or \ from L1C_input_path
    if args.input_dir.endswith("\\") or args.input_dir.endswith("/"):
        L1C_input_path = args.input_dir[:-1]
    else:
        L1C_input_path = args.input_dir

    L1C_file_name = os.path.basename(L1C_input_path)
    if re.match(r"S2[A|B|C|D]_\w*L1C\w*.SAFE", L1C_file_name) is not None:
        if os.path.isdir(L1C_input_path) is False:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid L1C_input_path {}.".format(L1C_input_path),
                SEN2COR_LOG_FILE_NAME,
            )
            return False
    else:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Invalid L1C input file name {}.".format(L1C_file_name),
            SEN2COR_LOG_FILE_NAME,
        )
        return False
    docker_input_path = os.path.join("/sen2cor/2.8/input", L1C_file_name)

    # --output_dir cheks
    if os.path.isdir(args.output_dir) is False:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Invalid output directory {}.".format(args.output_dir),
            SEN2COR_LOG_FILE_NAME,
        )
        return False

    # sen2cor_exec checks
    if args.local_run is True:
        if args.sen2cor_exec:
            if os.path.isfile(args.sen2cor_exec):
                ProcessorName = os.path.basename(args.sen2cor_exec).split(".")
                if ProcessorName[0] == "L2A_Process":
                    pass
                else:
                    log(
                        SEN2COR_LOG_DIR_PATH,
                        "(sen2cor err) Invalid Sen2Cor ProcessorName {}.".format(
                            ProcessorName[0]
                        ),
                        SEN2COR_LOG_FILE_NAME,
                    )
                    return False
            else:
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) Invalid Sen2Cor_exe_path {}.".format(
                        args.Sen2Cor_exe_path
                    ),
                    SEN2COR_LOG_FILE_NAME,
                )
                return False
        else:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) In local_run mode a path to sen2cor_exec must be given.",
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --working_dir cheks
    if args.working_dir:
        if os.path.isdir(args.working_dir) is False:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid working directory {}.".format(args.working_dir),
                SEN2COR_LOG_FILE_NAME,
            )
            return False
    else:
        # default working dir operations
        default_wrk_dir_name = "Sen2Cor_wrk"
        default_wrk_dir_path = os.path.join(
            os.path.dirname(os.path.realpath(__file__)), default_wrk_dir_name
        )
        if os.path.isdir(default_wrk_dir_path) is False:
            create_recursive_dirs(default_wrk_dir_path)

    # --resolution checks
    if args.resolution:
        valid_resolution_values = ["10", "20", "60"]
        for resolution in args.resolution:
            if resolution not in valid_resolution_values:
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) Invalid resolution values {}.".format(resolution),
                    SEN2COR_LOG_FILE_NAME,
                )
                return False

    # --processing_centre checks
    if args.processing_centre:
        if not re.match(r"^[a-zA-Z_]{4}$", args.processing_centre):
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid expression for processing_centre {}.".format(
                    args.processing_centre
                ),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --archiving_centre checks
    if args.archiving_centre:
        if not re.match(r"^[a-zA-Z_]{4}$", args.archiving_centre):
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid expression for processing_centre {}.".format(
                    args.archiving_centre
                ),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --processing_baseline checks
    if args.processing_baseline:
        if not re.match(r"^[0-9]{2}.[0-9]{2}$", args.processing_baseline):
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid expression for processing_baseline {}.".format(
                    args.processing_baseline
                ),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --GIP_L2A checks
    if args.GIP_L2A:
        if os.path.isfile(args.GIP_L2A):
            if os.path.isabs(args.GIP_L2A) == False:
                args.GIP_L2A = os.path.abspath(args.GIP_L2A)
        else:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid GIP_L2A path {}.".format(args.GIP_L2A),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --GIP_L2A_SC checks
    if args.GIP_L2A_SC:
        if os.path.isfile(args.GIP_L2A_SC) and os.path.isabs(args.GIP_L2A_SC):
            pass
        else:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid GIP_L2A_SC path {}.".format(args.GIP_L2A_SC),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --GIP_L2A_AC checks
    if args.GIP_L2A_AC:
        if os.path.isfile(args.GIP_L2A_AC) and os.path.isabs(args.GIP_L2A_AC):
            pass
        else:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid GIP_L2A_AC path {}.".format(args.GIP_L2A_AC),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --GIP_L2A_PB checks
    if args.GIP_L2A_PB:
        if os.path.isfile(args.GIP_L2A_PB) and os.path.isabs(args.GIP_L2A_PB):
            pass
        else:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid GIP_L2A_PB path {}.".format(args.GIP_L2A_PB),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --mode checks
    if args.mode:
        valid_modes = ["generate_datastrip", "process_tile"]
        if args.mode[0] not in valid_modes:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid mode {}.".format(args.mode),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --datastrip checks
    if args.datastrip:
        if os.path.isdir(args.datastrip[0]) == False:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid datastrip path {}.".format(args.datastrip[0]),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --tile checks
    if args.tile:
        if os.path.isdir(args.tile[0]) == False:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid tile path {}.".format(args.tile[0]),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --res_database_dir checks
    if args.res_database_dir:
        if os.path.isdir(args.res_database_dir[0]) == False:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid res_database_dir path {}.".format(
                    args.res_database_dir[0]
                ),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    # --img_database_dir checks
    if args.img_database_dir:
        if os.path.isdir(args.img_database_dir[0]) == False:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Invalid img_database_dir path {}.".format(
                    args.img_database_dir[0]
                ),
                SEN2COR_LOG_FILE_NAME,
            )
            return False

    if args.tif and args.cog:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Invalid output format, either cog or tif have to be chosen, not both of them.",
            SEN2COR_LOG_FILE_NAME,
        )
        return False
    else:
        if args.gdal_img_name and (args.tif or args.cog):
            default_gdal_image_name = args.gdal_img_name

    if args.local_run is False:
        # --default_sen2cor_image_name
        if args.s2c_img_name:
            default_sen2cor_image_name = args.s2c_img_name

        # --dem_path checks
        if args.dem_path:
            if os.path.isdir(args.dem_path):
                default_dem_path = args.dem_path
            else:
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) Invalid dem_path {}.".format(args.dem_path),
                    SEN2COR_LOG_FILE_NAME,
                )
                return False

        # --lc_wb_map_path
        if args.lc_wb_map_path:
            if os.path.isfile(args.lc_wb_map_path):
                pass
            else:
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) Invalid lc_wb_map_path path {}.".format(
                        args.lc_wb_map_path
                    ),
                    SEN2COR_LOG_FILE_NAME,
                )
                return False

        # --lc_wb_map_path
        if args.lc_lccs_map_path:
            if os.path.isfile(args.lc_lccs_map_path):
                pass
            else:
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) Invalid lc_lccs_map_path path {}.".format(
                        args.lc_lccs_map_path
                    ),
                    SEN2COR_LOG_FILE_NAME,
                )
                return False

        # --lc_snow_cond_path
        if args.lc_snow_cond_path:
            if os.path.isdir(args.lc_snow_cond_path):
                pass
            else:
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) Invalid lc_snow_cond_path path {}.".format(
                        args.lc_snow_cond_path
                    ),
                    SEN2COR_LOG_FILE_NAME,
                )
                return False

    return True


def CheckOutput():
    global default_wrk_dir_path
    if args.working_dir:
        wrk_dir = args.working_dir
    else:
        wrk_dir = default_wrk_dir_path

    # determine the pattern of the computed L2A product
    if args.input_dir.endswith("/"):
        L1C_name = os.path.basename(args.input_dir[:-1])
    else:
        L1C_name = os.path.basename(args.input_dir)
    L1C_name_split = L1C_name.split("_")
    L1C_satelite = L1C_name_split[0]
    L1C_aquistition = L1C_name_split[2]
    L1C_orbit = L1C_name_split[4]
    L1C_tile = L1C_name_split[5]
    L2a_product_pattern = "{}_MSIL2A_{}_N*_{}_{}_*T*.SAFE".format(
        L1C_satelite, L1C_aquistition, L1C_orbit, L1C_tile
    )

    # determine if a product with the specified pattern exists
    L2a_product_path = os.path.join(wrk_dir, L2a_product_pattern)
    L2a_products = glob.glob(L2a_product_path)
    if len(L2a_products) == 1:
        L2A_name = os.path.basename(L2a_products[0])
    elif len(L2a_products) > 1:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Multiple L2A products found in the working directory {} for one input l1c product.".format(
                L2a_products
            ),
            SEN2COR_LOG_FILE_NAME,
        )
        return False, None
    else:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Can NOT find any L2A product in the working directory.",
            SEN2COR_LOG_FILE_NAME,
        )
        return False, None

    # check for MTD_MSIL2A.xml
    mtd_path = os.path.join(wrk_dir, L2A_name, "MTD_MSIL2A.xml")
    if os.path.isfile(mtd_path):
        return True, L2A_name
    else:
        return False, None


def CopyOutput(L2A_product_name):
    global default_wrk_dir_path
    if args.working_dir:
        wrk_dir = args.working_dir
    else:
        wrk_dir = default_wrk_dir_path

    # after validation of the output product, copy it to the destination directory
    source_product = os.path.join(wrk_dir, L2A_product_name)
    destination_product = os.path.join(args.output_dir, L2A_product_name)
    return copy_directory(source_product, destination_product)


def RunSen2Cor():
    global default_wrk_dir_path
    global docker_input_path
    global default_sen2cor_image_name

    if args.working_dir:
        wrk_dir = args.working_dir
    else:
        wrk_dir = default_wrk_dir_path

    cmd = []
    if args.local_run == True:
        # local run
        cmd.append(args.sen2cor_exec)
        cmd.append(args.input_dir)
        cmd.append("--output_dir")
        cmd.append(wrk_dir)
        if args.resolution:
            cmd.append("--resolution")
            for resolution in args.resolution:
                cmd.append(resolution)
        if args.processing_centre:
            cmd.append("--processing_centre")
            cmd.append(args.processing_centre)
        if args.archiving_centre:
            cmd.append("--archiving_centre")
            cmd.append(args.archiving_centre)
        if args.processing_baseline:
            cmd.append("--processing_baseline")
            cmd.append(args.processing_baseline)
        if args.mode:
            cmd.append("--mode")
            cmd.append(args.mode)
        if args.raw:
            cmd.append("--raw")
        if args.debug:
            cmd.append("--debug")
        if args.sc_only:
            cmd.append("--sc_only")
        if args.cr_only:
            cmd.append("--cr_only")
        if args.GIP_L2A:
            cmd.append("--GIP_L2A")
            cmd.append(args.GIP_L2A)
        if args.GIP_L2A_SC:
            cmd.append("--GIP_L2A_SC")
            cmd.append(args.GIP_L2A_SC)
        if args.GIP_L2A_AC:
            cmd.append("--GIP_L2A_AC")
            cmd.append(args.GIP_L2A_AC)
        if args.GIP_L2A_PB:
            cmd.append("--GIP_L2A_PB")
            cmd.append(args.GIP_L2A_PB)
        if args.tile:
            cmd.append("--tile")
            cmd.append(args.tile[0])
        if args.datastrip:
            cmd.append("--datastrip")
            cmd.append(args.datastrip[0])
        if args.img_database_dir:
            cmd.append("--img_database_dir")
            cmd.append(args.img_database_dir[0])
        if args.res_database_dir:
            cmd.append("--res_database_dir")
            cmd.append(args.res_database_dir[0])
    else:
        # docker run
        cmd.append("docker")
        cmd.append("run")
        cmd.append("-it")
        cmd.append("--rm")
        cmd.append("-u")
        cmd.append("{}:{}".format(os.getuid(), os.getgid()))
        cmd.append("-v")
        cmd.append("{}:{}".format(os.path.abspath(default_dem_path), docker_dem_path))
        if args.lc_snow_cond_path:
            cmd.append("-v")
            cmd.append(
                "{}:{}".format(
                    os.path.abspath(args.lc_snow_cond_path), docker_lc_snow_cond_path
                )
            )
        if args.lc_lccs_map_path:
            cmd.append("-v")
            cmd.append(
                "{}:{}".format(
                    os.path.abspath(args.lc_lccs_map_path), docker_lc_lccs_map_path
                )
            )
        if args.lc_wb_map_path:
            cmd.append("-v")
            cmd.append(
                "{}:{}".format(
                    os.path.abspath(args.lc_wb_map_path), docker_lc_wb_map_path
                )
            )
        cmd.append("-v")
        cmd.append("{}:{}".format(os.path.abspath(args.input_dir), docker_input_path))
        cmd.append("-v")
        cmd.append("{}:{}".format(os.path.abspath(wrk_dir), docker_output_path))
        if args.GIP_L2A:
            cmd.append("-v")
            cmd.append("{}:{}".format(os.path.abspath(args.GIP_L2A), docker_L2A_path))
        if args.GIP_L2A_AC:
            cmd.append("-v")
            cmd.append(
                "{}:{}".format(os.path.abspath(args.GIP_L2A_AC), docker_L2A_AC_path)
            )
        if args.GIP_L2A_SC:
            cmd.append("-v")
            cmd.append(
                "{}:{}".format(os.path.abspath(args.GIP_L2A_SC), docker_L2A_SC_path)
            )
        if args.GIP_L2A_PB:
            cmd.append("-v")
            cmd.append(
                "{}:{}".format(os.path.abspath(args.GIP_L2A_PB), docker_L2A_PB_path)
            )
        if args.tile:
            cmd.append("-v")
            cmd.append("{}:{}".format(os.path.abspath(args.tile), docker_tile_path))
        if args.datastrip:
            cmd.append("-v")
            cmd.append(
                "{}:{}".format(os.path.abspath(args.datastrip), docker_tile_path)
            )
        if args.img_database_dir:
            cmd.append("-v")
            cmd.append(
                "{}:{}".format(
                    os.path.abspath(args.img_database_dir), docker_img_database_path
                )
            )
        if args.res_database_dir:
            cmd.append("-v")
            cmd.append(
                "{}:{}".format(
                    os.path.abspath(args.res_database_dir), docker_res_database_path
                )
            )
        if args.product_id:
            cmd.append("--name")
            cmd.append("sen2cor_{}".format(args.product_id))
        cmd.append(default_sen2cor_image_name)

        # actual sen2cor commands
        cmd.append(docker_sen2cor_exec_path)
        # input_dir
        cmd.append(os.path.abspath(docker_input_path))
        cmd.append("--work_dir")
        cmd.append(os.path.abspath(docker_wrk_path))
        cmd.append("--output_dir")
        cmd.append(os.path.abspath(docker_output_path))
        if args.resolution:
            cmd.append("--resolution")
            for resolution in args.resolution:
                cmd.append(resolution)
        if args.processing_centre:
            cmd.append("--processing_centre")
            cmd.append(args.processing_centre)
        if args.archiving_centre:
            cmd.append("--archiving_centre")
            cmd.append(args.archiving_centre)
        if args.processing_baseline:
            cmd.append("--processing_baseline")
            cmd.append(args.processing_baseline)
        if args.raw:
            cmd.append("--raw")
        if args.debug:
            cmd.append("--debug")
        if args.sc_only:
            cmd.append("--sc_only")
        if args.cr_only:
            cmd.append("--cr_only")
        if args.mode:
            cmd.append("--mode")
            cmd.append(args.mode)
        if args.GIP_L2A:
            cmd.append("--GIP_L2A")
            cmd.append(docker_L2A_path)
        if args.GIP_L2A_AC:
            cmd.append("--GIP_L2A_AC")
            cmd.append(docker_L2A_AC_path)
        if args.GIP_L2A_SC:
            cmd.append("--GIP_L2A_SC")
            cmd.append(docker_L2A_SC_path)
        if args.GIP_L2A_PB:
            cmd.append("--GIP_L2A_PB")
            cmd.append(docker_L2A_PB_path)
        if args.tile:
            cmd.append("--tile")
            cmd.append(docker_tile_path)
        if args.datastrip:
            cmd.append("--datastrip")
            cmd.append(docker_datastrip_path)
        if args.img_database_dir:
            cmd.append("--img_database_dir")
            cmd.append(docker_img_database_path)
        if args.res_database_dir:
            cmd.append("--res_database_dir")
            cmd.append(docker_res_database_path)

    try:
        if run_command(cmd, SEN2COR_LOG_DIR_PATH, SEN2COR_LOG_FILE_NAME, False) != 0:
            return False
        else:
            return True
    except (KeyboardInterrupt, SystemExit):
        print("(sen2cor err) Keyboard interrupted")
        sys.exit(1)
    except:
        return False


def TranslateToTif(L2A_product_name):
    global default_wrk_dir_path
    global default_gdal_image_name

    if args.working_dir:
        wrk_dir = args.working_dir
    else:
        wrk_dir = default_wrk_dir_path

    # translate jp2 images to tif images
    jp2_files = []
    img_dir_jp2_pattern = "GRANULE/L2A*/IMG_DATA/R*m/*.jp2"
    img_dir_jp2_path = os.path.join(wrk_dir, L2A_product_name, img_dir_jp2_pattern)
    jp2_files.extend(glob.glob(img_dir_jp2_path))
    qi_dir_jp2_pattern = "GRANULE/L2A*/QI_DATA/*.jp2"
    qi_dir_jp2_path = os.path.join(wrk_dir, L2A_product_name, qi_dir_jp2_pattern)
    jp2_files.extend(glob.glob(qi_dir_jp2_path))

    if len(jp2_files) != 0:
        for jp2 in jp2_files:
            if ("_PVI" in os.path.basename(jp2)) or ("_TCI_" in os.path.basename(jp2)):
                # jump to the next jp2 file
                print("Ignoring {}".format(os.path.basename(jp2)))
                continue
            if args.tif:
                print("Translating {} to GTiff".format(os.path.basename(jp2)))
            else:
                print("Translating {} to COG".format(os.path.basename(jp2)))
            cmd = []
            docker_jp2_file_name = os.path.basename(jp2)
            docker_jp2_file_path = os.path.join(docker_if_path, docker_jp2_file_name)
            docker_tif_file_name = os.path.basename(jp2)[:-3] + "tif"
            docker_tif_file_path = os.path.join(docker_of_path, docker_tif_file_name)
            local_jp2_dir = os.path.dirname(jp2)
            cmd.append("docker")
            cmd.append("run")
            cmd.append("-it")
            cmd.append("--rm")
            cmd.append("-u")
            cmd.append("{}:{}".format(os.getuid(), os.getgid()))
            cmd.append("-v")
            cmd.append("{}:{}".format(os.path.abspath(jp2), docker_jp2_file_path))
            cmd.append("-v")
            cmd.append("{}:{}".format(os.path.abspath(local_jp2_dir), docker_of_path))
            if args.product_id:
                cmd.append("--name")
                cmd.append("gdal_{}".format(args.product_id))
            cmd.append(default_gdal_image_name)
            # gdal command
            cmd.append("gdal_translate")
            if args.cog:
                cmd.append("-f")
                cmd.append("COG")
                cmd.append("-co")
                cmd.append("NUM_THREADS=ALL_CPUS")
            if args.tif:
                cmd.append("-f")
                cmd.append("GTiff")
            if (args.cog or args.tif) and args.compressTiffs:
                cmd.append("-co")
                cmd.append("COMPRESS=DEFLATE")
            cmd.append("-co")
            cmd.append("PREDICTOR=2")
            cmd.append(docker_jp2_file_path)
            cmd.append(docker_tif_file_path)

            # run command
            try:
                if (
                    run_command(cmd, SEN2COR_LOG_DIR_PATH, SEN2COR_LOG_FILE_NAME, False)
                    != 0
                ):
                    log(
                        SEN2COR_LOG_DIR_PATH,
                        "(sen2cor err) Can NOT translate {} to .tif".format(jp2),
                        SEN2COR_LOG_FILE_NAME,
                    )
                    return False
                else:
                    os.remove(jp2)
            except (KeyboardInterrupt, SystemExit):
                print("(sen2cor err) Keyboard interrupted")
                sys.exit(1)
            except:
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) Can NOT run the TIFF/COG translation command.",
                    SEN2COR_LOG_FILE_NAME,
                )
                return False

                # remove jp2 file
                try:
                    os.remove(jp2)
                except:
                    log(
                        SEN2COR_LOG_DIR_PATH,
                        "(warning) Can NOT remove file {} from {}.".format(
                            jp2, local_jp2_dir
                        ),
                        SEN2COR_LOG_FILE_NAME,
                    )
    else:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Can NOT find jp2 files in {}".format(wrk_dir),
            SEN2COR_LOG_FILE_NAME,
        )
        return False

    # change manifest.safe file extensions from jp2 to tif
    manifest_pattern = "manifest.safe"
    manifest_file_path = os.path.join(wrk_dir, L2A_product_name, manifest_pattern)
    manifest_files = glob.glob(manifest_file_path)
    if (len(manifest_files) == 1) and (os.path.isfile(manifest_files[0])):
        manifest_file_path = manifest_files[0]
        l2a_folder_name = manifest_file_path.split("/")[-2]
        try:
            tree = etree.parse(manifest_file_path)
            dataobjects = tree.find("dataObjectSection")
            for dataobject in dataobjects:
                file_location_href = dataobject[0][0].attrib["href"]
                if ("_PVI" in file_location_href) or ("_TCI_" in file_location_href):
                    # PVI and _TCI_ files are converted to jpg format not to tif
                    continue
                if file_location_href[-3:] == "jp2":
                    aux = file_location_href[:-3] + "tif"
                    dataobject[0][0].attrib["href"] = aux
                    tif_location_path = os.path.join(wrk_dir, l2a_folder_name, aux)
                    if os.path.isfile(tif_location_path):
                        # update MD5 signature
                        dataobject[0][1].text = hashlib.md5(
                            tif_location_path
                        ).hexdigest()
                        # update size
                        dataobject[0].attrib["size"] = str(
                            os.path.getsize(tif_location_path)
                        )
                    else:
                        log(
                            SEN2COR_LOG_DIR_PATH,
                            "(sen2cor err) Can NOT find the translated tif file in the location specified by manifest.safe {}".format(
                                tif_location_path
                            ),
                            SEN2COR_LOG_FILE_NAME,
                        )
                        return False
        except:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Can NOT parse manifest.safe",
                SEN2COR_LOG_FILE_NAME,
            )
            return False

        # write changes to file
        try:
            f = open(manifest_file_path, "wb")
            f.write(etree.tostring(tree))
            f.close()
        except:
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) Can NOT write the updates to manifest.safe",
                SEN2COR_LOG_FILE_NAME,
            )
            return False
    else:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Can NOT find the manifest.safe file",
            SEN2COR_LOG_FILE_NAME,
        )
        return False

    # chnage MTD_TL.xml file extenstions from jp2 to tif
    mtd_tl_pattern = "GRANULE/L2A*/MTD_TL.xml"
    mtd_tl_path = os.path.join(wrk_dir, L2A_product_name, mtd_tl_pattern)
    mtd_tl_files = glob.glob(mtd_tl_path)
    if len(mtd_tl_files) == 1:
        mtd_tl = mtd_tl_files[0]
    else:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Can NOT identify the correct MTD_TL.xml",
            SEN2COR_LOG_FILE_NAME,
        )
        return False
    try:
        tree = etree.parse(mtd_tl)
        ns = "{https://psd-14.sentinel2.eo.esa.int/PSD/S2_PDI_Level-2A_Tile_Metadata.xsd}"
        quality_indicators_info = tree.find(ns + "Quality_Indicators_Info")
        pixel_level_qi = quality_indicators_info.find("Pixel_Level_QI")
        for mask in pixel_level_qi:
            if mask.text[-3:] == "jp2":
                aux = mask.text[:-3] + "tif"
                mask.text = aux
        pvi_filename = quality_indicators_info.find("PVI_FILENAME")
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor info) Updated the MTD_TL.xml with tif extensions.",
            SEN2COR_LOG_FILE_NAME,
        )
    except:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Can NOT parse MTD_TL.xml.",
            SEN2COR_LOG_FILE_NAME,
        )
        return False

    # write changes to MTD_TL.xml
    try:
        f = open(mtd_tl, "wb")
        f.write(etree.tostring(tree))
        f.close()
    except:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Can NOT write the updates to MTD_TL.xml",
            SEN2COR_LOG_FILE_NAME,
        )
        return False

    log(
        SEN2COR_LOG_DIR_PATH,
        "(sen2cor info) Successful translation from jp2 to tif/cog format.",
        SEN2COR_LOG_FILE_NAME,
    )
    return True


# function used to convert PVI and TCI files from jp2 format to jpg format
# also a rescalling of the converted file is made to allow a faster load
def ConvertPreviews(L2A_product_name):
    if args.working_dir:
        wrk_dir = args.working_dir
    else:
        wrk_dir = default_wrk_dir_path

    jp2_files = []
    tci_jp2_pattern = "GRANULE/L2A*/IMG_DATA/R*m/*_TCI_*.jp2"
    tci_jp2_path = os.path.join(wrk_dir, L2A_product_name, tci_jp2_pattern)
    jp2_files.extend(glob.glob(tci_jp2_path))
    pvi_jp2_pattern = "GRANULE/L2A*/QI_DATA/*_PVI.jp2"
    pvi_jp2_path = os.path.join(wrk_dir, L2A_product_name, pvi_jp2_pattern)
    jp2_files.extend(glob.glob(pvi_jp2_path))

    if len(jp2_files) != 0:
        for jp2 in jp2_files:
            print("Translating {} to jpg".format(os.path.basename(jp2)))
            cmd = []
            docker_jp2_file_name = os.path.basename(jp2)
            docker_jp2_file_path = os.path.join(docker_if_path, docker_jp2_file_name)
            docker_jpeg_file_name = os.path.basename(jp2)[:-3] + "jpg"
            docker_jpeg_file_path = os.path.join(docker_of_path, docker_jpeg_file_name)
            local_jp2_dir = os.path.dirname(jp2)
            cmd.append("docker")
            cmd.append("run")
            cmd.append("-it")
            cmd.append("--rm")
            cmd.append("-u")
            cmd.append("{}:{}".format(os.getuid(), os.getgid()))
            cmd.append("-v")
            cmd.append("{}:{}".format(os.path.abspath(jp2), docker_jp2_file_path))
            cmd.append("-v")
            cmd.append("{}:{}".format(os.path.abspath(local_jp2_dir), docker_of_path))
            if args.product_id:
                cmd.append("--name")
                cmd.append("gdal_{}".format(args.product_id))
            cmd.append(default_gdal_image_name)
            # gdal command
            cmd.append("gdal_translate")
            cmd.append("-of")
            cmd.append("JPEG")
            cmd.append("-outsize")
            cmd.append("1000")
            cmd.append("0")
            cmd.append(docker_jp2_file_path)
            cmd.append(docker_jpeg_file_path)

            # run command
            try:
                if (
                    run_command(cmd, SEN2COR_LOG_DIR_PATH, SEN2COR_LOG_FILE_NAME, False)
                    != 0
                ):
                    log(
                        SEN2COR_LOG_DIR_PATH,
                        "(sen2cor err) Can NOT translate {} to .jpeg".format(jp2),
                        SEN2COR_LOG_FILE_NAME,
                    )
                    return False
                else:
                    try:
                        os.remove(jp2)
                    except:
                        log(
                            SEN2COR_LOG_DIR_PATH,
                            "(sen2cor err) Can NOT remove {} after JPEG translation".format(
                                jp2
                            ),
                            SEN2COR_LOG_FILE_NAME,
                        )
            except (KeyboardInterrupt, SystemExit):
                print("(sen2cor err) Keyboard interrupted")
                sys.exit(1)
            except:
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) Can NOT run the jpeg translation command".format(
                        jp2
                    ),
                    SEN2COR_LOG_FILE_NAME,
                )
                return False
    else:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Can NOT find PVI and TCI files in {}".format(wrk_dir),
            SEN2COR_LOG_FILE_NAME,
        )
        return False

    return True


def InitLog():
    log_file_path = os.path.join(SEN2COR_LOG_DIR_PATH, SEN2COR_LOG_FILE_NAME)
    if os.path.exists(log_file_path) == False:
        create_recursive_dirs(SEN2COR_LOG_DIR_PATH)

        # create the log file
        try:
            log_file = open(log_file_path, "a+")
            log_file.write(
                "{}:[{}]:{}\n".format(
                    str(datetime.datetime.now()),
                    os.getpid(),
                    "### Log file created ###",
                )
            )
            log_file.close()
            print(
                "(sen2cor info) Created a log file {} in dir {}".format(
                    SEN2COR_LOG_FILE_NAME, SEN2COR_LOG_DIR_PATH
                )
            )
        except:
            print(
                "(sen2cor err) Can NOT create file {} in {}.".format(
                    SEN2COR_LOG_FILE_NAME, SEN2COR_LOG_DIR_PATH
                )
            )
            return False
    else:
        if os.path.isfile(log_file_path):
            print(
                "(sen2cor info) Using log file {} in dir {}".format(
                    SEN2COR_LOG_FILE_NAME, SEN2COR_LOG_DIR_PATH
                )
            )
        else:
            print(
                "(sen2cor err) The name of the log file is used by another entity (not a file), aborting"
            )
            return False

    return True


def Cleanup():
    if args.working_dir:
        wrk_dir = args.working_dir
    else:
        wrk_dir = default_wrk_dir_path

    if remove_dir(wrk_dir) == False:
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor warning) Can NOT remove file {}".format(wrk_dir),
        )


# execution of the script
def RunScript():
    try:
        run_script_ok = True

        # initialisation operations
        if InitLog():
            print("(sen2cor info) NOMINAL initialisation of Sen2Cor script.")
        else:
            print("(sen2cor err) NON-NOMINAL initialisation of Sen2Cor script.")
            log(
                SEN2COR_LOG_DIR_PATH,
                "(sen2cor err) NON-NOMINAL initialisation of Sen2Cor script.",
                SEN2COR_LOG_FILE_NAME,
            )
            run_script_ok = False

        if run_script_ok:
            if CheckInput():
                print("(sen2cor info) NOMINAL input.")
            else:
                print("(sen2cor err) NON-NOMINAL input.")
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) NON-NOMINAL input.",
                    SEN2COR_LOG_FILE_NAME,
                )
                run_script_ok = False

        # run Sen2Cor processor
        if run_script_ok:
            if RunSen2Cor():
                print("(sen2cor info) NOMINAL execution of Sen2Cor.")
            else:
                print("(sen2cor err) NON-NOMINAL execution of Sen2Cor.")
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) NON-NOMINAL execution of Sen2Cor.",
                    SEN2COR_LOG_FILE_NAME,
                )
                run_script_ok = False

        # check the output product of Sen2cor processor
        if run_script_ok:
            output_ok, L2A_product_name = CheckOutput()
            if output_ok and L2A_product_name is not None:
                print("(sen2cor info) NOMINAL output of Sen2Cor processor.")
            else:
                print("(sen2cor err) NON-NOMINAL output of Sen2Cor processor.")
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) NON-NOMINAL output of Sen2Cor processor.",
                    SEN2COR_LOG_FILE_NAME,
                )
                run_script_ok = False

        # convert TCI and PVI files to jpeg format
        if run_script_ok:
            if ConvertPreviews(L2A_product_name):
                print(
                    "(sen2cor info) NOMINAL conversion of preview images (TCI, PVI) to jpeg format."
                )
            else:
                print(
                    "(sen2cor err) NON-NOMINAL conversion of preview images (TCI, PVI) to jpeg format."
                )
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) NON-NOMINAL conversion of preview images (TCI, PVI) to jpeg format.",
                    SEN2COR_LOG_FILE_NAME,
                )
                run_script_ok = False

        # translate jp2 images to jpeg format if neccessary
        if run_script_ok:
            if args.tif or args.cog:
                translate_ok = TranslateToTif(L2A_product_name)
            else:
                translate_ok = True

            if translate_ok:
                if args.tif:
                    log(
                        SEN2COR_LOG_DIR_PATH,
                        "(sen2cor info) NOMINAL Tiff translation.",
                        SEN2COR_LOG_FILE_NAME,
                    )
                if args.cog:
                    log(
                        SEN2COR_LOG_DIR_PATH,
                        "(sen2cor info) NOMINAL COG translation.",
                        SEN2COR_LOG_FILE_NAME,
                    )
            else:
                print("(sen2cor err) NON-NOMINAL Tiff/COG translation.")
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) NON-NOMINAL Tiff/COG translation.",
                    SEN2COR_LOG_FILE_NAME,
                )
                run_script_ok = False

        if run_script_ok:
            if CopyOutput(L2A_product_name):
                print(
                    "(sen2cor info) NOMIMAL copying of the product from working directory to output directory."
                )
            else:
                print(
                    "(sen2cor err) NON-NOMIMAL copying of the product from working directory to output directory."
                )
                log(
                    SEN2COR_LOG_DIR_PATH,
                    "(sen2cor err) NON-NOMIMAL copying of the product from working directory to output directory.",
                    SEN2COR_LOG_FILE_NAME,
                )
                run_script_ok = False

        if args.no_clean:
            pass
        else:
            Cleanup()

        return run_script_ok

    except (KeyboardInterrupt, SystemExit):
        print("(sen2cor err) Keyboard interrupted.")
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err) Keyboard interrupted.",
            SEN2COR_LOG_FILE_NAME,
        )
        sys.exit(1)
    except Exception as e:
        print("(sen2cor err): Exception {} encountered".format(e))
        log(
            SEN2COR_LOG_DIR_PATH,
            "(sen2cor err): Exception {} encountered".format(e),
            SEN2COR_LOG_FILE_NAME,
        )
        sys.exit(1)


# script argument operations
parser = argparse.ArgumentParser(
    description="Launches Sen2Cor for L2A product creation"
)
# minimum required flags
parser.add_argument(
    "-i", "--input_dir", required=True, help="Input L1C product directory"
)
parser.add_argument(
    "-o", "--output_dir", required=True, help="Output dir of the L2A product"
)
# flags related to the script execution
parser.add_argument(
    "-e",
    "--sen2cor_exec",
    required=False,
    help="Sen2Cor executable path (only available if -lr is True)",
)
parser.add_argument(
    "-w", "--working_dir", required=False, help="Temporary working directory"
)
parser.add_argument(
    "-nc",
    "--no_clean",
    required=False,
    action="store_true",
    help="Do not clear the L2A product from the working directory.",
)
parser.add_argument(
    "-lr",
    "--local_run",
    required=False,
    action="store_true",
    help="Values can be docker or local.",
)
# docker related flags
parser.add_argument(
    "-si",
    "--s2c_img_name",
    required=False,
    help="Name of the sen2cor docker image (only available when -lr is False).",
)
parser.add_argument(
    "-gi",
    "--gdal_img_name",
    required=False,
    help="Name of the gdal docker image (only available when --tif or --cog is True).",
)
parser.add_argument(
    "-pi",
    "--product_id",
    required=False,
    help="Product id (from downloader history) use to differentiate containers (only available when --lr is False).",
)
# sen2cor processor additional files
parser.add_argument(
    "-d",
    "--dem_path",
    required=False,
    help="Directory path to digital elevation map (only available when -lr is False).",
)
parser.add_argument(
    "-lwb",
    "--lc_wb_map_path",
    required=False,
    help="Path to ESACCI-LC-L4-WB-Map-150m-P13Y-2000-v4.0.tif (only available when -lr is False).",
)
parser.add_argument(
    "-lccs",
    "--lc_lccs_map_path",
    required=False,
    help="Path to ESACCI-LC-L4-LCCS-Map-300m-P1Y-2015-v2.0.7.tif (only available when -lr is False).",
)
parser.add_argument(
    "-lsnow",
    "--lc_snow_cond_path",
    required=False,
    help="Directory path to ESACCI-LC-L4-Snow-Cond-500m-P13Y7D-2000-2012-v2.0 (only available when -lr is False).",
)
# sen2cor processor execution flags
parser.add_argument(
    "--resolution",
    required=False,
    nargs=1,
    help="Target resolution, can be 10, 20 or 60m. If ommited, only 20  and 10m resolutions will be processed.",
)
parser.add_argument(
    "--processing_centre",
    type=str,
    required=False,
    help="Processing centre as regular expression, 4 Characters or '_',like	'SGS_'.",
)
parser.add_argument(
    "--archiving_centre",
    type=str,
    required=False,
    help="Processing centre as regular expression, 4 Characters or '_',like	'SGS_'.",
)
parser.add_argument(
    "--processing_baseline",
    type=str,
    required=False,
    help="Processing baseline in the format 'dd.dd', where d=[0:9], like '08.15'",
)
parser.add_argument(
    "--tif",
    required=False,
    action="store_true",
    help="Export raw images in GeoTiff format instead of JPEG-2000.",
)
parser.add_argument(
    "--cog",
    required=False,
    action="store_true",
    help="Export images in COG format instead of standard GeoTiff.",
)
parser.add_argument(
    "--compressTiffs",
    required=False,
    action="store_true",
    help="Use of COMPRESS=DEFLATE when GeoTiff or COG output format is selected.",
)
parser.add_argument(
    "--raw",
    required=False,
    action="store_true",
    help="Export raw images in rawl format with ENV1 hdr.",
)
parser.add_argument(
    "--debug", required=False, action="store_true", help="Performs in debug mode."
)
parser.add_argument(
    "--sc_only",
    required=False,
    action="store_true",
    help="Performs only	the	scene classification at	60 or 20m resolution.",
)
parser.add_argument(
    "--cr_only",
    required=False,
    action="store_true",
    help="Performs only	the	creation of the L2A product tree, no processing.",
)
parser.add_argument(
    "--GIP_L2A",
    required=False,
    help="Select the user GIPP (a filename with full path).",
)
parser.add_argument(
    "--GIP_L2A_SC",
    required=False,
    help="Select the scene clarification GIPP (a filename with full path).",
)
parser.add_argument(
    "--GIP_L2A_AC",
    required=False,
    help="Select the atmospheric correction GIPP (a filename with full path).",
)
parser.add_argument(
    "--GIP_L2A_PB",
    required=False,
    help="Select the prpcessing baseline GIPP (a filename with full path).",
)
parser.add_argument(
    "--mode",
    required=False,
    type=str,
    nargs=1,
    help="Mode: generate_datastrip, process_tile.",
)
parser.add_argument(
    "--datastrip", required=False, nargs=1, help="Datastrip folder, process_tile."
)
parser.add_argument(
    "--tile", required=False, nargs=1, help="Tile folder, process_tile."
)
parser.add_argument(
    "--img_database_dir", required=False, nargs=1, help="Tile folder, process_tile."
)
parser.add_argument(
    "--res_database_dir", required=False, nargs=1, help="Tile folder, process_tile."
)
args = parser.parse_args()

print("(sen2cor info) Start Sen2Cor script.")
nominal_run = RunScript()
if nominal_run == True:
    print("(sen2cor info) End Sen2Cor script with success.")
    sys.exit(0)
else:
    print("(sen2cor info) End Sen2Cor script with errors.")
    sys.exit(1)
