#!/usr/bin/env python
from __future__ import print_function

import argparse
import multiprocessing.dummy
import os
import os.path
from osgeo import osr
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


def get_footprint(raster):
    georef = raster.GetGeoTransform()
    (xmin, ymax) = georef[0], georef[3]
    (xcell, ycell) = georef[1], georef[5]
    (rows, cols) = raster.RasterYSize, raster.RasterXSize
    xmax = xmin + xcell * cols
    ymin = ymax + ycell * rows

    footprint = "(({}, {}), ({}, {}))".format(xmin, ymin, xmax, ymax)

    return footprint


def process(id, full_path, use_temp):
    old_path = full_path
    output = os.path.splitext(full_path)[0] + ".tif"

    if use_temp:
        fd, temp = tempfile.mkstemp(".tif")
        os.close(fd)
    else:
        temp = output

    print("autocrop-raster.py", full_path, temp)
    ret = subprocess.call(["autocrop-raster.py", full_path, temp])
    if ret == 2:
        return (id, full_path + "-INVALID", None, old_path)
    elif ret != 0:
        print("autocrop-raster.py returned", ret)
        return None
    elif not os.path.exists(temp):
        print("autocrop-raster.py didn't create output file", ret)
        return None

    raster = gdal.Open(temp, gdal.GA_Update)
    raster.SetProjection(osr.SRS_WKT_WGS84)
    footprint = get_footprint(raster)
    del raster

    if use_temp:
        print("cp", "-f", temp, output)
        ret = subprocess.call(["cp", "-f", temp, output])
        if ret != 0:
            print("cp returned", ret)
            return None
        os.remove(temp)

    return (id, output, footprint, old_path)


def remove_ncs(config, conn, use_temp):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select full_path
            from product
            where full_path like '%%.tif'
              and satellite_id = 3
            {}
            order by id
            """
        )

        if config.site_id is not None:
            site_filter = SQL("and site_id = {}").format(Literal(config.site_id))
        else:
            site_filter = SQL("")

        query = query.format(site_filter)
        print(query.as_string(conn))
        cursor.execute(query)

        products = cursor.fetchall()
        for product in products:
            full_path = product[0]

            raster = gdal.Open(full_path, gdal.GA_Update)
            ok = False
            if raster is not None:
                if raster.GetProjection() == '':
                    print("should set projection on", full_path)
                    if use_temp:
                        fd, temp = tempfile.mkstemp(".tif")
                        os.close(fd)
                        print("cp", "-f", full_path, temp)
                        ret = subprocess.call(["cp", "-f", full_path, temp])
                        if ret != 0:
                            print("cp returned", ret)
                            continue
                        raster = gdal.Open(temp, gdal.GA_Update)
                    raster.SetProjection(osr.SRS_WKT_WGS84)
                    del raster
                    if use_temp:
                        print("rm", "-f", full_path)
                        ret = subprocess.call(["rm", "-f", full_path])
                        if ret != 0:
                            print("rm returned", ret)
                            continue
                        print("cp", "-f", temp, full_path)
                        ret = subprocess.call(["cp", "-f", temp, full_path])
                        if ret != 0:
                            print("cp returned", ret)
                            continue
                        print("rm", "-f", temp)
                        ret = subprocess.call(["rm", "-f", temp])
                        if ret != 0:
                            print("rm returned", ret)
                            continue
                ok = True
            else:
                print("corrupted product", full_path)

            if ok:
                full_path = os.path.splitext(full_path)[0] + ".nc"
                print("removing processed nc", full_path)

                try:
                    os.remove(full_path)
                except OSError as e:
                    print(e.strerror)

        query = SQL(
            """
            select id, full_path
            from product
            where full_path like '%%.nc-INVALID'
              and satellite_id = 3
            {}
            order by id
            """
        )

        if config.site_id is not None:
            site_filter = SQL("and site_id = {}").format(Literal(config.site_id))
        else:
            site_filter = SQL("")

        query = query.format(site_filter)
        print(query.as_string(conn))
        cursor.execute(query)

        products = cursor.fetchall()
        products_to_remove = []
        for product in products:
            id = product[0]
            full_path = product[1].replace("-INVALID", "")
            try:
                print("removing invalid product", full_path)
                products_to_remove.append((id,))
                os.remove(full_path)
            except OSError as e:
                print(e.strerror)

        print("products to remove:", products_to_remove)
        q = "delete from product where id = %s"
        psycopg2.extras.execute_batch(cursor, q, products_to_remove)

        cursor.execute("delete from product where satellite_id = 3 and full_path = 'INVALID'")

        conn.commit()


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
    parser.add_argument('--fix', action='store_true', help="clean up existing products")
    args = parser.parse_args()

    config = Config(args)
    pool = multiprocessing.dummy.Pool(8)

    with psycopg2.connect(host=config.host, dbname=config.dbname, user=config.user, password=config.password) as conn:
        if args.fix:
            remove_ncs(config, conn, args.use_temp)

        last_id = None
        while True:
            res = get_updates(config, conn, pool, args.use_temp, last_id)
            if res is None:
                break
            (update_list, last_id) = res

            with conn.cursor() as cursor:
                for (id, full_path, footprint, _) in update_list:
                    print(id, full_path, footprint)
                q = "update product set full_path = %s, footprint = %s where id = %s"
                query_args = [(full_path, footprint, id) for (id, full_path, footprint, _) in update_list if full_path is not None]
                psycopg2.extras.execute_batch(cursor, q, query_args)

            conn.commit()

            invalid_products = [id for (id, full_path, _, _) in update_list if full_path is None]
            with conn.cursor() as cursor:
                for id in invalid_products:
                    print("removing invalid product", id, full_path)

                q = "delete from product where id = %s"
                query_args = [id for (id, _) in invalid_products]
                psycopg2.extras.execute_batch(cursor, q, query_args)

            conn.commit()

            for (_, _, _, old_path) in update_list:
                try:
                    print("removing processed product", old_path)
                    os.remove(old_path)
                except OSError as e:
                    print("unable to remove {}: {}".format(old_path, e))


if __name__ == "__main__":
    main()
