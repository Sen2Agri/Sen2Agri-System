#!/usr/bin/env python
import argparse
import multiprocessing
import multiprocessing.dummy
import os
import pipes
import subprocess
import sys


def run_command(args, env=None):
    args = list(map(str, args))
    cmd_line = " ".join(map(pipes.quote, args))
    print(cmd_line)
    subprocess.call(args, env=env)
    return


def parse_args():
    parser = argparse.ArgumentParser(description='Create cloud-optimized GeoTIFFs from a directory')
    parser.add_argument('-n', '--num-threads', help='concurrency', type=int, default=0)
    parser.add_argument('input', help='directory')

    return parser.parse_args()


def main():
    args = parse_args()

    commands = []
    for (dirpath, _, files) in os.walk(args.input, followlinks=True):
        for file in files:
            path = os.path.join(dirpath, file)

            (base, ext) = os.path.splitext(path)
            base = base.lower()
            if ext.lower() != '.tif':
                continue
            if 'tmpcog' in base:
                continue

            if '_cld' in base or '_msk' in base or '_qlt' in base:
                resampler = 'nearest'
            else:
                resampler = 'average'

            no_data = 'none'
            if '_atb' in base:
                no_data = '0'
            elif '_fre' in base or '_sre' in base:
                no_data = -10000

            if sys.version_info[0] < 3:
                command = ['python']
            else:
                command = ['python2']

            command += ['optimize_gtiff.py', '--no-threaded', '-r', resampler, '--no-data', no_data, path]

            commands.append(command)

    num_threads = args.num_threads
    if num_threads == 0:
        num_threads = multiprocessing.cpu_count()

    pool = multiprocessing.dummy.Pool(processes=num_threads)
    pool.map(run_command, commands)
    pool.close()
    pool.join()


if __name__ == '__main__':
    sys.exit(main())
