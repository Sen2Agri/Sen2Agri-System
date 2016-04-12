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

gt = [{'crop':"corn",'esu':1,'doy':200,'y':2013,'gai':2.481259206,'fapar':0.813549481,'fcover':0.586696278,'refls':[39,21,378,116],'lai-bvnet':2.866666667,'fapar-bvnet':0.79,'fcover-bvnet':0.75},
{'crop':"corn",'esu':2,'doy':200,'y':2013,'gai':3.075730172,'fapar':0.847548732,'fcover':0.672593468,'refls':[35,18,402,108],'lai-bvnet':3.3,'fapar-bvnet':0.83,'fcover-bvnet':0.8},
{'crop':"corn",'esu':3,'doy':200,'y':2013,'gai':2.033727506,'fapar':0.633255142,'fcover':0.423766937,'refls':[50,39,266,130],'lai-bvnet':1.266666667,'fapar-bvnet':0.51,'fcover-bvnet':0.46},
{'crop':"corn",'esu':4,'doy':200,'y':2013,'gai':1.486024569,'fapar':0.631246894,'fcover':0.414016144,'refls':[51,33,347,157],'lai-bvnet':1.8,'fapar-bvnet':0.65,'fcover-bvnet':0.63},
{'crop':"corn",'esu':5,'doy':200,'y':2013,'gai':2.8828713,'fapar':0.820176588,'fcover':0.599178025,'refls':[33,17,374,102],'lai-bvnet':3.1,'fapar-bvnet':0.81,'fcover-bvnet':0.76},
{'crop':"corn",'esu':6,'doy':200,'y':2013,'gai':2.472011373,'fapar':0.765480779,'fcover':0.556433618,'refls':[38,23,384,127],'lai-bvnet':2.766666667,'fapar-bvnet':0.78,'fcover-bvnet':0.75},
{'crop':"corn",'esu':7,'doy':200,'y':2013,'gai':1.463665218,'fapar':0.6080123,'fcover':0.453734547,'refls':[63,44,327,159],'lai-bvnet':1.4,'fapar-bvnet':0.58,'fcover-bvnet':0.56},
{'crop':"corn",'esu':8,'doy':200,'y':2013,'gai':2.52987554,'fapar':0.803628548,'fcover':0.722453329,'refls':[38,21,375,120],'lai-bvnet':2.766666667,'fapar-bvnet':0.78,'fcover-bvnet':0.74},
{'crop':"corn",'esu':9,'doy':200,'y':2013,'gai':1.918117909,'fapar':0.652589993,'fcover':0.506725236,'refls':[46,30,319,131],'lai-bvnet':1.866666667,'fapar-bvnet':0.64,'fcover-bvnet':0.59},
{'crop':"sunflower",'esu':10,'doy':200,'y':2013,'gai':1.222677073,'fapar':0.612836933,'fcover':0.475518221,'refls':[74,61,314,166],'lai-bvnet':1.066666667,'fapar-bvnet':0.5,'fcover-bvnet':0.49},
{'crop':"sunflower",'esu':11,'doy':200,'y':2013,'gai':0.831387164,'fapar':0.465528236,'fcover':0.290700177,'refls':[81,67,319,176],'lai-bvnet':0.966666667,'fapar-bvnet':0.48,'fcover-bvnet':0.48},
{'crop':"sunflower",'esu':2,'doy':200,'y':2013,'gai':1.813847408,'fapar':0.789797762,'fcover':0.673015078,'refls':[69,51,398,180],'lai-bvnet':1.8,'fapar-bvnet':0.68,'fcover-bvnet':0.69},
{'crop':"sunflower",'esu':3,'doy':200,'y':2013,'gai':1.765405855,'fapar':0.800063705,'fcover':0.634930478,'refls':[70,51,446,189],'lai-bvnet':2.266666667,'fapar-bvnet':0.77,'fcover-bvnet':0.78},
{'crop':"sunflower",'esu':5,'doy':200,'y':2013,'gai':0.855768095,'fapar':0.487606781,'fcover':0.404891881,'refls':[102,99,342,216],'lai-bvnet':0.7,'fapar-bvnet':0.41,'fcover-bvnet':0.41},
{'crop':"sunflower",'esu':6,'doy':200,'y':2013,'gai':1.175265724,'fapar':0.613455124,'fcover':0.607330587,'refls':[88,82,325,208],'lai-bvnet':0.8,'fapar-bvnet':0.43,'fcover-bvnet':0.42},
{'crop':"sunflower",'esu':7,'doy':200,'y':2013,'gai':1.631998055,'fapar':0.739068757,'fcover':0.646487104,'refls':[81,60,447,211],'lai-bvnet':1.966666667,'fapar-bvnet':0.73,'fcover-bvnet':0.75},
{'crop':"sunflower",'esu':8,'doy':200,'y':2013,'gai':0.986370593,'fapar':0.590692269,'fcover':0.539212097,'refls':[92,88,309,205],'lai-bvnet':0.666666667,'fapar-bvnet':0.38,'fcover-bvnet':0.38},
{'crop':"sunflower",'esu':9,'doy':200,'y':2013,'gai':1.638871375,'fapar':0.779202742,'fcover':0.747042574,'refls':[74,48,439,202],'lai-bvnet':2.066666667,'fapar-bvnet':0.74,'fcover-bvnet':0.75},
{'crop':"corn",'esu':1,'doy':216,'y':2013,'gai':2.81639155,'fapar':0.853941161,'fcover':0.704709756,'refls':[46,29,449,151],'lai-bvnet':3.166666667,'fapar-bvnet':0.85,'fcover-bvnet':0.84},
{'crop':"corn",'esu':2,'doy':216,'y':2013,'gai':3.06016443,'fapar':0.875435162,'fcover':0.749832308,'refls':[40,26,440,132],'lai-bvnet':3.366666667,'fapar-bvnet':0.86,'fcover-bvnet':0.84},
{'crop':"corn",'esu':5,'doy':216,'y':2013,'gai':3.425444624,'fapar':0.883971492,'fcover':0.649939745,'refls':[39,26,385,121],'lai-bvnet':2.833333333,'fapar-bvnet':0.8,'fcover-bvnet':0.76},
{'crop':"corn",'esu':6,'doy':216,'y':2013,'gai':2.742962582,'fapar':0.854176295,'fcover':0.6951267,'refls':[43,30,400,144],'lai-bvnet':2.633333333,'fapar-bvnet':0.79,'fcover-bvnet':0.76},
{'crop':"sunflower",'esu':1,'doy':216,'y':2013,'gai':1.326486831,'fapar':0.663107323,'fcover':0.511324604,'refls':[95,93,311,188],'lai-bvnet':0.733333333,'fapar-bvnet':0.41,'fcover-bvnet':0.39},
{'crop':"sunflower",'esu':10,'doy':216,'y':2013,'gai':1.012454769,'fapar':0.539880804,'fcover':0.420468142,'refls':[85,81,260,165],'lai-bvnet':0.7,'fapar-bvnet':0.36,'fcover-bvnet':0.34},
{'crop':"sunflower",'esu':11,'doy':216,'y':2013,'gai':0.919422037,'fapar':0.494768934,'fcover':0.337163695,'refls':[95,89,325,188],'lai-bvnet':0.833333333,'fapar-bvnet':0.44,'fcover-bvnet':0.43},
{'crop':"sunflower",'esu':2,'doy':216,'y':2013,'gai':1.188117653,'fapar':0.547757833,'fcover':0.3522885,'refls':[101,103,279,193],'lai-bvnet':0.566666667,'fapar-bvnet':0.32,'fcover-bvnet':0.3},
{'crop':"sunflower",'esu':3,'doy':216,'y':2013,'gai':1.118913371,'fapar':0.600927832,'fcover':0.437674861,'refls':[91,80,345,194],'lai-bvnet':0.966666667,'fapar-bvnet':0.49,'fcover-bvnet':0.5},
{'crop':"sunflower",'esu':4,'doy':216,'y':2013,'gai':0.163903271,'fapar':0.151345209,'fcover':0.165101225,'refls':[219,250,407,376],'lai-bvnet':0.133333333,'fapar-bvnet':0.17,'fcover-bvnet':0.13},
{'crop':"sunflower",'esu':5,'doy':216,'y':2013,'gai':0.650884743,'fapar':0.370851774,'fcover':0.253289984,'refls':[131,149,283,228],'lai-bvnet':0.333333333,'fapar-bvnet':0.21,'fcover-bvnet':0.2},
{'crop':"sunflower",'esu':6,'doy':216,'y':2013,'gai':0.886466946,'fapar':0.546600337,'fcover':0.45932515,'refls':[95,93,306,197],'lai-bvnet':0.7,'fapar-bvnet':0.39,'fcover-bvnet':0.37},
{'crop':"sunflower",'esu':8,'doy':216,'y':2013,'gai':0.932189172,'fapar':0.557923017,'fcover':0.530918856,'refls':[97,100,297,205],'lai-bvnet':0.6,'fapar-bvnet':0.35,'fcover-bvnet':0.33},
{'crop':"sunflower",'esu':9,'doy':216,'y':2013,'gai':1.261127063,'fapar':0.680364088,'fcover':0.629624537,'refls':[82,65,360,175],'lai-bvnet':1.3,'fapar-bvnet':0.59,'fcover-bvnet':0.59},
{'crop':"corn",'esu':1,'doy':232,'y':2013,'gai':1.982291134,'fapar':0.79643041,'fcover':0.750222479,'refls':[47,30,423,147],'lai-bvnet':2.866666667,'fapar-bvnet':0.83,'fcover-bvnet':0.8},
{'crop':"corn",'esu':3,'doy':232,'y':2013,'gai':2.803043063,'fapar':0.889379961,'fcover':0.786037254,'refls':[38,26,388,132],'lai-bvnet':2.766666667,'fapar-bvnet':0.81,'fcover-bvnet':0.76},
{'crop':"corn",'esu':4,'doy':232,'y':2013,'gai':2.454698091,'fapar':0.844615132,'fcover':0.68667929,'refls':[40,26,394,144],'lai-bvnet':2.666666667,'fapar-bvnet':0.8,'fcover-bvnet':0.76},
{'crop':"corn",'esu':5,'doy':232,'y':2013,'gai':3.073245738,'fapar':0.895287889,'fcover':0.785111137,'refls':[39,25,380,122],'lai-bvnet':2.8,'fapar-bvnet':0.81,'fcover-bvnet':0.75},
{'crop':"corn",'esu':6,'doy':232,'y':2013,'gai':3.002186628,'fapar':0.870108184,'fcover':0.761658524,'refls':[44,30,402,150],'lai-bvnet':2.633333333,'fapar-bvnet':0.8,'fcover-bvnet':0.76},
{'crop':"corn",'esu':7,'doy':232,'y':2013,'gai':2.63148912,'fapar':0.875614932,'fcover':0.763047284,'refls':[42,28,397,144],'lai-bvnet':2.666666667,'fapar-bvnet':0.8,'fcover-bvnet':0.76},
{'crop':"corn",'esu':8,'doy':232,'y':2013,'gai':3.008190181,'fapar':0.883979265,'fcover':0.800332498,'refls':[46,32,344,149],'lai-bvnet':1.966666667,'fapar-bvnet':0.7,'fcover-bvnet':0.64},
{'crop':"corn",'esu':9,'doy':232,'y':2013,'gai':2.967963438,'fapar':0.862466963,'fcover':0.641886189,'refls':[36,22,416,131],'lai-bvnet':3.2,'fapar-bvnet':0.85,'fcover-bvnet':0.81},
{'crop':"sunflower",'esu':1,'doy':232,'y':2013,'gai':1.00996327,'fapar':0.538307063,'fcover':0.358627479,'refls':[106,103,295,179],'lai-bvnet':0.6,'fapar-bvnet':0.38,'fcover-bvnet':0.36},
{'crop':"sunflower",'esu':10,'doy':232,'y':2013,'gai':0.685249354,'fapar':0.424958953,'fcover':0.321600696,'refls':[92,88,261,172],'lai-bvnet':0.6,'fapar-bvnet':0.36,'fcover-bvnet':0.33},
{'crop':"sunflower",'esu':11,'doy':232,'y':2013,'gai':0.593192047,'fapar':0.346135198,'fcover':0.234726749,'refls':[113,118,290,203],'lai-bvnet':0.466666667,'fapar-bvnet':0.31,'fcover-bvnet':0.3},
{'crop':"sunflower",'esu':2,'doy':232,'y':2013,'gai':0.463542451,'fapar':0.275973343,'fcover':0.170547481,'refls':[115,122,251,202],'lai-bvnet':0.3,'fapar-bvnet':0.23,'fcover-bvnet':0.22},
{'crop':"sunflower",'esu':3,'doy':232,'y':2013,'gai':0.661582093,'fapar':0.393002469,'fcover':0.211135283,'refls':[113,113,292,206],'lai-bvnet':0.5,'fapar-bvnet':0.32,'fcover-bvnet':0.31},
{'crop':"sunflower",'esu':4,'doy':232,'y':2013,'gai':0.382526184,'fapar':0.319675935,'fcover':0.364276516,'refls':[159,169,416,307],'lai-bvnet':0.666666667,'fapar-bvnet':0.32,'fcover-bvnet':0.33},
{'crop':"sunflower",'esu':5,'doy':232,'y':2013,'gai':0.444898986,'fapar':0.274427702,'fcover':0.163180154,'refls':[131,143,279,219],'lai-bvnet':0.266666667,'fapar-bvnet':0.23,'fcover-bvnet':0.22},
{'crop':"sunflower",'esu':6,'doy':232,'y':2013,'gai':0.702948617,'fapar':0.458639421,'fcover':0.346056785,'refls':[98,89,315,188],'lai-bvnet':0.8,'fapar-bvnet':0.44,'fcover-bvnet':0.42},
{'crop':"sunflower",'esu':7,'doy':232,'y':2013,'gai':0.637846118,'fapar':0.436255414,'fcover':0.37405274,'refls':[110,96,375,202],'lai-bvnet':1.033333333,'fapar-bvnet':0.52,'fcover-bvnet':0.53},
{'crop':"sunflower",'esu':8,'doy':232,'y':2013,'gai':0.714358322,'fapar':0.47140277,'fcover':0.402593897,'refls':[91,85,308,194],'lai-bvnet':0.766666667,'fapar-bvnet':0.43,'fcover-bvnet':0.41},
{'crop':"sunflower",'esu':9,'doy':232,'y':2013,'gai':0.894790376,'fapar':0.525236116,'fcover':0.504394488,'refls':[86,61,362,161],'lai-bvnet':1.5,'fapar-bvnet':0.63,'fcover-bvnet':0.62},
{'crop':"corn",'esu':3,'doy':248,'y':2013,'gai':2.706960296,'fapar':0.876049212,'fcover':0.617251538,'refls':[34,21,378,131],'lai-bvnet':2.866666667,'fapar-bvnet':0.83,'fcover-bvnet':0.75},
{'crop':"corn",'esu':4,'doy':248,'y':2013,'gai':2.349263552,'fapar':0.865956059,'fcover':0.681014357,'refls':[38,24,385,143],'lai-bvnet':2.7,'fapar-bvnet':0.82,'fcover-bvnet':0.75},
{'crop':"corn",'esu':5,'doy':248,'y':2013,'gai':2.816706018,'fapar':0.841070144,'fcover':0.643473634,'refls':[46,28,355,120],'lai-bvnet':2.533333333,'fapar-bvnet':0.79,'fcover-bvnet':0.71},
{'crop':"corn",'esu':6,'doy':248,'y':2013,'gai':2.363544878,'fapar':0.814216116,'fcover':0.595257231,'refls':[50,33,381,148],'lai-bvnet':2.366666667,'fapar-bvnet':0.78,'fcover-bvnet':0.72},
{'crop':"corn",'esu':7,'doy':248,'y':2013,'gai':2.357393563,'fapar':0.866416674,'fcover':0.722686767,'refls':[42,26,394,143],'lai-bvnet':2.733333333,'fapar-bvnet':0.82,'fcover-bvnet':0.76},
{'crop':"corn",'esu':8,'doy':248,'y':2013,'gai':2.652271464,'fapar':0.806049655,'fcover':0.639710365,'refls':[56,40,317,155],'lai-bvnet':1.6,'fapar-bvnet':0.65,'fcover-bvnet':0.58},
{'crop':"sunflower",'esu':1,'doy':248,'y':2013,'gai':0.342987854,'fapar':0.231312214,'fcover':0.098201469,'refls':[108,127,213,193],'lai-bvnet':0.3,'fapar-bvnet':0.18,'fcover-bvnet':0.14},
{'crop':"sunflower",'esu':10,'doy':248,'y':2013,'gai':0.302354795,'fapar':0.231311947,'fcover':0.14030453,'refls':[96,115,197,192],'lai-bvnet':0.3,'fapar-bvnet':0.16,'fcover-bvnet':0.13},
{'crop':"sunflower",'esu':11,'doy':248,'y':2013,'gai':0.194351063,'fapar':0.148000703,'fcover':0.049448163,'refls':[129,162,230,263],'lai-bvnet':0.2,'fapar-bvnet':0.08,'fcover-bvnet':0.07},
{'crop':"sunflower",'esu':2,'doy':248,'y':2013,'gai':0.073787127,'fapar':0.050101249,'fcover':0.018118705,'refls':[111,139,194,234],'lai-bvnet':0.166666667,'fapar-bvnet':0.06,'fcover-bvnet':0.06},
{'crop':"sunflower",'esu':3,'doy':248,'y':2013,'gai':0.074546042,'fapar':0.048614579,'fcover':0.031820469,'refls':[108,137,220,247],'lai-bvnet':0.233333333,'fapar-bvnet':0.12,'fcover-bvnet':0.09},
{'crop':"sunflower",'esu':4,'doy':248,'y':2013,'gai':0.403076273,'fapar':0.328782153,'fcover':0.314145598,'refls':[138,142,409,269],'lai-bvnet':0.666666667,'fapar-bvnet':0.42,'fcover-bvnet':0.41},
{'crop':"sunflower",'esu':5,'doy':248,'y':2013,'gai':0.176212546,'fapar':0.136716291,'fcover':0.076637566,'refls':[152,185,257,243],'lai-bvnet':0.233333333,'fapar-bvnet':0.13,'fcover-bvnet':0.09},
{'crop':"sunflower",'esu':6,'doy':248,'y':2013,'gai':0.184654849,'fapar':0.145650415,'fcover':0.102652986,'refls':[122,142,241,225],'lai-bvnet':0.3,'fapar-bvnet':0.18,'fcover-bvnet':0.14},
{'crop':"sunflower",'esu':7,'doy':248,'y':2013,'gai':0.205449802,'fapar':0.172798211,'fcover':0.128902431,'refls':[149,170,283,245],'lai-bvnet':0.266666667,'fapar-bvnet':0.19,'fcover-bvnet':0.15},
{'crop':"sunflower",'esu':8,'doy':248,'y':2013,'gai':0.312693526,'fapar':0.239414137,'fcover':0.193543872,'refls':[119,132,258,209],'lai-bvnet':0.366666667,'fapar-bvnet':0.24,'fcover-bvnet':0.2},
{'crop':"sunflower",'esu':9,'doy':248,'y':2013,'gai':0.281109437,'fapar':0.216855617,'fcover':0.138285266,'refls':[116,125,232,167],'lai-bvnet':0.4,'fapar-bvnet':0.24,'fcover-bvnet':0.21},
{'crop':"corn",'esu':3,'doy':280,'y':2013,'gai':1.623558579,'fapar':0.79570692,'fcover':0.573221086,'refls':[43,32,327,142],'lai-bvnet':2.1,'fapar-bvnet':0.78,'fcover-bvnet':0.64},
{'crop':"corn",'esu':4,'doy':280,'y':2013,'gai':0.939251209,'fapar':0.563910096,'fcover':0.438323201,'refls':[51,45,268,160],'lai-bvnet':1.266666667,'fapar-bvnet':0.62,'fcover-bvnet':0.48},
{'crop':"corn",'esu':7,'doy':280,'y':2013,'gai':0.858307137,'fapar':0.561144477,'fcover':0.312610227,'refls':[55,49,275,154],'lai-bvnet':1.333333333,'fapar-bvnet':0.63,'fcover-bvnet':0.49},
{'crop':"corn",'esu':9,'doy':280,'y':2013,'gai':0.737880863,'fapar':0.472979445,'fcover':0.198232931,'refls':[47,42,247,138],'lai-bvnet':1.333333333,'fapar-bvnet':0.62,'fcover-bvnet':0.46}]


rsr_dir = os.environ['HOME']+"/Dev/otb-bv/data/"
lsat_rsr = rsr_dir+"landsat8_4b.rsr"

lsat_data = ["landsat8", lsat_rsr]
 
lsat_200 = {'doy': 200, 'to': 0, 'po': 0, 'ts': 62.06299464, 'ps': 136.3247244, 'gt': [s for s in gt if s['doy']==200]}
lsat_data.append(lsat_200)
lsat_216 = {'doy': 216, 'to': 0, 'po': 0, 'ts': 58.97139762, 'ps': 140.2322277, 'gt': [s for s in gt if s['doy']==216]}
lsat_data.append(lsat_216)
lsat_232 = {'doy': 232, 'to': 0, 'po': 0, 'ts': 54.98524874, 'ps': 145.3518399, 'gt': [s for s in gt if s['doy']==232]}
lsat_data.append(lsat_232)
lsat_248 = {'doy': 248, 'to': 0, 'po': 0, 'ts': 50.24043514, 'ps': 150.7526791, 'gt': [s for s in gt if s['doy']==248]}
lsat_data.append(lsat_248)
lsat_280 = {'doy': 280, 'to': 0, 'po': 0, 'ts': 39.32826351, 'ps': 159.6916544, 'gt': [s for s in gt if s['doy']==280]}
lsat_data.append(lsat_280)
