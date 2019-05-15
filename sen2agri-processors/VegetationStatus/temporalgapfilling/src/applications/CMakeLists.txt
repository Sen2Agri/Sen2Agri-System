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
IF (WIN32)
    link_directories(C:/OTB-release-xdk-win32/lib)
ENDIF(WIN32)

add_executable(gapfilling gapfilling.cxx)
target_link_libraries(gapfilling ${OTB_LIBRARIES} ${PHENOTB_LIBRARY} gsl gslcblas)
