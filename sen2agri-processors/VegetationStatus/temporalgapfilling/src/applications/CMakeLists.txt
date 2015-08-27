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

add_executable(gapfilling gapfilling.cxx)
target_link_libraries(gapfilling ${OTB_LIBRARIES} ${PHENOTB_LIBRARY} gsl gslcblas)
