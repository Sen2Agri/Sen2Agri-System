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

import sys
import os
import string
from config import Config
import otbApplication as otb

# The indices here have to be coherent with the order used in the bv file (and the definition of the vars in otbBVTypes.h
bvindex = {"MLAI": 0, "ALA": 1, "CrownCover": 2, "HsD": 3, "N": 4, "Cab": 5, "Car": 6, "Cdm": 7, "CwRel": 4, "Cbp": 9, "Bs": 10, "FAPAR": 11, "FCOVER": 12}

bv_val_names = {"MLAI": ['gai', 'lai-bvnet'], "FAPAR": ['fapar', 'fapar-bvnet'], "FCOVER": ['fcover', 'fcover-bvnet']}

def parseConfigFile(cfg):
    distFileName = cfg.bvDistribution.fileName
    nSamples = cfg.bvDistribution.samples
    simuPars = {}
    simuPars['soilFile'] = cfg.simulation.soilFile
    simuPars['rsrFile'] = cfg.simulation.rsrFile
    simuPars['outputFile'] = cfg.simulation.outputFile
    simuPars['solarZenithAngle'] = cfg.simulation.solarZenithAngle
    simuPars['sensorZenithAngle'] = cfg.simulation.sensorZenithAngle
    simuPars['solarSensorAzimuth'] = cfg.simulation.solarSensorAzimuth
    trainingFile = cfg.training.fileName
    modelFile = cfg.learning.outputFileName
    return (distFileName, nSamples, simuPars, trainingFile, modelFile)

def addVI(reflectances_file, red_index, nir_index):
    rff = open(reflectances_file)
    allfields = rff.readlines()
    rff.close()
    with open(reflectances_file, 'w') as rf:
        for l in allfields:
            rfls = string.split(l)
            if len(rfls)>red_index:
                outline = string.join(string.split(l))
                red = float(rfls[red_index-1])
                pir = float(rfls[nir_index-1])
                epsilon = 0.001
                ndvi = (pir-red)/(pir+red+epsilon)
                rvi = pir/(red+epsilon)
                outline += " "+str(ndvi)+" "+str(rvi)+"\n"
                rf.write(outline)


def generateInputBVDistribution(bvFile, nSamples):
    app = otb.Registry.CreateApplication("BVInputVariableGeneration")
    app.SetParameterInt("samples", nSamples)
    app.SetParameterString("out", bvFile)
    app.ExecuteAndWriteOutput()

def generateTrainingData(bvFile, simuPars, trainingFile, bvidx, add_angles=False, red_index=0, nir_index=0):
    """
    Generate a training file using the file of biophysical vars (bvFile) and the simulation parameters dictionary (simuPars).
    Write the result to trainingFile. The first column will be the biovar to learn and the following columns will be 
    the reflectances. The add_angles parameter is used to store the viewing and solar angles as features. If red_index and
    nir_index are set, the ndvi and the rvi are also used as features. red_index=3 means that the red reflectance is the 3rd column
    (starting at 1) in the reflectances file.
    """
    app = otb.Registry.CreateApplication("ProSailSimulator")
    app.SetParameterString("bvfile", bvFile)
    app.SetParameterString("soilfile", simuPars['soilFile'])
    app.SetParameterString("rsrfile", simuPars['rsrFile'])
    app.SetParameterString("out", simuPars['outputFile'])
    app.SetParameterFloat("solarzenith", simuPars['solarZenithAngle'])
    app.SetParameterFloat("sensorzenith", simuPars['sensorZenithAngle'])
    app.SetParameterFloat("azimuth", simuPars['solarSensorAzimuth'])
    app.SetParameterFloat("noisevar", simuPars['noisevar'])
    app.ExecuteAndWriteOutput()
    #combine the bv samples, the angles and the simulated reflectances for variable inversion and produce the training file
    with open(trainingFile, 'w') as tf:
        with open(bvFile, 'r') as bvf:
            bvf.readline() #header line
            with open(simuPars['outputFile'], 'r') as rf:
                #the output line follows the format: outputvar inputvar1 inputvar2 ... inputvarN
                for (refline, bvline) in zip(rf.readlines(), bvf.readlines()):
                    outline = ""
                    if bvidx == bvindex["FCOVER"] :
                        outline = string.split(refline)[-1]
                    elif bvidx == bvindex["FAPAR"] : 
                        outline = string.split(refline)[-2]
                    else:
                        outline = string.split(bvline)[bvidx]
                    outline = outline+" "+string.join(string.split(refline[:-1])[:-2], ' ')
                    outline.rstrip()
                    if add_angles:
                        angles = `simuPars['solarZenithAngle']`+" "+`simuPars['sensorZenithAngle']`+" "+`simuPars['solarSensorAzimuth']`
                        outline += " "+angles
                    outline += "\n"
                    tf.write(outline)
    if red_index!=0 and nir_index!=0:
        # we need to add 1 to the indices since the file already contains the variable in the first column
        addVI(trainingFile, red_index+1, nir_index+1)
                
                
def learnBVModel(trainingFile, outputFile, regressionType, normalizationFile, bestof=1):
    app = otb.Registry.CreateApplication("InverseModelLearning")
    app.SetParameterString("training", trainingFile)
    app.SetParameterString("out", outputFile)
    app.SetParameterString("errest", outputFile+"_errest")
    app.SetParameterString("regression", regressionType)
    app.SetParameterString("normalization", normalizationFile)
    app.SetParameterInt("bestof", bestof)
    app.ExecuteAndWriteOutput()

def invertBV(reflectanceFile, modelFile, normalizationFile, outputFile, removeFaparFcover=False, red_index=0, nir_index=0):
    if removeFaparFcover:
        #the reflectance file contains also the simulations of fapar and fcover
        rff = open(reflectanceFile)
        allfields = rff.readlines()
        rff.close()
        with open(reflectanceFile, 'w') as rf:
            for l in allfields:
                outline = string.join(string.split(l)[:-2])+"\n"
                rf.write(outline)

    if red_index!=0 and nir_index!=0:
        addVI(reflectanceFile, red_index, nir_index)

                
    app = otb.Registry.CreateApplication("BVInversion")
    app.SetParameterString("reflectances", reflectanceFile)
    app.SetParameterString("model", modelFile)
    app.SetParameterString("normalization", normalizationFile)
    app.SetParameterString("out", outputFile)
    app.ExecuteAndWriteOutput()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.exit('Usage: %s <configurationFilePath>.cfg' % sys.argv[0])
    if not os.path.exists(sys.argv[1]):
        sys.exit('ERROR: Configuration File path %s was not found!'% sys.argv[1] )
    f = file ( sys.argv[1] )
    cfg=Config(f)

    (bvDistributionFileName, numberOfSamples, simulationParameters, trainingDataFileName, outputModelFileName) = parseConfigFile(cfg)
    generateInputBVDistribution(bvDistributionFileName, numberOfSamples)
    generateTrainingData(bvDistributionFileName, simulationParameters, trainingDataFileName, bvindex[cfg.training.invertBV])
    learnBVModel(trainingDataFileName, outputModelFileName)


