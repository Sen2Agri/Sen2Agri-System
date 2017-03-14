#!/usr/bin/env python
from __future__ import print_function
import argparse
import os
import re
import sys


parser = argparse.ArgumentParser()
parser.add_argument('--s2-tiles', nargs='*')
parser.add_argument('--l8-tiles', nargs='*')
parser.add_argument('input', help="input path")
parser.add_argument('output', help="output path")
args = parser.parse_args()

out_l2a_path = os.path.join(args.output, "l2a")

out_l8_l2a_path = os.path.join(out_l2a_path, "l8")
if not os.path.exists(out_l8_l2a_path):
    os.makedirs(out_l8_l2a_path)

if args.l8_tiles:
    l8_re = re.compile("^LC8" + "|".join(args.l8_tiles))
    for file in os.listdir(args.input):
        if l8_re.search(file):
            source_path = os.path.join(args.input, file)
            destination_path = os.path.join(out_l8_l2a_path, file)
            print("{} -> {}".format(source_path, destination_path))
            if not os.path.exists(destination_path):
                os.symlink(source_path, destination_path)

out_s2_l2a_path = os.path.join(out_l2a_path, "s2")
if not os.path.exists(out_s2_l2a_path):
    os.makedirs(out_s2_l2a_path)

if args.s2_tiles:
    s2_product_re = re.compile("^S2A_OPER.*.SAFE$")
    s2_tile_re = re.compile("^S2A_OPER_SSC_L2VALD_" + "|".join(args.s2_tiles))
    for product in os.listdir(args.input):
        if s2_product_re.search(product):
            product_path = os.path.join(args.input, product)
            for file in os.listdir(product_path):
                if s2_tile_re.search(file):
                    destination_product_path = os.path.join(out_s2_l2a_path, product)
                    if not os.path.exists(destination_product_path):
                        os.makedirs(destination_product_path)

                    source_path = os.path.join(product_path, file)
                    destination_path = os.path.join(destination_product_path, file)
                    print("{} -> {}".format(source_path, destination_path))
                    if not os.path.exists(destination_path):
                        os.symlink(source_path, destination_path)

    s2_product_re = re.compile("S2A_MSIL2A.*T{}".format("|".join(args.s2_tiles)))
    for product in os.listdir(args.input):
        if s2_product_re.search(product):
            source_path = os.path.join(args.input, product)
            destination_path = os.path.join(out_s2_l2a_path, product)
            print("{} -> {}".format(source_path, destination_path))
            if not os.path.exists(destination_path):
                os.symlink(source_path, destination_path)
