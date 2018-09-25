#!/usr/bin/env python
from __future__ import print_function

import argparse
import multiprocessing.dummy
import os
import os.path
from osgeo import gdal
import psycopg2
from psycopg2.sql import SQL, Literal
import psycopg2.extras
import subprocess
import tempfile

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


def get_footprint(src_raster):
    raster = gdal.Open(src_raster)

    georef = raster.GetGeoTransform()
    (xmin, ymax) = georef[0], georef[3]
    (xcell, ycell) = georef[1], georef[5]
    (rows, cols) = raster.RasterYSize, raster.RasterXSize
    xmax = xmin + xcell * cols
    ymin = ymax + ycell * rows

    footprint = "(({}, {}), ({}, {}))".format(xmin, ymin, xmax, ymax)

    return footprint


def process(id, full_path, use_temp):
    output = os.path.splitext(full_path)[0] + ".tif"

    if use_temp:
        fd, temp = tempfile.mkstemp(".tif")
        os.close(fd)
    else:
        temp = output

    print("autocrop-raster.py", full_path, temp)
    ret = subprocess.call(["autocrop-raster.py", full_path, temp])
    if ret == 2:
        return (id, full_path + "-INVALID", None)
        pass
    elif ret != 0:
        print("autocrop-raster.py returned", ret)
        return None
    elif not os.path.exists(temp):
        print("autocrop-raster.py didn't create output file", ret)
        return None

    if use_temp:
        print("cp", "-f", temp, output)
        ret = subprocess.call(["cp", "-f", temp, output])
        if ret != 0:
            print("cp returned", ret)
            return None
        os.remove(temp)

    footprint = get_footprint(output)

    return (id, output, footprint)


def get_updates(config, conn, pool, use_temp, old_last_id):
    print("old_last_id: ", old_last_id)
    update_list = []
    with conn.cursor() as cursor:
        query = SQL(
            """
            select id,
                   full_path
            from product
            where full_path like '%%.nc'
            {}
            {}
            order by id
            limit 128
            """
        )

        if config.site_id is not None:
            site_filter = SQL("and site_id = {}").format(Literal(config.site_id))
        else:
            site_filter = SQL("")

        if old_last_id is not None:
            id_filter = SQL("and id > {}").format(Literal(old_last_id))
        else:
            id_filter = SQL("")

        query = query.format(site_filter, id_filter)
        print(query.as_string(conn))
        cursor.execute(query)

        products = cursor.fetchall()
        conn.commit()

    if len(products) == 0:
        return None

    last_id = max([p[0] for p in products])
    print("last_id: ", last_id)

    update_list = pool.map(lambda p: process(p[0], p[1], use_temp), products)
    update_list = [u for u in update_list if u is not None]

    return (update_list, last_id)


def main():
    parser = argparse.ArgumentParser(description="Crops and recompresses S1 L2A products")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="site ID to filter by")
    parser.add_argument('--use-temp', action='store_true', help="site ID to filter by")
    args = parser.parse_args()

    config = Config(args)
    pool = multiprocessing.dummy.Pool(8)

    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        last_id = None
        while True:
            (update_list, last_id) = get_updates(config, conn, pool, args.use_temp, last_id)
            if update_list is None:
                break

            with conn.cursor() as cursor:
                for (id, full_path, footprint) in update_list:
                    print(id, full_path, footprint)
                q = "update product set full_path = %s, footprint = %s where id = %s"
                query_args = [(full_path, footprint, id) for (id, full_path, footprint) in update_list]
                psycopg2.extras.execute_batch(cursor, q, query_args)

            conn.commit()


if __name__ == "__main__":
    main()
