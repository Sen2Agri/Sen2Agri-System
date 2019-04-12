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

OTB_CREATE_APPLICATION(NAME           BVInputVariableGeneration
  SOURCES        otbBVInputVariableGeneration.cxx
  LINK_LIBRARIES ${OTB_LIBRARIES};OTBBVUtil)

OTB_CREATE_APPLICATION(NAME           ProSailSimulator
  SOURCES        otbProSailSimulator.cxx
  LINK_LIBRARIES Sen2AgriProductReaders;${OTB_LIBRARIES};OTBBVUtil)

OTB_CREATE_APPLICATION(NAME           ProSailSimulatorNew
  SOURCES        otbProSailSimulatorNew.cxx
  LINK_LIBRARIES Sen2AgriProductReaders;${OTB_LIBRARIES};OTBBVUtil)

OTB_CREATE_APPLICATION(NAME           InverseModelLearning
  SOURCES        otbInverseModelLearning.cxx
  LINK_LIBRARIES Sen2AgriProductReaders;${OTB_LIBRARIES};OTBBVUtil;gsl;gslcblas)

OTB_CREATE_APPLICATION(NAME           BVInversion
  SOURCES        otbBVInversion.cxx
  LINK_LIBRARIES ${OTB_LIBRARIES};OTBBVUtil;gsl;gslcblas)

OTB_CREATE_APPLICATION(NAME           BVImageInversion
  SOURCES        otbBVImageInversion.cxx
  LINK_LIBRARIES Sen2AgriProductReaders;Sen2AgriCommonUtils;${OTB_LIBRARIES};OTBBVUtil;gsl;gslcblas)
  
OTB_CREATE_APPLICATION(NAME           ProfileReprocessing
  SOURCES        otbProfileReprocessing.cxx
  LINK_LIBRARIES Sen2AgriProductReaders;Sen2AgriCommonUtils;${OTB_LIBRARIES};OTBBVUtil;${PHENOTB_LIBRARY};gsl;gslcblas)
  
otb_create_application(
  NAME           TrainingDataGenerator
  SOURCES        otbTrainingDataGenerator.cpp
  LINK_LIBRARIES Sen2AgriProductReaders ${OTB_LIBRARIES})

otb_create_application(
  NAME           TrainingDataGeneratorNew
  SOURCES        otbTrainingDataGeneratorNew.cpp CommonFunctions.h
  LINK_LIBRARIES Sen2AgriProductReaders ${OTB_LIBRARIES})

otb_create_application(
  NAME           NdviRviExtraction2
  SOURCES        otbNdviRviExtraction2.cpp
  LINK_LIBRARIES Sen2AgriProductReaders Sen2AgriCommonUtils ${OTB_LIBRARIES})

otb_create_application(
  NAME           NdviRviExtractionNew
  SOURCES        otbNdviRviExtractionNew.cpp CommonFunctions.h
  LINK_LIBRARIES Sen2AgriProductReaders Sen2AgriCommonUtils ${OTB_LIBRARIES})

otb_create_application(
  NAME           TimeSeriesBuilder
  SOURCES        otbTimeSeriesBuilder.cpp
  LINK_LIBRARIES Sen2AgriProductReaders Sen2AgriCommonUtils ${OTB_LIBRARIES})

otb_create_application(
  NAME           GetLaiRetrievalModel
  SOURCES        otbGetLaiRetrievalModel.cpp
  LINK_LIBRARIES Sen2AgriProductReaders ${OTB_LIBRARIES})

otb_create_application(
  NAME           ReprocessedProfileSplitter2
  SOURCES        otbReprocessedProfileSplitter2.cpp
  LINK_LIBRARIES Sen2AgriProductReaders Sen2AgriCommonUtils ${OTB_LIBRARIES})

otb_create_application(
  NAME           GenerateLaiMonoDateMaskFlags
  SOURCES        otbGenerateLaiMonoDateMaskFlags.cpp
  LINK_LIBRARIES Sen2AgriProductReaders;${OTB_LIBRARIES})

otb_create_application(
  NAME           BVLaiNewProcessor
  SOURCES        otbBVLaiNewProcessor.cxx
  LINK_LIBRARIES Sen2AgriProductReaders ${OTB_LIBRARIES} OTBBVUtil)

otb_create_application(
  NAME           CreateAnglesRaster
  SOURCES        otbCreateAnglesRaster.cpp
  LINK_LIBRARIES Sen2AgriProductReaders ${OTB_LIBRARIES})

otb_create_application(
  NAME           GenerateDomainQualityFlags
  SOURCES        otbGenerateDomainQualityFlags.cxx
  LINK_LIBRARIES Sen2AgriProductReaders ${OTB_LIBRARIES})

otb_create_application(
  NAME           ImageAsImageResampler
  SOURCES        otbImageAsImageResampler.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES})

add_executable(testSimus testSimus.cxx)
target_link_libraries(testSimus ${OTB_LIBRARIES} OTBBVUtil gsl gslcblas)

install(TARGETS otbapp_BVInputVariableGeneration DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_ProSailSimulator DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_ProSailSimulatorNew DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_InverseModelLearning DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_BVInversion DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_BVImageInversion DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_ProfileReprocessing DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_TrainingDataGenerator DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_TrainingDataGeneratorNew DESTINATION usr/lib/otb/applications/)
#install(TARGETS otbapp_NdviRviExtraction2 DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_NdviRviExtractionNew DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_TimeSeriesBuilder DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_GetLaiRetrievalModel DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_ReprocessedProfileSplitter2 DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_GenerateLaiMonoDateMaskFlags DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_BVLaiNewProcessor DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_CreateAnglesRaster DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_GenerateDomainQualityFlags DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_ImageAsImageResampler DESTINATION usr/lib/otb/applications/)
