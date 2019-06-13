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

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser


PRODUCT_TYPE_LPIS = 14
PROCESSOR_LPIS = 8


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
        self.tiles = args.tiles


class RasterizeDatasetCommand(object):
    def __init__(self, input, output, tile, resolution, sql, field, srs, dst_xmin, dst_ymin, dst_xmax, dst_ymax):
        self.input = input
        self.output = output
        self.tile = tile
        self.resolution = resolution
        self.sql = sql
        self.field = field
        self.srs = srs
        self.dst_xmin = dst_xmin
        self.dst_ymin = dst_ymin
        self.dst_xmax = dst_xmax
        self.dst_ymax = dst_ymax

    def run(self):
        command = []
        command += ["gdal_rasterize", "-q"]
        command += ["-a", self.field]
        command += ["-a_srs", self.srs]
        command += ["-te", self.dst_xmin, self.dst_ymin, self.dst_xmax, self.dst_ymax]
        command += ["-tr", self.resolution, self.resolution]
        command += ["-sql", self.sql]
        command += ["-ot", "Int32"]
        command += ["-co", "COMPRESS=DEFLATE"]
        command += ["-co", "PREDICTOR=2"]
        command += [self.input, self.output]
        run_command(command)


class ComputeClassCountsCommand(object):
    def __init__(self, input, output):
        self.input = input
        self.output = output

    def run(self):
        command = []
        command += ["otbcli", "ComputeClassCounts"]
        command += ["-in", self.input]
        command += ["-out", self.output]
        run_command(command)


class MergeClassCountsCommand(object):
    def __init__(self, inputs, output):
        self.inputs = inputs
        self.output = output

    def run(self):
        command = []
        command += ["merge-counts"]
        command += [self.output]
        command += self.inputs
        run_command(command)


def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)


class Tile(object):
    def __init__(self, tile_id, epsg_code, tile_extent):
        self.tile_id = tile_id
        self.epsg_code = epsg_code
        self.tile_extent = tile_extent


def prepare_lpis(conn, lpis_table, lut_table, tiles):
    with conn.cursor() as cursor:
        lpis_pkey_name = Identifier("{}_pkey".format(lpis_table))
        lut_pkey_name = Identifier("{}_pkey".format(lut_table))
        lut_key_name = Identifier("{}_ori_crop_key".format(lut_table))
        idx_name = Identifier("idx_{}_wkb_geometry".format(lpis_table))
        lpis_table_str = Literal(lpis_table)
        lpis_table = Identifier(lpis_table)
        lut_table = Identifier(lut_table)

        query = SQL(
            """
            alter table {}
            drop column ogc_fid,
            alter column ctnum type int using ctnum :: int,
            alter column ctnum set not null,
            alter column lc type int using lc :: int,
            alter column lc set not null,
            add constraint {} primary key(ctnum),
            add constraint {} unique(ori_crop)
            """
        ).format(lut_table, lut_pkey_name, lut_key_name)
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()

        query = SQL(
            """
            alter table {}
            add column "NewID" bigint,
            add column "HoldID" bigint
            """
        ).format(lpis_table)
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()

        query = SQL(
            """
            update {}
            set "NewID" = t.new_id,
                "HoldID" = t.hold_id
            from (
                select ogc_fid,
                        row_number() over (order by ogc_fid) as new_id,
                        dense_rank() over (order by ori_hold) as hold_id
                from {}
            ) t
            where t.ogc_fid = {}.ogc_fid
            """
        ).format(lpis_table, lpis_table, lpis_table)
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()

        lpis_pkey_name = Identifier("{}_pkey".format(lpis_table))
        query = SQL(
            """
            alter table {}
            alter column "NewID" set not null,
            alter column "HoldID" set not null,
            drop column ogc_fid,
            add constraint {} primary key("NewID")
            """
        ).format(lpis_table, lpis_pkey_name)
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()

        query = SQL(
            """
            create index {} on {} using gist(wkb_geometry);
            """
        ).format(idx_name, lpis_table)
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()

        query = SQL(
            """
            alter table {}
            add column "GeomValid" boolean,
            add column "Duplic" boolean,
            add column "Overlap" boolean,
            add column "Area_meters" real,
            add column "ShapeInd" real,
            add column "CTnum" int,
            add column "CT" text,
            add column "LC" int,
            add column "S1Pix" int,
            add column "S2Pix" int
            """
        ).format(lpis_table)
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()

        query = SQL(
            """
            update {}
            set "GeomValid" = ST_IsValid(wkb_geometry),
                "Overlap" = false,
                "Area_meters" = ST_Area(ST_Transform(wkb_geometry, 4326) :: geography),
                "ShapeInd" = ST_Perimeter(ST_Transform(wkb_geometry, 4326) :: geography) / (2 * sqrt(pi() * nullif(ST_Area(ST_Transform(wkb_geometry, 4326) :: geography), 0)))
            """
        ).format(lpis_table)
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()

        for tile in tiles:
            tile_id = Literal(tile.tile_id)
            query = SQL(
                """
                with tile as (
                    select ST_Transform(geom, Find_SRID('public', {}, 'wkb_geometry')) as geom
                    from shape_tiles_s2
                    where tile_id = {}
                )
                update {}
                set "Overlap" = true
                from tile
                where "GeomValid"
                  and exists (
                        select 1
                          from {} t
                          where t."NewID" != {}."NewID"
                           and t."GeomValid"
                           and ST_Intersects(t.wkb_geometry, tile.geom)
                           and ST_Intersects(t.wkb_geometry, {}.wkb_geometry)
                           having sum(ST_Area(ST_Transform(ST_Intersection(t.wkb_geometry, {}.wkb_geometry), 4326) :: geography)) / nullif({}."Area_meters", 0) > 0.1
                      )
                  and ST_Intersects({}.wkb_geometry, tile.geom)
                """
            ).format(lpis_table_str, tile_id, lpis_table, lpis_table, lpis_table, lpis_table, lpis_table, lpis_table, lpis_table)
            print(query.as_string(conn))
            cursor.execute(query)
            conn.commit()

        for tile in tiles:
            tile_id = Literal(tile.tile_id)
            query = SQL(
                """
                with tile as (
                    select ST_Transform(geom, Find_SRID('public', {}, 'wkb_geometry')) as geom
                    from shape_tiles_s2
                    where tile_id = {}
                )
                update {}
                set "Duplic" = "NewID" in (
                    select "NewID"
                    from (
                        select "NewID",
                                count(*) over(partition by wkb_geometry) as count
                        from {}, tile
                        where ST_Intersects(wkb_geometry, tile.geom)
                    ) t where count > 1
                )
                from tile
                where ST_Intersects({}.wkb_geometry, tile.geom)
                """
            ).format(lpis_table_str, tile_id, lpis_table, lpis_table, lpis_table)
            print(query.as_string(conn))
            cursor.execute(query)
            conn.commit()

        query = SQL(
            """
            select data_type
            from information_schema.columns
            where table_name = {}
            and column_name = 'ori_crop'
            """
        ).format(lpis_table_str)
        print(query.as_string(conn))
        cursor.execute(query)
        col_type = cursor.fetchone()[0]
        conn.commit()

        if col_type == 'numeric':
            query = SQL(
                """
                alter table {}
                alter column ori_crop type integer
                """
            ).format(lpis_table)
            print(query.as_string(conn))
            cursor.execute(query)

        if col_type != 'text':
            query = SQL(
                """
                update {} lpis
                set "CTnum" = lut.ctnum,
                    "CT" = lut.ct,
                    "LC" = lut.lc
                from {} lut
                where lut.ori_crop = lpis.ori_crop :: text
                """
            ).format(lpis_table, lut_table)
        else:
            query = SQL(
                """
                update {} lpis
                set "CTnum" = lut.ctnum,
                    "CT" = lut.ct,
                    "LC" = lut.lc
                from {} lut
                where lut.ori_crop = lpis.ori_crop
                """
            ).format(lpis_table, lut_table)

        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()

        query = SQL(
            """
            alter table {}
            alter column "GeomValid" set not null,
            alter column "Overlap" set not null,
            alter column "Area_meters" set not null
            """
        ).format(lpis_table)
        print(query.as_string(conn))
        cursor.execute(query)
        conn.commit()


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


def get_site_tiles(conn, site_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select shape_tiles_s2.tile_id,
                   shape_tiles_s2.epsg_code,
                   ST_AsBinary(ST_SnapToGrid(ST_Transform(shape_tiles_s2.geom, shape_tiles_s2.epsg_code), 1)) as tile_extent
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
        for (tile_id, epsg_code, tile_extent) in rows:
            print(tile_id)
            tile_extent = ogr.CreateGeometryFromWkb(bytes(tile_extent))
            result.append(Tile(tile_id, epsg_code, tile_extent))

        return result


def get_lpis_path(conn, site_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select value
            from sp_get_parameters('processor.lpis.path')
            where site_id is null or site_id = {}
            order by site_id
            """
        )
        query = query.format(Literal(site_id))
        print(query.as_string(conn))
        cursor.execute(query)

        path = cursor.fetchone()[0]
        conn.commit()

        return path


def read_counts_csv(path):
    counts = {}

    with open(path, "r") as file:
        reader = csv.reader(file)

        for row in reader:
            seq_id = int(row[0])
            count = int(row[1])

            counts[seq_id] = count

    return counts


def get_import_table_command(destination, source, *options):
    command = []
    command += ["ogr2ogr"]
    command += options
    command += [destination, source]
    return command


def main():
    parser = argparse.ArgumentParser(description="Crops and recompresses S1 L2A products")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="site ID to filter by")
    parser.add_argument('-y', '--year', help="year")
    parser.add_argument('--force', help="overwrite field", action='store_true')
    parser.add_argument('--tiles', nargs='+', help="tile filter")
    parser.add_argument('input', default='.', help="declarations")

    args = parser.parse_args()

    config = Config(args)
    pool = multiprocessing.dummy.Pool(8)

    pg_path = 'PG:dbname={} host={} port={} user={} password={}'.format(config.dbname, config.host,
                                                                        config.port, config.user, config.password)

    schema = 'public'
    dir = os.path.dirname(args.input)
    column = 'wkb_geometry'

    lut = glob(os.path.join(dir, "*.csv"))[0]

    with psycopg2.connect(host=config.host, port=config.port, dbname=config.dbname, user=config.user, password=config.password) as conn:
        site_name = get_site_name(conn, config.site_id)
        year = args.year or date.today().year
        lpis_table = "decl_{}_{}".format(site_name, year)
        lut_table = "lut_{}_{}".format(site_name, year)

        lpis_path = get_lpis_path(conn, config.site_id)
        lpis_path = lpis_path.replace('{site}', site_name)
        try:
            os.makedirs(lpis_path)
        except OSError:
            pass

        print("Importing LPIS...")
        commands = []
        commands.append(get_import_table_command(pg_path, args.input, "-lco", "UNLOGGED=YES", "-lco", "SPATIAL_INDEX=OFF", "-nlt", "MULTIPOLYGON", "-nln", lpis_table))
        commands.append(get_import_table_command(pg_path, lut, "-nln", lut_table))
        for command in commands:
            run_command(command)

        print("Retrieving site tiles...")
        tiles = get_site_tiles(conn, config.site_id)

        print("Preparing LPIS...")
        prepare_lpis(conn, lpis_table, lut_table, tiles)

    if config.tiles is not None:
        tiles = [tile for tile in tiles if tile.tile_id in config.tiles]

    print("Rasterizing LPIS data...")
    commands = []
    class_counts = []
    class_counts_20m = []
    base = lpis_table
    field = 'NewID'

    for tile in tiles:
        for resolution in [10, 20]:
            if resolution == 10:
                satellite = "S2"
            else:
                satellite = "S1"

            output = "{}_{}_{}.tif".format(base, tile.tile_id, satellite)
            output = os.path.join(lpis_path, output)

            zone_srs = osr.SpatialReference()
            zone_srs.ImportFromEPSG(tile.epsg_code)

            (dst_xmin, dst_xmax, dst_ymin, dst_ymax) = tile.tile_extent.GetEnvelope()

            sql = SQL(
                """
                with transformed as (
                    select epsg_code, ST_Transform(shape_tiles_s2.geom, Find_SRID({}, {}, {})) as geom
                    from shape_tiles_s2
                    where tile_id = {}
                )
                select {}, ST_Buffer(ST_Transform({}, epsg_code), {})
                from {}, transformed
                where ST_Intersects({}, transformed.geom);
                """)
            sql = sql.format(Literal(schema),
                             Literal(lpis_table), Literal(column), Literal(tile.tile_id), Identifier(field),
                             Identifier(column), Literal(int(-resolution / 2)),
                             Identifier(lpis_table), Identifier(column))
            sql = sql.as_string(conn)

            rasterize_dataset = RasterizeDatasetCommand(pg_path, output, tile.tile_id, resolution,
                                                        sql, field, "EPSG:{}".format(tile.epsg_code),
                                                        int(dst_xmin), int(dst_ymin), int(dst_xmax), int(dst_ymax))
            commands.append(rasterize_dataset)

    pool.map(lambda c: c.run(), commands)

    commands = []
    class_counts = []
    class_counts_20m = []
    for tile in tiles:
        output = "{}_{}_S2.tif".format(base, tile.tile_id)
        output = os.path.join(lpis_path, output)

        counts = "counts_{}.csv".format(tile.tile_id)
        counts = os.path.join(lpis_path, counts)
        class_counts.append(counts)

        output_20m = "{}_{}_S1.tif".format(base, tile.tile_id)
        output_20m = os.path.join(lpis_path, output_20m)

        counts_20m = "counts_{}_20m.csv".format(tile.tile_id)
        counts_20m = os.path.join(lpis_path, counts_20m)
        class_counts_20m.append(counts_20m)

        compute_class_counts = ComputeClassCountsCommand(output, counts)
        commands.append(compute_class_counts)
        compute_class_counts = ComputeClassCountsCommand(output_20m, counts_20m)
        commands.append(compute_class_counts)

    pool.map(lambda c: c.run(), commands)

    print("Merging pixel counts...")
    commands = []
    counts = "counts.csv"
    counts = os.path.join(lpis_path, counts)

    counts_20m = "counts_20m.csv"
    counts_20m = os.path.join(lpis_path, counts_20m)

    merge_class_counts = MergeClassCountsCommand(class_counts, counts)
    commands.append(merge_class_counts)
    merge_class_counts = MergeClassCountsCommand(class_counts_20m, counts_20m)
    commands.append(merge_class_counts)

    pool.map(lambda c: c.run(), commands)

    print("Updating pixel counts...")
    class_counts = read_counts_csv(counts)
    class_counts_20m = read_counts_csv(counts_20m)
    counts = defaultdict(lambda: (0, 0))
    for (id, count) in class_counts.items():
        counts[id] = (count, counts[id][1])
    for (id, count) in class_counts_20m.items():
        counts[id] = (counts[id][0], count)
    del counts[0]

    sql = SQL(
        """
        update {}
        set "S2Pix" = %s,
            "S1Pix" = %s
        where {} = %s
        """)
    sql = sql.format(Identifier(lpis_table), Identifier(field))
    sql = sql.as_string(conn)

    with conn.cursor() as cursor:
        args = [(s2pix, s1pix, id) for (id, (s2pix, s1pix)) in counts.items()]
        psycopg2.extras.execute_batch(cursor, sql, args, page_size=1000)
        conn.commit()

        sql = SQL(
            """
            update {}
            set "S2Pix" = coalesce("S2Pix", 0),
                "S1Pix" = coalesce("S1Pix", 0)
            where "S2Pix" is null or "S1Pix" is null;
            """)
        sql = sql.format(Identifier(lpis_table))
        print(sql.as_string(conn))
        cursor.execute(sql)
        conn.commit()

        sql = SQL(
            """
            alter table {}
            alter column "S2Pix" set not null,
            alter column "S1Pix" set not null;
            """)
        sql = sql.format(Identifier(lpis_table))
        print(sql.as_string(conn))
        cursor.execute(sql)
        conn.commit()

        tiles = [t.tile_id for t in tiles]
        sql = SQL(
            """
            insert into product(product_type_id, processor_id, site_id, full_path, created_timestamp, tiles)
            values({}, {}, {}, {}, {}, {})
            """
        )
        sql = sql.format(Literal(PRODUCT_TYPE_LPIS), Literal(PROCESSOR_LPIS), Literal(config.site_id), Literal(lpis_path), Literal(date.today()), Literal(tiles))
        print(sql.as_string(conn))
        cursor.execute(sql)
        conn.commit()

    command = []
    command += ["export-lpis.py"]
    command += ["-s", config.site_id]
    command += ["-p", lpis_path]
    run_command(command)

    output = "parcels.csv"
    output = os.path.join(lpis_path, output)

    command = []
    command += ["data-preparation.py"]
    command += ["-s", config.site_id]
    command += ["-y", year]
    command += [output]
    run_command(command)


if __name__ == "__main__":
    main()
