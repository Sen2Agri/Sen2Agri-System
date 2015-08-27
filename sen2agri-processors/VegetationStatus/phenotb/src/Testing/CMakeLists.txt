# =========================================================================
# Program:   pĥenotb
# Language:  C++
#
# Copyright (c) CESBIO. All rights reserved.
#
# See pĥenotb-copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.  See the above copyright notices for more information.
#
# =========================================================================

set(PHENO_TESTS ${phenotb_BINARY_DIR}/Testing/otbPhenoTests)

add_test(phTuParameterCostFunctionInstance ${PHENO_TESTS}
  phenoParameterCostFunctionInstance)

add_test(phTrphenoNormalizedSigmoid ${PHENO_TESTS}
  phenoNormalizedSigmoid)       

add_test(phTrphenoNormalizedSigmoidMetrics ${PHENO_TESTS}
  phenoNormalizedSigmoidMetrics)       

add_test(phTrphenoEarlyEmergence ${PHENO_TESTS}
  phenoEarlyEmergence)       

add_test(phTrphenoEarlyEmergenceCB ${PHENO_TESTS}
  phenoEarlyEmergenceCB)       

add_test(phTrTwoCycleFittingFunctor ${PHENO_TESTS}
  phenoTwoCycleFittingFunctor)       

set(PhenoTests_SRCS
  otbPhenoTests.cxx
  phenoParameterCostFunctionInstance.cxx
  phenoNormalizedSigmoidTest.cxx
  phenoNormalizedSigmoidMetrics.cxx
  phenoEarlyEmergenceTests.cxx
  phenoTwoCycleFittingFunctor.cxx)

add_executable(otbPhenoTests ${PhenoTests_SRCS})
target_link_libraries(otbPhenoTests ${OTB_LIBRARIES} OTBPheno gsl gslcblas)
