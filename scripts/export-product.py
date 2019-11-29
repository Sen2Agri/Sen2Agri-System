#!/usr/bin/env python
from __future__ import print_function

import argparse
import multiprocessing.dummy
import os
import os.path
import shutil
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

        self.ogr2ogr_path = args.ogr2ogr_path
        self.filter = args.filter


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


def get_export_table_command(ogr2ogr_path, destination, source, *options):
    command = []
    command += [ogr2ogr_path]
    command += [destination, source]
    command += options
    return command


def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)


def get_srid(conn, lpis_table):
    with conn.cursor() as cursor:
        query = SQL("select Find_SRID('public', {}, 'wkb_geometry')")
        query = query.format(Literal(lpis_table))
        print(query.as_string(conn))

        cursor.execute(query)
        srid = cursor.fetchone()[0]
        conn.commit()
        return srid


def export_crop_type(
    config,
    conn,
    pg_path,
    pool,
    product_id,
    lpis_table,
    lut_table,
    gpkg_path,
    csv_path,
    lut_path,
):
    commands = []
    srid = get_srid(conn, lpis_table)

    if config.filter:
        filter = "and " + config.filter
    else:
        filter = ""

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
        where not is_deleted {}
        """
    ).format(Identifier(lpis_table), Literal(product_id), SQL(filter))
    query = query.as_string(conn)

    name = os.path.splitext(os.path.basename(gpkg_path))[0]
    command = get_export_table_command(
        config.ogr2ogr_path,
        gpkg_path,
        pg_path,
        "-nln",
        name,
        "-sql",
        query,
        "-a_srs",
        "EPSG:" + str(srid),
        "-gt",
        100000,
    )
    commands.append(command)

    query = SQL(
        """
        select
            lpis."NewID",
            lpis."HoldID",
            lpis."GeomValid",
            lpis."Duplic",
            lpis."Overlap",
            lpis."Area_meters",
            lpis."S2Pix",
            lpis."S1Pix",
            lut.lc as "LC",
            lut.ctnumdiv as "CTnumDIV",
            product_details_l4a."CT_decl",
            product_details_l4a."CT_pred_1",
            product_details_l4a."CT_conf_1",
            product_details_l4a."CT_pred_2",
            product_details_l4a."CT_conf_2"
        from {} lpis
        inner join {} lut using (ori_crop)
        left outer join product_details_l4a on (product_details_l4a."NewID", product_details_l4a.product_id) = (lpis."NewID", {})
        where not is_deleted {}
        """
    ).format(
        Identifier(lpis_table), Identifier(lut_table), Literal(product_id), SQL(filter)
    )
    query = query.as_string(conn)

    command = get_export_table_command(
        config.ogr2ogr_path, csv_path, pg_path, "-sql", query, "-gt", 100000
    )
    commands.append(command)

    command = get_export_table_command(
        config.ogr2ogr_path, lut_path, pg_path, lut_table, "-gt", 100000
    )
    commands.append(command)

    pool.map(run_command, commands)

    return [gpkg_path, csv_path, lut_path]


def export_agricultural_practices(
    config, conn, pg_path, pool, product_id, lpis_table, lut_table, path
):
    commands = []
    srid = get_srid(conn, lpis_table)

    if config.filter:
        filter = "and " + config.filter
    else:
        filter = ""

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

    outputs = []
    for practice_id in practices:
        query = SQL(
            """
            select
                lpis.*,
                lut.lc as "LC",
                lut.ctnum as "CTnum",
                lut.ct as "CT",
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
                s1_pix as "S1PIX_",
                s1_gaps as "S1GAPS",
                h_s1_gaps as "H_S1GAPS",
                p_s1_gaps as "P_S1GAPS",
                h_w_s1_gaps as "H_W_S1_GAPS",
                h_quality as "H_QUALITY",
                c_quality as "C_QUALITY"
            from product_details_l4c ap
            inner join {} lpis on lpis."NewID" = ap."NewID"
            inner join {} lut using (ori_crop)
            where (ap.product_id, ap.practice_id) = ({}, {})
              and not is_deleted {}
            """
        ).format(
            Identifier(lpis_table),
            Identifier(lut_table),
            Literal(product_id),
            Literal(practice_id),
            SQL(filter),
        )
        query = query.as_string(conn)

        practice_name = get_practice_name(practice_id)
        if not practice_name:
            print("Unknown practice id {}".format(practice_id))
            sys.exit(1)

        (dir, name) = (os.path.dirname(path), os.path.basename(path))
        name = name.replace("PRACTICE", practice_name)

        file = os.path.join(dir, name)
        outputs.append(file)
        table_name = os.path.splitext(name)[0].lower()
        command = get_export_table_command(
            config.ogr2ogr_path,
            file,
            pg_path,
            "-nln",
            table_name,
            "-sql",
            query,
            "-a_srs",
            "EPSG:" + str(srid),
            "-gt",
            100000,
        )
        commands.append(command)

    pool.map(run_command, commands)

    return outputs


def main():
    parser = argparse.ArgumentParser(
        description="Exports product contents from the database"
    )
    parser.add_argument(
        "-c",
        "--config-file",
        default="/etc/sen2agri/sen2agri.conf",
        help="configuration file location",
    )
    parser.add_argument("-p", "--product-id", type=int, help="product id")
    parser.add_argument("-w", "--working-path", help="working path")
    parser.add_argument(
        "-g", "--ogr2ogr-path", default="ogr2ogr", help="The path to ogr2ogr"
    )
    parser.add_argument("--filter", help="additional SQL filter")
    parser.add_argument("output", help="output path")

    args = parser.parse_args()

    config = Config(args)

    pg_path = "PG:dbname={} host={} port={} user={} password={}".format(
        config.dbname, config.host, config.port, config.user, config.password
    )

    pool = multiprocessing.dummy.Pool()

    if args.working_path:
        output = os.path.join(args.working_path, os.path.basename(args.output))
    else:
        output = args.output
    with psycopg2.connect(
        host=config.host,
        port=config.port,
        dbname=config.dbname,
        user=config.user,
        password=config.password,
    ) as conn:
        r = get_product_info(conn, args.product_id)
        if r is None:
            print("Invalid product id {}".format(args.product_id))
            return 1

        (product_type, created_timestamp, site_short_name) = r
        lpis_table = "decl_{}_{}".format(site_short_name, created_timestamp.year)
        lut_table = "lut_{}_{}".format(site_short_name, created_timestamp.year)

        if product_type == PRODUCT_TYPE_CROP_TYPE:
            csv_path = os.path.splitext(output)[0] + ".csv"
            lut_path = os.path.splitext(output)[0] + "_LUT.csv"
            outputs = export_crop_type(
                config,
                conn,
                pg_path,
                pool,
                args.product_id,
                lpis_table,
                lut_table,
                output,
                csv_path,
                lut_path,
            )
        elif product_type == PRODUCT_TYPE_AGRICULTURAL_PRACTICES:
            outputs = export_agricultural_practices(
                config,
                conn,
                pg_path,
                pool,
                args.product_id,
                lpis_table,
                lut_table,
                output,
            )
        else:
            print("Unknown product type {}".format(product_type))
            return 1

    if args.working_path:
        dest = os.path.dirname(args.output) or "."
        for f in outputs:
            shutil.move(f, dest)


if __name__ == "__main__":
    sys.exit(main())
