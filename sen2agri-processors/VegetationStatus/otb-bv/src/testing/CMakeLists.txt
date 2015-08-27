# =========================================================================
# Program:   otb-bv
# Language:  C++
#
# Copyright (c) CESBIO. All rights reserved.
#
# See otb-bv-copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.  See the above copyright notices for more information.
#
# =========================================================================

### Test the applications
add_test(appBvGenInputVars otbApplicationLauncherCommandLine 
  BVInputVariableGeneration ${otb-bv_BINARY_DIR}/applications/ 
  -samples 2000 
  -out ${TEMP}/appBvGenInputVarssamples.txt)

add_test(appBvProSailSim otbApplicationLauncherCommandLine
  ProSailSimulator ${otb-bv_BINARY_DIR}/applications/ 
  -bvfile ${otb-bv_SOURCE_DIR}/../data/appBvGenInputVarssamples.txt
  -rsrfile ${otb-bv_SOURCE_DIR}/../data/formosat2_4b.rsr
  -out ${TEMP}/appProSailSimus.txt
  -solarzenith 33.469
  -sensorzenith 20.071
  -azimuth 169.0)

add_test(appBvInvModLear otbApplicationLauncherCommandLine
  InverseModelLearning ${otb-bv_BINARY_DIR}/applications/
  -training ${otb-bv_SOURCE_DIR}/../data/train-refls-lai.txt
  -regression nn
  -out ${TEMP}/appInvMod.txt
  -errest ${TEMP}/appInvModErrEst.txt)

add_test(appBvInversion otbApplicationLauncherCommandLine
  BVInversion ${otb-bv_BINARY_DIR}/applications/
  -reflectances ${otb-bv_SOURCE_DIR}/../data/relfs_sim.txt
  -model ${otb-bv_SOURCE_DIR}/../data/appInvMod.txt
  -out ${TEMP}/appInvLai.txt)

### Test functions and classes
set(OTBBV_TESTS ${otb-bv_BINARY_DIR}/testing/otbBVTests)

add_test(bvProSailSimulatorFunctor ${OTBBV_TESTS}
  bvProSailSimulatorFunctor ${otb-bv_SOURCE_DIR}/../data/formosat2_4b.rsr)

add_test(bvMultiLinearFitting ${OTBBV_TESTS}
  bvMultiLinearFitting)       

add_test(bvMultiLinearFittingConversions ${OTBBV_TESTS}
  bvMultiLinearFittingConversions)

add_test(bvMultiTemporalInversion ${OTBBV_TESTS}
  bvMultiTemporalInversion ${otb-bv_SOURCE_DIR}/../data/formosat2_4b.rsr
  33.469
  20.071
  169.0
  ${otb-bv_SOURCE_DIR}/../data/appInvMod.txt
  ${otb-bv_SOURCE_DIR}/../data/appInvModErrEst.txt
  ${TEMP}/bvMulTeInvRes.txt)

add_test(bvMultiTemporalInversionRadius ${OTBBV_TESTS}
  bvMultiTemporalInversion ${otb-bv_SOURCE_DIR}/../data/formosat2_4b.rsr
  33.469
  20.071
  169.0
  ${otb-bv_SOURCE_DIR}/../data/appInvMod.txt
  ${otb-bv_SOURCE_DIR}/../data/appInvModErrEst.txt
  ${TEMP}/bvMulTeInvResRadius.txt
  5
  0)

add_test(bvMultiTemporalInversionFile ${OTBBV_TESTS}
  bvMultiTemporalInversionFromFile ${otb-bv_SOURCE_DIR}/../data/bvMulTeInvRes.txt
  1
  1
  10e-5)

add_test(bvLaiLogNdvi ${OTBBV_TESTS}
  bvLaiLogNdvi)

set(BVTests_SRCS
  otbBVTests.cxx
  bvProSailSimulatorFunctor.cxx
  bvMultiLinearFitting.cxx
  bvMultiTemporalInversion.cxx
  bvLaiLogNdviTest.cxx)

add_executable(otbBVTests ${BVTests_SRCS})
target_link_libraries(otbBVTests ${OTB_LIBRARIES} OTBBVUtil ${PHENOTB_LIBRARY} gsl gslcblas)



