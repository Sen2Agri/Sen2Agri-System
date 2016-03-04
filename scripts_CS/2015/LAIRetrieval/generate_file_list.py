#!/usr/bin/env python

import os

import datetime
from datetime import timedelta
import argparse
import re

parser = argparse.ArgumentParser(description='Composite tile list generation')

parser.add_argument('--inputdir', help='The path to the input dir', required=True)
parser.add_argument('--instrument', help='The SPOT4  instrument', required=True)
parser.add_argument('--outfile', help="Output file which contain the tile list", required=True)

args = parser.parse_args()

inputDir = args.inputdir
instrument = args.instrument
outFile = args.outfile


if not (os.path.isdir(os.path.dirname(outFile))):
	os.makedirs(os.path.dirname(outFile))

f = open(outFile, 'w')

patternSPOT4="SPOT5_INST_XS_" 
#print instrument
patternSPOT4withINSTR=patternSPOT4.replace("INST", instrument)
#print patternSPOT4withINSTR

listFile = os.listdir(inputDir);
listFile.sort()
  
listDate=[]
  
for file in listFile:
	#print file
	if re.search(patternSPOT4withINSTR,file):
		#print '->', file
		dt = datetime.datetime.strptime((re.search('2015[0-1][1-9][0-3][0-9]',file).group(0)),"%Y%m%d")
		#print dt
		listDate.append(dt)
		f.write(inputDir + file + '/' + file + '.xml\n')

#f.write(listDate[0].strftime("%Y%m%d") + '\n') 
#f.write(listDate[-1].strftime("%Y%m%d")+ '\n')	
f.close()
