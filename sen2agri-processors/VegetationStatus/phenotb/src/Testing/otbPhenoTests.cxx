/*=========================================================================

  Program:   phenotb
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See phenotb-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "otbTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(phenoParameterCostFunctionInstance);
  REGISTER_TEST(phenoNormalizedSigmoid);
  REGISTER_TEST(phenoNormalizedSigmoidMetrics);
  REGISTER_TEST(phenoEarlyEmergence);
  REGISTER_TEST(phenoEarlyEmergenceCB);
  REGISTER_TEST(phenoTwoCycleFittingFunctor);
}
