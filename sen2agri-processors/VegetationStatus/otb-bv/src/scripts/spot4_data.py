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
# -*- coding: utf-8 -*-
# =========================================================================
#   Program:   otb-bv
#   Language:  python
#
#   Copyright (c) CESBIO. All rights reserved.
#
#   See otb-bv-copyright.txt for details.
#
#   This software is distributed WITHOUT ANY WARRANTY; without even
#   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#   PURPOSE.  See the above copyright notices for more information.
#
# =========================================================================

import os

gt = [{'crop':"wheat",'esu':1,'doy':107,'y':2013,'gai':4.598938342,'fapar':0.952820346,'fcover':0.743817789,'refls':[37,27,353,101],'lai-bvnet':3.333333333,'fapar-bvnet':0.84,'fcover-bvnet':0.78},
{'crop':"wheat",'esu':15,'doy':107,'y':2013,'gai':4.030745706,'fapar':0.906458612,'fcover':0.423831553,'refls':[44,36,301,113],'lai-bvnet':2.333333333,'fapar-bvnet':0.74,'fcover-bvnet':0.66},
{'crop':"wheat",'esu':3,'doy':107,'y':2013,'gai':1.963504272,'fapar':0.757541529,'fcover':0.421095642,'refls':[42,34,285,109],'lai-bvnet':2.233333333,'fapar-bvnet':0.72,'fcover-bvnet':0.63},
{'crop':"wheat",'esu':9,'doy':107,'y':2013,'gai':3.892803681,'fapar':0.910040766,'fcover':0.629218887,'refls':[35,29,381,119],'lai-bvnet':3.433333333,'fapar-bvnet':0.86,'fcover-bvnet':0.82},
{'crop':"corn",'esu':8,'doy':137,'y':2013,'gai':0.043186459,'fapar':0.038940853,'fcover':0.04425332,'refls':[74,98,159,191],'lai-bvnet':0.2,'fapar-bvnet':0.11,'fcover-bvnet':0.1},
{'crop':"corn",'esu':8,'doy':147,'y':2013,'gai':0.187780948,'fapar':0.124884862,'fcover':0.109114207,'refls':[82,116,161,211],'lai-bvnet':0.166666667,'fapar-bvnet':0.06,'fcover-bvnet':0.06},
{'crop':"wheat",'esu':1,'doy':158,'y':2013,'gai':2.908603306,'fapar':0.849761551,'fcover':0.567338346,'refls':[36,39,343,133],'lai-bvnet':2.433333333,'fapar-bvnet':0.72,'fcover-bvnet':0.67},
{'crop':"wheat",'esu':2,'doy':158,'y':2013,'gai':2.631727382,'fapar':0.871692187,'fcover':0.735462693,'refls':[38,42,313,136],'lai-bvnet':2.066666667,'fapar-bvnet':0.66,'fcover-bvnet':0.61},
{'crop':"wheat",'esu':3,'doy':158,'y':2013,'gai':2.264815271,'fapar':0.821427243,'fcover':0.664233327,'refls':[31,31,376,139],'lai-bvnet':2.8,'fapar-bvnet':0.77,'fcover-bvnet':0.73},
{'crop':"wheat",'esu':9,'doy':158,'y':2013,'gai':2.895295362,'fapar':0.810735955,'fcover':0.606327087,'refls':[30,31,458,130],'lai-bvnet':3.9,'fapar-bvnet':0.88,'fcover-bvnet':0.86},
{'crop':"corn",'esu':1,'doy':158,'y':2013,'gai':0.083441602,'fapar':0.074986814,'fcover':0.069875309,'refls':[171,166,273,284],'lai-bvnet':0.333333333,'fapar-bvnet':0.18,'fcover-bvnet':0.17},
{'crop':"corn",'esu':2,'doy':158,'y':2013,'gai':0.102669005,'fapar':0.073588721,'fcover':0.076962938,'refls':[173,166,278,292],'lai-bvnet':0.366666667,'fapar-bvnet':0.18,'fcover-bvnet':0.18},
{'crop':"corn",'esu':4,'doy':158,'y':2013,'gai':0.060109731,'fapar':0.058074827,'fcover':0.044188193,'refls':[186,167,343,339],'lai-bvnet':0.566666667,'fapar-bvnet':0.27,'fcover-bvnet':0.26},
{'crop':"corn",'esu':5,'doy':158,'y':2013,'gai':0.167574996,'fapar':0.132010048,'fcover':0.099260877,'refls':[142,165,261,274],'lai-bvnet':0.233333333,'fapar-bvnet':0.16,'fcover-bvnet':0.14},
{'crop':"corn",'esu':6,'doy':158,'y':2013,'gai':0.124369197,'fapar':0.104253135,'fcover':0.070485264,'refls':[187,167,396,334],'lai-bvnet':0.733333333,'fapar-bvnet':0.36,'fcover-bvnet':0.36},
{'crop':"wheat",'esu':1,'doy':163,'y':2013,'gai':2.165232676,'fapar':0.794602254,'fcover':0.638557465,'refls':[55,54,297,130],'lai-bvnet':1.8,'fapar-bvnet':0.6,'fcover-bvnet':0.56},
{'crop':"wheat",'esu':15,'doy':163,'y':2013,'gai':0.967860722,'fapar':0.592375939,'fcover':0.521565504,'refls':[58,60,257,140],'lai-bvnet':1.266666667,'fapar-bvnet':0.5,'fcover-bvnet':0.46},
{'crop':"wheat",'esu':2,'doy':163,'y':2013,'gai':2.18227355,'fapar':0.811441219,'fcover':0.691013161,'refls':[55,56,279,135],'lai-bvnet':1.533333333,'fapar-bvnet':0.56,'fcover-bvnet':0.52},
{'crop':"wheat",'esu':3,'doy':163,'y':2013,'gai':2.076944515,'fapar':0.829778321,'fcover':0.714586019,'refls':[53,50,341,139],'lai-bvnet':2.133333333,'fapar-bvnet':0.68,'fcover-bvnet':0.65},
{'crop':"wheat",'esu':9,'doy':163,'y':2013,'gai':2.041852817,'fapar':0.767525933,'fcover':0.666375922,'refls':[44,40,411,127],'lai-bvnet':3.233333333,'fapar-bvnet':0.82,'fcover-bvnet':0.79},
{'crop':"corn",'esu':1,'doy':163,'y':2013,'gai':0.243540661,'fapar':0.166113169,'fcover':0.126042967,'refls':[141,160,256,250],'lai-bvnet':0.266666667,'fapar-bvnet':0.17,'fcover-bvnet':0.15},
{'crop':"corn",'esu':2,'doy':163,'y':2013,'gai':0.303506056,'fapar':0.185329255,'fcover':0.133090191,'refls':[143,160,264,263],'lai-bvnet':0.3,'fapar-bvnet':0.18,'fcover-bvnet':0.16},
{'crop':"corn",'esu':4,'doy':163,'y':2013,'gai':0.132614424,'fapar':0.106701983,'fcover':0.070109962,'refls':[134,155,267,234],'lai-bvnet':0.3,'fapar-bvnet':0.2,'fcover-bvnet':0.19},
{'crop':"corn",'esu':5,'doy':163,'y':2013,'gai':0.446715853,'fapar':0.271343457,'fcover':0.198634831,'refls':[118,141,251,241],'lai-bvnet':0.333333333,'fapar-bvnet':0.2,'fcover-bvnet':0.18},
{'crop':"corn",'esu':6,'doy':163,'y':2013,'gai':0.259791343,'fapar':0.1892929,'fcover':0.140685949,'refls':[111,128,247,223],'lai-bvnet':0.4,'fapar-bvnet':0.23,'fcover-bvnet':0.22},
{'crop':"corn",'esu':8,'doy':163,'y':2013,'gai':0.713809669,'fapar':0.381491964,'fcover':0.267078055,'refls':[98,116,277,253],'lai-bvnet':0.533333333,'fapar-bvnet':0.3,'fcover-bvnet':0.27},
{'crop':"corn",'esu':4,'doy':167,'y':2013,'gai':0.190618178,'fapar':0.145603708,'fcover':0.090847378,'refls':[134,162,257,267],'lai-bvnet':0.233333333,'fapar-bvnet':0.16,'fcover-bvnet':0.13},
{'crop':"corn",'esu':5,'doy':167,'y':2013,'gai':0.670028538,'fapar':0.382810185,'fcover':0.278133994,'refls':[82,101,229,206],'lai-bvnet':0.5,'fapar-bvnet':0.27,'fcover-bvnet':0.26},
{'crop':"corn",'esu':6,'doy':167,'y':2013,'gai':0.36812906,'fapar':0.257324711,'fcover':0.196846497,'refls':[141,162,315,263],'lai-bvnet':0.4,'fapar-bvnet':0.28,'fcover-bvnet':0.25},
{'crop':"corn",'esu':8,'doy':167,'y':2013,'gai':0.845316849,'fapar':0.445643739,'fcover':0.306569017,'refls':[54,63,221,189],'lai-bvnet':0.766666667,'fapar-bvnet':0.37,'fcover-bvnet':0.34}]

rsr_dir = os.environ['HOME']+"/Dev/otb-bv/data/"
spot4_rsr = rsr_dir+"spot4hrvir1.rsr"

spot4_data = ["spot4", spot4_rsr]

spot4_107 = {'doy':107, 'to':13.71259947, 'po':-76.57636044, 'ts':42.85287493, 'ps':132.8643351, 'gt':[s for s in gt if s['doy']==107]}
spot4_data.append(spot4_107)
spot4_137 = {'doy':137, 'to':11.48903591, 'po':-76.89109332, 'ts':35.45797813, 'ps':123.7352241, 'gt':[s for s in gt if s['doy']==137]}
spot4_data.append(spot4_137)
spot4_147 = {'doy':147, 'to':11.80717999, 'po': -76.8844394, 'ts':34.08922593, 'ps':120.8016084, 'gt':[s for s in gt if s['doy']==147]}
spot4_data.append(spot4_147)
spot4_158 = {'doy':158, 'to':13.12962672, 'po': 100.3004711, 'ts':36.10594062, 'ps':113.8471485, 'gt':[s for s in gt if s['doy']==158]}
spot4_data.append(spot4_158)
spot4_163 = {'doy':163, 'to':13.12976268, 'po': 100.2980078, 'ts':36.03377624, 'ps':112.9455688, 'gt':[s for s in gt if s['doy']==163]}
spot4_data.append(spot4_163)
spot4_167 = {'doy':167, 'to':11.49005525, 'po': -76.8844394, 'ts':33.37841681, 'ps': 116.698214, 'gt':[s for s in gt if s['doy']==167]}
spot4_data.append(spot4_167)
