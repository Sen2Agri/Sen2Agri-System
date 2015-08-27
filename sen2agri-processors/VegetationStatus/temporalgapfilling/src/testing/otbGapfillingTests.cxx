/*=========================================================================

  Program:   gapfilling
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See gapfilling-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "otbTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(linearGapfillingTest);
  REGISTER_TEST(splineGapfillingTest);
  REGISTER_TEST(multiComponentGapfillingTest);
  REGISTER_TEST(multiComponentOutputDatesGapfillingTest);
  REGISTER_TEST(imageFunctionGapfillingTest);
  REGISTER_TEST(deinterlacingTest);
  REGISTER_TEST(interlacingTest);
  REGISTER_TEST(linearWithOutputDatesGapfillingTest);
  REGISTER_TEST(linearWithRealDates);
  REGISTER_TEST(splineWithRealDates);
}
