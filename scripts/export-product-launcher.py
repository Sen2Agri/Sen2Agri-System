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
    parser.add_argument('-o', '--out', default="", help="Out file name")

    args = parser.parse_args()
    
    file=open( args.product_ids_file, "r")
    reader = csv.reader(file)
    for line in reader:
        if len(line) == 2 :
            prdId = line[1]
            prdPath = line[2]
            
        os.system("import-product-details.py -p ".join(prdId))
        
        exportCmd = "export-product.py -p " + prdId + " " + os.path.join(prdPath, "VECTOR_DATA", args.out)
        
        os.system(exportCmd)
                
if __name__ == "__main__":
    sys.exit(main())
