#!/usr/bin/env python
from __future__ import print_function

import argparse
import hashlib
import multiprocessing.dummy
import os
import os.path
from osgeo import osr
from osgeo import ogr
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
        self.port = int(parser.get("Database", "Port", vars={"Port": "5432"}))
        self.dbname = parser.get("Database", "DatabaseName")
        self.user = parser.get("Database", "UserName")
        self.password = parser.get("Database", "Password")

        self.site_id = args.site_id


def get_extent(raster):
    gt = raster.GetGeoTransform()
    (cols, rows) = raster.RasterXSize, raster.RasterYSize

    extent = []

    x = gt[0]
    y = gt[3]
    extent.append((x, y))

    x = gt[0] + rows * gt[2]
    y = gt[3] + rows * gt[5]
    extent.append((x, y))

    x = gt[0] + cols * gt[1] + rows * gt[2]
    y = gt[3] + cols * gt[4] + rows * gt[5]
    extent.append((x, y))

    x = gt[0] + cols * gt[1]
    y = gt[3] + cols * gt[4]
    extent.append((x, y))

    return extent


def get_wgs84_extent_as_wkt(raster):
    extent = get_extent(raster)

    ring = ogr.Geometry(ogr.wkbLinearRing)
    ring.AddPoint_2D(extent[0][0], extent[0][1])
    ring.AddPoint_2D(extent[3][0], extent[3][1])
    ring.AddPoint_2D(extent[2][0], extent[2][1])
    ring.AddPoint_2D(extent[1][0], extent[1][1])
    ring.AddPoint_2D(extent[0][0], extent[0][1])

    geom = ogr.Geometry(ogr.wkbPolygon)
    geom.AddGeometry(ring)

    raster_srs = osr.SpatialReference()
    raster_srs.ImportFromWkt(raster.GetProjection())
    wgs84_srs = osr.SpatialReference()
    wgs84_srs.ImportFromEPSG(4326)

    transform = osr.CoordinateTransformation(raster_srs, wgs84_srs)

    geom.Transform(transform)
    return geom.ExportToWkt()


def get_footprint(raster):
    georef = raster.GetGeoTransform()
    (xmin, ymax) = georef[0], georef[3]
    (xcell, ycell) = georef[1], georef[5]
    (rows, cols) = raster.RasterYSize, raster.RasterXSize
    xmax = xmin + xcell * cols
    ymin = ymax + ycell * rows

    footprint = "(({}, {}), ({}, {}))".format(xmin, ymin, xmax, ymax)
    geog = get_wgs84_extent_as_wkt(raster)
    return (footprint, geog)


def md5(file):
    try:
        hasher = hashlib.md5()
        with open(file, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hasher.update(chunk)
        return hasher.hexdigest()
    except Exception as e:
        print(e)
        return None


def process(id, full_path, use_temp):
    old_path = full_path
    output = os.path.splitext(full_path)[0] + ".tif"

    if use_temp:
        fd, temp = tempfile.mkstemp(".tif")
        os.close(fd)
    else:
        temp = output

    try:
        os.remove(temp)
    except Exception:
        pass

    print("autocrop-raster.py", full_path, temp)
    ret = subprocess.call(["autocrop-raster.py", full_path, temp])
    if ret == 2:
        return (id, full_path + "-INVALID", None, None, old_path)
    elif ret != 0:
        print("autocrop-raster.py returned", ret)
        return None
    elif not os.path.exists(temp):
        print("autocrop-raster.py didn't create output file", ret)
        return None

    raster = gdal.Open(temp, gdal.GA_Update)
    raster.SetProjection(osr.SRS_WKT_WGS84)
    (footprint, geog) = get_footprint(raster)
    del raster

    if use_temp:
        print("cp", "-f", temp, output)
        ret = subprocess.call(["cp", "-f", temp, output])
        if ret != 0:
            print("cp returned", ret)
            return None
        os.remove(temp)

    return (id, output, footprint, geog, old_path)


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
        conn.commit()

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
        conn.commit()

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


def fix_footprints(config, conn):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select id,
                   full_path
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

        update_list = []
        products = cursor.fetchall()
        conn.commit()

        for (id, full_path) in products:
            raster = gdal.Open(full_path, gdal.GA_ReadOnly)
            if not raster:
                print("unable to open", full_path)
            else:
                geog = get_wgs84_extent_as_wkt(raster)
                update_list.append((geog, id))
        q = "update product set geog = %s where id = %s"
        psycopg2.extras.execute_batch(cursor, q, update_list)


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


def remove_duplicates(config, conn):
    with conn.cursor() as cursor:
        query = SQL(
            """
                select id,
                    full_path
                from (
                    select id,
                        full_path,
                        row_number() over (partition by md5sum, site_id order by inserted_timestamp desc) as count
                    from product
                    where satellite_id = 3
                      and md5sum is not null
                      {}
                ) as products
                where count > 1;
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
        conn.commit()

        print("removing duplicates")
        query = "delete from product where id = %s"
        for (id, path) in products:
            try:
                print(path)
                os.remove(path)
                cursor.execute(query, (id,))
            except OSError as e:
                print("unable to remove {}: {}".format(path, e))
        conn.commit()


def main():
    parser = argparse.ArgumentParser(description="Crops and recompresses S1 L2A products")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="site ID to filter by")
    parser.add_argument('--use-temp', action='store_true', help="site ID to filter by")
    parser.add_argument('--fix-geog', action='store_true', help="update footprints to match the files")
    args = parser.parse_args()

    config = Config(args)
    pool = multiprocessing.dummy.Pool(8)

    with psycopg2.connect(host=config.host, port=config.port, dbname=config.dbname, user=config.user, password=config.password) as conn:
        if args.fix_geog:
            fix_footprints(config, conn)

        last_id = None
        while True:
            res = get_updates(config, conn, pool, args.use_temp, last_id)
            if res is None:
                break
            (update_list, last_id) = res

            with conn.cursor() as cursor:
                query_args = []
                for (id, full_path, footprint, geog, old_path) in update_list:
                    if full_path is None or full_path.endswith("-INVALID"):
                        continue
                    print(id, full_path)
                    md5sum = md5(full_path)
                    query_args.append((full_path, footprint, geog, md5sum, id))
                q = "update product set full_path = %s, footprint = %s, geog = %s, md5sum = %s where id = %s"
                psycopg2.extras.execute_batch(cursor, q, query_args)

            conn.commit()

            invalid_products = [(id, full_path) for (id, full_path, _, _, _) in update_list if full_path is None or full_path.endswith("-INVALID")]
            with conn.cursor() as cursor:
                for (id, _) in invalid_products:
                    print("removing invalid product", id, full_path)

                q = "delete from product where id = %s"
                query_args = [(id,) for (id, _) in invalid_products]
                psycopg2.extras.execute_batch(cursor, q, query_args)

            conn.commit()

            for (_, _, _, _, old_path) in update_list:
                try:
                    print("not removing processed product", old_path)
                    # os.remove(old_path)
                except OSError as e:
                    print("unable to remove {}: {}".format(old_path, e))

        remove_duplicates(config, conn)


if __name__ == "__main__":
    main()
