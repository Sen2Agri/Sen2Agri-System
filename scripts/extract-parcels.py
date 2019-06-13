#!/usr/bin/env python
from __future__ import print_function

import argparse
import csv
from datetime import date
import os.path
from osgeo import gdal, ogr
import psycopg2
from psycopg2.sql import SQL, Literal, Identifier
import psycopg2.extras
import sys
try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser


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


def get_site_name(conn, site_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select short_name
            from site
            where id = {}
            """
        )
        site = Literal(site_id)
        query = query.format(site)
        print(query.as_string(conn))

        cursor.execute(query)
        rows = cursor.fetchall()
        conn.commit()
        return rows[0][0]


def save_to_csv(cursor, path):
    with open(path, 'wb') as csvfile:
        writer = csv.writer(csvfile, quoting=csv.QUOTE_MINIMAL)

        writer.writerow(['NewID', 'Area_meters', 'S1Pix', 'S2Pix', 'CTnumL4A', 'LC'])
        for row in cursor:
            writer.writerow(row)


def extract_parcels(config, args, lpis_table, lut_table, id, geom):
    with psycopg2.connect(host=config.host, port=config.port, dbname=config.dbname, user=config.user, password=config.password) as conn:
        q1 = """
            select lpis."NewID",
                lpis."Area_meters",
                lpis."S1Pix",
                lpis."S2Pix",
                lut."ctnuml4a" as "CTnumL4A",
                lpis."LC"
            from {} lpis
            inner join {} lut on lut.ctnum = lpis."CTnum"
            where "GeomValid"
            and not "Duplic"
            and not "Overlap"
            order by "NewID"
            """
        q2 = """
            with stratum as (
                select ST_Transform(ST_GeomFromText({}, {}), Find_SRID('public', {}, 'wkb_geometry')) as geom
            )
            select lpis."NewID",
                lpis."Area_meters",
                lpis."S1Pix",
                lpis."S2Pix",
                lut."ctnuml4a" as "CTnumL4A",
                lpis."LC"
            from stratum
            inner join {} lpis on lpis.wkb_geometry && stratum.geom and ST_Relate(lpis.wkb_geometry, stratum.geom, '2********')
            inner join {} lut on lut.ctnum = lpis."CTnum"
            where "GeomValid"
            and not "Duplic"
            and not "Overlap"
            order by "NewID"
            """

        with conn.cursor() as cursor:
            if args.strata is None:
                output = args.output
                query = SQL(q1).format(Identifier(lpis_table), Identifier(lut_table))
                print(query.as_string(conn))

                cursor.execute(query)
                save_to_csv(cursor, output)
                conn.commit()
            else:
                dirname = os.path.dirname(args.output)
                basename = os.path.basename(args.output)
                split = os.path.splitext(basename)

                query = SQL(q2).format(Literal(geom.ExportToWkt()), Literal(args.srid), Literal(lpis_table), Identifier(lpis_table), Identifier(lut_table))

                print(query.as_string(conn))

                cursor.execute(query)
                output = os.path.join(dirname, "{}-{}{}".format(split[0], id, split[1]))
                save_to_csv(cursor, output)
                conn.commit()


def main():
    parser = argparse.ArgumentParser(description="Crops and recompresses S1 L2A products")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="site ID to filter by")
    parser.add_argument('-y', '--year', help="year")
    parser.add_argument('--strata', help="strata definition")
    parser.add_argument('--srid', help="strata SRID")
    parser.add_argument('output', help="output file", default="parcels.csv")

    args = parser.parse_args()

    config = Config(args)

    if args.strata is not None:
        if args.srid is None:
            print("--srid is required with --strata")
            sys.exit(1)

    with psycopg2.connect(host=config.host, port=config.port, dbname=config.dbname, user=config.user, password=config.password) as conn:
        site_name = get_site_name(conn, config.site_id)
        year = args.year or date.today().year
        lpis_table = "decl_{}_{}".format(site_name, year)
        lut_table = "lut_{}_{}".format(site_name, year)

    if args.strata is None:
        extract_parcels(config, args, lpis_table, lut_table, None, None)
    else:
        ds = ogr.Open(args.strata, gdal.gdalconst.GA_ReadOnly)
        layer = ds.GetLayer()
        for feature in layer:
            id = feature.GetField("id")
            geom = feature.GetGeometryRef()
            if id == 2:
                extract_parcels(config, args, lpis_table, lut_table, id, geom)


if __name__ == "__main__":
    main()
