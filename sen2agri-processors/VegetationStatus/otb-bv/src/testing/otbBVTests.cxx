/*=========================================================================
  Program:   otb-bv
  Language:  C++

  Copyright (c) CESBIO. All rights reserved.

  See otb-bv-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "otbTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(bvProSailSimulatorFunctor);
  REGISTER_TEST(bvMultiLinearFitting);
  REGISTER_TEST(bvMultiLinearFittingConversions);
  REGISTER_TEST(bvMultiTemporalInversion);
  REGISTER_TEST(bvMultiTemporalInversionFromFile);
  REGISTER_TEST(bvLaiLogNdvi);
}
