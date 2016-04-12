#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
_____________________________________________________________________________

   Program:      Sen2Agri-Processors
   Language:     Python
   Copyright:    2015-2016, CS Romania, office@c-s.ro
   See COPYRIGHT file for details.
   
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
_____________________________________________________________________________

"""
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
    
