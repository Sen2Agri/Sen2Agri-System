otb_create_application(
  NAME           TrainImagesClassifierNew
  SOURCES        otbTrainImagesClassifier.cxx otbListSampleGeneratorRaster.txx otbTrainImagesClassifier.h otbListSampleGeneratorEx.h otbListSampleGeneratorEx.txx otbListSampleGeneratorRaster.h otbTrainSVM.cxx otbTrainRandomForests.cxx otbTrainNormalBayes.cxx otbTrainNeuralNetwork.cxx otbTrainLibSVM.cxx otbTrainKNN.cxx otbTrainGradientBoostedTree.cxx otbTrainDecisionTree.cxx otbTrainBoost.cxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_TrainImagesClassifierNew DESTINATION usr/lib/otb/applications/)
