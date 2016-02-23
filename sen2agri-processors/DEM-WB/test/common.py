#!/usr/bin/env python
from __future__ import print_function
import argparse
import re
import glob
import gdal
import osr
import subprocess
import lxml.etree
from lxml.builder import E
#import logging
import math
import os
from os.path import isfile, isdir, join
import glob
import sys
import time, datetime
import pipes
import shutil

FAKE_COMMAND = 0

def run_command(cmd_array, use_shell=False):
    start = time.time()
    print(" ".join(map(pipes.quote, cmd_array)))    
    res = 0
    if not FAKE_COMMAND:
        res = subprocess.call(cmd_array, shell=use_shell)
    print("App finished in: {}".format(datetime.timedelta(seconds=(time.time() - start))))
    if res != 0:
        print("Application error")        
    return res


def create_recursive_dirs(dir_name):
    try:
        #create recursive dir
        os.makedirs(dir_name)
    except:
        pass
    #check if it already exists.... otherwise the makedirs function will raise an exception 
    if os.path.exists(dir_name):
        if not os.path.isdir(dir_name):
            print("Can't create the directory because there is a file with the same name: {}".format(dir_name))
            print("Remove: {}".format(dir_name))
            return False
    else:
        #for sure, the problem is with access rights
        print("Can't create the directory due to access rights {}".format(dir_name))
        return False
    return True


class Config(object):
    def __init__(self, section):
        self.section="[{}]".format(section)
        self.host = ""
        self.database = ""
        self.user = ""
        self.password = ""
    def loadConfig(self, configFile):
        try:
            with open(configFile, 'r') as config:
                foundDwnSection = False
                for line in config:
                    line = line.strip(" \n\t\r")
                    if foundDwnSection and line.startswith('['):
                        break
                    elif foundDwnSection:
                        elements = line.split('=')
                        if len(elements) == 2:
                            if elements[0].lower() == "host":
                                self.host = elements[1]
                            elif elements[0].lower() == "database" or elements[0].lower() == "db":
                                self.database = elements[1]
                            elif elements[0].lower() == "user":                            
                                self.user = elements[1]
                            elif elements[0].lower() == "pass" or elements[0].lower() == "password":                            
                                self.password = elements[1]
                            else:
                                print("Unkown key for {} section".format(self.section))
                        else:
                            print("Error in config file, found more than on keys, line: {}".format(line))
                    elif line == self.section:
                        foundDwnSection = True
                        
        except:
            print("Error in opening the config file ".format(str(configFile)))
            return False
        if len(self.host) <= 0 or len(self.database) <= 0:
            return False
        return True
