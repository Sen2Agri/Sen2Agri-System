#!/usr/bin/env python
from __future__ import print_function

import argparse
import csv
from datetime import date
import psycopg2
from psycopg2.sql import SQL, Literal, Identifier
import psycopg2.extras
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
    parser = argparse.ArgumentParser(description="Crops and recompresses S1 L2A products")
    parser.add_argument('-c', '--config-file', default='/etc/sen2agri/sen2agri.conf', help="configuration file location")
    parser.add_argument('-s', '--site-id', type=int, help="site ID to filter by")
    parser.add_argument('-p', '--path', default='.', help="working path")
    parser.add_argument('-y', '--year', help="year")

    args = parser.parse_args()

    config = Config(args)

    with psycopg2.connect(host=config.host, port=config.port, dbname=config.dbname, user=config.user, password=config.password) as conn:
        site_name = get_site_name(conn, config.site_id)
        year = args.year or date.today().year
        lpis_table = "decl_{}_{}".format(site_name, year)
        lut_table = "lut_{}_{}".format(site_name, year)

        with conn.cursor() as cursor:
            query = SQL(
                """
                select lpis."NewID",
                    lpis."Area_meters" as "AREA",
                    lut."ctnuml4a" as "CTnum",
                    lpis."LC"
                from {} lpis
                inner join {} lut on lut.ctnum :: int = lpis."CTnum"
                where lpis."LC" in (1, 2, 3, 4)
                and lpis."S1Pix" > 0
                and lpis."S2Pix" > 2
                and "GeomValid"
                and not "Duplic"
                and not "Overlap"
                order by "NewID"
                """)
            query = query.format(Identifier(lpis_table), Identifier(lut_table))
            print(query.as_string(conn))

            outfile = "parcels.csv"
            with open(outfile, 'wb') as csvfile:
                writer = csv.writer(csvfile, quoting=csv.QUOTE_MINIMAL)

                cursor.execute(query)
                writer.writerow(['NewID', 'AREA', 'CTnum', 'LC'])
                for row in cursor:
                    writer.writerow(row)

            conn.commit()


if __name__ == "__main__":
    main()
