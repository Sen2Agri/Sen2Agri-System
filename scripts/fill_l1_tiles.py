#!/usr/bin/env python
import argparse
import glob
import os.path
import psycopg2
from psycopg2.sql import SQL, Placeholder
import psycopg2.extras
import re
import traceback

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser


class Config(object):
    def __init__(self, args):
        parser = ConfigParser()
        parser.read([args.config_file])

        self.host = parser.get("Database", "HostName")
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")

        self.site_id = args.site_id


def get_sentinel_tiles(product_path):
    dirname = os.path.basename(product_path)

    print(product_path)
    s2_re = re.compile(r"^S2\w_\w{6}_\w{15}_\w{5}_\w{4}_T(\w{5})", re.I)
    s2_tile_re = re.compile(r"^S2\w_\w+_\w+_\w+_\w+_\w+__\w{15}_\w+_T(\w{5})_", re.I)

    m = s2_re.search(dirname)
    if m:
        tile_id = m.group(1)
        if len(glob.glob(os.path.join(product_path, "GRANULE", "*_T{}_*".format(tile_id), "IMG_DATA", "*"))) == 0:
            return []
        else:
            return [tile_id]

    tiles = []
    tile_path = os.path.join(product_path, "GRANULE", "*")
    for e in glob.glob(tile_path):
        m = s2_tile_re.match(os.path.basename(e))
        if m:
            tile_id = m.group(1)
            if len(glob.glob(os.path.join(e, "IMG_DATA", "*"))) > 0:
                tiles.append(tile_id)
    return tiles


def get_landsat_tiles(product_path):
    dirname = os.path.basename(product_path)

    l8_re = re.compile(r"^LC8(\d{6})|^LC08_\w{4}_(\d{6})_", re.I)
    m = l8_re.search(dirname)
    if m:
        tile_id = m.group(1) or m.group(2)

        if len(glob.glob(os.path.join(product_path, "*{}*.*".format(tile_id)))) == 0:
            return []
        else:
            return [tile_id]

    return []


def get_updates(config, conn):
    update_list = []
    with conn.cursor() as cursor:
        query = SQL(
            """
            select id,
                   satellite_id,
                   full_path
            from downloader_history
            where tiles is null
              and status_id in (2, 5, 6, 7)
              {}
            """
        )
        if config.site_id is not None:
            query = query.format(SQL("and site_id = {}").format(Placeholder()))
            cursor.execute(query, (config.site_id, ))
        else:
            query = query.format(SQL(""))
            cursor.execute(query)

        products = cursor.fetchall()
        conn.commit()

    for (id, satellite_id, full_path) in products:
        try:
            if satellite_id == 1:
                tiles = get_sentinel_tiles(full_path)
            elif satellite_id == 2:
                tiles = get_landsat_tiles(full_path)
            else:
                print("Unknown satellite id {} for product {}".format(satellite_id, full_path))
                continue

            if len(tiles) > 0:
                update_list.append((id, tiles))
            elif len(tiles) == 0:
                print("No tiles found for product {}".format(full_path))

                update_list.append((id, []))
            else:
                print("Unable to recognize product {}".format(full_path))
        except Exception:
            print("While checking product {}".format(full_path))
            traceback.print_exc()

    return update_list


def main():
    parser = argparse.ArgumentParser(description="Fills in the tile list for the downloaded products")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="site ID to filter by")
    parser.add_argument('-p', '--pretend', action='store_true', help="don't apply any changes to the database")
    args = parser.parse_args()

    config = Config(args)

    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        update_list = get_updates(config, conn)

        with conn.cursor() as cursor:
            q = "update downloader_history set tiles = %s where id = %s"
            args = [(tiles, id) for (id, tiles) in update_list]
            psycopg2.extras.execute_batch(cursor, q, args)

        conn.commit()


if __name__ == "__main__":
    main()
