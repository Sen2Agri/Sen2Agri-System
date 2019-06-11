#!/usr/bin/env python
from __future__ import print_function

import argparse
from datetime import date
import multiprocessing.dummy
from osgeo import osr
import pipes
import psycopg2
from psycopg2.sql import SQL, Literal, Identifier
import psycopg2.extras
import subprocess

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
        self.path = args.path
        self.tiles = args.tiles


def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)


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


def get_site_epsg_codes(conn, site_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select distinct
                   shape_tiles_s2.epsg_code
            from sp_get_site_tiles({} :: smallint, 1 :: smallint) site_tiles
            inner join shape_tiles_s2 on shape_tiles_s2.tile_id = site_tiles.tile_id;
            """
        )

        site = Literal(site_id)

        query = query.format(site)
        print(query.as_string(conn))
        cursor.execute(query)

        rows = cursor.fetchall()
        conn.commit()

        result = []
        for (epsg_code, ) in rows:
            result.append(epsg_code)

        return result


def get_esri_wkt(epsg_code):
    srs = osr.SpatialReference()
    srs.ImportFromEPSG(epsg_code)
    srs.MorphToESRI()
    return srs.ExportToWkt()


def main():
    parser = argparse.ArgumentParser(description="Crops and recompresses S1 L2A products")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="site ID to filter by")
    parser.add_argument('-p', '--path', default='.', help="working path")
    parser.add_argument('-y', '--year', help="year")
    parser.add_argument('--force', help="overwrite field", action='store_true')
    parser.add_argument('--tiles', nargs='+', help="tile filter")

    args = parser.parse_args()

    config = Config(args)
    pool = multiprocessing.dummy.Pool(8)

    pg_path = 'PG:dbname={} host={} port={} user={} password={}'.format(config.dbname, config.host,
                                                                        config.port, config.user, config.password)

    with psycopg2.connect(host=config.host, port=config.port, dbname=config.dbname, user=config.user, password=config.password) as conn:
        site_name = get_site_name(conn, config.site_id)
        year = args.year or date.today().year
        lpis_table = "decl_{}_{}".format(site_name, year)

        commands = []

        gpkg_name = "{}.gpkg".format(lpis_table)
        command = []
        command += ["ogr2ogr"]
        command += [gpkg_name]
        command += [pg_path, lpis_table]
        commands.append(command)

        epsg_codes = get_site_epsg_codes(conn, config.site_id)
        epsg_codes.append(3035)

        for epsg_code in epsg_codes:
            wkt = get_esri_wkt(epsg_code)

            for buf in [5, 10]:
                if buf == 2 and epsg_code == 3035:
                    continue

                output = "{}_{}_buf_{}m.shp".format(lpis_table, epsg_code, buf)
                prj = "{}_{}_buf_{}m.prj".format(lpis_table, epsg_code, buf)

                with open(prj, 'wb') as f:
                    f.write(wkt)

                sql = SQL(
                    """
                    select "NewID", ST_Buffer(ST_Transform(wkb_geometry, {}), {})
                    from {}
                    """)
                sql = sql.format(Literal(epsg_code), Literal(-buf),
                                 Identifier(lpis_table))
                sql = sql.as_string(conn)

                command = []
                command += ["ogr2ogr"]
                command += [output, pg_path]
                command += ["-sql", sql]
                commands.append(command)

        pool.map(lambda c: run_command(c), commands)


if __name__ == "__main__":
    main()
