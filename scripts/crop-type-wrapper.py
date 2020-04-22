#!/usr/bin/env python
from __future__ import print_function

import argparse
from collections import defaultdict
from datetime import date, datetime
from datetime import timedelta
from glob import glob
import multiprocessing
import multiprocessing.dummy
import os
import os.path
from osgeo import ogr
import pipes
import psycopg2
from psycopg2.sql import SQL, Literal
import psycopg2.extras
import subprocess
import tempfile

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser


def parse_date(str):
    return datetime.strptime(str, "%Y-%m-%d").date()


def format_date(dt):
    return dt.strftime("%Y-%m-%d")


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


class Config(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.config_file])

        self.host = parser.get("Database", "HostName")
        self.port = int(parser.get("Database", "Port", vars={"Port": "5432"}))
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")

        self.site_id = args.site_id
        self.tiles = args.tiles
        self.products = args.products

        self.season_start = parse_date(args.season_start)
        self.season_end = parse_date(args.season_end)
        self.year = get_year(self.season_start, self.season_end)


def get_lpis_path(conn, site_id, season_end):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select full_path
            from product
            where
                site_id = {}
            and product_type_id = 14
            and created_timestamp <= {}
            order by created_timestamp desc
            limit 1
            """
        )

        query = query.format(Literal(site_id), Literal(season_end))
        print(query.as_string(conn))
        cursor.execute(query)

        results = cursor.fetchall()
        conn.commit()

        return results[0][0]


def check_file(p):
    if not os.path.exists(p):
        return False
    with open(p) as f:
        if f.readline() and f.readline():
            return True
    return False


def get_year(start, end):
    if start.year == end.year:
        return start.year
    d1 = start.replace(month=12, day=31) - start
    d2 = end - end.replace(month=1, day=1)
    if d2 >= d1:
        return end.year
    else:
        return start.year


def main():
    parser = argparse.ArgumentParser(description="Crop type processor wrapper")
    parser.add_argument(
        "-c",
        "--config-file",
        default="/etc/sen2agri/sen2agri.conf",
        help="configuration file location",
    )
    parser.add_argument("-s", "--site-id", type=int, help="site ID to filter by")
    parser.add_argument(
        "-m",
        "--mode",
        help="processing mode",
        choices=["s1-only", "s2-only", "both"],
        default="both",
    )
    parser.add_argument("--season-start", help="season start date")
    parser.add_argument("--season-end", help="season end date")
    parser.add_argument("--out-path", default=".", help="output path")
    parser.add_argument("--working-path", default=".", help="working path")
    parser.add_argument("--tiles", nargs="+", help="tile filter")
    parser.add_argument("--products", nargs="+", help="product filter")
    parser.add_argument("--lc", help="LC classes to assess", default="1234")
    parser.add_argument(
        "--min-s2-pix", type=int, help="minimum number of S2 pixels", default=3
    )
    parser.add_argument(
        "--min-s1-pix", type=int, help="minimum number of S1 pixels", default=1
    )
    parser.add_argument(
        "--best-s2-pix",
        type=int,
        help="minimum number of S2 pixels for parcels to use in training",
        default=10,
    )
    parser.add_argument(
        "--pa-min", type=int, help="minimum parcels to assess a crop type", default=30
    )
    parser.add_argument(
        "--pa-train-h",
        type=int,
        help="upper threshold for parcel counts by crop type",
        default=4000,
    )
    parser.add_argument(
        "--pa-train-l",
        type=int,
        help="lower threshold for parcel counts by crop type",
        default=1100,
    )
    parser.add_argument(
        "--sample-ratio-h",
        type=float,
        help="training ratio for common crop types",
        default=0.25,
    )
    parser.add_argument(
        "--sample-ratio-l",
        type=float,
        help="training ratio for uncommon crop types",
        default=0.75,
    )
    parser.add_argument(
        "--smote-target", type=int, help="target sample count for SMOTE", default=1000
    )
    parser.add_argument(
        "--smote-k", type=int, help="number of SMOTE neighbours", default=5
    )
    parser.add_argument("--num-trees", type=int, help="number of RF trees", default=300)
    parser.add_argument(
        "--min-node-size", type=int, help="minimum node size", default=10
    )

    args = parser.parse_args()

    config = Config(args)

    with psycopg2.connect(
        host=config.host,
        dbname=config.dbname,
        user=config.user,
        password=config.password,
    ) as conn:
        lpis_path = get_lpis_path(conn, config.site_id, config.season_end)
        print("Using LPIS from {}".format(lpis_path))

    if args.config_file:
        config_file = os.path.realpath(args.config_file)
    else:
        config_file = None

    current_path = os.getcwd()
    os.chdir(args.working_path)
    if args.mode != "s1-only":
        try:
            os.mkdir("optical")
        except OSError:
            pass
    if args.mode != "s2-only":
        try:
            os.mkdir("sar")
        except OSError:
            pass
        try:
            os.mkdir("sar-merged")
        except OSError:
            pass
    try:
        os.mkdir("features")
    except OSError:
        pass

    if args.mode != "s1-only":
        os.chdir("optical")

        command = []
        command += ["crop-type-parcels.py"]
        command += ["-m", "optical"]
        command += ["-s", config.site_id]
        command += ["--season-start", format_date(config.season_start)]
        command += ["--season-end", format_date(config.season_end)]
        command += ["--lpis-path", lpis_path]
        if config.tiles:
            command += ["--tiles"] + config.tiles
        if config.products:
            command += ["--products"] + config.products
        if config_file:
            command += ["-c", config_file]

        run_command(command)
        os.chdir("..")

    if args.mode != "s2-only":
        os.chdir("sar")
        command = []
        command += ["crop-type-parcels.py"]
        command += ["-m", "sar"]
        command += ["-s", config.site_id]
        command += ["--season-start", format_date(config.season_start)]
        command += ["--season-end", format_date(config.season_end)]
        command += ["--lpis-path", lpis_path]
        if config.tiles:
            command += ["--tiles"] + config.tiles
        if config.products:
            command += ["--products"] + config.products
        if config_file:
            command += ["-c", config_file]

        run_command(command)

        os.chdir("..")
        command = []
        command += ["merge-sar.py"]
        command += ["sar", "sar-merged"]

        run_command(command)

    if args.mode != "s1-only":
        os.rename("optical/optical-features.csv", "features/optical-features.csv")
    if args.mode != "s2-only":
        os.rename("sar-merged/sar-features.csv", "features/sar-features.csv")
        os.rename("sar-merged/sar-temporal.csv", "features/sar-temporal.csv")

    os.chdir(current_path)

    parcels = os.path.join(args.working_path, "parcels.csv")
    lut = os.path.join(args.working_path, "lut.csv")

    command = []
    command += ["extract-parcels.py"]
    command += ["-s", config.site_id]
    command += ["-y", config.year]
    command += [parcels, lut]

    run_command(command)

    optical_features = os.path.join(args.working_path, "features/optical-features.csv")
    sar_features = os.path.join(args.working_path, "features/sar-features.csv")
    sar_temporal = os.path.join(args.working_path, "features/sar-temporal.csv")

    if args.mode == "s1-only" or not check_file(optical_features):
        optical_features = "0"
    if args.mode == "s2-only" or not check_file(sar_features):
        sar_features = "0"
    if args.mode == "s2-only" or not check_file(sar_temporal):
        sar_temporal = "0"

    if args.mode == "s2-only":
        args.min_s1_pix = 0
    elif args.mode == "s1-only":
        args.min_s2_pix = 0

    command = []
    command += ["crop_type.R"]
    command += [args.out_path + "/"]
    command += [sar_features]
    command += [optical_features]
    command += [sar_temporal]
    command += [parcels]
    command += ["CTnumL4A"]
    command += [args.lc]
    command += ["Area_meters"]
    command += [args.min_s2_pix]
    command += [args.min_s1_pix]
    command += [args.pa_min]
    command += [args.best_s2_pix]
    command += [args.pa_train_h]
    command += [args.pa_train_l]
    command += [args.sample_ratio_h]
    command += [args.sample_ratio_l]
    command += ["Smote"]
    command += [args.smote_target]
    command += [args.smote_k]
    command += [args.num_trees]
    command += [args.min_node_size]
    command += [lut]

    run_command(command)


if __name__ == "__main__":
    main()
