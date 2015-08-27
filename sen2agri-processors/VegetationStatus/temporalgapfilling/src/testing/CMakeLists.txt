# =========================================================================
# Program:   gapfilling
# Language:  C++
#
# Copyright (c) CESBIO. All rights reserved.
#
# See gapfilling-copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.  See the above copyright notices for more information.
#
# =========================================================================

set(GAPFILLING_TESTS ${GAPFILLING_BINARY_DIR}/testing/otbGapfillingTests)

add_test(linearGapfillingTest ${GAPFILLING_TESTS}
  linearGapfillingTest)

add_test(splineGapfillingTest ${GAPFILLING_TESTS}
  splineGapfillingTest)

add_test(multiIdentityComponentGapfillingTest ${GAPFILLING_TESTS}
  multiComponentGapfillingTest 0)

add_test(multiLinearComponentGapfillingTest ${GAPFILLING_TESTS}
  multiComponentGapfillingTest 1)

add_test(multiSplineComponentGapfillingTest ${GAPFILLING_TESTS}
  multiComponentGapfillingTest 2)

add_test(imageFunctionGapfillingTest ${GAPFILLING_TESTS}
  imageFunctionGapfillingTest ${TEMP}/ 1)

add_test(imageFunctionGapfillingMultiCompTest ${GAPFILLING_TESTS}
  imageFunctionGapfillingTest ${TEMP}/ 2)

add_test(deinterlacingTest ${GAPFILLING_TESTS}
  deinterlacingTest)

add_test(interlacingTest ${GAPFILLING_TESTS}
  interlacingTest)

add_test(linearWithOutputDatesGapfillingTest ${GAPFILLING_TESTS}
  linearWithOutputDatesGapfillingTest)

add_test(multiComponentOutputDatesGapfillingTest ${GAPFILLING_TESTS}
  multiComponentOutputDatesGapfillingTest)

add_test(linearWithRealDatesTest ${GAPFILLING_TESTS}
  linearWithRealDates)

add_test(splineWithRealDatesTest ${GAPFILLING_TESTS}
  splineWithRealDates)

set(GapfillingTests_SRCS
  otbGapfillingTests.cxx
  LinearGapfillingTest.cxx
  SplineGapfillingTest.cxx
  MultiComponentTest.cxx
  ImageFunctionGapfillingTest.cxx
  InterlacingTests.cxx
  MultiComponentOutputDatesTest.cxx)

add_executable(otbGapfillingTests ${GapfillingTests_SRCS})
target_link_libraries(otbGapfillingTests ${OTB_LIBRARIES} ${PHENOTB_LIBRARY} gsl gslcblas)
