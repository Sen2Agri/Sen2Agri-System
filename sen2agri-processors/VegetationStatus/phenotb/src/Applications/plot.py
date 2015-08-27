#!/usr/bin/env python
# usage: python plot.py /tmp/p9.pdf /tmp/test-9*gp > /tmp/plots9.gp
#        gnuplot /tmp/plots9.gp
import sys

gplot_commands = [
    "set terminal pdf\n",
    "set output \""+sys.argv[1]+"\"\n"
    ]

for f in sys.argv[2:]:
    with open(f) as ff:
        gplot_commands.append(ff.readline()[1:-1])
        gplot_commands.append(ff.readline()[1:-1])

for l in gplot_commands:
    print l
    
