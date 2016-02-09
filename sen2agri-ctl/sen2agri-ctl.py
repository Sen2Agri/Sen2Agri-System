#!/usr/bin/env python
from __future__ import print_function
import argparse
import dbus
import json
import numbers
import sys
import psycopg2
import psycopg2.extras


class Site(object):

    def __init__(self, site_id, name):
        self.site_id = site_id
        self.name = name

    def __cmp__(self, other):
        if hasattr(other, 'site_id'):
            return self.site_id.__cmp__(other.site_id)


class NewJob(object):

    def __init__(self, name, description, processor_id, site_id, start_type,
                 parameters, configuration):
        self.name = name
        self.description = description
        self.processor_id = processor_id
        self.site_id = site_id
        self.start_type = start_type
        self.parameters = parameters
        self.configuration = configuration


class Sen2AgriClient(object):

    def __init__(self):
        bus = dbus.SystemBus()
        self.proxy = bus.get_object('org.esa.sen2agri.persistenceManager',
                                    '/org/esa/sen2agri/persistenceManager')

    def get_sites(self):
        connection = self.get_connection()
        cur = self.get_cursor(connection)
        cur.execute("""SELECT * FROM sp_get_sites()""")
        rows = cur.fetchall()
        connection.commit()

        sites = []
        for row in rows:
            sites.append(Site(row['id'], row['name']))

        return sorted(sites)

    def submit_job(self, job):
        connection = self.get_connection()
        cur = self.get_cursor(connection)
        cur.execute("""SELECT * FROM sp_submit_job(%(name)s :: character varying, %(description)s ::
        character varying, %(processor_id)s :: smallint,
                       %(site_id)s :: smallint, %(start_type_id)s :: smallint, %(parameters)s ::
                       json, %(configuration)s :: json)""",
                    {"name": job.name,
                     "description": job.description,
                     "processor_id": job.processor_id,
                     "site_id": job.site_id,
                     "start_type_id": job.start_type,
                     "parameters": job.parameters,
                     "configuration": json.JSONEncoder().encode([dict(c) for c in job.configuration]) # [{"key": c.key, "value": c.value} for c in job.configuration]
                    })
        rows = cur.fetchall()
        connection.commit()

        jobId = rows[0][0]

        bus = dbus.SystemBus()
        orchestrator_proxy = bus.get_object('org.esa.sen2agri.orchestrator',
                                            '/org/esa/sen2agri/orchestrator')
        orchestrator_proxy.NotifyEventsAvailable()

        return jobId

    def get_connection(self):
        return psycopg2.connect(
            "dbname='sen2agri' user='admin' host='localhost' password='sen2agri'")

    def get_cursor(self, connection):
        return connection.cursor(cursor_factory=psycopg2.extras.DictCursor)


class Sen2AgriCtl(object):

    def __init__(self):
        self.client = Sen2AgriClient()

        parser = argparse.ArgumentParser(
            description="Controls the Sen2Agri system")
        subparsers = parser.add_subparsers()

        parser_list_sites = subparsers.add_parser(
            'list-sites', help="Lists the available sites")
        parser_list_sites.set_defaults(func=self.list_sites)

        parser_submit_job = subparsers.add_parser(
            'submit-job', help="Submits a new job")
        parser_submit_job.add_argument('-s', '--site',
                                       required=True, help="site")
        parser_submit_job_subparsers = parser_submit_job.add_subparsers()

        parser_composite = parser_submit_job_subparsers.add_parser(
            'composite', help="Submits a new composite type job")
        parser_composite.add_argument('-i', '--input',
                                      nargs='+', required=True,
                                      help="input products")
        parser_composite.add_argument(
            '--synthesis-date',
            required=True, help="The synthesis date (YYYYMMDD)")
        parser_composite.add_argument(
            '--half-synthesis',
            required=True, help="Half synthesis interval in days")
        parser_composite.add_argument(
            '--composite', help="composite")
        parser_composite.add_argument(
            '--resolution', type=int, help="resolution in m")
        parser_composite.add_argument(
            '-p', '--parameter', action='append', nargs=2,
            metavar=('KEY', 'VALUE'), help="override configuration parameter")
        parser_composite.set_defaults(func=self.submit_composite)

        parser_lai = parser_submit_job_subparsers.add_parser(
            'lai', help="Submits a new LAI retrieval type job")
        parser_lai.add_argument('-i', '--input',
                                      nargs='+', required=True,
                                      help="input products")
        parser_lai.add_argument(
            '--lai', help="lai")
        parser_lai.add_argument(
            '--resolution', type=int, help="resolution in m")
        parser_lai.add_argument(
            '-p', '--parameter', action='append', nargs=2,
            metavar=('KEY', 'VALUE'), help="override configuration parameter")
        parser_lai.set_defaults(func=self.submit_lai)

        parser_pheno_ndvi = parser_submit_job_subparsers.add_parser(
            'phenondvi', help="Submits a new Phenological NDVI Metrics type job")
        parser_pheno_ndvi.add_argument('-i', '--input',
                                       nargs='+', required=True,
                                       help="input products")
        parser_pheno_ndvi.add_argument(
            '--phenondvi', help="phenondvi")
        parser_pheno_ndvi.add_argument(
            '--resolution', type=int, help="resolution in m")
        parser_pheno_ndvi.add_argument(
            '-p', '--parameter', action='append', nargs=2,
            metavar=('KEY', 'VALUE'), help="override configuration parameter")
        parser_pheno_ndvi.set_defaults(func=self.submit_pheno_ndvi)

        parser_crop_mask = parser_submit_job_subparsers.add_parser(
            'crop-mask', help="Submits a new crop mask job")
        parser_crop_mask.add_argument('-i', '--input',
                                      nargs='+', required=True,
                                      help='input products')
        parser_crop_mask.add_argument('-r', '--reference',
                                      required=True, metavar="SHAPEFILE",
                                      help="reference polygons")
        parser_crop_mask.add_argument(
            '--date-start',
            required=True, help="temporal resampling start date (YYYYMMDD)")
        parser_crop_mask.add_argument(
            '--date-end',
            required=True, help="temporal resampling end date (YYYYMMDD)")
        parser_crop_mask.add_argument(
            '--resolution', type=int, help="resolution in m")
        parser_crop_mask.add_argument(
            '-p', '--parameter', action='append', nargs=2,
            metavar=('KEY', 'VALUE'), help="override configuration parameter")
        parser_crop_mask.set_defaults(func=self.submit_crop_mask)

        parser_crop_type = parser_submit_job_subparsers.add_parser(
            'crop-type', help="Submits a new crop type job")
        parser_crop_type.add_argument('-i', '--input',
                                      nargs='+', required=True,
                                      help="input products")
        parser_crop_type.add_argument(
            '-r', '--reference',
            required=True, metavar="SHAPEFILE", help="reference polygons")
        parser_crop_type.add_argument(
            '--date-start',
            required=True, help="temporal resampling start date (YYYYMMDD)")
        parser_crop_type.add_argument(
            '--date-end',
            required=True, help="temporal resampling end date (YYYYMMDD)")
        parser_crop_type.add_argument(
            '--crop-mask', help="crop mask")
        parser_crop_type.add_argument(
            '--resolution', type=int, help="resolution in m")
        parser_crop_type.add_argument(
            '-p', '--parameter', action='append', nargs=2,
            metavar=('KEY', 'VALUE'), help="override configuration parameter")
        parser_crop_type.set_defaults(func=self.submit_crop_type)

        args = parser.parse_args(sys.argv[1:])
        args.func(args)

    def list_sites(self, args):
        for site in self.client.get_sites():
            print("{} {}".format(site.site_id, site.name))

    def submit_composite(self, args):
        parameters = {'input_products': args.input,
                      'synthesis_date': args.synthesis_date,
                      'half_synthesis': args.half_synthesis}

        if args.resolution:
            parameters['resolution'] = args.resolution

        job = self.create_job(1, parameters, args)
        job_id = self.client.submit_job(job)

	print("Submitted job {}".format(job_id))

    def submit_lai(self, args):
        parameters = {'input_products': args.input}

        if args.resolution:
            parameters['resolution'] = args.resolution

        job = self.create_job(2, parameters, args)
        job_id = self.client.submit_job(job)

	print("Submitted job {}".format(job_id))

    def submit_pheno_ndvi(self, args):
        parameters = {'input_products': args.input}

        if args.resolution:
            parameters['resolution'] = args.resolution

        job = self.create_job(3, parameters, args)
        job_id = self.client.submit_job(job)

	print("Submitted job {}".format(job_id))

    def submit_crop_mask(self, args):
        parameters = {'input_products': args.input,
                      'reference_polygons': args.reference,
                      'date_start': args.date_start,
                      'date_end': args.date_end}

        if args.resolution:
            parameters['resolution'] = args.resolution

        job = self.create_job(4, parameters, args)
        job_id = self.client.submit_job(job)

	print("Submitted job {}".format(job_id))

    def submit_crop_type(self, args):
        parameters = {'input_products': args.input,
                      'reference_polygons': args.reference,
                      'date_start': args.date_start,
                      'date_end': args.date_end}

        if args.crop_mask:
            parameters['crop_mask'] = args.crop_mask
        if args.resolution:
            parameters['resolution'] = args.resolution

        job = self.create_job(5, parameters, args)
        job_id = self.client.submit_job(job)

	print("Submitted job {}".format(job_id))

    def create_job(self, processor_id, parameters, args):
        config = config_from_parameters(args.parameter)

        site_id = self.get_site_id(args.site)
        if site_id is None:
            raise RuntimeError("Invalid site '{}'".format(args.site))

        job = NewJob("", "", processor_id, site_id, 2,
                     json.JSONEncoder().encode(parameters), config)
        return job

    def get_site_id(self, name):
        if isinstance(name, numbers.Integral):
            return name

        sites = self.client.get_sites()
        for site in sites:
            if site.name == name:
                return site.site_id

        return None


def config_from_parameters(parameters):
    config = []
    if parameters:
        for param in parameters:
            config.append({'key': param[0], 'value': param[1]})
    return config

if __name__ == '__main__':
    try:
        Sen2AgriCtl()
    except Exception, err:
        print("ERROR:", err, file=sys.stderr)
