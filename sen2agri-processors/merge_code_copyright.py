#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:      Sen2Agri-Processors
   Language:     Python
   Copyright:    Year(s), Author (Company), e-mail
   See COPYRIGHT file for details.
   
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
_____________________________________________________________________________

"""


from __future__ import print_function
import glob
import argparse
import os
import fnmatch
from os.path import isfile, isdir, join
from sen2agri_common_db import *

parser = argparse.ArgumentParser(
    description="Merges two files")
parser.add_argument('-p', '--path', default="", help="path where to look for files with provided extension")
parser.add_argument('-e', '--extension', default="", help="extension of files too look for")
parser.add_argument('-l', '--line', default="", help="line from files after which the content of the provided file will be added")
parser.add_argument('-f', '--file', help="content of this file will be added at the beginning of the files with provided extension from the provided path")

args = parser.parse_args()

if not os.path.isfile(args.file):
    print("The provided file {} does not exist".format(args.file))
    sys.exit(-1)

if not os.path.isdir(args.path):
    print("The provided path {} does not exist".format(args.path))
    sys.exit(-1)

#for filename in glob.iglob("{}/*.{}".format(args.path, args.extension), recursive=True):
for root, dirnames, filenames in os.walk(args.path):
    for filename in fnmatch.filter(filenames, '*.{}'.format(args.extension)):
        filename = os.path.join(root, filename)
        print(filename)
        run_command(["rm", "/tmp/merge_tmpfile.tmp".format(args.extension)], "/tmp/", "merge.log")
        
        if int(args.line) == 0:
            cmd = "cat {} {} > /tmp/merge_tmpfile.tmp".format(pipes.quote(args.file), pipes.quote(filename) )
        else:
            sed_arg = "'{}r {}'".format(args.line, args.file)
            cmd = "sed {} < {} > /tmp/merge_tmpfile.tmp".format(sed_arg, filename)
        print(cmd)

        if(subprocess.call(cmd, shell=True)):
            print("Error cat for {}".format(filename))
            continue
        if run_command(["mv", "/tmp/merge_tmpfile.tmp".format(args.extension), filename], "/tmp/", "merge.log") != 0:
            print("Error mv for {}".format(filename))
            continue
        if(args.extension == 'py'):
            run_command(["chmod", "a+x", filename], "/tmp/", "merge.log")

