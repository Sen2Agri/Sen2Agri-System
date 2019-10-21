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


class Config(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.config_file])

        self.host = parser.get("Database", "HostName")
        self.port = int(parser.get("Database", "Port", vars={"Port": "5432"}))
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")
        
        self.ogr2ogr_path = args.ogr2ogr_path


def get_product_info(conn, product_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select product_type_id, full_path
            from product
            where id = {}
            """
        )
        query = query.format(Literal(product_id))
        print(query.as_string(conn))

        cursor.execute(query)
        row = cursor.fetchone()
        conn.commit()
        return row


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


def get_import_table_command(ogr2ogr_path, destination, source, *options):
    command = []
    command += [ogr2ogr_path]
    command += options
    command += [destination, source]
    return command


def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)


def drop_table(conn, name):
    with conn.cursor() as cursor:
        query = SQL(
            """
            drop table if exists {}
            """
        ).format(Identifier(name))
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()


def import_crop_type(config, conn, pg_path, product_id, path):
    path = os.path.join(path, "VECTOR_DATA", "Predict_classif_*.csv")
    for file in glob(path):
        table_name = "pd_ct_staging_{}".format(product_id)

        drop_table(conn, table_name)

        command = get_import_table_command(config.ogr2ogr_path, pg_path, file, "-nln", table_name, "-gt", 100000, "-lco", "UNLOGGED=YES", "-oo", "AUTODETECT_TYPE=YES")
        run_command(command)

        with conn.cursor() as cursor:
            query = SQL(
                """
                    insert into product_details_l4a(
                        product_id,
                        "NewID",
                        "CT_decl",
                        "CT_pred_1",
                        "CT_conf_1",
                        "CT_pred_2",
                        "CT_conf_2"
                    )
                    select
                        {},
                        newid,
                        ct_decl,
                        ct_pred_1,
                        ct_conf_1,
                        ct_pred_2,
                        ct_conf_2
                    from {}
                    """
            ).format(Literal(product_id), Identifier(table_name))
            print(query.as_string(conn))
            cursor.execute(query)
            conn.commit()

        drop_table(conn, table_name)


def import_agricultural_practices(config, conn, pg_path, product_id, path):
    path = os.path.join(path, "VECTOR_DATA", "*.csv")
    for file in glob(path):
        practice = os.path.basename(file).split("_")[2]
        practice_id = get_practice(practice)

        if not practice_id:
            print("Unknown practice {}".format(practice))
            sys.exit(1)

        table_name = "pd_ap_staging_{}_{}".format(product_id, practice.lower())

        drop_table(conn, table_name)

        command = get_import_table_command(config.ogr2ogr_path, pg_path, file, "-nln", table_name, "-gt", 100000, "-lco", "UNLOGGED=YES")
        run_command(command)

        with conn.cursor() as cursor:
            query = SQL(
                """
                    insert into product_details_l4c(
                        product_id,
                        "NewID",
                        practice_id,
                        orig_id,
                        country,
                        year,
                        main_crop,
                        veg_start,
                        h_start,
                        h_end,
                        practice,
                        p_type,
                        p_start,
                        p_end,
                        l_week,
                        m1,
                        m2,
                        m3,
                        m4,
                        m5,
                        h_week,
                        h_w_start,
                        h_w_end,
                        h_w_s1,
                        m6,
                        m7,
                        m8,
                        m9,
                        m10,
                        c_index,
                        s1_pix,
                        s1_gaps,
                        h_s1_gaps,
                        p_s1_gaps
                    )
                    select
                        {},
                        field_id :: int,
                        {},
                        orig_id,
                        country,
                        year :: int,
                        main_crop,
                        veg_start,
                        h_start,
                        h_end,
                        practice,
                        p_type,
                        p_start,
                        p_end,
                        l_week,
                        m1,
                        m2,
                        m3,
                        m4,
                        m5,
                        h_week,
                        h_w_start,
                        h_w_end,
                        h_w_s1,
                        m6,
                        m7,
                        m8,
                        m9,
                        m10,
                        c_index,
                        s1pix,
                        s1gaps,
                        h_s1gaps,
                        p_s1gaps
                    from {}
                    """
            ).format(Literal(product_id), Literal(practice_id), Identifier(table_name))
            print(query.as_string(conn))
            cursor.execute(query)
            conn.commit()

        drop_table(conn, table_name)


def main():
    parser = argparse.ArgumentParser(description="Imports product contents into the database")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-p', '--product-id', type=int, help="product id")
    parser.add_argument('-g', '--ogr2ogr-path', default='ogr2ogr', help="The path to ogr2ogr")

    args = parser.parse_args()

    config = Config(args)

    pg_path = 'PG:dbname={} host={} port={} user={} password={}'.format(config.dbname, config.host,
                                                                        config.port, config.user, config.password)

    with psycopg2.connect(host=config.host, port=config.port, dbname=config.dbname, user=config.user, password=config.password) as conn:
        r = get_product_info(conn, args.product_id)
        if r is None:
            print("Invalid product id {}".format(args.product_id))
            return 1
        (product_type, path) = r

        if product_type == PRODUCT_TYPE_CROP_TYPE:
            import_crop_type(config, conn, pg_path, args.product_id, path)
        elif product_type == PRODUCT_TYPE_AGRICULTURAL_PRACTICES:
            import_agricultural_practices(config, conn, pg_path, args.product_id, path)
        else:
            print("Unknown product type {}".format(product_type))
            return 1


if __name__ == "__main__":
    sys.exit(main())
