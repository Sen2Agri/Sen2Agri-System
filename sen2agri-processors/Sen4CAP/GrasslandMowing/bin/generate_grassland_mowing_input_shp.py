#!/usr/bin/env python
from __future__ import print_function

import argparse
from datetime import date
import multiprocessing.dummy
import os
import os.path
from osgeo import osr
from osgeo import ogr
from gdal import gdalconst
import pipes
import psycopg2
from psycopg2.sql import SQL, Literal, Identifier
import psycopg2.extras
import subprocess
from time import sleep

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import ConfigParser

class NewFieldDef(object):
    def __init__(self, name, type, defVal):
        self.name = name
        self.type = type
        self.defVal = defVal

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

def main():
    # Preserve encoding to UTF-8 
    os.environ['SHAPE_ENCODING'] = "utf-8"

    newFields = [NewFieldDef("mow_n", ogr.OFTInteger, "0"), 
                NewFieldDef("m1_dstart", ogr.OFTString, "0"), 
                NewFieldDef("m1_dend", ogr.OFTString, "0"), 
                NewFieldDef("m1_conf", ogr.OFTReal, "0"), 
                NewFieldDef("m1_mis", ogr.OFTString, "0"), 
                NewFieldDef("m2_dstart", ogr.OFTString, "0"), 
                NewFieldDef("m2_dend", ogr.OFTString, "0"), 
                NewFieldDef("m2_conf", ogr.OFTReal, "0"), 
                NewFieldDef("m2_mis", ogr.OFTString, "0"), 
                NewFieldDef("m3_dstart", ogr.OFTString, "0"), 
                NewFieldDef("m3_dend", ogr.OFTString, "0"), 
                NewFieldDef("m3_conf", ogr.OFTReal, "0"), 
                NewFieldDef("m3_mis", ogr.OFTString, "0"), 
                NewFieldDef("m4_dstart", ogr.OFTString, "0"), 
                NewFieldDef("m4_dend", ogr.OFTString, "0"), 
                NewFieldDef("m4_conf", ogr.OFTReal, "0"), 
                NewFieldDef("m4_mis", ogr.OFTString, "0"), 
                NewFieldDef("proc", ogr.OFTInteger, "0"), 
                NewFieldDef("compl", ogr.OFTInteger, "0")]

    parser = argparse.ArgumentParser(description="Creates the grassland mowing input shapefile")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="site ID to filter by")
    parser.add_argument('-p', '--path', default='.', help="working path")
    parser.add_argument('-y', '--year', help="year")
    parser.add_argument('-f', '--filter-ctnum', default="", help="Filtering CTnum fields")
    parser.add_argument('--force', help="overwrite field", action='store_true')
    
    args = parser.parse_args()

    config = Config(args)
    pool = multiprocessing.dummy.Pool(8)

    pg_path = 'PG:dbname={} host={} port={} user={} password={}'.format(config.dbname, config.host,
                                                                        config.port, config.user, config.password)

    with psycopg2.connect(host=config.host, port=config.port, dbname=config.dbname, user=config.user, password=config.password) as conn:
        site_name = get_site_name(conn, config.site_id)
        year = args.year or date.today().year
        lpis_table = "decl_{}_{}".format(site_name, year)
        lut_table = "lut_{}_{}".format(site_name, year)

        commands = []
        shp = args.path
        ctnumFilter = ""
       
        ctnums = []
        if args.filter_ctnum : 
            ctnums = map(int, args.filter_ctnum.split(','))
        if len(ctnums) > 0 : 
            sql = "select \"NewID\", ori_hold as \"Ori_hold\", ori_id as \"Ori_id\", ori_crop as \"Ori_crop\", wkb_geometry from {} natural join {} where ctnum in ({})".format(lpis_table, lut_table, ', '.join(str(x) for x in ctnums))
        else :
            sql = SQL('select "NewID", ori_hold as "Ori_hold", ori_id as "Ori_id", ori_crop as "Ori_crop", wkb_geometry from {} natural join {}')
            sql = sql.format(Identifier(lpis_table), Identifier(lut_table))
            sql = sql.as_string(conn)

        command = []
        command += ["ogr2ogr"]
        command += ["-sql", sql]
        command += [shp]
        command += [pg_path]
        command += ["-lco", "ENCODING=UTF-8"]        
        commands.append(command)

        print ("Starting executing the commands ...")
        pool.map(lambda c: run_command(c), commands)
        print ("Finished executing commands!")
        
        dataset = ogr.Open(shp, gdalconst.GA_Update)
        layer = dataset.GetLayer()
        feature_count = layer.GetFeatureCount()
        print("{} feature(s) found".format(feature_count))

        for newField in newFields:
            dataset.StartTransaction()
            print("Adding new field {}, type {} with default value {}".format(newField.name, newField.type, newField.defVal))
            field_idx = layer.FindFieldIndex(newField.name, False)
            if field_idx != -1:
                if args.force:
                    print("Field {} already exists, removing it".format(newField.name))
                    layer.DeleteField(field_idx)
                else:
                    print("Field {} already exists, ignoring".format(newField.name))
                    continue

            layer.CreateField(ogr.FieldDefn(newField.name, newField.type))
            dataset.CommitTransaction()

        print("Setting default values ...")
        dataset.StartTransaction()        
        for feature in layer:
            for newField in newFields:
                field_idx = layer.FindFieldIndex(newField.name, False)
                feature.SetField(field_idx, newField.defVal)
                
            layer.SetFeature(feature)
                
        dataset.CommitTransaction()
    
    print("Done!")

if __name__ == "__main__":
    main()
