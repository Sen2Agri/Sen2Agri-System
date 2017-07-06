/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
/*=========================================================================
 Program:   ORFEO Toolbox
 Language:  C++
 Date:      $Date$
 Version:   $Revision$


 Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
 See OTBCopyright.txt for details.


 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "otbTrainImagesClassifier.h"

namespace otb
{
namespace Wrapper
{

void CropMaskTrainImagesClassifier::DoInit()
{
  SetName("CropMaskTrainImagesClassifier");
  SetDescription(
    "Train a classifier from multiple pairs of images and training vector data.");

  // Documentation
  SetDocName("Train a classifier from multiple images");
  SetDocLongDescription(
    "This application performs a classifier training from multiple pairs of input images and training vector data. "
    "Samples are composed of pixel values in each band optionally centered and reduced using an XML statistics file produced by "
    "the ComputeImagesStatistics application.\n The training vector data must contain polygons with a positive integer field "
    "representing the class label. The name of this field can be set using the \"Class label field\" parameter. Training and validation "
    "sample lists are built such that each class is equally represented in both lists. One parameter allows to control the ratio "
    "between the number of samples in training and validation sets. Two parameters allow to manage the size of the training and "
    "validation sets per class and per image.\n Several classifier parameters can be set depending on the chosen classifier. In the "
    "validation process, the confusion matrix is organized the following way: rows = reference labels, columns = produced labels. "
    "In the header of the optional confusion matrix output file, the validation (reference) and predicted (produced) class labels"
    " are ordered according to the rows/columns of the confusion matrix.\n This application is based on LibSVM and on OpenCV Machine Learning "
    "classifiers, and is compatible with OpenCV 2.3.1 and later.");
  SetDocLimitations("None");
  SetDocAuthors("OTB-Team");
  SetDocSeeAlso("OpenCV documentation for machine learning http://docs.opencv.org/modules/ml/doc/ml.html ");

  AddDocTag(Tags::Learning);

  //Group IO
  AddParameter(ParameterType_Group, "io", "Input and output data");
  SetParameterDescription("io", "This group of parameters allows to set input and output data.");
//  AddParameter(ParameterType_InputImageList, "io.il", "Input Image List");
//  SetParameterDescription("io.il", "A list of input images.");
  AddParameter(ParameterType_InputVectorData, "io.vd", "Input Vector Data");
  SetParameterDescription("io.vd", "Vector data to select the training samples.");
  AddParameter(ParameterType_OutputFilename, "io.confmatout", "Output confusion matrix");
  SetParameterDescription("io.confmatout", "Output file containing the confusion matrix (.csv format).");
  MandatoryOff("io.confmatout");
  AddParameter(ParameterType_OutputFilename, "io.out", "Output model");
  SetParameterDescription("io.out", "Output file containing the model estimated (.txt format).");

  //LBU
  // Add the possibility to use a raster to describe the training samples.
  MandatoryOff("io.vd");
  AddParameter(ParameterType_InputImage, "io.rs", "Training samples in a raster");
  SetParameterDescription("io.rs", "A raster containing the training samples.");
  MandatoryOff("io.rs");
  AddParameter(ParameterType_Int, "nodatalabel", "No data label");
  SetParameterDescription("nodatalabel", "The label of the ignored pixels from the raster");
  MandatoryOff("nodatalabel");
  SetDefaultParameterInt("nodatalabel", 0);

  AddParameter(ParameterType_InputFilename, "imstat", "Statistics file");
  SetParameterDescription("imstat", "An XML file containing mean and standard deviation to center and reduce samples before classification (produced by ComputeImagesStatistics application).");
  MandatoryOff("imstat");

  AddParameter(ParameterType_OutputFilename, "outdays", "Resampled output days");
  SetParameterDescription("outdays", "The output days after temporal resampling.");

  AddParameter(ParameterType_InputFilenameList, "il", "Input descriptors");
  SetParameterDescription( "il", "The list of descriptors. They must be sorted by tiles." );

  AddParameter(ParameterType_StringList, "prodpertile", "Products per tile");
  SetParameterDescription("prodpertile", "The number of products corresponding to each tile");
  MandatoryOff("prodpertile");

  AddParameter(ParameterType_StringList, "sp", "Temporal sampling rate");

  AddParameter(ParameterType_Choice, "mode", "Mode");
  SetParameterDescription("mode", "Specifies the choice of output dates (default: resample)");
  AddChoice("mode.resample", "Specifies the temporal resampling mode");
  AddChoice("mode.gapfill", "Specifies the gapfilling mode");
  AddChoice("mode.gapfillmain", "Specifies the gapfilling mode, but only use non-main series products to fill in the main one");
  SetParameterString("mode", "resample");


  AddParameter(ParameterType_Int, "window", "The number of dates in the temporal window");
  SetDefaultParameterInt("window", 2);

  AddParameter(ParameterType_Empty,
               "bm",
               "If set use the features from Benchmarking instead of the features from ATBD");
  MandatoryOff("bm");

  AddParameter(ParameterType_Float, "pixsize", "The size of a pixel, in meters");
  SetDefaultParameterFloat("pixsize", 10.0); // The default value is 10 meters
  SetMinimumParameterFloatValue("pixsize", 1.0);
  MandatoryOff("pixsize");

  AddParameter(ParameterType_String, "mission", "The main raster series that will be used. By default SPOT is used");
  MandatoryOff("mission");

  AddParameter(ParameterType_Empty, "rededge", "Include Sentinel-2 vegetation red edge bands");
  MandatoryOff("rededge");

  //LBU

  // Elevation
  ElevationParametersHandler::AddElevationParameters(this, "elev");

  //Group Sample list
  AddParameter(ParameterType_Group, "sample", "Training and validation samples parameters");
  SetParameterDescription("sample",
                          "This group of parameters allows to set training and validation sample lists parameters.");

  AddParameter(ParameterType_Int, "sample.mt", "Maximum training sample size per class");
  //MandatoryOff("mt");
  SetDefaultParameterInt("sample.mt", 1000);
  SetParameterDescription("sample.mt", "Maximum size per class (in pixels) of the training sample list (default = 1000) (no limit = -1). If equal to -1, then the maximal size of the available training sample list per class will be equal to the surface area of the smallest class multiplied by the training sample ratio.");
  AddParameter(ParameterType_Int, "sample.mv", "Maximum validation sample size per class");
  // MandatoryOff("mv");
  SetDefaultParameterInt("sample.mv", 1000);
  SetParameterDescription("sample.mv", "Maximum size per class (in pixels) of the validation sample list (default = 1000) (no limit = -1). If equal to -1, then the maximal size of the available validation sample list per class will be equal to the surface area of the smallest class multiplied by the validation sample ratio.");

  AddParameter(ParameterType_Int, "sample.bm", "Bound sample number by minimum");
  SetDefaultParameterInt("sample.bm", 1);
  SetParameterDescription("sample.bm", "Bound the number of samples for each class by the number of available samples by the smaller class. Proportions between training and validation are respected. Default is true (=1).");


  AddParameter(ParameterType_Empty, "sample.edg", "On edge pixel inclusion");
  SetParameterDescription("sample.edg",
                          "Takes pixels on polygon edge into consideration when building training and validation samples.");
  MandatoryOff("sample.edg");

  AddParameter(ParameterType_Float, "sample.vtr", "Training and validation sample ratio");
  SetParameterDescription("sample.vtr",
                          "Ratio between training and validation samples (0.0 = all training, 1.0 = all validation) (default = 0.5).");
  SetParameterFloat("sample.vtr", 0.5);

  AddParameter(ParameterType_String, "sample.vfn", "Name of the discrimination field");
  SetParameterDescription("sample.vfn", "Name of the field used to discriminate class labels in the input vector data files.");
  SetParameterString("sample.vfn", "Class");

  AddParameter(ParameterType_Choice, "classifier", "Classifier to use for the training");
  SetParameterDescription("classifier", "Choice of the classifier to use for the training.");

  //Group LibSVM
#ifdef OTB_USE_LIBSVM
  InitLibSVMParams();
#endif

#ifdef OTB_USE_OPENCV
  InitSVMParams();
  InitBoostParams();
  InitDecisionTreeParams();
  InitGradientBoostedTreeParams();
  InitNeuralNetworkParams();
  InitNormalBayesParams();
  InitRandomForestsParams();
  InitKNNParams();
#endif

  AddRANDParameter();
  // Doc example parameter settings
//  SetDocExampleParameterValue("io.il", "QB_1_ortho.tif");
  SetDocExampleParameterValue("io.vd", "VectorData_QB1.shp");
  SetDocExampleParameterValue("sample.mv", "100");
  SetDocExampleParameterValue("sample.mt", "100");
  SetDocExampleParameterValue("sample.vtr", "0.5");
  SetDocExampleParameterValue("sample.edg", "false");
  SetDocExampleParameterValue("sample.vfn", "Class");
  SetDocExampleParameterValue("classifier", "libsvm");
  SetDocExampleParameterValue("classifier.libsvm.k", "linear");
  SetDocExampleParameterValue("classifier.libsvm.c", "1");
  SetDocExampleParameterValue("classifier.libsvm.opt", "false");
  SetDocExampleParameterValue("io.out", "svmModelQB1.txt");
  SetDocExampleParameterValue("io.confmatout", "svmConfusionMatrixQB1.csv");
}

void CropMaskTrainImagesClassifier::DoUpdateParameters()
{
  // Nothing to do here : all parameters are independent
}

void CropMaskTrainImagesClassifier::DoExecute()
{
  GetLogger()->Debug("Entering DoExecute\n");

  TemporalResamplingMode resamplingMode = TemporalResamplingMode::Resample;
  const auto &modeStr = GetParameterString("mode");
  if (modeStr == "gapfill") {
      resamplingMode = TemporalResamplingMode::GapFill;
  } else if (modeStr == "gapfillmain") {
      resamplingMode = TemporalResamplingMode::GapFillMainMission;
  }

  std::vector<SensorPreferences> sp;
  if (HasValue("sp")) {
      const auto &spValues = GetParameterStringList("sp");
      sp = parseSensorPreferences(spValues);
  }

  // Get the list of input files
  const std::vector<std::string> &descriptors = this->GetParameterStringList("il");
  if( descriptors.size()== 0 )
    {
    itkExceptionMacro("No input file set...");
    }

  // Get the number of products per tile
  std::vector<int> prodPerTile;
  unsigned int numDesc = 0;
  if (HasValue("prodpertile")) {
      const auto &prodPerTileStrings = GetParameterStringList("prodpertile");
      auto n = prodPerTileStrings.size();

      for (size_t i = 0; i < n; i ++) {
          const auto ppts = prodPerTileStrings[i];
          auto ppt = std::stoi(ppts);
          if (ppt <= 0) {
              itkExceptionMacro("Invalid number of products " << ppts)
          }
          prodPerTile.push_back(ppt);
          numDesc += ppt;
      }
  } else {
      // All products belong to the same tile
      prodPerTile.push_back(descriptors.size());
      numDesc = descriptors.size();
  }

  // get the required pixel size
  auto pixSize = this->GetParameterFloat("pixsize");
  // get the main mission
  std::string mission = SPOT;
  if (HasValue("mission")) {
      mission = this->GetParameterString("mission");
  }

  if( descriptors.size() != numDesc )
    {
    itkExceptionMacro("The number of descriptors (" << descriptors.size() << ") is not consistent with the sum of products per tile (" << numDesc << ")")
    }


  auto preprocessors = CropMaskPreprocessingList::New();
  auto bm = GetParameterEmpty("bm");
  auto window = GetParameterInt("window");

  // Loop through the sets of products
  int startIndex = 0;
  int endIndex;
  for (size_t i = 0; i < prodPerTile.size(); i ++) {
      int sz = prodPerTile[i];
      endIndex = startIndex + sz;
      TileData td;

      auto preprocessor = CropMaskPreprocessing::New();
      preprocessors->PushBack(preprocessor);
      preprocessor->SetPixelSize(pixSize);
      preprocessor->SetMission(mission);
      if (GetParameterEmpty("rededge")) {
          preprocessor->SetIncludeRedEdge(true);
      }

      preprocessor->SetBM(bm);
      preprocessor->SetW(window);
      preprocessor->SetDelta(0.05f);
      preprocessor->SetTSoil(0.2f);

      // compute the desired size of the processed rasters
      preprocessor->updateRequiredImageSize(descriptors, startIndex, endIndex, td);
      preprocessor->Build(descriptors.begin() + startIndex, descriptors.begin() + endIndex, td);

    startIndex = endIndex;
  }

  const auto &sensorOutDays = getOutputDays(preprocessors, resamplingMode, mission, sp);

  writeOutputDays(sensorOutDays, GetParameterString("outdays"));

  std::vector<FloatVectorImageType::Pointer> images;
  images.reserve(preprocessors->Size());

  for (size_t i = 0; i < prodPerTile.size(); i++) {
      auto preprocessor = preprocessors->GetNthElement(i);
      auto output = preprocessor->GetOutput(sensorOutDays);

      images.emplace_back(output);
  }

  auto app = otb::Wrapper::ApplicationRegistry::CreateApplication("TrainImagesClassifierNew");
  if (!app) {
        itkExceptionMacro("Unable to load the TrainImagesClassifierNew application");
  }

  // TODO these are extracted with !OTB_USE_LIBSVM && OTB_USE_OPENCV
  std::string booleanParams[] = {
      "classifier.dt.r",
      "classifier.dt.t",
      "classifier.libsvm.opt",
      "classifier.svm.opt",
      "sample.edg"
  };
  std::string floatParams[] = {
      "classifier.ann.a",
      "classifier.ann.b",
      "classifier.ann.bpdw",
      "classifier.ann.bpms",
      "classifier.ann.eps",
      "classifier.ann.rdw",
      "classifier.ann.rdwm",
      "classifier.boost.r",
      "classifier.dt.ra",
      "classifier.gbt.p",
      "classifier.gbt.s",
      "classifier.libsvm.c",
      "classifier.rf.acc",
      "classifier.rf.ra",
      "classifier.svm.c",
      "classifier.svm.coef0",
      "classifier.svm.degree",
      "classifier.svm.gamma",
      "classifier.svm.nu",
      "elev.default",
      "sample.vtr"
  };
  std::string intParams[] = {
      "classifier.ann.iter",
      "classifier.boost.m",
      "classifier.boost.w",
      "classifier.dt.cat",
      "classifier.dt.f",
      "classifier.dt.max",
      "classifier.dt.min",
      "classifier.gbt.max",
      "classifier.gbt.w",
      "classifier.knn.k",
      "classifier.rf.cat",
      "classifier.rf.max",
      "classifier.rf.min",
      "classifier.rf.nbtrees",
      "classifier.rf.var",
      "nodatalabel",
      "rand",
      "sample.bm",
      "sample.mt",
      "sample.mv"
  };
  std::string stringParams[] = {
      "classifier.ann.f",
      "classifier.ann.sizes",
      "classifier.ann.term",
      "classifier.ann.t",
      "classifier.boost.t",
      "classifier.libsvm.k",
      "classifier",
      "classifier.svm.k",
      "classifier.svm.m",
      "elev.dem",
      "elev.geoid",
      "io.confmatout",
      "io.out",
      "io.rs",
      "sample.vfn"
  };

  for (const auto &key : booleanParams) {
      if (HasValue(key)) {
          app->EnableParameter(key);
          app->SetParameterEmpty(key, GetParameterEmpty(key));
      }
  }

  for (const auto &key : floatParams) {
      if (HasValue(key)) {
          app->EnableParameter(key);
          app->SetParameterFloat(key, GetParameterFloat(key));
      }
  }

  for (const auto &key : intParams) {
      if (HasValue(key)) {
          app->EnableParameter(key);
          app->SetParameterInt(key, GetParameterInt(key));
      }
  }

  for (const auto &key : stringParams) {
      if (HasValue(key)) {
          app->EnableParameter(key);
          app->SetParameterString(key, GetParameterString(key));
      }
  }

  if (HasValue("io.vd")) {
      app->EnableParameter("io.vd");
      app->SetParameterString("io.vd", GetParameterString("io.vd"));
  }

  if (HasValue("imstat")) {
    app->EnableParameter("io.imstat");
    app->SetParameterString("io.imstat", GetParameterString("imstat"));
  }

  app->EnableParameter("io.il");
  auto imageList = dynamic_cast<InputImageListParameter *>(app->GetParameterByKey("io.il"));

  for (const auto &img : images) {
      imageList->AddImage(img);
  }
  app->UpdateParameters();

  otbAppLogINFO("Training the classifier");
  app->ExecuteAndWriteOutput();
  otbAppLogINFO("Training completed");
}

#ifdef OTB_USE_LIBSVM
  void CropMaskTrainImagesClassifier::InitLibSVMParams()
  {
    AddChoice("classifier.libsvm", "LibSVM classifier");
    SetParameterDescription("classifier.libsvm", "This group of parameters allows to set SVM classifier parameters.");
    AddParameter(ParameterType_Choice, "classifier.libsvm.k", "SVM Kernel Type");
    AddChoice("classifier.libsvm.k.linear", "Linear");
    AddChoice("classifier.libsvm.k.rbf", "Gaussian radial basis function");
    AddChoice("classifier.libsvm.k.poly", "Polynomial");
    AddChoice("classifier.libsvm.k.sigmoid", "Sigmoid");
    SetParameterString("classifier.libsvm.k", "linear");
    SetParameterDescription("classifier.libsvm.k", "SVM Kernel Type.");
    AddParameter(ParameterType_Float, "classifier.libsvm.c", "Cost parameter C");
    SetParameterFloat("classifier.libsvm.c", 1.0);
    SetParameterDescription(
        "classifier.libsvm.c",
        "SVM models have a cost parameter C (1 by default) to control the trade-off between training errors and forcing rigid margins.");
    AddParameter(ParameterType_Empty, "classifier.libsvm.opt", "Parameters optimization");
    MandatoryOff("classifier.libsvm.opt");
    SetParameterDescription("classifier.libsvm.opt", "SVM parameters optimization flag.");
  }
#endif

#ifdef OTB_USE_OPENCV
  void CropMaskTrainImagesClassifier::InitBoostParams()
  {
    AddChoice("classifier.boost", "Boost classifier");
    SetParameterDescription("classifier.boost", "This group of parameters allows to set Boost classifier parameters. "
        "See complete documentation here \\url{http://docs.opencv.org/modules/ml/doc/boosting.html}.");
    //BoostType
    AddParameter(ParameterType_Choice, "classifier.boost.t", "Boost Type");
    AddChoice("classifier.boost.t.discrete", "Discrete AdaBoost");
    AddChoice("classifier.boost.t.real", "Real AdaBoost (technique using confidence-rated predictions "
                                                "and working well with categorical data)");
    AddChoice("classifier.boost.t.logit", "LogitBoost (technique producing good regression fits)");
    AddChoice("classifier.boost.t.gentle", "Gentle AdaBoost (technique setting less weight on outlier data points "
                                               "and, for that reason, being often good with regression data)");
    SetParameterString("classifier.boost.t", "real");
    SetParameterDescription("classifier.boost.t", "Type of Boosting algorithm.");
    //Do not expose SplitCriteria
    //WeakCount
    AddParameter(ParameterType_Int, "classifier.boost.w", "Weak count");
    SetParameterInt("classifier.boost.w", 100);
    SetParameterDescription("classifier.boost.w","The number of weak classifiers.");
    //WeightTrimRate
    AddParameter(ParameterType_Float, "classifier.boost.r", "Weight Trim Rate");
    SetParameterFloat("classifier.boost.r", 0.95);
    SetParameterDescription("classifier.boost.r","A threshold between 0 and 1 used to save computational time. "
                            "Samples with summary weight <= (1 - weight_trim_rate) do not participate in the next iteration of training. "
                            "Set this parameter to 0 to turn off this functionality.");
    //MaxDepth : Not sure that this parameter has to be exposed.
    AddParameter(ParameterType_Int, "classifier.boost.m", "Maximum depth of the tree");
    SetParameterInt("classifier.boost.m", 1);
    SetParameterDescription("classifier.boost.m","Maximum depth of the tree.");
  }

  void CropMaskTrainImagesClassifier::InitSVMParams()
  {
    AddChoice("classifier.svm", "SVM classifier (OpenCV)");
    SetParameterDescription("classifier.svm", "This group of parameters allows to set SVM classifier parameters. "
        "See complete documentation here \\url{http://docs.opencv.org/modules/ml/doc/support_vector_machines.html}.");
    AddParameter(ParameterType_Choice, "classifier.svm.m", "SVM Model Type");
    AddChoice("classifier.svm.m.csvc", "C support vector classification");
    AddChoice("classifier.svm.m.nusvc", "Nu support vector classification");
    AddChoice("classifier.svm.m.oneclass", "Distribution estimation (One Class SVM)");
    //AddChoice("classifier.svm.m.epssvr", "Epsilon Support Vector Regression");
    //AddChoice("classifier.svm.m.nusvr", "Nu Support Vector Regression");
    SetParameterString("classifier.svm.m", "csvc");
    SetParameterDescription("classifier.svm.m", "Type of SVM formulation.");
    AddParameter(ParameterType_Choice, "classifier.svm.k", "SVM Kernel Type");
    AddChoice("classifier.svm.k.linear", "Linear");
    AddChoice("classifier.svm.k.rbf", "Gaussian radial basis function");
    AddChoice("classifier.svm.k.poly", "Polynomial");
    AddChoice("classifier.svm.k.sigmoid", "Sigmoid");
    SetParameterString("classifier.svm.k", "linear");
    SetParameterDescription("classifier.svm.k", "SVM Kernel Type.");
    AddParameter(ParameterType_Float, "classifier.svm.c", "Cost parameter C");
    SetParameterFloat("classifier.svm.c", 1.0);
    SetParameterDescription(
        "classifier.svm.c",
        "SVM models have a cost parameter C (1 by default) to control the trade-off between training errors and forcing rigid margins.");
    AddParameter(ParameterType_Float, "classifier.svm.nu",
                 "Parameter nu of a SVM optimization problem (NU_SVC / ONE_CLASS)");
    SetParameterFloat("classifier.svm.nu", 0.0);
    SetParameterDescription("classifier.svm.nu", "Parameter nu of a SVM optimization problem.");
    //AddParameter(ParameterType_Float, "classifier.svm.p", "Parameter epsilon of a SVM optimization problem (EPS_SVR)");
    //SetParameterFloat("classifier.svm.p", 0.0);
    //SetParameterDescription("classifier.svm.p", "Parameter epsilon of a SVM optimization problem (EPS_SVR).");
    AddParameter(ParameterType_Float, "classifier.svm.coef0", "Parameter coef0 of a kernel function (POLY / SIGMOID)");
    SetParameterFloat("classifier.svm.coef0", 0.0);
    SetParameterDescription("classifier.svm.coef0", "Parameter coef0 of a kernel function (POLY / SIGMOID).");
    AddParameter(ParameterType_Float, "classifier.svm.gamma",
                 "Parameter gamma of a kernel function (POLY / RBF / SIGMOID)");
    SetParameterFloat("classifier.svm.gamma", 1.0);
    SetParameterDescription("classifier.svm.gamma", "Parameter gamma of a kernel function (POLY / RBF / SIGMOID).");
    AddParameter(ParameterType_Float, "classifier.svm.degree", "Parameter degree of a kernel function (POLY)");
    SetParameterFloat("classifier.svm.degree", 1.0);
    SetParameterDescription("classifier.svm.degree", "Parameter degree of a kernel function (POLY).");
    AddParameter(ParameterType_Empty, "classifier.svm.opt", "Parameters optimization");
    MandatoryOff("classifier.svm.opt");
    SetParameterDescription("classifier.svm.opt", "SVM parameters optimization flag.\n-If set to True, then the optimal SVM parameters will be estimated. "
                            "Parameters are considered optimal by OpenCV when the cross-validation estimate of the test set error is minimal. "
                            "Finally, the SVM training process is computed 10 times with these optimal parameters over subsets corresponding to 1/10th of "
                            "the training samples using the k-fold cross-validation (with k = 10).\n-If set to False, the SVM classification process will be "
                            "computed once with the currently set input SVM parameters over the training samples.\n-Thus, even with identical input SVM "
                            "parameters and a similar random seed, the output SVM models will be different according to the method used (optimized or not) "
                            "because the samples are not identically processed within OpenCV.");
  }

  void CropMaskTrainImagesClassifier::InitDecisionTreeParams()
  {
    AddChoice("classifier.dt", "Decision Tree classifier");
    SetParameterDescription("classifier.dt",
                            "This group of parameters allows to set Decision Tree classifier parameters. "
                            "See complete documentation here \\url{http://docs.opencv.org/modules/ml/doc/decision_trees.html}.");
    //MaxDepth
    AddParameter(ParameterType_Int, "classifier.dt.max", "Maximum depth of the tree");
    SetParameterInt("classifier.dt.max", 65535);
    SetParameterDescription(
        "classifier.dt.max", "The training algorithm attempts to split each node while its depth is smaller than the maximum "
        "possible depth of the tree. The actual depth may be smaller if the other termination criteria are met, and/or "
        "if the tree is pruned.");

    //MinSampleCount
    AddParameter(ParameterType_Int, "classifier.dt.min", "Minimum number of samples in each node");
    SetParameterInt("classifier.dt.min", 10);
    SetParameterDescription("classifier.dt.min", "If the number of samples in a node is smaller than this parameter, "
                            "then this node will not be split.");

    //RegressionAccuracy
    AddParameter(ParameterType_Float, "classifier.dt.ra", "Termination criteria for regression tree");
    SetParameterFloat("classifier.dt.ra", 0.01);
    SetParameterDescription("classifier.dt.min", "If all absolute differences between an estimated value in a node "
                            "and the values of the train samples in this node are smaller than this regression accuracy parameter, "
                            "then the node will not be split.");

    //UseSurrogates : don't need to be exposed !
    //AddParameter(ParameterType_Empty, "classifier.dt.sur", "Surrogate splits will be built");
    //SetParameterDescription("classifier.dt.sur","These splits allow to work with missing data and compute variable importance correctly.");

    //MaxCategories
    AddParameter(ParameterType_Int, "classifier.dt.cat",
                 "Cluster possible values of a categorical variable into K <= cat clusters to find a suboptimal split");
    SetParameterInt("classifier.dt.cat", 10);
    SetParameterDescription(
        "classifier.dt.cat",
        "Cluster possible values of a categorical variable into K <= cat clusters to find a suboptimal split.");

    //CVFolds
    AddParameter(ParameterType_Int, "classifier.dt.f", "K-fold cross-validations");
    SetParameterInt("classifier.dt.f", 10);
    SetParameterDescription(
        "classifier.dt.f", "If cv_folds > 1, then it prunes a tree with K-fold cross-validation where K is equal to cv_folds.");

    //Use1seRule
    AddParameter(ParameterType_Empty, "classifier.dt.r", "Set Use1seRule flag to false");
    SetParameterDescription(
        "classifier.dt.r",
        "If true, then a pruning will be harsher. This will make a tree more compact and more resistant to the training data noise but a bit less accurate.");

    //TruncatePrunedTree
    AddParameter(ParameterType_Empty, "classifier.dt.t", "Set TruncatePrunedTree flag to false");
    SetParameterDescription("classifier.dt.t", "If true, then pruned branches are physically removed from the tree.");

    //Priors are not exposed.

  }

  void CropMaskTrainImagesClassifier::InitGradientBoostedTreeParams()
  {
    AddChoice("classifier.gbt", "Gradient Boosted Tree classifier");
    SetParameterDescription(
        "classifier.gbt",
        "This group of parameters allows to set Gradient Boosted Tree classifier parameters. "
        "See complete documentation here \\url{http://docs.opencv.org/modules/ml/doc/gradient_boosted_trees.html}.");
    //LossFunctionType : not exposed, as only one type is used for Classification,
    // the other three are used for regression.

    //WeakCount
    AddParameter(ParameterType_Int, "classifier.gbt.w", "Number of boosting algorithm iterations");
    SetParameterInt("classifier.gbt.w", 200);
    SetParameterDescription(
        "classifier.gbt.w",
        "Number \"w\" of boosting algorithm iterations, with w*K being the total number of trees in "
        "the GBT model, where K is the output number of classes.");

    //Shrinkage
    AddParameter(ParameterType_Float, "classifier.gbt.s", "Regularization parameter");
    SetParameterFloat("classifier.gbt.s", 0.01);
    SetParameterDescription("classifier.gbt.s", "Regularization parameter.");

    //SubSamplePortion
    AddParameter(ParameterType_Float, "classifier.gbt.p",
                 "Portion of the whole training set used for each algorithm iteration");
    SetParameterFloat("classifier.gbt.p", 0.8);
    SetParameterDescription(
        "classifier.gbt.p",
        "Portion of the whole training set used for each algorithm iteration. The subset is generated randomly.");

    //MaxDepth
    AddParameter(ParameterType_Int, "classifier.gbt.max", "Maximum depth of the tree");
    SetParameterInt("classifier.gbt.max", 3);
    SetParameterDescription(
          "classifier.gbt.max", "The training algorithm attempts to split each node while its depth is smaller than the maximum "
          "possible depth of the tree. The actual depth may be smaller if the other termination criteria are met, and/or "
          "if the tree is pruned.");

    //UseSurrogates : don't need to be exposed !
    //AddParameter(ParameterType_Empty, "classifier.gbt.sur", "Surrogate splits will be built");
    //SetParameterDescription("classifier.gbt.sur","These splits allow to work with missing data and compute variable importance correctly.");

  }

  void CropMaskTrainImagesClassifier::InitNeuralNetworkParams()
  {
    AddChoice("classifier.ann", "Artificial Neural Network classifier");
    SetParameterDescription("classifier.ann",
                            "This group of parameters allows to set Artificial Neural Network classifier parameters. "
                            "See complete documentation here \\url{http://docs.opencv.org/modules/ml/doc/neural_networks.html}.");

    //TrainMethod
    AddParameter(ParameterType_Choice, "classifier.ann.t", "Train Method Type");
    AddChoice("classifier.ann.t.reg", "RPROP algorithm");
    AddChoice("classifier.ann.t.back", "Back-propagation algorithm");
    SetParameterString("classifier.ann.t", "reg");
    SetParameterDescription("classifier.ann.t", "Type of training method for the multilayer perceptron (MLP) neural network.");

    //LayerSizes
    //There is no ParameterType_IntList, so i use a ParameterType_StringList and convert it.
    /*std::vector<std::string> layerSizes;
     layerSizes.push_back("100");
     layerSizes.push_back("100"); */
    AddParameter(ParameterType_StringList, "classifier.ann.sizes", "Number of neurons in each intermediate layer");
    //SetParameterStringList("classifier.ann.sizes", layerSizes);
    SetParameterDescription("classifier.ann.sizes",
                            "The number of neurons in each intermediate layer (excluding input and output layers).");

    //ActivateFunction
    AddParameter(ParameterType_Choice, "classifier.ann.f", "Neuron activation function type");
    AddChoice("classifier.ann.f.ident", "Identity function");
    AddChoice("classifier.ann.f.sig", "Symmetrical Sigmoid function");
    AddChoice("classifier.ann.f.gau", "Gaussian function (Not completely supported)");
    SetParameterString("classifier.ann.f", "sig");
    SetParameterDescription("classifier.ann.f", "Neuron activation function.");

    //Alpha
    AddParameter(ParameterType_Float, "classifier.ann.a", "Alpha parameter of the activation function");
    SetParameterFloat("classifier.ann.a", 1.);
    SetParameterDescription("classifier.ann.a",
                            "Alpha parameter of the activation function (used only with sigmoid and gaussian functions).");

    //Beta
    AddParameter(ParameterType_Float, "classifier.ann.b", "Beta parameter of the activation function");
    SetParameterFloat("classifier.ann.b", 1.);
    SetParameterDescription("classifier.ann.b",
                            "Beta parameter of the activation function (used only with sigmoid and gaussian functions).");

    //BackPropDWScale
    AddParameter(ParameterType_Float, "classifier.ann.bpdw",
                 "Strength of the weight gradient term in the BACKPROP method");
    SetParameterFloat("classifier.ann.bpdw", 0.1);
    SetParameterDescription(
        "classifier.ann.bpdw",
        "Strength of the weight gradient term in the BACKPROP method. The recommended value is about 0.1.");

    //BackPropMomentScale
    AddParameter(ParameterType_Float, "classifier.ann.bpms",
                 "Strength of the momentum term (the difference between weights on the 2 previous iterations)");
    SetParameterFloat("classifier.ann.bpms", 0.1);
    SetParameterDescription(
        "classifier.ann.bpms",
        "Strength of the momentum term (the difference between weights on the 2 previous iterations). "
        "This parameter provides some inertia to smooth the random fluctuations of the weights. "
        "It can vary from 0 (the feature is disabled) to 1 and beyond. The value 0.1 or so is good enough.");

    //RegPropDW0
    AddParameter(ParameterType_Float, "classifier.ann.rdw",
                 "Initial value Delta_0 of update-values Delta_{ij} in RPROP method");
    SetParameterFloat("classifier.ann.rdw", 0.1);
    SetParameterDescription("classifier.ann.rdw", "Initial value Delta_0 of update-values Delta_{ij} in RPROP method (default = 0.1).");

    //RegPropDWMin
    AddParameter(ParameterType_Float, "classifier.ann.rdwm", "Update-values lower limit Delta_{min} in RPROP method");
    SetParameterFloat("classifier.ann.rdwm", 1e-7);
    SetParameterDescription(
        "classifier.ann.rdwm",
        "Update-values lower limit Delta_{min} in RPROP method. It must be positive (default = 1e-7).");

    //TermCriteriaType
    AddParameter(ParameterType_Choice, "classifier.ann.term", "Termination criteria");
    AddChoice("classifier.ann.term.iter", "Maximum number of iterations");
    AddChoice("classifier.ann.term.eps", "Epsilon");
    AddChoice("classifier.ann.term.all", "Max. iterations + Epsilon");
    SetParameterString("classifier.ann.term", "all");
    SetParameterDescription("classifier.ann.term", "Termination criteria.");

    //Epsilon
    AddParameter(ParameterType_Float, "classifier.ann.eps", "Epsilon value used in the Termination criteria");
    SetParameterFloat("classifier.ann.eps", 0.01);
    SetParameterDescription("classifier.ann.eps", "Epsilon value used in the Termination criteria.");

    //MaxIter
    AddParameter(ParameterType_Int, "classifier.ann.iter",
                 "Maximum number of iterations used in the Termination criteria");
    SetParameterInt("classifier.ann.iter", 1000);
    SetParameterDescription("classifier.ann.iter", "Maximum number of iterations used in the Termination criteria.");

  }

  void CropMaskTrainImagesClassifier::InitNormalBayesParams()
  {
    AddChoice("classifier.bayes", "Normal Bayes classifier");
    SetParameterDescription("classifier.bayes", "Use a Normal Bayes Classifier. "
        "See complete documentation here \\url{http://docs.opencv.org/modules/ml/doc/normal_bayes_classifier.html}.");

  }

  void CropMaskTrainImagesClassifier::InitRandomForestsParams()
  {
    AddChoice("classifier.rf", "Random forests classifier");
    SetParameterDescription("classifier.rf",
                            "This group of parameters allows to set Random Forests classifier parameters. "
                            "See complete documentation here \\url{http://docs.opencv.org/modules/ml/doc/random_trees.html}.");
    //MaxDepth
    AddParameter(ParameterType_Int, "classifier.rf.max", "Maximum depth of the tree");
    SetParameterInt("classifier.rf.max", 5);
    SetParameterDescription(
        "classifier.rf.max",
        "The depth of the tree. A low value will likely underfit and conversely a high value will likely overfit. "
        "The optimal value can be obtained using cross validation or other suitable methods.");

    //MinSampleCount
    AddParameter(ParameterType_Int, "classifier.rf.min", "Minimum number of samples in each node");
    SetParameterInt("classifier.rf.min", 10);
    SetParameterDescription(
        "classifier.rf.min", "If the number of samples in a node is smaller than this parameter, "
        "then the node will not be split. A reasonable value is a small percentage of the total data e.g. 1 percent.");

    //RegressionAccuracy
    AddParameter(ParameterType_Float, "classifier.rf.ra", "Termination Criteria for regression tree");
    SetParameterFloat("classifier.rf.ra", 0.);
    SetParameterDescription("classifier.rf.ra", "If all absolute differences between an estimated value in a node "
                            "and the values of the train samples in this node are smaller than this regression accuracy parameter, "
                            "then the node will not be split.");

    //UseSurrogates : don't need to be exposed !
    //AddParameter(ParameterType_Empty, "classifier.rf.sur", "Surrogate splits will be built");
    //SetParameterDescription("classifier.rf.sur","These splits allow to work with missing data and compute variable importance correctly.");

    //MaxNumberOfCategories
    AddParameter(ParameterType_Int, "classifier.rf.cat",
                 "Cluster possible values of a categorical variable into K <= cat clusters to find a suboptimal split");
    SetParameterInt("classifier.rf.cat", 10);
    SetParameterDescription(
        "classifier.rf.cat",
        "Cluster possible values of a categorical variable into K <= cat clusters to find a suboptimal split.");

    //Priors are not exposed.

    //CalculateVariableImportance not exposed

    //MaxNumberOfVariables
    AddParameter(ParameterType_Int, "classifier.rf.var",
                 "Size of the randomly selected subset of features at each tree node");
    SetParameterInt("classifier.rf.var", 0);
    SetParameterDescription(
        "classifier.rf.var",
        "The size of the subset of features, randomly selected at each tree node, that are used to find the best split(s). "
        "If you set it to 0, then the size will be set to the square root of the total number of features.");

    //MaxNumberOfTrees
    AddParameter(ParameterType_Int, "classifier.rf.nbtrees",
                 "Maximum number of trees in the forest");
    SetParameterInt("classifier.rf.nbtrees", 100);
    SetParameterDescription(
        "classifier.rf.nbtrees",
        "The maximum number of trees in the forest. Typically, the more trees you have, the better the accuracy. "
        "However, the improvement in accuracy generally diminishes and reaches an asymptote for a certain number of trees. "
        "Also to keep in mind, increasing the number of trees increases the prediction time linearly.");

    //ForestAccuracy
    AddParameter(ParameterType_Float, "classifier.rf.acc",
                 "Sufficient accuracy (OOB error)");
    SetParameterFloat("classifier.rf.acc", 0.01);
    SetParameterDescription("classifier.rf.acc","Sufficient accuracy (OOB error).");


    //TerminationCriteria not exposed
  }

  void CropMaskTrainImagesClassifier::InitKNNParams()
  {
    AddChoice("classifier.knn", "KNN classifier");
    SetParameterDescription("classifier.knn", "This group of parameters allows to set KNN classifier parameters. "
        "See complete documentation here \\url{http://docs.opencv.org/modules/ml/doc/k_nearest_neighbors.html}.");

    //K parameter
    AddParameter(ParameterType_Int, "classifier.knn.k", "Number of Neighbors");
    SetParameterInt("classifier.knn.k", 32);
    SetParameterDescription("classifier.knn.k","The number of neighbors to use.");

  }

#endif

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::CropMaskTrainImagesClassifier)
