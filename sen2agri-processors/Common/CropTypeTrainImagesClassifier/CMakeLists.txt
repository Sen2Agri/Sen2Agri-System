otb_create_application(
  NAME           CropTypeTrainImagesClassifier
  SOURCES        otbTrainImagesClassifier.cxx otbListSampleGeneratorRaster.txx otbTrainImagesClassifier.h otbListSampleGeneratorRaster.h otbTrainSVM.cxx otbTrainRandomForests.cxx otbTrainNormalBayes.cxx otbTrainNeuralNetwork.cxx otbTrainLibSVM.cxx otbTrainKNN.cxx otbTrainGradientBoostedTree.cxx otbTrainDecisionTree.cxx otbTrainBoost.cxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_CropTypeTrainImagesClassifier DESTINATION usr/lib/otb/applications/)
