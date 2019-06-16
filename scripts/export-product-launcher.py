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
    parser.add_argument('-c', '--country', help="the country")
    parser.add_argument('-y', '--year', type=int, help="year")

    args = parser.parse_args()
    
    file=open( args.product_ids_file, "r")
    reader = csv.reader(file)
    for line in reader:
        if len(line) == 2 :
            prdId = line[1]
            prdPath = line[2]
            os.system("import-product-details.py -p ".join(prdId))

            fileName = "Sen4CAP_L4C_PRACTICE_" + country + "_" + year + ".gpkg"
            vectDataDirPath = os.path.join(prdPath, "VECTOR_DATA", fileName)
            os.system("export-product.py -p " + prdId + " " + vectDataDirPath)
                
if __name__ == "__main__":
    sys.exit(main())
