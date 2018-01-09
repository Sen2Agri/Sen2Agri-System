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

add_library(OTBBVUtil otbBVUtil.cxx)

set(BelcamNeural_SRCS
  belcamApplyTrainedNeuralNetwork.cpp
)

add_executable(ApplyTrainedNeuralNetwork ${BelcamNeural_SRCS})

target_link_libraries(ApplyTrainedNeuralNetwork ${OTB_LIBRARIES})

install(TARGETS OTBBVUtil DESTINATION usr/lib/)
