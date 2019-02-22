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
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")

        self.site_id = args.site_id
        self.tiles = args.tiles

        self.season_start = parse_date(args.season_start)
        self.season_end = parse_date(args.season_end)


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


def main():
    parser = argparse.ArgumentParser(description="Crop type processor wrapper")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="site ID to filter by")
    parser.add_argument('--season-start', help="season start date")
    parser.add_argument('--season-end', help="season end date")
    parser.add_argument('--out-path', default='.', help="output path")
    parser.add_argument('--working-path', default='.', help="working path")
    parser.add_argument('--tiles', default=None, nargs="+", help="tile filter")
    parser.add_argument('--training-ratio', type=float, help="training/total samples ratio", default=0.5)
    parser.add_argument('--num-trees', type=int, help="number of RF trees", default=300)
    parser.add_argument('--sample-size', type=float, help="sample size", default=0.2)
    parser.add_argument('--count-threshold', type=int, help="count threshold", default=1000)
    parser.add_argument('--count-min', type=int, help="minimum count", default=10)
    parser.add_argument('--smote-target', type=int, help="target sample count for SMOTE", default=1000)
    parser.add_argument('--smote-k', type=int, help="number of SMOTE neighbours", default=5)

    args = parser.parse_args()

    config = Config(args)

    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        lpis_path = get_lpis_path(conn, config.site_id, config.season_end)
        print("Using LPIS from {}".format(lpis_path))

    current_path = os.getcwd()
    os.chdir(args.working_path)
    try:
        os.mkdir("optical")
    except OSError:
        pass
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

    os.chdir("optical")
    command = []
    command += ["crop-type-parcels.py"]
    command += ["-m", "optical"]
    command += ["-s", config.site_id]
    command += ["--season-start", format_date(config.season_start)]
    command += ["--season-end", format_date(config.season_end)]
    command += ["--lpis-path", lpis_path]
    command += ["--tiles"] + config.tiles

    run_command(command)

    os.chdir("../sar")
    command = []
    command += ["crop-type-parcels.py"]
    command += ["-m", "sar"]
    command += ["-s", config.site_id]
    command += ["--season-start", format_date(config.season_start)]
    command += ["--season-end", format_date(config.season_end)]
    command += ["--lpis-path", lpis_path]
    if config.tiles:
        command += ["--tiles"] + config.tiles

    run_command(command)

    os.chdir("..")
    command = []
    command += ["merge-sar.py"]
    command += ["sar", "sar-merged"]

    run_command(command)

    os.rename("optical/optical-features.csv", "features/optical-features.csv")
    os.rename("sar-merged/sar-features.csv", "features/sar-features.csv")
    os.rename("sar-merged/sar-temporal.csv", "features/sar-temporal.csv")

    os.chdir(current_path)

    command = []
    command += ["crop_type.R"]
    command += [args.out_path + "/"]
    command += [os.path.join(args.working_path, "features/sar-features.csv")]
    command += [os.path.join(args.working_path, "features/optical-features.csv")]
    command += [os.path.join(args.working_path, "features/sar-temporal.csv")]
    command += [os.path.join(lpis_path, "parcels.csv")]
    command += [args.training_ratio]
    command += ["Smote"]
    command += [args.num_trees]
    command += [args.sample_size]
    command += [args.count_threshold]
    command += [args.count_min]
    command += [args.smote_target]
    command += [args.smote_k]

    run_command(command)


if __name__ == "__main__":
    main()
