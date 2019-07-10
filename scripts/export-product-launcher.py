#!/usr/bin/env python
from __future__ import print_function

import argparse
import csv
from collections import defaultdict
from datetime import date
from glob import glob
import multiprocessing.dummy
import os
import os.path
import sys

def get_practice(name):
    if name == "NA":
        return PRACTICE_NA
    elif name == "CatchCrop":
        return PRACTICE_CATCH_CROP
    elif name == "NFC":
        return PRACTICE_NFC
    elif name == "Fallow":
        return PRACTICE_FALLOW
    else:
        return None


def main():
    parser = argparse.ArgumentParser(description="Imports product contents into the database")
    parser.add_argument('-f', '--product-ids-file', help="product ids file")
    parser.add_argument('-o', '--out', default="", help="Out file name")

    args = parser.parse_args()
    
    file=open( args.product_ids_file, "r")
    reader = csv.reader(file, delimiter=";")
    for line in reader:
        if len(line) == 2 :
            prdId = line[0]
            prdPath = line[1]
        
        importCmd = "/usr/bin/import-product-details.py -p " + prdId
        print("Executing import cmd: {}".format(importCmd))
        os.system(importCmd)

        vectDataDirPath = os.path.join(prdPath, "VECTOR_DATA")        
        vectDataPath = os.path.join(vectDataDirPath, args.out)
        exportCmd = "/usr/bin/export-product.py -p " + prdId + " " + vectDataPath
        print("Executing export cmd: {}".format(exportCmd))
        os.system(exportCmd)

        print ("Checking files in {}".format(vectDataDirPath))        
        for f in glob(os.path.join(vectDataDirPath,'*.gpkg')):  
            gpkgFileName = os.path.splitext(os.path.basename(f))[0]
            ogr2ogrCmd = "ogr2ogr " + os.path.join(vectDataDirPath, gpkgFileName + '.shp') + " " + os.path.join(vectDataDirPath, gpkgFileName + '.gpkg') + " -lco ENCODING=UTF-8"
            print ("Executing ogr2ogr command : {}".format(ogr2ogrCmd))
            os.system(ogr2ogrCmd)
                
if __name__ == "__main__":
    sys.exit(main())
