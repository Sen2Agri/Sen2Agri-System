#!/usr/bin/env python
from __future__ import print_function

import argparse
import csv
from collections import defaultdict
from datetime import date
import logging
import multiprocessing.dummy
import os
import os.path
from osgeo import osr
from osgeo import ogr
import pipes
import psycopg2
from psycopg2.sql import SQL, Literal, Identifier
import psycopg2.extras
import psycopg2.extensions
import shutil
import subprocess
import sys

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser


PRODUCT_TYPE_LPIS = 14
PROCESSOR_LPIS = 8

MODE_UPDATE = 0
MODE_REPLACE = 1
MODE_INCREMENTAL = 2


def try_rm_file(f):
    try:
        os.remove(f)
        return True
    except OSError:
        return False


def try_mkdir(p):
    try:
        os.makedirs(p)
    except OSError:
        pass


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


class RasterizeDatasetCommand(object):
    def __init__(
        self,
        input,
        output,
        tile,
        resolution,
        sql,
        field,
        srs,
        dst_xmin,
        dst_ymin,
        dst_xmax,
        dst_ymax,
    ):
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
    logging.debug(cmd_line)
    subprocess.call(args, env=env)


def get_esri_wkt(epsg_code):
    srs = osr.SpatialReference()
    srs.ImportFromEPSG(epsg_code)
    srs.MorphToESRI()
    return srs.ExportToWkt()


class Tile(object):
    def __init__(self, tile_id, epsg_code, tile_extent):
        self.tile_id = tile_id
        self.epsg_code = epsg_code
        self.tile_extent = tile_extent


def get_column_type(conn, schema, table, column):
    with conn.cursor() as cursor:
        query = SQL(
            """
select data_type
from information_schema.columns
where table_schema = %s
  and table_name = %s
  and column_name = %s;"""
        )
        cursor.execute(query, (schema, table, column))
        res = cursor.fetchone()
        if res:
            col_type = res[0]
        else:
            col_type = None
        return col_type


def get_table_columns(conn, schema, table):
    with conn.cursor() as cursor:
        query = SQL(
            """
select column_name
from information_schema.columns
where table_schema = %s
    and table_name = %s;"""
        )
        cursor.execute(query, (schema, table))
        cols = []
        for row in cursor:
            cols.append(row[0])
        return cols


def get_column_concat_sql(cols):
    idents = [Identifier(col) for col in cols]
    return SQL(" || '-' || ").join(idents)


def table_exists(conn, schema, name):
    with conn.cursor() as cursor:
        query = SQL(
            """
select exists (
    select *
    from pg_class
    inner join pg_namespace on pg_namespace.oid = pg_class.relnamespace
    where pg_class.relname = %s
      and pg_namespace.nspname = %s
);"""
        )
        cursor.execute(query, (name, schema))
        return cursor.fetchone()[0]


def index_exists(conn, name):
    with conn.cursor() as cursor:
        query = SQL(
            """
select exists (
    select *
    from pg_index
    inner join pg_class on pg_class.oid = pg_index.indexrelid
    where pg_class.relname = %s
);"""
        )
        cursor.execute(query, (name,))
        return cursor.fetchone()[0]


def column_exists(conn, schema, table, column):
    return get_column_type(conn, schema, table, column) is not None


def add_table_columns(conn, schema, table, columns):
    with conn.cursor() as cursor:
        query = SQL("alter table {}\n").format(Identifier(table))
        to_add = []
        for (column, ty) in columns:
            if get_column_type(conn, schema, table, column) is None:
                q = SQL("add column {} {}").format(Identifier(column), SQL(ty))
                to_add.append(q)
        if len(to_add) > 0:
            query += SQL(",\n").join(to_add)
            query += SQL(";")
            logging.debug(query.as_string(conn))
            cursor.execute(query)


def drop_table_columns(conn, schema, table, columns):
    with conn.cursor() as cursor:
        query = SQL("alter table {}\n").format(Identifier(table))
        to_add = []
        for column in columns:
            if get_column_type(conn, schema, table, column) is not None:
                q = SQL("drop column if exists {}").format(Identifier(column))
                to_add.append(q)
        if len(to_add) > 0:
            query += SQL(",\n").join(to_add)
            query += SQL(";")
            logging.debug(query.as_string(conn))
            cursor.execute(query)


def create_index(conn, table, columns):
    with conn.cursor() as cursor:
        name = "ix_{}_{}".format(table, "_".join(columns))
        if not index_exists(conn, name):
            cols = SQL(", ").join([Identifier(c) for c in columns])
            query = SQL("create index {} on {}({});").format(
                Identifier(name), Identifier(table), cols
            )
            logging.debug(query.as_string(conn))
            cursor.execute(query)


def drop_index(conn, table, columns):
    with conn.cursor() as cursor:
        name = "ix_{}_{}".format(table, "_".join(columns))
        query = SQL("drop index if exists {};").format(Identifier(name))
        logging.debug(query.as_string(conn))
        cursor.execute(query)


def create_spatial_index(conn, table, column):
    with conn.cursor() as cursor:
        name = "ix_{}_{}".format(table, column)
        if not index_exists(conn, name):
            query = SQL("create index {} on {} using gist({});").format(
                Identifier(name), Identifier(table), Identifier(column)
            )
            logging.debug(query.as_string(conn))
            cursor.execute(query)


def drop_spatial_index(conn, table, column):
    with conn.cursor() as cursor:
        name = "ix_{}_{}".format(table, column)
        query = SQL("drop index if exists {};").format(Identifier(name))
        logging.debug(query.as_string(conn))
        cursor.execute(query)


def create_primary_key(conn, table, columns):
    with conn.cursor() as cursor:
        name = "{}_pkey".format(table)
        if not index_exists(conn, name):
            cols = SQL(", ").join([Identifier(c) for c in columns])
            query = SQL("alter table {} add constraint {} primary key({});").format(
                Identifier(table), Identifier(name), cols
            )
            logging.debug(query.as_string(conn))
            cursor.execute(query)


def get_site_name(conn, site_id):
    with conn.cursor() as cursor:
        query = SQL("select short_name from site where id = %s")
        cursor.execute(query, (site_id,))
        rows = cursor.fetchall()
        conn.commit()
        return rows[0][0]


def get_site_srid(conn, lpis_table):
    with conn.cursor() as cursor:
        query = SQL("select Find_SRID('public', %s, 'wkb_geometry')")
        cursor.execute(query, (lpis_table,))
        rows = cursor.fetchall()
        conn.commit()
        return rows[0][0]


def get_site_utm_epsg_codes(conn, site_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
            select distinct
                   shape_tiles_s2.epsg_code
            from sp_get_site_tiles(%s :: smallint, 1 :: smallint) site_tiles
            inner join shape_tiles_s2 on shape_tiles_s2.tile_id = site_tiles.tile_id;"""
        )

        cursor.execute(query, (site_id,))
        rows = cursor.fetchall()

        result = []
        for (epsg_code,) in rows:
            result.append(epsg_code)

        return result


def get_site_tiles(conn, site_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
select shape_tiles_s2.tile_id,
       shape_tiles_s2.epsg_code,
       ST_AsBinary(ST_SnapToGrid(ST_Transform(shape_tiles_s2.geom, shape_tiles_s2.epsg_code), 1)) as tile_extent
from sp_get_site_tiles(%s :: smallint, 1 :: smallint) site_tiles
inner join shape_tiles_s2 on shape_tiles_s2.tile_id = site_tiles.tile_id;"""
        )
        logging.debug(query.as_string(conn))
        cursor.execute(query, (site_id,))

        rows = cursor.fetchall()
        conn.commit()

        result = []
        for (tile_id, epsg_code, tile_extent) in rows:
            tile_extent = ogr.CreateGeometryFromWkb(bytes(tile_extent))
            result.append(Tile(tile_id, epsg_code, tile_extent))

        return result


class DataPreparation(object):
    DB_UPDATE_BATCH_SIZE = 1000

    def __init__(self, config, year, working_path):
        self.config = config
        self.year = year
        self.pool = multiprocessing.dummy.Pool()

        with self.get_connection() as conn:
            print("Retrieving site tiles")
            site_name = get_site_name(conn, config.site_id)
            self.tiles = get_site_tiles(conn, config.site_id)
            # self.srid = get_site_srid(conn, lpis_table)

        self.lpis_table = "decl_{}_{}".format(site_name, year)
        self.lpis_table_staging = "decl_{}_{}_staging".format(site_name, year)
        self.lut_table = "lut_{}_{}".format(site_name, year)

        lpis_path = get_lpis_path(conn, config.site_id)
        lpis_path = lpis_path.replace("{year}", str(year))
        lpis_path = lpis_path.replace("{site}", site_name)

        if not working_path:
            working_path = lpis_path
        self.lpis_path = lpis_path
        self.working_path = working_path

    def get_connection(self):
        return psycopg2.connect(
            host=self.config.host,
            port=self.config.port,
            dbname=self.config.dbname,
            user=self.config.user,
            password=self.config.password,
        )

    def get_ogr_connection_string(self):
        return "PG:dbname={} host={} port={} user={} password={}".format(
            self.config.dbname,
            self.config.host,
            self.config.port,
            self.config.user,
            self.config.password,
        )

    def find_overlaps(self, srid, tile_counts, total):
        q = multiprocessing.dummy.Queue()
        res = self.pool.map_async(
            lambda t: self.get_overlapping_parcels(srid, q, t), self.tiles
        )

        progress = 0
        sys.stdout.write("Finding overlapping parcels: 0.00%")
        sys.stdout.flush()
        for i in range(len(self.tiles)):
            tile = q.get()
            progress += tile_counts[tile.tile_id]
            sys.stdout.write(
                "\rFinding overlapping parcels: {0:.2f}%".format(
                    100.0 * progress / total
                )
            )
            sys.stdout.flush()
        sys.stdout.write("\n")
        sys.stdout.flush()

        overlaps = list(set.union(*map(set, res.get())))
        logging.info("{} overlapping parcels".format(len(overlaps)))
        self.mark_overlapping_parcels(overlaps)

    def find_duplicates(self, srid, tile_counts, total):
        q = multiprocessing.dummy.Queue()
        res = self.pool.map_async(
            lambda t: self.get_duplicate_parcels(srid, q, t), self.tiles
        )

        progress = 0
        sys.stdout.write("Finding duplicate parcels: 0.00%")
        sys.stdout.flush()
        for i in range(len(self.tiles)):
            tile = q.get()
            progress += tile_counts[tile.tile_id]
            sys.stdout.write(
                "\rFinding duplicate parcels: {0:.2f}%".format(100.0 * progress / total)
            )
            sys.stdout.flush()
        sys.stdout.write("\n")
        sys.stdout.flush()

        duplicates = list(set.union(*map(set, res.get())))
        logging.info("{} duplicate parcels".format(len(duplicates)))
        self.mark_duplicate_parcels(duplicates)

    def prepare_lut(self, lut_path):
        with self.get_connection() as conn:
            print("Importing LUT")
            cmd = get_import_table_command(
                self.get_ogr_connection_string(),
                lut_path,
                "-nln",
                self.lut_table,
                "-overwrite",
                "-oo",
                "AUTODETECT_TYPE=YES",
            )
            run_command(cmd)

            print("Preparing LUT")
            with conn.cursor() as cursor:
                lut_pkey_name = "{}_pkey".format(self.lut_table)
                lut_key_name = "{}_ori_crop_key".format(self.lut_table)

                query = SQL(
                    """alter table {}
drop column ogc_fid,
alter column ctnum type int using ctnum :: int,
alter column lc type int using lc :: int,
alter column ctnuml4a type int using ctnuml4a :: int,
alter column ctnumdiv type int using ctnumdiv :: int,
alter column eaa type bit using eaa :: bit,
alter column al type bit using al :: bit,
alter column pgrass type bit using pgrass :: bit,
alter column tgrass type bit using tgrass :: bit,
alter column fallow type bit using fallow :: bit,
alter column cwater type bit using cwater :: bit,
alter column ctnum set not null,
alter column lc set not null,
alter column ctnuml4a set not null,
alter column ctnumdiv set not null,
alter column eaa set not null,
alter column al set not null,
alter column pgrass set not null,
alter column tgrass set not null,
alter column fallow set not null,
alter column cwater set not null,
add constraint {} primary key(ctnum),
add constraint {} unique(ori_crop);"""
                ).format(
                    Identifier(self.lut_table),
                    Identifier(lut_pkey_name),
                    Identifier(lut_key_name),
                )
                logging.debug(query.as_string(conn))
                cursor.execute(query)
                conn.commit()

    def prepare_lpis_staging(
        self, mode, parcel_id_cols, holding_id_cols, crop_code_col, lpis, parcel_id_offset, holding_id_offset,
    ):
        parcel_id_cols = [col.lower() for col in parcel_id_cols]
        holding_id_cols = [col.lower() for col in holding_id_cols]
        crop_code_col = crop_code_col.lower()

        print("Importing LPIS")
        cmd = get_import_table_command(
            self.get_ogr_connection_string(),
            lpis,
            "-lco",
            "UNLOGGED=YES",
            "-lco",
            "SPATIAL_INDEX=OFF",
            "-nlt",
            "MULTIPOLYGON",
            "-nln",
            self.lpis_table_staging,
            "-overwrite",
        )
        run_command(cmd)

        print("Preparing LPIS")
        with self.get_connection() as conn:
            with conn.cursor() as cursor:
                print("Initializing")
                lpis_table_id = Identifier(self.lpis_table)
                lpis_table_staging_id = Identifier(self.lpis_table_staging)

                for col in ["ori_id", "ori_hold", "ori_crop"]:
                    if column_exists(conn, "public", self.lpis_table_staging, col):
                        logging.error("`{}` is not an allowed LPIS column name")
                        sys.exit(1)

                ori_crop_type = get_column_type(
                    conn, "public", self.lpis_table_staging, crop_code_col
                )
                if ori_crop_type == "character varying":
                    ori_crop_type = "text"
                elif ori_crop_type == "numeric":
                    ori_crop_type = "int"
                    query = SQL(
                        "alter table {} alter column {} type int;"
                    ).format(lpis_table_staging_id, Identifier(crop_code_col))
                    logging.debug(query.as_string(conn))
                    cursor.execute(query)

                print("Adding computed columns")
                cols = []
                cols.append(("ori_id", "text"))
                cols.append(("ori_hold", "text"))
                cols.append(("ori_crop", ori_crop_type))
                cols.append(("NewID", "int"))
                cols.append(("HoldID", "int"))
                add_table_columns(conn, "public", self.lpis_table_staging, cols)

                print("Creating indexes")
                create_index(conn, self.lpis_table_staging, ["ori_id"])
                create_index(conn, self.lpis_table_staging, ["ori_hold"])

                print("Computing parcel identifiers")
                cols = [Identifier(col) for col in parcel_id_cols]
                partition_key = SQL(", ").join(cols)

                parcel_id_expr = get_column_concat_sql(parcel_id_cols)
                holding_id_expr = get_column_concat_sql(holding_id_cols)
                crop_code_expr = Identifier(crop_code_col)
                query = SQL(
                    """update {} lpis
set ori_id = {} || d.discriminator,
    ori_hold = {},
    ori_crop = {}
from (
    select ogc_fid,
            case
                when count = 1 then ''
                when rn <= 26 then '-' || chr(64 + rn :: int)
                else '-' || chr(64 + (rn / 26) :: int) || chr(64 + (rn % 26) :: int)
            end as discriminator
        from (
            select ogc_fid,
                row_number() over(partition by {} order by ogc_fid) as rn,
                count(*) over(partition by {}) as count
            from {}
        ) t
) d
where d.ogc_fid = lpis.ogc_fid;"""
                ).format(
                    lpis_table_staging_id,
                    parcel_id_expr,
                    holding_id_expr,
                    crop_code_expr,
                    partition_key,
                    partition_key,
                    lpis_table_staging_id,
                )
                logging.debug(query.as_string(conn))
                cursor.execute(query)
                # conn.commit()

                print("Fixing up types")
                query = SQL(
                    """alter table {}
alter column ori_id set not null,
alter column ori_hold set not null,
alter column ori_crop set not null;"""
                ).format(lpis_table_staging_id)
                logging.debug(query.as_string(conn))
                cursor.execute(query)
                # conn.commit()

                if mode == MODE_REPLACE:
                    print("Removing old LPIS")
                    query = SQL("drop table if exists {};").format(lpis_table_id)
                    logging.debug(query.as_string(conn))
                    cursor.execute(query)

                    # conn.commit()

                print("Making sure destination table exists")
                query = SQL(
                    """create table if not exists {} (
like {},
"GeomValid" boolean not null,
"Duplic" boolean,
"Overlap" boolean not null,
"Area_meters" real not null,
"ShapeInd" real,
"S1Pix" int not null default 0,
"S2Pix" int not null default 0,
is_deleted boolean default false,
inserted_timestamp timestamp with time zone not null default now(),
updated_timestamp timestamp with time zone,
geom_change_ratio real
);"""
                ).format(lpis_table_id, lpis_table_staging_id)
                logging.debug(query.as_string(conn))
                cursor.execute(query)
                # conn.commit()

                print("Fixing up columns")
                drop_table_columns(conn, "public", self.lpis_table, ["ogc_fid"])

                cols = []
                cols.append(("is_deleted", "boolean default false"))
                cols.append(
                    (
                        "inserted_timestamp",
                        "timestamp with time zone not null default now()",
                    )
                )
                cols.append(("updated_timestamp", "timestamp with time zone"))
                cols.append(("geom_change_ratio", "real"))
                add_table_columns(conn, "public", self.lpis_table, cols)

                create_index(conn, self.lpis_table, ["ori_id"])
                create_index(conn, self.lpis_table, ["ori_hold"])
                # create_index(conn, self.lpis_table, ["NewID"])
                # create_index(conn, self.lpis_table, ["HoldID"])

                add_table_columns(
                    conn, "public", self.lpis_table_staging, [("is_new", "bool")]
                )

                print("Finding existing parcels")
                query = SQL(
                    """update {} new
set is_new = true
where not exists (
    select *
    from {} old
    where old.ori_id = new.ori_id
);"""
                ).format(lpis_table_staging_id, lpis_table_id)
                logging.debug(query.as_string(conn))
                cursor.execute(query)
                # conn.commit()

                print("Copying old parcel identifiers")
                query = SQL(
                    """update {} new
set "NewID" = old."NewID"
from {} old
where old.ori_id = new.ori_id;"""
                ).format(lpis_table_staging_id, lpis_table_id)
                logging.debug(query.as_string(conn))
                cursor.execute(query)
                #         conn.commit()

                print("Renumbering new parcels")
                query = SQL(
                    """update {} new
set "NewID" = t.rn + %s + (
    select coalesce(max("NewID"), 0)
    from {}
)
from (
    select ogc_fid as id, row_number() over (order by ori_id) as rn
    from {}
    where "NewID" is null
) t
where new.ogc_fid = t.id;"""
                ).format(lpis_table_staging_id, lpis_table_id, lpis_table_staging_id)
                logging.debug(query.as_string(conn))
                cursor.execute(query, (parcel_id_offset, ))
                # conn.commit()

                print("Copying old holding identifiers")
                query = SQL(
                    """update {} new
set "HoldID" = old."HoldID"
from {} old
where old.ori_hold = new.ori_hold;"""
                ).format(lpis_table_staging_id, lpis_table_id)
                logging.debug(query.as_string(conn))
                cursor.execute(query)
                # conn.commit()

                print("Renumbering new holdings")
                query = SQL(
                    """
update {} new
set "HoldID" = t.rk + %s + (
    select coalesce(max("HoldID"), 0)
    from {}
)
from (
    select ogc_fid as id,
        dense_rank() over (order by ori_hold) as rk
    from {}
    where "HoldID" is null
) t
where new.ogc_fid = t.id;"""
                ).format(lpis_table_staging_id, lpis_table_id, lpis_table_staging_id)
                logging.debug(query.as_string(conn))
                cursor.execute(query, (holding_id_offset, ))
                # conn.commit()

                conn.commit()
                if mode != MODE_INCREMENTAL:
                    print("Marking deleted parcels")
                    query = SQL(
                        """
update {} old
set is_deleted = true,
    updated_timestamp = now()
where not exists (
    select *
    from {} new
    where new.ori_id = old.ori_id
);"""
                    ).format(lpis_table_id, lpis_table_staging_id)
                    logging.debug(query.as_string(conn))
                    cursor.execute(query)
                    conn.commit()

                drop_index(conn, self.lpis_table, ["ori_id"])
                drop_index(conn, self.lpis_table, ["ori_hold"])
                drop_spatial_index(conn, self.lpis_table, "wkb_geometry")

                print("Updating existing parcels")
                cols = get_table_columns(conn, "public", self.lpis_table_staging)
                p_old_cols = SQL("(")
                old_cols = SQL("(")
                new_cols = SQL("(")
                for col in cols:
                    if col in ("ogc_fid", "is_new", "NewID", "ori_id", "wkb_geometry"):
                        continue

                    col_id = Identifier(col)
                    old_cols += SQL("{}, ").format(col_id)
                    p_old_cols += SQL("old.{}, ").format(col_id)
                    new_cols += SQL("new.{}, ").format(col_id)
                old_cols += SQL("wkb_geometry)")
                p_old_cols += SQL("old.wkb_geometry)")
                new_cols += SQL("new.wkb_geometry)")
                query = SQL(
                    """
update {} old
set {} = {},
    "GeomValid" = coalesce(ST_IsValid(new.wkb_geometry), false),
    "Overlap" = false,
    "Duplic" = false,
    "Area_meters" = coalesce(ST_Area(new.wkb_geometry), 0),
    "ShapeInd" = ST_Perimeter(new.wkb_geometry) / (2 * sqrt(pi() * nullif(ST_Area(new.wkb_geometry), 0))),
    "S1Pix" = 0,
    "S2Pix" = 0,
    is_deleted = false,
    updated_timestamp = now(),
    geom_change_ratio = ST_Area(ST_SymDifference(ST_MakeValid(old.wkb_geometry), ST_MakeValid(new.wkb_geometry))) / ST_Area(ST_Union(ST_MakeValid(old.wkb_geometry), ST_MakeValid(new.wkb_geometry)))
from {} new
where new.ori_id = old.ori_id
and {} is distinct from {};"""
                ).format(
                    lpis_table_id,
                    old_cols,
                    new_cols,
                    lpis_table_staging_id,
                    new_cols,
                    p_old_cols,
                )
                logging.debug(query.as_string(conn))
                cursor.execute(query)
                conn.commit()

                print("Inserting new parcels")
                insert_cols = SQL(", ").join(
                    [
                        Identifier(col)
                        for col in cols
                        if col not in ("ogc_fid", "is_new")
                    ]
                )
                query = SQL(
                    """
insert into {}({}, "GeomValid", "Duplic", "Overlap", "Area_meters", "ShapeInd")
select {},
    coalesce(ST_IsValid(wkb_geometry), false) as "GeomValid",
    false as "Duplic",
    false as "Overlap",
    coalesce(ST_Area(wkb_geometry), 0) as "Area_meters",
    ST_Perimeter(wkb_geometry) / (2 * sqrt(pi() * nullif(ST_Area(wkb_geometry), 0))) as "ShapeInd"
from {}
where is_new;"""
                ).format(lpis_table_id, insert_cols, insert_cols, lpis_table_staging_id)
                logging.debug(query.as_string(conn))
                cursor.execute(query)

                conn.commit()

                print("Cleaning up")
                query = SQL("drop table {};").format(lpis_table_staging_id)
                logging.debug(query.as_string(conn))
                cursor.execute(query)

                print("Creating indexes")
                create_spatial_index(conn, self.lpis_table, "wkb_geometry")
                create_primary_key(conn, self.lpis_table, ["NewID"])

    def prepare_lpis(self):
        with self.get_connection() as conn:
            print("Retrieving site SRID")
            srid = get_site_srid(conn, self.lpis_table)

        tile_counts = self.get_tile_parcel_counts(srid)
        total = 0
        for c in tile_counts.values():
            total += c
        self.find_overlaps(srid, tile_counts, total)
        self.find_duplicates(srid, tile_counts, total)

    def export_lpis(self):
        with self.get_connection() as conn:
            if not table_exists(conn, "public", self.lpis_table):
                logging.info("LPIS table does not exist, skipping export")
                return
            if not table_exists(conn, "public", self.lut_table):
                logging.info("LUT table does not exist, skipping export")
                return

        try_mkdir(self.lpis_path)
        try_mkdir(self.working_path)

        commands = []
        class_counts = []
        class_counts_20m = []
        base = self.lpis_table

        with self.get_connection() as conn:
            for tile in self.tiles:
                zone_srs = osr.SpatialReference()
                zone_srs.ImportFromEPSG(tile.epsg_code)

                (
                    dst_xmin,
                    dst_xmax,
                    dst_ymin,
                    dst_ymax,
                ) = tile.tile_extent.GetEnvelope()

                for resolution in [10, 20]:
                    if resolution == 10:
                        satellite = "S2"
                    else:
                        satellite = "S1"

                    output = "{}_{}_{}.tif".format(base, tile.tile_id, satellite)
                    output = os.path.join(self.lpis_path, output)

                    sql = SQL(
                        """
with transformed as (
    select epsg_code, ST_Transform(shape_tiles_s2.geom, Find_SRID('public', {}, 'wkb_geometry')) as geom
    from shape_tiles_s2
    where tile_id = {}
)
select "NewID", ST_Buffer(ST_Transform(wkb_geometry, epsg_code), {})
from {}, transformed
where ST_Intersects(wkb_geometry, transformed.geom)
and not is_deleted;"""
                    )
                    sql = sql.format(
                        Literal(self.lpis_table),
                        Literal(tile.tile_id),
                        Literal(int(-resolution / 2)),
                        Identifier(self.lpis_table),
                    )
                    sql = sql.as_string(conn)

                    rasterize_dataset = RasterizeDatasetCommand(
                        self.get_ogr_connection_string(),
                        output,
                        tile.tile_id,
                        resolution,
                        sql,
                        "NewID",
                        "EPSG:{}".format(tile.epsg_code),
                        int(dst_xmin),
                        int(dst_ymin),
                        int(dst_xmax),
                        int(dst_ymax),
                    )

                    if resolution == 10:
                        cost = 5
                    else:
                        cost = 2
                    commands.append((rasterize_dataset, cost))

        q = multiprocessing.dummy.Queue()

        def work(w):
            (c, cost) = w
            c.run()
            q.put(cost)

        res = self.pool.map_async(work, commands)

        total = len(self.tiles) * 7
        progress = 0
        sys.stdout.write("Rasterizing LPIS: 0.00%")
        sys.stdout.flush()
        for i in range(len(commands)):
            progress += q.get()
            sys.stdout.write(
                "\rRasterizing LPIS: {0:.2f}%".format(100.0 * progress / total)
            )
            sys.stdout.flush()
        sys.stdout.write("\n")
        sys.stdout.flush()

        res.get()

        commands = []
        class_counts = []
        class_counts_20m = []
        for tile in self.tiles:
            output = "{}_{}_S2.tif".format(base, tile.tile_id)
            output = os.path.join(self.lpis_path, output)

            counts = "counts_{}.csv".format(tile.tile_id)
            counts = os.path.join(self.working_path, counts)
            class_counts.append(counts)

            output_20m = "{}_{}_S1.tif".format(base, tile.tile_id)
            output_20m = os.path.join(self.lpis_path, output_20m)

            counts_20m = "counts_{}_20m.csv".format(tile.tile_id)
            counts_20m = os.path.join(self.working_path, counts_20m)
            class_counts_20m.append(counts_20m)

            compute_class_counts = ComputeClassCountsCommand(output, counts)
            commands.append((compute_class_counts, 19))
            compute_class_counts = ComputeClassCountsCommand(output_20m, counts_20m)
            commands.append((compute_class_counts, 10))

        q = multiprocessing.dummy.Queue()

        def work(w):
            (c, cost) = w
            c.run()
            q.put(cost)

        res = self.pool.map_async(work, commands)

        total = len(self.tiles) * 29
        progress = 0
        sys.stdout.write("Counting pixels: 0.00%")
        sys.stdout.flush()
        for i in range(len(commands)):
            progress += q.get()
            sys.stdout.write(
                "\rCounting pixels: {0:.2f}%".format(100.0 * progress / total)
            )
            sys.stdout.flush()
        sys.stdout.write("\n")
        sys.stdout.flush()

        res.get()

        print("Merging pixel counts")
        commands = []
        counts = "counts.csv"
        counts = os.path.join(self.working_path, counts)

        counts_20m = "counts_20m.csv"
        counts_20m = os.path.join(self.working_path, counts_20m)

        merge_class_counts = MergeClassCountsCommand(class_counts, counts)
        commands.append((merge_class_counts, 5))
        merge_class_counts = MergeClassCountsCommand(class_counts_20m, counts_20m)
        commands.append((merge_class_counts, 4))

        def work(w):
            (c, cost) = w
            c.run()
            q.put(cost)

        res = self.pool.map_async(work, commands)

        total = 9
        progress = 0
        sys.stdout.write("Merging pixel counts: 0.00%")
        sys.stdout.flush()
        for i in range(len(commands)):
            progress += q.get()
            sys.stdout.write(
                "\rMerging pixel counts: {0:.2f}%".format(100.0 * progress / total)
            )
            sys.stdout.flush()
        sys.stdout.write("\n")
        sys.stdout.flush()

        res.get()

        for f in class_counts:
            try_rm_file(f)
        for f in class_counts_20m:
            try_rm_file(f)

        print("Reading pixel counts")
        class_counts = read_counts_csv(counts)
        class_counts_20m = read_counts_csv(counts_20m)

        c = defaultdict(lambda: (0, 0))
        for (id, count) in class_counts.items():
            c[id] = (count, c[id][1])
        for (id, count) in class_counts_20m.items():
            c[id] = (c[id][0], count)
        del c[0]

        if c:
            updates = [(id, s2pix, s1pix) for (id, (s2pix, s1pix)) in c.items()]
            del c

            def update_batch(b):
                id = [e[0] for e in b]
                s2_pix = [e[1] for e in b]
                s1_pix = [e[2] for e in b]

                sql = SQL(
                    """update {} lpis
set "S2Pix" = s2_pix,
    "S1Pix" = s1_pix
from (select unnest(%s) as id,
                unnest(%s) as s2_pix,
                unnest(%s) as s1_pix) upd
where upd.id = lpis."NewID";""")
                sql = sql.format(Identifier(self.lpis_table))

                with self.get_connection() as conn:
                    with conn.cursor() as cursor:
                        logging.debug(sql.as_string(conn))
                        cursor.execute(sql, (id, s2_pix, s1_pix))
                        conn.commit()

            def work(w):
                (c, cost) = w
                c()
                q.put(cost)

            total = len(updates)
            progress = 0
            sys.stdout.write("Updating pixel counts: 0.00%")
            sys.stdout.flush()

            commands = []
            for b in batch(updates, self.DB_UPDATE_BATCH_SIZE):
                def f(b=b):
                    update_batch(b)

                commands.append((f, len(b)))

            res = self.pool.map_async(work, commands)

            for i in range(len(commands)):
                progress += q.get()
                sys.stdout.write(
                    "\rUpdating pixel counts: {0:.2f}%".format(
                        100.0 * progress / total
                    )
                )
                sys.stdout.flush()
            sys.stdout.write("\n")
            sys.stdout.flush()

            res.get()

            try_rm_file(counts)
            try_rm_file(counts_20m)

            with conn.cursor() as cursor:
                print("Cleaning up")
                conn.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_AUTOCOMMIT)
                sql = SQL("vacuum full {};").format(Identifier(self.lpis_table))
                logging.debug(sql.as_string(conn))
                cursor.execute(sql)
                conn.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_DEFAULT)

                tiles = [t.tile_id for t in self.tiles]
                name = "SEN4CAP_LPIS_S{}_{}".format(self.config.site_id, self.year)
                dt = date(self.year, 1, 1)
                sql = SQL(
                    """delete
from product
where site_id = %s
and product_type_id = %s
and created_timestamp = %s;"""
                )
                logging.debug(sql.as_string(conn))
                cursor.execute(sql, (self.config.site_id, PRODUCT_TYPE_LPIS, dt))

                sql = SQL(
                    """
insert into product(product_type_id, processor_id, site_id, name, full_path, created_timestamp, tiles)
values(%s, %s, %s, %s, %s, %s, %s);"""
                )
                logging.debug(sql.as_string(conn))
                cursor.execute(
                    sql,
                    (
                        PRODUCT_TYPE_LPIS,
                        PROCESSOR_LPIS,
                        self.config.site_id,
                        name,
                        self.lpis_path,
                        dt,
                        tiles,
                    ),
                )
                conn.commit()

                epsg_codes = get_site_utm_epsg_codes(conn, self.config.site_id)
                epsg_codes.append(3035)

                commands = []

                csv = "{}.csv".format(self.lpis_table)
                csv = os.path.join(self.lpis_path, csv)

                gpkg = "{}.gpkg".format(self.lpis_table)
                gpkg_working = os.path.join(self.working_path, gpkg)
                gpkg = os.path.join(self.lpis_path, gpkg)

                try_rm_file(gpkg_working)

                sql = SQL(
                    """
select *
from {}
inner join {} using (ori_crop)
where not is_deleted"""
                ).format(Identifier(self.lpis_table), Identifier(self.lut_table))
                sql = sql.as_string(conn)

                command = []
                command += ["ogr2ogr"]
                command += ["-sql", sql]
                command += [csv]
                command += [self.get_ogr_connection_string()]
                commands.append((command, 5))

                srid = get_site_srid(conn, self.lpis_table)
                command = []
                command += ["ogr2ogr"]
                command += ["-a_srs", "EPSG:{}".format(srid)]
                command += ["-fieldTypeToString", "DateTime"]
                command += ["-sql", sql]
                command += [gpkg_working]
                command += [self.get_ogr_connection_string()]
                commands.append((command, 12))

                for epsg_code in epsg_codes:
                    wkt = get_esri_wkt(epsg_code)

                    for resolution in [10, 20]:
                        if resolution == 10 and epsg_code == 3035:
                            continue

                        buf = resolution / 2
                        output = "{}_{}_buf_{}m.shp".format(
                            self.lpis_table, epsg_code, buf
                        )
                        output = os.path.join(self.lpis_path, output)
                        prj = "{}_{}_buf_{}m.prj".format(
                            self.lpis_table, epsg_code, buf
                        )
                        prj = os.path.join(self.lpis_path, prj)

                        with open(prj, "wb") as f:
                            f.write(wkt)

                        sql = SQL(
                            'select "NewID", ST_Buffer(ST_Transform(wkb_geometry, {}), -{}) from {} where not is_deleted'
                        )
                        sql = sql.format(
                            Literal(epsg_code),
                            Literal(buf),
                            Identifier(self.lpis_table),
                        )
                        sql = sql.as_string(conn)

                        command = []
                        command += ["ogr2ogr"]
                        command += [output, self.get_ogr_connection_string()]
                        command += ["-sql", sql]
                        commands.append((command, 23))

        q = multiprocessing.dummy.Queue()

        def work(w):
            (c, cost) = w
            run_command(c)
            q.put(cost)

        res = self.pool.map_async(work, commands)

        total = sum([cost for (_, cost) in commands])
        progress = 0
        sys.stdout.write("Exporting data: 0.00%")
        sys.stdout.flush()
        for i in range(len(commands)):
            progress += q.get()
            sys.stdout.write(
                "\rExporting data: {0:.2f}%".format(100.0 * progress / total)
            )
            sys.stdout.flush()
        sys.stdout.write("\n")
        sys.stdout.flush()

        res.get()

        if self.working_path != self.lpis_path:
            print("Moving exported table")
            shutil.copy2(gpkg_working, gpkg)

    def get_tile_parcel_counts(self, srid):
        print("Counting parcels")
        with self.get_connection() as conn:
            with conn.cursor() as cursor:
                query = SQL(
                    """
with tiles as (
    select tile_id, ST_Transform(geom, %s) as geom
    from shape_tiles_s2
    where tile_id = any(%s)
)
select tile_id, (
    select count(*)
    from {} lpis
    where not lpis.is_deleted
        and ST_Intersects(lpis.wkb_geometry, tiles.geom)
) as count
from tiles;"""
                ).format(Identifier(self.lpis_table))
                logging.debug(query.as_string(conn))
                tiles = [t.tile_id for t in self.tiles]
                cursor.execute(query, (srid, tiles))

                counts = {}
                for r in cursor:
                    counts[r[0]] = r[1]

                conn.commit()
                return counts

    def get_overlapping_parcels(self, srid, q, tile):
        with self.get_connection() as conn:
            with conn.cursor() as cursor:
                query = SQL(
                    """
with tile as (
    select ST_Transform(geom, %s) as geom
    from shape_tiles_s2
    where tile_id = %s
)
select "NewID"
from {} lpis, tile
where "GeomValid"
and not is_deleted
and exists (
    select 1
    from {} t
    where t."NewID" != lpis."NewID"
    and t."GeomValid"
    and not t.is_deleted
    and ST_Intersects(t.wkb_geometry, tile.geom)
    and ST_Intersects(t.wkb_geometry, lpis.wkb_geometry)
    having sum(ST_Area(ST_Intersection(t.wkb_geometry, lpis.wkb_geometry))) / nullif(lpis."Area_meters", 0) > 0.1
)
and ST_Intersects(lpis.wkb_geometry, tile.geom);"""
                )
                query = query.format(
                    Identifier(self.lpis_table), Identifier(self.lpis_table)
                )
                logging.debug(query.as_string(conn))
                cursor.execute(query, (srid, tile.tile_id))

                q.put(tile)
                return [r[0] for r in cursor]

    def get_duplicate_parcels(self, srid, q, tile):
        with self.get_connection() as conn:
            with conn.cursor() as cursor:
                query = SQL(
                    """
with tile as (
    select ST_Transform(geom, %s) as geom
    from shape_tiles_s2
    where tile_id = %s
)
select "NewID"
from (
    select "NewID",
            count(*) over(partition by wkb_geometry) as count
    from {}, tile
    where ST_Intersects(wkb_geometry, tile.geom)
        and not is_deleted
) t where count > 1;"""
                )
                query = query.format(Identifier(self.lpis_table))
                logging.debug(query.as_string(conn))
                cursor.execute(query, (srid, tile.tile_id))

                q.put(tile)
                return [r[0] for r in cursor]

    def mark_overlapping_parcels(self, parcels):
        total = len(parcels)
        if not total:
            return
        progress = 0
        sys.stdout.write("Marking overlapping parcels: 0.00%")
        sys.stdout.flush()
        with self.get_connection() as conn:
            with conn.cursor() as cursor:
                for b in batch(parcels, self.DB_UPDATE_BATCH_SIZE):
                    sql = SQL('update {} set "Overlap" = true where "NewID" = any(%s)')
                    sql = sql.format(Identifier(self.lpis_table))
                    logging.debug(sql.as_string(conn))
                    cursor.execute(sql, (b,))
                    conn.commit()

                    progress += len(b)
                    sys.stdout.write(
                        "\rMarking overlapping parcels: {0:.2f}%".format(
                            100.0 * progress / total
                        )
                    )
                    sys.stdout.flush()
                sys.stdout.write("\n")
                sys.stdout.flush()

    def mark_duplicate_parcels(self, parcels):
        total = len(parcels)
        if not total:
            return
        progress = 0
        sys.stdout.write("Marking duplicate parcels: 0.00%")
        sys.stdout.flush()
        with self.get_connection() as conn:
            with conn.cursor() as cursor:
                for b in batch(parcels, self.DB_UPDATE_BATCH_SIZE):
                    sql = SQL('update {} set "Duplic" = true where "NewID" = any(%s)')
                    sql = sql.format(Identifier(self.lpis_table))
                    logging.debug(sql.as_string(conn))
                    cursor.execute(sql, (b,))
                    conn.commit()

                    progress += len(b)
                    sys.stdout.write(
                        "\rMarking duplicate parcels: {0:.2f}%".format(
                            100.0 * progress / total
                        )
                    )
                    sys.stdout.flush()
                sys.stdout.write("\n")
                sys.stdout.flush()


def batch(iterable, n=1):
    count = len(iterable)
    for first in range(0, count, n):
        yield iterable[first : min(first + n, count)]


def get_lpis_path(conn, site_id):
    with conn.cursor() as cursor:
        query = SQL(
            """
select value
from sp_get_parameters('processor.lpis.path')
where site_id is null or site_id = %s
order by site_id;"""
        )
        cursor.execute(query, (site_id,))

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
    parser = argparse.ArgumentParser(description="Imports an LPIS or LUT")
    parser.add_argument(
        "-c",
        "--config-file",
        default="/etc/sen2agri/sen2agri.conf",
        help="configuration file location",
    )
    parser.add_argument("--year", help="year", type=int, default=date.today().year)
    parser.add_argument("--lpis", help="LPIS dataset")
    parser.add_argument("--lut", help="LUT dataset")
    parser.add_argument("--export", help="export dataset", action="store_true")
    parser.add_argument("--parcel-id-cols", nargs="+", help="parcel id columns")
    parser.add_argument("--holding-id-cols", nargs="+", help="holding id columns")
    parser.add_argument("--crop-code-col", help="crop code column")

    required_args = parser.add_argument_group("required named arguments")
    required_args.add_argument("-s", "--site-id", type=int, required=True, help="site ID to filter by")
    parser.add_argument(
        "-m",
        "--mode",
        help="run mode",
        default="update",
        choices=["update", "replace", "incremental"],
    )
    parser.add_argument("-d", "--debug", help="debug mode", action="store_true")
    parser.add_argument("--working-path", help="working path")
    parser.add_argument("--parcel-id-offset", help="offset for parcel renumbering", type=int, default=0)
    parser.add_argument("--holding-id-offset", help="offset for holding renumbering", type=int, default=0)

    args = parser.parse_args()

    if args.debug:
        level = logging.DEBUG
    else:
        level = logging.INFO

    if args.mode == "replace":
        mode = MODE_REPLACE
    elif args.mode == "incremental":
        mode = MODE_INCREMENTAL
    else:
        mode = MODE_UPDATE

    logging.basicConfig(level=level)

    config = Config(args)
    data_preparation = DataPreparation(config, args.year, args.working_path)

    if args.lut is not None:
        data_preparation.prepare_lut(args.lut)

    if args.lpis is not None:
        data_preparation.prepare_lpis_staging(
            mode,
            args.parcel_id_cols,
            args.holding_id_cols,
            args.crop_code_col,
            args.lpis,
            args.parcel_id_offset,
            args.holding_id_offset,
        )
        data_preparation.prepare_lpis()

    if args.lpis or args.lut or args.export:
        data_preparation.export_lpis()


if __name__ == "__main__":
    main()
