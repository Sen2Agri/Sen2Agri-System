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
from osgeo import osr
from osgeo import ogr
import pipes
import psycopg2
from psycopg2.sql import SQL, Literal, Identifier
import psycopg2.extras
import subprocess
import sys


try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser


PRODUCT_TYPE_CROP_TYPE = 4
PRODUCT_TYPE_AGRICULTURAL_PRACTICES = 6

PRACTICE_NA = 1
PRACTICE_CATCH_CROP = 2
PRACTICE_NFC = 3
PRACTICE_FALLOW = 4


def get_practice_name(practice_id):
    if practice_id == PRACTICE_NA:
        return "NA"
    elif practice_id == PRACTICE_CATCH_CROP:
        return "CatchCrop"
    elif practice_id == PRACTICE_NFC:
        return "NFC"
    elif practice_id == PRACTICE_FALLOW:
        return "Fallow"
    else:
        return None


class Config(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.config_file])

        self.host = parser.get("Database", "HostName")
        self.port = int(parser.get("Database", "Port", vars={"Port": "5432"}))
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")


def get_product_info(conn, product_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select
                product_type_id,
                created_timestamp,
                short_name
            from product
            inner join site on site.id = product.site_id
            where product.id = {}
            """
        )
        query = query.format(Literal(product_id))
        print(query.as_string(conn))

        cursor.execute(query)
        row = cursor.fetchone()
        conn.commit()
        return row


def get_export_table_command(destination, source, *options):
    command = []
    command += ["ogr2ogr"]
    command += options
    command += [destination, source]
    return command


def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)


def export_crop_type(conn, pg_path, product_id, lpis_table, path):
    query = SQL(
        """
            select
                lpis.*,
                ct."CT_decl",
                ct."CT_pred_1",
                ct."CT_conf_1",
                ct."CT_pred_2",
                ct."CT_conf_2"
            from {} lpis
            left outer join product_details_l4a ct on (ct."NewID", ct.product_id) = (lpis."NewID", {})
            """
    ).format(Identifier(lpis_table), Literal(product_id))
    query = query.as_string(conn)

    name = os.path.splitext(os.path.basename(path))[0]
    command = get_export_table_command(path, pg_path, "-nln", name, "-sql", query, "-gt", 100000)
    run_command(command)


def export_agricultural_practices(conn, pg_path, product_id, lpis_table, path):
    practices = []
    with conn.cursor() as cursor:
        query = SQL(
            """
            select distinct practice_id
            from product_details_l4c
            where product_id = {}
            """
        )
        query = query.format(Literal(product_id))
        print(query.as_string(conn))

        cursor.execute(query)
        for row in cursor:
            practices.append(row[0])
        conn.commit()

    for practice_id in practices:
        query = SQL(
            """
                select
                    lpis.*,
                    orig_id as "ORIG_ID",
                    country as "COUNTRY",
                    year as "YEAR",
                    main_crop as "MAIN_CROP",
                    veg_start as "VEG_START",
                    h_start as "H_START",
                    h_end as "H_END",
                    practice as "PRACTICE",
                    p_type as "P_TYPE",
                    p_start as "P_START",
                    p_end as "P_END",
                    l_week as "L_WEEK",
                    m1 as "M1",
                    m2 as "M2",
                    m3 as "M3",
                    m4 as "M4",
                    m5 as "M5",
                    h_week as "H_WEEK",
                    h_w_start as "H_W_START",
                    h_w_end as "H_W_END",
                    h_w_s1 as "H_W_S1",
                    m6 as "M6",
                    m7 as "M7",
                    m8 as "M8",
                    m9 as "M9",
                    m10 as "M10",
                    c_index as "C_INDEX",
                    s1_pix as "S1PIX",
                    s1_gaps as "S1GAPS",
                    h_s1_gaps as "H_S1GAPS",
                    p_s1_gaps as "P_S1GAPS"
                from product_details_l4c ap
                inner join {} lpis on lpis."NewID" = ap."NewID"
                where (ap.product_id, ap.practice_id) = ({}, {})
                """
        ).format(Identifier(lpis_table), Literal(product_id), Literal(practice_id))
        query = query.as_string(conn)

        practice_name = get_practice_name(practice_id)
        if not practice_name:
            print("Unknown practice id {}".format(practice_id))
            sys.exit(1)

        (dir, name) = (os.path.dirname(path), os.path.basename(path))
        name = name.replace("PRACTICE", practice_name)

        file = os.path.join(dir, name)
        table_name = os.path.splitext(name)[0].lower()
        command = get_export_table_command(file, pg_path, "-nln", table_name, "-sql", query, "-gt", 100000)
        run_command(command)


def main():
    parser = argparse.ArgumentParser(description="Exports product contents from the database")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-p', '--product-id', type=int, help="product id")
    parser.add_argument('output', help="output path")

    args = parser.parse_args()

    config = Config(args)

    pg_path = 'PG:dbname={} host={} port={} user={} password={}'.format(config.dbname, config.host,
                                                                        config.port, config.user, config.password)

    with psycopg2.connect(host=config.host, port=config.port, dbname=config.dbname, user=config.user, password=config.password) as conn:
        r = get_product_info(conn, args.product_id)
        if r is None:
            print("Invalid product id {}".format(args.product_id))
            return 1

        (product_type, created_timestamp, site_short_name) = r
        lpis_table = "decl_{}_{}".format(site_short_name, created_timestamp.year)

        if product_type == PRODUCT_TYPE_CROP_TYPE:
            export_crop_type(conn, pg_path, args.product_id, lpis_table, args.output)
        elif product_type == PRODUCT_TYPE_AGRICULTURAL_PRACTICES:
            export_agricultural_practices(conn, pg_path, args.product_id, lpis_table, args.output)
        else:
            print("Unknown product type {}".format(product_type))
            return 1


if __name__ == "__main__":
    sys.exit(main())
