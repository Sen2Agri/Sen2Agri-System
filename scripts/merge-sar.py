#!/usr/bin/env python
from __future__ import print_function

import argparse
from collections import defaultdict, OrderedDict
import csv
from glob import glob
import multiprocessing
import multiprocessing.dummy
import os.path
import pipes
import re
import subprocess
import tempfile


def run_command(args, env=None, retry=False):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))

    retries = 5 if retry else 1
    while retries > 0:
        retries -= 1
        print(cmd_line)
        result = subprocess.call(args, env=env)
        if result != 0:
            print("Exit code: {}".format(result))
        else:
            break


PERIOD_TYPE_WEEK = 1
PERIOD_TYPE_MONTH = 2
PERIOD_TYPE_SEASON = 3

ORBIT_TYPE_ASC = 1
ORBIT_TYPE_DESC = 2
ORBIT_TYPE_NONE = 3

POLARIZATION_VV = 1
POLARIZATION_VH = 2
POLARIZATION_RATIO = 3

TYPE_BACKSCATTER = 1
TYPE_COHERENCE = 2


class TileGroup(object):
    def __init__(
        self, prefix, period_type, period, orbit_type, polarization, type, name
    ):
        self.prefix = prefix
        self.period_type = period_type
        self.period = period
        self.orbit_type = orbit_type
        self.polarization = polarization
        self.type = type
        self.name = name

    def erase_period(self):
        return TileGroup(
            self.prefix,
            self.period_type,
            0,
            self.orbit_type,
            self.polarization,
            self.type,
            self.name,
        )

    def format(self, tile_id, include_prefix=True):
        s = ""
        if include_prefix:
            s += self.prefix + "_"
        if self.period_type == PERIOD_TYPE_WEEK:
            s += "W"
        elif self.period_type == PERIOD_TYPE_MONTH:
            s += "M"
        elif self.period_type == PERIOD_TYPE_SEASON:
            s += "S"
        if self.period != 0:
            s += str(self.period)
        s += "_"

        if tile_id is not None:
            s += tile_id + "_"

        if self.orbit_type == ORBIT_TYPE_ASC:
            s += "ASC_"
        elif self.orbit_type == ORBIT_TYPE_DESC:
            s += "DESC_"

        if self.polarization == POLARIZATION_VV:
            s += "VV_"
        elif self.polarization == POLARIZATION_VH:
            s += "VH_"
        elif self.polarization == POLARIZATION_RATIO:
            s += "RATIO_"

        if self.type == TYPE_BACKSCATTER:
            s += "BCK"
        elif self.type == TYPE_COHERENCE:
            s += "COHE"

        return s

    def __lt__(self, other):
        return self.__key() < other.__key()

    def __le__(self, other):
        return self.__key() <= other.__key()

    def __eq__(self, other):
        return self.__key() == other.__key()

    def __ne__(self, other):
        return self.__key() != other.__key()

    def __ge__(self, other):
        return self.__key() >= other.__key()

    def __gt__(self, other):
        return self.__key() > other.__key()

    def __hash__(self):
        return hash(self.__key())

    def __str__(self):
        return self.name

    def __repr__(self):
        return self.name

    def __key(self):
        return (
            self.prefix,
            self.period_type,
            self.period,
            self.orbit_type != ORBIT_TYPE_NONE,
            self.orbit_type,
            self.polarization,
            self.type,
        )


def get_period_type(s):
    if s == "W":
        return PERIOD_TYPE_WEEK
    elif s == "M":
        return PERIOD_TYPE_MONTH
    elif s == "S":
        return PERIOD_TYPE_SEASON


def get_period(s):
    if s is not None:
        return int(s)
    else:
        return 0


def get_orbit_type(s):
    if s == "ASC":
        return ORBIT_TYPE_ASC
    elif s == "DESC":
        return ORBIT_TYPE_DESC
    elif s is None:
        return ORBIT_TYPE_NONE


def get_polarization(s):
    if s == "VV":
        return POLARIZATION_VV
    elif s == "VH":
        return POLARIZATION_VH
    elif s == "RATIO":
        return POLARIZATION_RATIO


def get_type(s):
    if s == "BCK":
        return TYPE_BACKSCATTER
    elif s == "COHE":
        return TYPE_COHERENCE


def paste_files(file1, file2, out):
    fd, temp = tempfile.mkstemp(".csv")
    os.close(fd)

    command = []
    command += ["sh"]
    command += ["-c", "cut -d, -f2- {} > {}".format(file2, temp)]
    run_command(command)

    command = []
    command += ["sh"]
    command += ["-c", "paste -d, {} {} >> {}".format(file1, temp, out)]
    run_command(command)

    os.remove(temp)


def split_file(file, pos, out1, out2):
    command = []
    command += ["sh"]
    command += ["-c", "cut -d, -f1-{} {} > {}".format(pos + 1, file, out1)]
    run_command(command)

    command = []
    command += ["sh"]
    command += ["-c", "cut -d, -f1,{}- {} > {}".format(pos + 2, file, out2)]
    run_command(command)


def main():
    parser = argparse.ArgumentParser(description="Combine SAR statistics")
    parser.add_argument("input_path", default=".", help="input statistics path")
    parser.add_argument("output_path", default=".", help="output statistics path")
    args = parser.parse_args()

    regex = re.compile(
        r"(SEN4CAP_L2A_\w+_S\d+)_([MWS])(\d+)?_T([A-Z0-9]+)_(?:(\w+)?_)?(\w+)_(\w+)_\w+.csv"
    )
    all_groups = set()
    tile_groups = defaultdict(list)
    for file in glob(os.path.join(args.input_path, "*_MEAN.csv")):
        name = os.path.basename(file)
        m = regex.match(name)
        if m:
            name = name.replace("_MEAN.csv", "")
            prefix = m.group(1)
            period_type = get_period_type(m.group(2))
            period = get_period(m.group(3))
            tile_id = m.group(4)
            orbit_type = get_orbit_type(m.group(5))
            polarization = get_polarization(m.group(6))
            type = get_type(m.group(7))

            group = TileGroup(
                prefix, period_type, period, orbit_type, polarization, type, name
            )
            tile_groups[tile_id].append(group)
            all_groups.add(group)
        else:
            print("unknown file: {}".format(file))

    reverse_groups = defaultdict(set)
    for tile_id, groups in tile_groups.items():
        for group in groups:
            reverse_groups[tile_id].add(group)

    excluded_tiles = []
    selected_groups = all_groups.copy()
    for tile_id, groups in reverse_groups.items():
        if tile_id not in excluded_tiles:
            print(tile_id, len(groups))
            selected_groups &= groups

    sar_features = os.path.join(args.output_path, "sar-features.csv")
    sar_temporal = os.path.join(args.output_path, "sar-temporal.csv")

    simple_features_columns = []
    temporal_features_columns = []
    for group in sorted(list(selected_groups)):
        name = group.format(None, include_prefix=False)
        if group.period_type == PERIOD_TYPE_MONTH:
            temporal_features_columns.append(name + "_MEAN")
            if group.type == TYPE_BACKSCATTER:
                temporal_features_columns.append(name + "_CVAR")
            else:
                temporal_features_columns.append(name + "_MIN")
        elif group.period_type == PERIOD_TYPE_SEASON:
            temporal_features_columns.append(name)
        elif group.period_type == PERIOD_TYPE_WEEK:
            simple_features_columns.append(name)

    simple_features_count = len(simple_features_columns)

    all_columns = ["NewID"]
    for col in simple_features_columns:
        all_columns.append("XX_" + col + "_MEAN")
    for col in simple_features_columns:
        all_columns.append("XX_" + col + "_DEV")

    with open(sar_features, "wb") as out_file:
        writer = csv.writer(out_file, quoting=csv.QUOTE_MINIMAL)
        writer.writerow(all_columns)

    all_columns = ["NewID"]
    for col in temporal_features_columns:
        all_columns.append("XX_" + col + "_MEAN")
    for col in temporal_features_columns:
        all_columns.append("XX_" + col + "_DEV")

    with open(sar_temporal, "wb") as out_file:
        writer = csv.writer(out_file, quoting=csv.QUOTE_MINIMAL)
        writer.writerow(all_columns)

    pool = multiprocessing.dummy.Pool(multiprocessing.cpu_count())

    print(len(selected_groups))

    commands = []
    merge_commands = []
    tile_statistics = []
    for tile_id, groups in tile_groups.items():
        if tile_id not in excluded_tiles:
            selected = []
            for group in groups:
                if group in selected_groups:
                    selected.append(group)
            selected.sort()

            print(tile_id)
            period_type_groups = OrderedDict()

            for group in selected:
                period_type_groups.setdefault(group.erase_period(), []).append(group)

            pat = "{}_{}.csv"
            merged = []
            for key, groups in period_type_groups.items():
                name = key.format(tile_id)
                print(name)

                out_mean = pat.format(name, "MEAN")
                out_dev = pat.format(name, "DEV")
                out_count = pat.format(name, "COUNT")

                out_mean = os.path.join(args.output_path, out_mean)
                out_dev = os.path.join(args.output_path, out_dev)
                out_count = os.path.join(args.output_path, out_count)

                files = []
                for group in groups:
                    mean = group.name + "_MEAN.csv"
                    dev = group.name + "_DEV.csv"
                    count = group.name + "_COUNT.csv"

                    mean = os.path.join(args.input_path, mean)
                    dev = os.path.join(args.input_path, dev)
                    count = os.path.join(args.input_path, count)

                    files += [mean, dev, count]

                command = []
                command += ["gapfill-statistics"]
                command += [out_mean, out_dev, out_count]
                command += files

                commands.append(command)

                key.name = name
                merged.append(key)
            pool.map(run_command, commands)

            if len(merged) == 0:
                continue

            name = "{}_{}".format(merged[0].prefix, tile_id)
            out_mean = pat.format(name, "MEAN")
            out_dev = pat.format(name, "DEV")
            out_count = pat.format(name, "COUNT")

            out_mean = os.path.join(args.output_path, out_mean)
            out_dev = os.path.join(args.output_path, out_dev)
            out_count = os.path.join(args.output_path, out_count)
            print(name)

            files = []
            for group in merged:
                mean = group.name + "_MEAN.csv"
                dev = group.name + "_DEV.csv"
                count = group.name + "_COUNT.csv"

                mean = os.path.join(args.output_path, mean)
                dev = os.path.join(args.output_path, dev)
                count = os.path.join(args.output_path, count)

                files += [mean, dev, count]

            tile_statistics += [out_mean, out_dev, out_count]

            command = []
            command += ["cat-columns"]
            command += [out_mean, out_dev, out_count]
            command += files

            merge_commands.append(command)
    pool.map(run_command, merge_commands)

    mean_sar = os.path.join(args.output_path, "mean-sar.csv")
    dev_sar = os.path.join(args.output_path, "dev-sar.csv")

    mean_simple_sar = os.path.join(args.output_path, "mean-simple-sar.csv")
    dev_simple_sar = os.path.join(args.output_path, "dev-simple-sar.csv")
    mean_temporal_sar = os.path.join(args.output_path, "mean-temporal-sar.csv")
    dev_temporal_sar = os.path.join(args.output_path, "dev-temporal-sar.csv")

    command = []
    command += ["merge-statistics"]
    command += [mean_sar, dev_sar]
    command += tile_statistics
    run_command(command)

    split_file(mean_sar, simple_features_count, mean_simple_sar, mean_temporal_sar)
    split_file(dev_sar, simple_features_count, dev_simple_sar, dev_temporal_sar)

    paste_files(mean_simple_sar, dev_simple_sar, sar_features)
    paste_files(mean_temporal_sar, dev_temporal_sar, sar_temporal)


if __name__ == "__main__":
    main()
