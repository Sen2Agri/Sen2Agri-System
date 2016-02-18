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
  LINK_LIBRARIES MACCSMetadata;MetadataHelper;${OTB_LIBRARIES};OTBBVUtil)

OTB_CREATE_APPLICATION(NAME           InverseModelLearning
  SOURCES        otbInverseModelLearning.cxx
  LINK_LIBRARIES MACCSMetadata;MetadataHelper;${OTB_LIBRARIES};OTBBVUtil;gsl;gslcblas)

OTB_CREATE_APPLICATION(NAME           BVInversion
  SOURCES        otbBVInversion.cxx
  LINK_LIBRARIES ${OTB_LIBRARIES};OTBBVUtil;gsl;gslcblas)

OTB_CREATE_APPLICATION(NAME           BVImageInversion
  SOURCES        otbBVImageInversion.cxx
  LINK_LIBRARIES MACCSMetadata;MetadataHelper;${OTB_LIBRARIES};OTBBVUtil;gsl;gslcblas)
  
OTB_CREATE_APPLICATION(NAME           ProfileReprocessing
  SOURCES        otbProfileReprocessing.cxx
  LINK_LIBRARIES MACCSMetadata;MetadataHelper;${OTB_LIBRARIES};OTBBVUtil;${PHENOTB_LIBRARY};gsl;gslcblas)
  
otb_create_application(
  NAME           TrainingDataGenerator
  SOURCES        otbTrainingDataGenerator.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})
  
otb_create_application(
  NAME           NdviRviExtraction
  SOURCES        otbNdviRviExtraction.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

otb_create_application(
  NAME           NdviRviExtraction2
  SOURCES        otbNdviRviExtraction2.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})
  
otb_create_application(
  NAME           TimeSeriesBuilder
  SOURCES        otbTimeSeriesBuilder.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

otb_create_application(
  NAME           GetLaiRetrievalModel
  SOURCES        otbGetLaiRetrievalModel.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

otb_create_application(
  NAME           ReprocessedProfileSplitter
  SOURCES        otbReprocessedProfileSplitter.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

otb_create_application(
  NAME           ReprocessedProfileSplitter2
  SOURCES        otbReprocessedProfileSplitter2.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

otb_create_application(
  NAME           GenerateLaiMonoDateMaskFlags
  SOURCES        otbGenerateLaiMonoDateMaskFlags.cpp
  LINK_LIBRARIES MACCSMetadata;MetadataHelper;${OTB_LIBRARIES})

add_executable(testSimus testSimus.cxx)
target_link_libraries(testSimus ${OTB_LIBRARIES} OTBBVUtil gsl gslcblas)
