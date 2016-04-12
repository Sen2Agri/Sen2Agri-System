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
gt = [{'crop':"wheat",'esu':1,'doy':126,'y':2013,'gai':3.265145793,'fapar':0.930802592,'fcover':0.710144159,'refls':[3,22,11,404],'lai-bvnet':3.818518519,'fapar-bvnet':0.873333333,'fcover-bvnet':0.852222222},
{'crop':"wheat",'esu':15,'doy':126,'y':2013,'gai':3.10831565,'fapar':0.882737121,'fcover':0.506409094,'refls':[4,25,18,348],'lai-bvnet':3.196296296,'fapar-bvnet':0.817777778,'fcover-bvnet':0.777777778},
{'crop':"wheat",'esu':2,'doy':126,'y':2013,'gai':3.217486521,'fapar':0.901689317,'fcover':0.627143081,'refls':[4,21,14,404],'lai-bvnet':3.818518519,'fapar-bvnet':0.874444444,'fcover-bvnet':0.854444444},
{'crop':"wheat",'esu':3,'doy':126,'y':2013,'gai':2.169218405,'fapar':0.7890973,'fcover':0.443355192,'refls':[7,27,19,334],'lai-bvnet':2.97037037,'fapar-bvnet':0.793333333,'fcover-bvnet':0.747777778},
{'crop':"wheat",'esu':9,'doy':126,'y':2013,'gai':3.8220201,'fapar':0.92106748,'fcover':0.754263839,'refls':[8,23,16,401],'lai-bvnet':3.707407407,'fapar-bvnet':0.866666667,'fcover-bvnet':0.845555556},
{'crop':"wheat",'esu':1,'doy':177,'y':2013,'gai':0.562424787,'fapar':0.401679884,'fcover':0.395162469,'refls':[45,97,117,316],'lai-bvnet':0.588888889,'fapar-bvnet':0.343333333,'fcover-bvnet':0.333333333},
{'crop':"wheat",'esu':15,'doy':177,'y':2013,'gai':0.389601619,'fapar':0.386943349,'fcover':0.404738474,'refls':[32,71,92,224],'lai-bvnet':0.522222222,'fapar-bvnet':0.28,'fcover-bvnet':0.253333333},
{'crop':"wheat",'esu':2,'doy':177,'y':2013,'gai':0.765166119,'fapar':0.429939706,'fcover':0.387427764,'refls':[34,75,94,250],'lai-bvnet':0.577777778,'fapar-bvnet':0.315555556,'fcover-bvnet':0.294444444},
{'crop':"wheat",'esu':3,'doy':177,'y':2013,'gai':0.833283576,'fapar':0.447890976,'fcover':0.35318526,'refls':[16,48,70,188],'lai-bvnet':0.674074074,'fapar-bvnet':0.315555556,'fcover-bvnet':0.273333333},
{'crop':"wheat",'esu':9,'doy':177,'y':2013,'gai':0.440596326,'fapar':0.415327677,'fcover':0.437879843,'refls':[24,59,68,298],'lai-bvnet':1.074074074,'fapar-bvnet':0.49,'fcover-bvnet':0.47},
{'crop':"corn",'esu':1,'doy':177,'y':2013,'gai':0.756189167,'fapar':0.421359018,'fcover':0.345275291,'refls':[42,86,87,253],'lai-bvnet':0.622222222,'fapar-bvnet':0.338888889,'fcover-bvnet':0.325555556},
{'crop':"corn",'esu':2,'doy':177,'y':2013,'gai':0.851551237,'fapar':0.452844021,'fcover':0.343762894,'refls':[30,65,69,267],'lai-bvnet':0.877777778,'fapar-bvnet':0.426666667,'fcover-bvnet':0.406666667},
{'crop':"corn",'esu':5,'doy':177,'y':2013,'gai':0.917288666,'fapar':0.505081078,'fcover':0.313337847,'refls':[27,65,68,271],'lai-bvnet':0.92962963,'fapar-bvnet':0.44,'fcover-bvnet':0.42},
{'crop':"corn",'esu':6,'doy':177,'y':2013,'gai':0.728072672,'fapar':0.429052761,'fcover':0.275443692,'refls':[45,96,95,336],'lai-bvnet':0.848148148,'fapar-bvnet':0.44,'fcover-bvnet':0.441111111},
{'crop':"corn",'esu':8,'doy':177,'y':2013,'gai':1.269172395,'fapar':0.587734731,'fcover':0.39247181,'refls':[15,45,41,273],'lai-bvnet':1.355555556,'fapar-bvnet':0.541111111,'fcover-bvnet':0.522222222},
{'crop':"sunflower",'esu':1,'doy':177,'y':2013,'gai':0.85288191,'fapar':0.534479494,'fcover':0.400378267,'refls':[44,91,104,310],'lai-bvnet':0.648148148,'fapar-bvnet':0.362222222,'fcover-bvnet':0.353333333},
{'crop':"sunflower",'esu':10,'doy':177,'y':2013,'gai':0.730542199,'fapar':0.433981517,'fcover':0.348864655,'refls':[33,80,78,272],'lai-bvnet':0.751851852,'fapar-bvnet':0.391111111,'fcover-bvnet':0.376666667},
{'crop':"sunflower",'esu':2,'doy':177,'y':2013,'gai':1.202271231,'fapar':0.638268539,'fcover':0.459282373,'refls':[37,84,89,356],'lai-bvnet':0.988888889,'fapar-bvnet':0.484444444,'fcover-bvnet':0.483333333},
{'crop':"sunflower",'esu':3,'doy':177,'y':2013,'gai':1.310929214,'fapar':0.689493666,'fcover':0.490735268,'refls':[38,85,86,401],'lai-bvnet':1.288888889,'fapar-bvnet':0.566666667,'fcover-bvnet':0.578888889},
{'crop':"sunflower",'esu':6,'doy':177,'y':2013,'gai':0.381358252,'fapar':0.284853804,'fcover':0.235209292,'refls':[44,99,111,260],'lai-bvnet':0.47037037,'fapar-bvnet':0.283333333,'fcover-bvnet':0.266666667},
{'crop':"sunflower",'esu':8,'doy':177,'y':2013,'gai':0.161769278,'fapar':0.151717565,'fcover':0.160470671,'refls':[48,100,121,227],'lai-bvnet':0.333333333,'fapar-bvnet':0.185555556,'fcover-bvnet':0.165555556},
{'crop':"sunflower",'esu':9,'doy':177,'y':2013,'gai':0.371450302,'fapar':0.280001454,'fcover':0.228986914,'refls':[45,98,107,251],'lai-bvnet':0.466666667,'fapar-bvnet':0.27,'fcover-bvnet':0.255555556},
{'crop':"corn",'esu':1,'doy':187,'y':2013,'gai':1.351855093,'fapar':0.63448711,'fcover':0.466148069,'refls':[25,56,49,309],'lai-bvnet':1.737037037,'fapar-bvnet':0.632222222,'fcover-bvnet':0.61},
{'crop':"corn",'esu':2,'doy':187,'y':2013,'gai':1.928933577,'fapar':0.683491644,'fcover':0.498858565,'refls':[17,37,32,315],'lai-bvnet':2.411111111,'fapar-bvnet':0.724444444,'fcover-bvnet':0.684444444},
{'crop':"corn",'esu':3,'doy':187,'y':2013,'gai':0.583779732,'fapar':0.318710696,'fcover':0.201541278,'refls':[50,97,106,243],'lai-bvnet':0.451851852,'fapar-bvnet':0.258888889,'fcover-bvnet':0.242222222},
{'crop':"corn",'esu':4,'doy':187,'y':2013,'gai':0.640332523,'fapar':0.365464395,'fcover':0.156870581,'refls':[38,85,86,249],'lai-bvnet':0.659259259,'fapar-bvnet':0.34,'fcover-bvnet':0.333333333},
{'crop':"corn",'esu':5,'doy':187,'y':2013,'gai':1.89710742,'fapar':0.675071509,'fcover':0.38293792,'refls':[18,45,43,271],'lai-bvnet':1.648148148,'fapar-bvnet':0.591111111,'fcover-bvnet':0.553333333},
{'crop':"corn",'esu':6,'doy':187,'y':2013,'gai':1.482818128,'fapar':0.610686962,'fcover':0.399683211,'refls':[15,41,35,290],'lai-bvnet':2.048148148,'fapar-bvnet':0.666666667,'fcover-bvnet':0.626666667},
{'crop':"corn",'esu':7,'doy':187,'y':2013,'gai':0.546368822,'fapar':0.319440166,'fcover':0.202918637,'refls':[68,119,132,271],'lai-bvnet':0.355555556,'fapar-bvnet':0.223333333,'fcover-bvnet':0.205555556},
{'crop':"corn",'esu':8,'doy':187,'y':2013,'gai':1.843167558,'fapar':0.702269734,'fcover':0.495131118,'refls':[17,42,34,306],'lai-bvnet':2.27037037,'fapar-bvnet':0.704444444,'fcover-bvnet':0.672222222},
{'crop':"sunflower",'esu':1,'doy':187,'y':2013,'gai':1.085436103,'fapar':0.629231693,'fcover':0.569171048,'refls':[32,69,65,347],'lai-bvnet':1.555555556,'fapar-bvnet':0.617777778,'fcover-bvnet':0.614444444},
{'crop':"sunflower",'esu':10,'doy':187,'y':2013,'gai':0.910521494,'fapar':0.51542864,'fcover':0.411626424,'refls':[30,66,62,282],'lai-bvnet':1.192592593,'fapar-bvnet':0.511111111,'fcover-bvnet':0.493333333},
{'crop':"sunflower",'esu':11,'doy':187,'y':2013,'gai':0.419100254,'fapar':0.268907724,'fcover':0.21059941,'refls':[60,117,135,272],'lai-bvnet':0.351851852,'fapar-bvnet':0.218888889,'fcover-bvnet':0.203333333},
{'crop':"sunflower",'esu':2,'doy':187,'y':2013,'gai':1.348450196,'fapar':0.722166097,'fcover':0.654208343,'refls':[31,65,59,379],'lai-bvnet':1.959259259,'fapar-bvnet':0.694444444,'fcover-bvnet':0.696666667},
{'crop':"sunflower",'esu':3,'doy':187,'y':2013,'gai':1.552802797,'fapar':0.762181809,'fcover':0.653367943,'refls':[31,65,56,407],'lai-bvnet':2.296296296,'fapar-bvnet':0.748888889,'fcover-bvnet':0.755555556},
{'crop':"sunflower",'esu':5,'doy':187,'y':2013,'gai':0.470231907,'fapar':0.325441772,'fcover':0.252449502,'refls':[42,97,99,277],'lai-bvnet':0.633333333,'fapar-bvnet':0.347777778,'fcover-bvnet':0.341111111},
{'crop':"sunflower",'esu':7,'doy':187,'y':2013,'gai':0.94114769,'fapar':0.565040687,'fcover':0.545369996,'refls':[35,76,65,334],'lai-bvnet':1.4,'fapar-bvnet':0.582222222,'fcover-bvnet':0.582222222},
{'crop':"sunflower",'esu':8,'doy':187,'y':2013,'gai':0.48853297,'fapar':0.336310748,'fcover':0.35623016,'refls':[45,94,102,259],'lai-bvnet':0.522222222,'fapar-bvnet':0.293333333,'fcover-bvnet':0.28},
{'crop':"sunflower",'esu':9,'doy':187,'y':2013,'gai':0.986301604,'fapar':0.549561342,'fcover':0.459110695,'refls':[34,75,72,304],'lai-bvnet':1.122222222,'fapar-bvnet':0.501111111,'fcover-bvnet':0.492222222},
{'crop':"corn",'esu':1,'doy':201,'y':2013,'gai':2.572118553,'fapar':0.822606546,'fcover':0.598273109,'refls':[12,33,25,364],'lai-bvnet':2.588888889,'fapar-bvnet':0.763333333,'fcover-bvnet':0.741111111},
{'crop':"corn",'esu':2,'doy':201,'y':2013,'gai':3.124329034,'fapar':0.852327859,'fcover':0.68649802,'refls':[11,26,19,387],'lai-bvnet':2.911111111,'fapar-bvnet':0.798888889,'fcover-bvnet':0.777777778},
{'crop':"corn",'esu':3,'doy':201,'y':2013,'gai':2.172889543,'fapar':0.660331417,'fcover':0.445564898,'refls':[24,49,39,280],'lai-bvnet':1.666666667,'fapar-bvnet':0.605555556,'fcover-bvnet':0.574444444},
{'crop':"corn",'esu':4,'doy':201,'y':2013,'gai':1.55556381,'fapar':0.651989434,'fcover':0.440128785,'refls':[23,47,39,300],'lai-bvnet':1.807407407,'fapar-bvnet':0.642222222,'fcover-bvnet':0.615555556},
{'crop':"corn",'esu':5,'doy':201,'y':2013,'gai':2.910370738,'fapar':0.823644406,'fcover':0.615467521,'refls':[9,28,19,326],'lai-bvnet':2.544444444,'fapar-bvnet':0.742222222,'fcover-bvnet':0.706666667},
{'crop':"corn",'esu':6,'doy':201,'y':2013,'gai':2.532527872,'fapar':0.774198588,'fcover':0.565122804,'refls':[13,31,24,351],'lai-bvnet':2.592592593,'fapar-bvnet':0.758888889,'fcover-bvnet':0.731111111},
{'crop':"corn",'esu':7,'doy':201,'y':2013,'gai':1.540305745,'fapar':0.629227207,'fcover':0.47623618,'refls':[29,55,45,324],'lai-bvnet':1.740740741,'fapar-bvnet':0.642222222,'fcover-bvnet':0.628888889},
{'crop':"corn",'esu':8,'doy':201,'y':2013,'gai':2.575803851,'fapar':0.810676553,'fcover':0.742159704,'refls':[13,36,27,361],'lai-bvnet':2.525925926,'fapar-bvnet':0.76,'fcover-bvnet':0.742222222},
{'crop':"corn",'esu':9,'doy':201,'y':2013,'gai':2.028628411,'fapar':0.674433447,'fcover':0.540595822,'refls':[21,44,36,318],'lai-bvnet':2.181481481,'fapar-bvnet':0.704444444,'fcover-bvnet':0.678888889},
{'crop':"sunflower",'esu':11,'doy':201,'y':2013,'gai':0.851516638,'fapar':0.473407312,'fcover':0.298152393,'refls':[44,88,87,291],'lai-bvnet':0.851851852,'fapar-bvnet':0.431111111,'fcover-bvnet':0.416666667},
{'crop':"sunflower",'esu':2,'doy':201,'y':2013,'gai':1.782631251,'fapar':0.776359593,'fcover':0.648124112,'refls':[36,72,68,342],'lai-bvnet':1.433333333,'fapar-bvnet':0.6,'fcover-bvnet':0.591111111},
{'crop':"sunflower",'esu':3,'doy':201,'y':2013,'gai':1.71628522,'fapar':0.786241537,'fcover':0.62429046,'refls':[34,70,61,396],'lai-bvnet':2.022222222,'fapar-bvnet':0.714444444,'fcover-bvnet':0.708888889},
{'crop':"corn",'esu':1,'doy':211,'y':2013,'gai':2.870557184,'fapar':0.856165545,'fcover':0.677378244,'refls':[15,38,29,389],'lai-bvnet':2.925925926,'fapar-bvnet':0.81,'fcover-bvnet':0.794444444},
{'crop':"corn",'esu':2,'doy':211,'y':2013,'gai':3.177691726,'fapar':0.873621164,'fcover':0.746324999,'refls':[12,31,27,405],'lai-bvnet':3.222222222,'fapar-bvnet':0.836666667,'fcover-bvnet':0.821111111},
{'crop':"corn",'esu':3,'doy':211,'y':2013,'gai':2.745872218,'fapar':0.795468393,'fcover':0.589041301,'refls':[15,35,27,356],'lai-bvnet':2.944444444,'fapar-bvnet':0.798888889,'fcover-bvnet':0.77},
{'crop':"corn",'esu':4,'doy':211,'y':2013,'gai':1.971715736,'fapar':0.759327492,'fcover':0.576157114,'refls':[17,38,32,348],'lai-bvnet':2.637037037,'fapar-bvnet':0.772222222,'fcover-bvnet':0.747777778},
{'crop':"corn",'esu':5,'doy':211,'y':2013,'gai':3.241319108,'fapar':0.862855213,'fcover':0.663887818,'refls':[11,31,24,342],'lai-bvnet':2.82962963,'fapar-bvnet':0.782222222,'fcover-bvnet':0.747777778},
{'crop':"corn",'esu':6,'doy':211,'y':2013,'gai':2.75734044,'fapar':0.833673353,'fcover':0.651832539,'refls':[11,33,26,356],'lai-bvnet':2.822222222,'fapar-bvnet':0.79,'fcover-bvnet':0.761111111},
{'crop':"corn",'esu':7,'doy':211,'y':2013,'gai':2.021228136,'fapar':0.749981968,'fcover':0.609977061,'refls':[22,43,36,365],'lai-bvnet':2.603703704,'fapar-bvnet':0.772222222,'fcover-bvnet':0.755555556},
{'crop':"corn",'esu':8,'doy':211,'y':2013,'gai':2.814777782,'fapar':0.848893171,'fcover':0.816395715,'refls':[14,37,29,334],'lai-bvnet':2.503703704,'fapar-bvnet':0.752222222,'fcover-bvnet':0.722222222},
{'crop':"corn",'esu':9,'doy':211,'y':2013,'gai':2.58117998,'fapar':0.784176087,'fcover':0.66847995,'refls':[12,33,24,365],'lai-bvnet':3.07037037,'fapar-bvnet':0.814444444,'fcover-bvnet':0.793333333},
{'crop':"sunflower",'esu':1,'doy':211,'y':2013,'gai':1.388895042,'fapar':0.683128801,'fcover':0.547820697,'refls':[30,69,61,321],'lai-bvnet':1.4,'fapar-bvnet':0.586666667,'fcover-bvnet':0.576666667},
{'crop':"sunflower",'esu':10,'doy':211,'y':2013,'gai':1.108905509,'fapar':0.567593345,'fcover':0.442777895,'refls':[27,64,62,258],'lai-bvnet':0.974074074,'fapar-bvnet':0.458888889,'fcover-bvnet':0.427777778},
{'crop':"sunflower",'esu':11,'doy':211,'y':2013,'gai':1.013805264,'fapar':0.536060647,'fcover':0.360545902,'refls':[38,83,81,303],'lai-bvnet':0.948148148,'fapar-bvnet':0.472222222,'fcover-bvnet':0.458888889},
{'crop':"sunflower",'esu':2,'doy':211,'y':2013,'gai':1.449424474,'fapar':0.637473027,'fcover':0.412135927,'refls':[33,73,71,286],'lai-bvnet':1.018518519,'fapar-bvnet':0.487777778,'fcover-bvnet':0.466666667},
{'crop':"sunflower",'esu':3,'doy':211,'y':2013,'gai':1.248318482,'fapar':0.651689661,'fcover':0.513387726,'refls':[31,69,58,344],'lai-bvnet':1.659259259,'fapar-bvnet':0.642222222,'fcover-bvnet':0.64},
{'crop':"sunflower",'esu':5,'doy':211,'y':2013,'gai':0.70872397,'fapar':0.396683532,'fcover':0.281820893,'refls':[39,92,99,247],'lai-bvnet':0.518518519,'fapar-bvnet':0.301111111,'fcover-bvnet':0.276666667},
{'crop':"sunflower",'esu':6,'doy':211,'y':2013,'gai':0.908943983,'fapar':0.550647697,'fcover':0.48116574,'refls':[28,73,75,268],'lai-bvnet':0.874074074,'fapar-bvnet':0.436666667,'fcover-bvnet':0.414444444},
{'crop':"sunflower",'esu':8,'doy':211,'y':2013,'gai':0.981111943,'fapar':0.569807638,'fcover':0.562611544,'refls':[30,73,75,303],'lai-bvnet':0.974074074,'fapar-bvnet':0.48,'fcover-bvnet':0.462222222},
{'crop':"sunflower",'esu':9,'doy':211,'y':2013,'gai':1.349842251,'fapar':0.712877774,'fcover':0.643667347,'refls':[22,60,53,368],'lai-bvnet':1.759259259,'fapar-bvnet':0.661111111,'fcover-bvnet':0.651111111},
{'crop':"corn",'esu':1,'doy':223,'y':2013,'gai':2.537245697,'fapar':0.835321571,'fcover':0.73006676,'refls':[16,39,31,386],'lai-bvnet':3.103703704,'fapar-bvnet':0.826666667,'fcover-bvnet':0.803333333},
{'crop':"corn",'esu':5,'doy':223,'y':2013,'gai':3.393558642,'fapar':0.896224837,'fcover':0.685737127,'refls':[9,34,27,344],'lai-bvnet':2.881481481,'fapar-bvnet':0.795555556,'fcover-bvnet':0.75},
{'crop':"corn",'esu':6,'doy':223,'y':2013,'gai':2.816751486,'fapar':0.867595021,'fcover':0.733581759,'refls':[13,35,30,367],'lai-bvnet':3.059259259,'fapar-bvnet':0.82,'fcover-bvnet':0.784444444},
{'crop':"sunflower",'esu':1,'doy':223,'y':2013,'gai':1.239115335,'fapar':0.635077253,'fcover':0.460230075,'refls':[32,80,71,330],'lai-bvnet':1.311111111,'fapar-bvnet':0.578888889,'fcover-bvnet':0.567777778},
{'crop':"sunflower",'esu':10,'doy':223,'y':2013,'gai':0.877423734,'fapar':0.501083246,'fcover':0.389234489,'refls':[30,72,71,264],'lai-bvnet':0.885185185,'fapar-bvnet':0.436666667,'fcover-bvnet':0.411111111},
{'crop':"sunflower",'esu':11,'doy':223,'y':2013,'gai':0.78728552,'fapar':0.436960535,'fcover':0.304428604,'refls':[41,89,91,318],'lai-bvnet':0.907407407,'fapar-bvnet':0.472222222,'fcover-bvnet':0.454444444},
{'crop':"sunflower",'esu':2,'doy':223,'y':2013,'gai':0.822288103,'fapar':0.422156561,'fcover':0.268502102,'refls':[39,90,90,275],'lai-bvnet':0.72962963,'fapar-bvnet':0.388888889,'fcover-bvnet':0.372222222},
{'crop':"sunflower",'esu':3,'doy':223,'y':2013,'gai':0.937746217,'fapar':0.529861271,'fcover':0.331676851,'refls':[35,86,76,317],'lai-bvnet':1.159259259,'fapar-bvnet':0.531111111,'fcover-bvnet':0.525555556},
{'crop':"sunflower",'esu':4,'doy':223,'y':2013,'gai':0.288436416,'fapar':0.248210142,'fcover':0.28859757,'refls':[91,156,181,376],'lai-bvnet':0.366666667,'fapar-bvnet':0.241111111,'fcover-bvnet':0.225555556},
{'crop':"sunflower",'esu':5,'doy':223,'y':2013,'gai':0.569909824,'fapar':0.334687313,'fcover':0.21334671,'refls':[40,93,97,276],'lai-bvnet':0.640740741,'fapar-bvnet':0.362222222,'fcover-bvnet':0.343333333},
{'crop':"sunflower",'esu':6,'doy':223,'y':2013,'gai':0.854999095,'fapar':0.540934032,'fcover':0.428748325,'refls':[30,73,70,293],'lai-bvnet':1.148148148,'fapar-bvnet':0.525555556,'fcover-bvnet':0.504444444},
{'crop':"sunflower",'esu':7,'doy':223,'y':2013,'gai':0.904168922,'fapar':0.544432044,'fcover':0.4730451,'refls':[32,79,66,367],'lai-bvnet':1.655555556,'fapar-bvnet':0.64,'fcover-bvnet':0.64},
{'crop':"sunflower",'esu':8,'doy':223,'y':2013,'gai':0.863697292,'fapar':0.541284549,'fcover':0.486549092,'refls':[33,76,69,332],'lai-bvnet':1.311111111,'fapar-bvnet':0.576666667,'fcover-bvnet':0.566666667},
{'crop':"sunflower",'esu':9,'doy':223,'y':2013,'gai':1.136925799,'fapar':0.634844927,'fcover':0.609964604,'refls':[30,69,58,373],'lai-bvnet':1.922222222,'fapar-bvnet':0.695555556,'fcover-bvnet':0.683333333},
{'crop':"corn",'esu':3,'doy':234,'y':2013,'gai':2.795565134,'fapar':0.887540322,'fcover':0.76064418,'refls':[14,36,30,374],'lai-bvnet':2.755555556,'fapar-bvnet':0.801111111,'fcover-bvnet':0.767777778},
{'crop':"corn",'esu':4,'doy':234,'y':2013,'gai':2.444223791,'fapar':0.848397923,'fcover':0.68440736,'refls':[15,38,31,351],'lai-bvnet':2.537037037,'fapar-bvnet':0.774444444,'fcover-bvnet':0.736666667},
{'crop':"corn",'esu':5,'doy':234,'y':2013,'gai':3.102458151,'fapar':0.898142496,'fcover':0.775365515,'refls':[10,35,30,353],'lai-bvnet':2.522222222,'fapar-bvnet':0.771111111,'fcover-bvnet':0.727777778},
{'crop':"corn",'esu':6,'doy':234,'y':2013,'gai':2.958531199,'fapar':0.872377757,'fcover':0.743048856,'refls':[15,36,32,354],'lai-bvnet':2.444444444,'fapar-bvnet':0.762222222,'fcover-bvnet':0.723333333},
{'crop':"corn",'esu':7,'doy':234,'y':2013,'gai':2.590474159,'fapar':0.874115814,'fcover':0.755687426,'refls':[15,37,30,359],'lai-bvnet':2.477777778,'fapar-bvnet':0.767777778,'fcover-bvnet':0.733333333},
{'crop':"corn",'esu':8,'doy':234,'y':2013,'gai':3.02323239,'fapar':0.88568392,'fcover':0.787499419,'refls':[13,38,32,323],'lai-bvnet':2.2,'fapar-bvnet':0.725555556,'fcover-bvnet':0.68},
{'crop':"corn",'esu':9,'doy':234,'y':2013,'gai':2.954777427,'fapar':0.864992167,'fcover':0.652282514,'refls':[12,32,25,379],'lai-bvnet':2.974074074,'fapar-bvnet':0.824444444,'fcover-bvnet':0.791111111},
{'crop':"sunflower",'esu':1,'doy':234,'y':2013,'gai':0.926591343,'fapar':0.499932707,'fcover':0.326074228,'refls':[38,91,84,272],'lai-bvnet':0.685185185,'fapar-bvnet':0.392222222,'fcover-bvnet':0.367777778},
{'crop':"sunflower",'esu':10,'doy':234,'y':2013,'gai':0.637387534,'fapar':0.400753078,'fcover':0.298938675,'refls':[37,83,88,237],'lai-bvnet':0.507407407,'fapar-bvnet':0.305555556,'fcover-bvnet':0.267777778},
{'crop':"sunflower",'esu':11,'doy':234,'y':2013,'gai':0.543336924,'fapar':0.321368386,'fcover':0.211566926,'refls':[54,112,124,268],'lai-bvnet':0.377777778,'fapar-bvnet':0.251111111,'fcover-bvnet':0.224444444},
{'crop':"sunflower",'esu':2,'doy':234,'y':2013,'gai':0.414823035,'fapar':0.247739332,'fcover':0.151493884,'refls':[46,98,112,210],'lai-bvnet':0.322222222,'fapar-bvnet':0.18,'fcover-bvnet':0.164444444},
{'crop':"sunflower",'esu':3,'doy':234,'y':2013,'gai':0.588202587,'fapar':0.349953983,'fcover':0.188720931,'refls':[46,97,111,238],'lai-bvnet':0.4,'fapar-bvnet':0.244444444,'fcover-bvnet':0.215555556},
{'crop':"sunflower",'esu':4,'doy':234,'y':2013,'gai':0.385094945,'fapar':0.320814212,'fcover':0.358010152,'refls':[71,130,142,373],'lai-bvnet':0.551851852,'fapar-bvnet':0.344444444,'fcover-bvnet':0.337777778},
{'crop':"sunflower",'esu':5,'doy':234,'y':2013,'gai':0.411313181,'fapar':0.257213776,'fcover':0.15236233,'refls':[46,101,114,239],'lai-bvnet':0.362962963,'fapar-bvnet':0.23,'fcover-bvnet':0.205555556},
{'crop':"sunflower",'esu':6,'doy':234,'y':2013,'gai':0.638161896,'fapar':0.419515795,'fcover':0.31563131,'refls':[35,85,86,264],'lai-bvnet':0.655555556,'fapar-bvnet':0.38,'fcover-bvnet':0.353333333},
{'crop':"sunflower",'esu':7,'doy':234,'y':2013,'gai':0.583796578,'fapar':0.403323264,'fcover':0.343408951,'refls':[42,94,94,317],'lai-bvnet':0.811111111,'fapar-bvnet':0.421111111,'fcover-bvnet':0.405555556},
{'crop':"sunflower",'esu':8,'doy':234,'y':2013,'gai':0.664150222,'fapar':0.442404191,'fcover':0.376462644,'refls':[39,84,84,302],'lai-bvnet':0.8,'fapar-bvnet':0.433333333,'fcover-bvnet':0.407777778},
{'crop':"sunflower",'esu':9,'doy':234,'y':2013,'gai':0.818080259,'fapar':0.486688554,'fcover':0.458630836,'refls':[35,76,69,315],'lai-bvnet':1.096296296,'fapar-bvnet':0.516666667,'fcover-bvnet':0.497777778},
{'crop':"corn",'esu':3,'doy':244,'y':2013,'gai':2.758175487,'fapar':0.878342127,'fcover':0.633678812,'refls':[16,42,37,373],'lai-bvnet':1.981481481,'fapar-bvnet':0.718888889,'fcover-bvnet':0.691111111},
{'crop':"corn",'esu':4,'doy':244,'y':2013,'gai':2.391852293,'fapar':0.867311879,'fcover':0.673047707,'refls':[14,36,31,350],'lai-bvnet':2.037037037,'fapar-bvnet':0.718888889,'fcover-bvnet':0.67},
{'crop':"corn",'esu':5,'doy':244,'y':2013,'gai':3.248520216,'fapar':0.912415531,'fcover':0.726637402,'refls':[10,42,30,337],'lai-bvnet':1.848148148,'fapar-bvnet':0.69,'fcover-bvnet':0.655555556},
{'crop':"corn",'esu':6,'doy':244,'y':2013,'gai':2.740254054,'fapar':0.883725622,'fcover':0.650000518,'refls':[12,41,33,361],'lai-bvnet':1.9,'fapar-bvnet':0.706666667,'fcover-bvnet':0.681111111},
{'crop':"corn",'esu':7,'doy':244,'y':2013,'gai':2.385399352,'fapar':0.866620227,'fcover':0.718888134,'refls':[8,30,24,323],'lai-bvnet':1.877777778,'fapar-bvnet':0.686666667,'fcover-bvnet':0.636666667},
{'crop':"corn",'esu':8,'doy':244,'y':2013,'gai':3.098443433,'fapar':0.894207196,'fcover':0.723334026,'refls':[14,46,37,318],'lai-bvnet':1.562962963,'fapar-bvnet':0.636666667,'fcover-bvnet':0.593333333},
{'crop':"sunflower",'esu':1,'doy':244,'y':2013,'gai':0.509731708,'fapar':0.308060926,'fcover':0.163307971,'refls':[37,89,96,210],'lai-bvnet':0.385185185,'fapar-bvnet':0.238888889,'fcover-bvnet':0.204444444},
{'crop':"sunflower",'esu':10,'doy':244,'y':2013,'gai':0.398078435,'fapar':0.279723699,'fcover':0.185628572,'refls':[45,100,113,239],'lai-bvnet':0.311111111,'fapar-bvnet':0.203333333,'fcover-bvnet':0.171111111},
{'crop':"sunflower",'esu':11,'doy':244,'y':2013,'gai':0.294061309,'fapar':0.197534327,'fcover':0.095767809,'refls':[60,116,139,231],'lai-bvnet':0.281481481,'fapar-bvnet':0.16,'fcover-bvnet':0.127777778},
{'crop':"sunflower",'esu':2,'doy':244,'y':2013,'gai':0.171225958,'fapar':0.106569273,'fcover':0.056225899,'refls':[44,95,114,176],'lai-bvnet':0.237037037,'fapar-bvnet':0.125555556,'fcover-bvnet':0.103333333},
{'crop':"sunflower",'esu':3,'doy':244,'y':2013,'gai':0.221305055,'fapar':0.134711552,'fcover':0.076649172,'refls':[43,89,110,193],'lai-bvnet':0.314814815,'fapar-bvnet':0.175555556,'fcover-bvnet':0.144444444},
{'crop':"sunflower",'esu':4,'doy':244,'y':2013,'gai':0.397938751,'fapar':0.326505599,'fcover':0.326678328,'refls':[67,122,132,372],'lai-bvnet':0.596296296,'fapar-bvnet':0.347777778,'fcover-bvnet':0.345555556},
{'crop':"sunflower",'esu':5,'doy':244,'y':2013,'gai':0.243384156,'fapar':0.171144144,'fcover':0.098273213,'refls':[55,120,138,248],'lai-bvnet':0.292592593,'fapar-bvnet':0.18,'fcover-bvnet':0.148888889},
{'crop':"sunflower",'esu':6,'doy':244,'y':2013,'gai':0.314228291,'fapar':0.223897666,'fcover':0.163503936,'refls':[45,107,116,249],'lai-bvnet':0.359259259,'fapar-bvnet':0.236666667,'fcover-bvnet':0.197777778},
{'crop':"corn",'esu':3,'doy':264,'y':2013,'gai':2.314439362,'fapar':0.864301907,'fcover':0.668146464,'refls':[18,40,35,365],'lai-bvnet':2.303703704,'fapar-bvnet':0.774444444,'fcover-bvnet':0.703333333},
{'crop':"corn",'esu':7,'doy':264,'y':2013,'gai':2.159724313,'fapar':0.834762453,'fcover':0.708144891,'refls':[16,42,34,351],'lai-bvnet':2.025925926,'fapar-bvnet':0.736666667,'fcover-bvnet':0.663333333},
{'crop':"corn",'esu':9,'doy':264,'y':2013,'gai':2.544363003,'fapar':0.852859964,'fcover':0.731209045,'refls':[17,46,35,386],'lai-bvnet':2.537037037,'fapar-bvnet':0.805555556,'fcover-bvnet':0.766666667},
{'crop':"corn",'esu':3,'doy':285,'y':2013,'gai':1.248135642,'fapar':0.684651679,'fcover':0.469472646,'refls':[22,48,47,309],'lai-bvnet':1.381481481,'fapar-bvnet':0.651111111,'fcover-bvnet':0.535555556},
{'crop':"corn",'esu':4,'doy':285,'y':2013,'gai':0.644535457,'fapar':0.442294029,'fcover':0.351740485,'refls':[22,48,52,230],'lai-bvnet':0.87037037,'fapar-bvnet':0.481111111,'fcover-bvnet':0.36},
{'crop':"corn",'esu':7,'doy':285,'y':2013,'gai':0.547703951,'fapar':0.426793439,'fcover':0.208692875,'refls':[28,57,61,234],'lai-bvnet':0.744444444,'fapar-bvnet':0.444444444,'fcover-bvnet':0.34},
{'crop':"corn",'esu':9,'doy':285,'y':2013,'gai':0.410192996,'fapar':0.349707739,'fcover':0.110641869,'refls':[19,48,58,211],'lai-bvnet':0.685185185,'fapar-bvnet':0.416666667,'fcover-bvnet':0.315555556}]

rsr_dir = os.environ['HOME']+"/Dev/otb-bv/data/"
fsat_rsr = rsr_dir+"formosat2_4b.rsr"

fsat_data = ["formosat2", fsat_rsr]

fsat_126 = {'doy': 126, 'to': 20.071, 'po': 307.601, 'ts':33.469, 'ps': 138.026, 'gt': [s for s in gt if s['doy']==126]}
fsat_data.append(fsat_126)


fsat_177 = {'doy': 177, 'to': 19.205, 'po': 290.135, 'ts': 27.993, 'ps': 127.855, 'gt': [s for s in gt if s['doy']==177]}
fsat_data.append(fsat_177)

fsat_187 = {'doy': 187, 'to': 18.841, 'po': 289.533, 'ts': 28.636, 'ps': 127.846, 'gt': [s for s in gt if s['doy']==187]}
fsat_data.append(fsat_187)

fsat_201 = {'doy': 201, 'to': 18.377 ,'po': 289.554,'ts':  30.285 , 'ps': 129.475, 'gt': [s for s in gt if s['doy']==201]}
# fsat_data.append(fsat_201)      

fsat_211 = {'doy': 211, 'to': 18.067 ,'po': 288.603,'ts':  31.936 , 'ps': 131.699, 'gt': [s for s in gt if s['doy']==211]}
fsat_data.append(fsat_211)

fsat_223 = {'doy': 223, 'to': 17.734 ,'po': 288.522,'ts':  34.399 , 'ps': 135.262, 'gt': [s for s in gt if s['doy']==223]}
fsat_data.append(fsat_223)

fsat_234 = {'doy': 234, 'to':  17.45 ,'po': 287.784,'ts':  37.085 , 'ps':  139.09, 'gt': [s for s in gt if s['doy']==234]}
fsat_data.append(fsat_234)

fsat_244 = {'doy': 244, 'to': 17.249 ,'po': 289.633,'ts':  39.852 , 'ps':  142.77, 'gt': [s for s in gt if s['doy']==244]}
fsat_data.append(fsat_244)

fsat_264 = {'doy': 264,'to': 16.885 ,'po': 288.309,'ts':  46.135 , 'ps':  149.87,  'gt': [s for s in gt if s['doy']==264]}
fsat_data.append(fsat_264)

fsat_285 = {'doy': 285,'to': 16.638 ,'po':  287.73,'ts':  53.283 , 'ps': 155.773,  'gt': [s for s in gt if s['doy']==285]}
fsat_data.append(fsat_285)



