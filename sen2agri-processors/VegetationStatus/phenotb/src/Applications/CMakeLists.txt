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

add_executable(explore explore.cxx)
target_link_libraries(explore ${OTB_LIBRARIES} OTBPheno)

add_executable(date_parsing date_parsing.cxx)
target_link_libraries(date_parsing ${OTB_LIBRARIES} OTBPheno)

add_executable(explore-maiseo explore-maiseo.cxx)
target_link_libraries(explore-maiseo ${OTB_LIBRARIES} OTBPheno)

add_executable(explore-sirhyus explore-sirhyus.cxx)
target_link_libraries(explore-sirhyus ${OTB_LIBRARIES} OTBPheno)

add_executable(explore-projections explore-projections.cxx)
target_link_libraries(explore-projections ${OTB_LIBRARIES} OTBPheno)

add_executable(explore-lai explore-lai.cxx)
target_link_libraries(explore-lai ${OTB_LIBRARIES} OTBPheno)

add_executable(explore-sg explore-sg.cxx)
target_link_libraries(explore-sg ${OTB_LIBRARIES} OTBPheno)

otb_create_application(NAME           PhenologicalNDVIMetrics
  SOURCES        PhenologicalNDVIMetrics.cxx
  LINK_LIBRARIES ${OTB_LIBRARIES};OTBPheno;Sen2AgriProductReaders;Sen2AgriCommonUtils)

otb_create_application(NAME           PhenoMetricsSplitter
  SOURCES        otbPhenoMetricsSplitter.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES};OTBPheno;Sen2AgriProductReaders;Sen2AgriCommonUtils)

otb_create_application(NAME           NdviMaskSeriesExtractor
  SOURCES        NdviMaskSeriesExtractor.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES};Sen2AgriProductReaders;Sen2AgriCommonUtils)

add_executable(test-sigmo test-sigmo.cxx)
target_link_libraries(test-sigmo ${OTB_LIBRARIES} OTBPheno)

add_executable(sigmo-maiseo sigmo-maiseo.cxx)
target_link_libraries(sigmo-maiseo ${OTB_LIBRARIES} OTBPheno)

add_executable(pheno-metrics-simulator pheno-metrics-simulator.cxx)
target_link_libraries(pheno-metrics-simulator ${OTB_LIBRARIES} OTBPheno)

add_executable(pheno-metrics-estimator pheno-metrics-estimator.cxx)
target_link_libraries(pheno-metrics-estimator ${OTB_LIBRARIES} OTBPheno)

install(TARGETS otbapp_PhenologicalNDVIMetrics DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_PhenoMetricsSplitter DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_NdviMaskSeriesExtractor DESTINATION usr/lib/otb/applications/)
