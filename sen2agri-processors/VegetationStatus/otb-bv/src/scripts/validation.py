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

import string
import os
import sys
import otbApplication as otb
import bv_net as bv
from formosat_data import *
from spot4_data import *
from landsat_data import *

varName = "MLAI"
working_dir = "/tmp/"+varName+"/"
rsr_dir = os.environ['HOME']+"/Dev/otb-bv/data/"
input_var_file = working_dir+"input-vars"
input_var_file_test = working_dir+"input-vars-test"
nbSamples_train = 5000
nbSamples_test = 200
noise_var = 0.000001
bestof = 3
simulate = False
regressor = "svr" # nn svr rfr mlr

d = os.path.dirname(working_dir)
if not os.path.exists(d):
    os.makedirs(d)

if simulate :
    bv.generateInputBVDistribution(input_var_file, nbSamples_train)
    bv.generateInputBVDistribution(input_var_file_test, nbSamples_test)

simus_list = []
#simus_list.append(fsat_data)
simus_list.append(spot4_data)
simus_list.append(lsat_data)

for sat in simus_list:
    sat_name = sat[0]
    rsr_file = sat[1]
    red_index = 0
    nir_index = 0
    if sat_name == "formosat2":
        red_index = 3
        nir_index = 4
    if sat_name == "spot4":
        red_index = 2
        nir_index = 3
    if sat_name == "landsat8":
        red_index = 2
        nir_index = 3
    for acqu in sat[2:]:
        print "-------"+sat_name+"_"+str(acqu['doy'])+"_"+regressor
        reflectance_file = working_dir+sat_name+"_"+str(acqu['doy'])+"_reflectances"
        training_file = working_dir+sat_name+"_"+str(acqu['doy'])+"_training"
        reflectance_file_test = working_dir+sat_name+"_"+str(acqu['doy'])+"_reflectances_test"
        training_file_test = working_dir+sat_name+"_"+str(acqu['doy'])+"_training_test"
        normalization_file = working_dir+sat_name+"_"+str(acqu['doy'])+"_normalization"
        inversion_file = working_dir+sat_name+"_"+str(acqu['doy'])+"_inversion_"+regressor
        model_file = working_dir+sat_name+"_"+str(acqu['doy'])+"_model_"+regressor
        validation_file = working_dir+sat_name+"_"+str(acqu['doy'])+"_validation_"+regressor
        reflectances_gt_file = working_dir+sat_name+"_"+str(acqu['doy'])+"_reflectances_gt"
        inversion_gt_file = working_dir+sat_name+"_"+str(acqu['doy'])+"_inversion_gt_"+regressor
        validation_gt_file = working_dir+sat_name+"_"+str(acqu['doy'])+"_validation_gt_"+regressor
        simuPars = {}
        simuPars['rsrFile'] = rsr_file
        simuPars['outputFile'] = reflectance_file
        simuPars['solarZenithAngle'] = acqu['ts']
        simuPars['sensorZenithAngle'] = acqu['to']
        simuPars['solarSensorAzimuth'] = acqu['ps']-acqu['po']
        simuPars['soilFile'] = "whatever"
        simuPars['noisevar'] = noise_var
        if simulate :
            print "\tSimulation training"
            bv.generateTrainingData(input_var_file, simuPars, training_file, bv.bvindex[varName], False, red_index, nir_index)
        simuPars['outputFile'] = reflectance_file_test
        if simulate :
            print "\tSimulation testing data"
            bv.generateTrainingData(input_var_file_test, simuPars, training_file_test, bv.bvindex[varName], False, red_index, nir_index)
        print "\tLearning model"
        bv.learnBVModel(training_file, model_file, regressor, normalization_file, bestof)
        print "\tInversion for test data"
        bv.invertBV(reflectance_file_test, model_file, normalization_file, inversion_file, True, red_index, nir_index)
        with open(inversion_file, 'r') as ivf:
            with open(training_file_test, 'r') as tft:
                with open(validation_file, 'w') as vaf:
                    for(ivline, tftline) in zip(ivf.readlines(), tft.readlines()):
                        outline = string.split(ivline)[0]+" "+string.split(tftline)[0]+"\n"
                        vaf.write(outline)
        var_values_gt = []
        var_values_bvnet = []
        with open(reflectances_gt_file, 'w') as rfgtf:
            for gt_case in acqu['gt']:
                for refl in gt_case['refls']:
                    rfgtf.write(str(refl/1000.0)+" ")
                rfgtf.write("\n")
                var_values_gt.append(gt_case[bv.bv_val_names[varName][0]])
                var_values_bvnet.append(gt_case[bv.bv_val_names[varName][1]])
        bv.addVI(reflectances_gt_file, red_index, nir_index)
        print "\tInversion for validation data"
        bv.invertBV(reflectances_gt_file, model_file, normalization_file, inversion_gt_file)
        with open(inversion_gt_file, 'r') as ivgtf:
            with open(validation_gt_file, 'w') as vgtf:
                for (ival, gtval, bvnetval) in zip(ivgtf.readlines(),var_values_gt,var_values_bvnet):
                    vgtf.write(str(gtval)+" "+str(bvnetval)+" "+str(ival))
                
                        
