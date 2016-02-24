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

add_library(OTBPheno dateUtils.cxx phenoFunctions.cxx miscUtils.cxx)
target_link_libraries(OTBPheno ITKOptimizers boost_regex)

install(TARGETS OTBPheno DESTINATION usr/lib/)
