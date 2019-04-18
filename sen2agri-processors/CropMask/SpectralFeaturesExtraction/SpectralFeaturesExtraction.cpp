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

//  Software Guide : BeginCommandLineArgs
//    INPUTS: {reference polygons}, {sample ratio}
//    OUTPUTS: {training polygons}, {validation_polygons}
//  Software Guide : EndCommandLineArgs

//  Software Guide : BeginLatex
// The sample selection consists in splitting the reference data into 2 disjoint sets, the training
// set and the validation set.
// These sets are composed of polygons, not individual pixels.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
#include <algorithm>

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "itkBinaryFunctorImageFilter.h"

#include "otbTemporalResamplingFilter.h"
#include "TimeSeriesReader.h"
#include "DataSmoothingFilter.h"
#include "CropMaskSpectralFeaturesFilter.h"

typedef otb::ImageFileReader<ImageType> ReaderType;
typedef DataSmoothingFunctor<ImageType::PixelType, MaskImageType::PixelType> DataSmoothingFunctorType;
typedef itk::BinaryFunctorImageFilter<ImageType, MaskImageType, ImageType, DataSmoothingFunctorType> DataSmoothingFilterType;

typedef CropMaskSpectralFeaturesFilter                                  CropMaskSpectralFeaturesFilterType;

class SpectralFeaturesPreprocessing : public TimeSeriesReader
{
public:
    typedef SpectralFeaturesPreprocessing Self;
    typedef TimeSeriesReader Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(SpectralFeaturesPreprocessing, TimeSeriesReader)

    itkSetMacro(IncludeRedEdge, bool)
    itkGetMacro(IncludeRedEdge, bool)

    SpectralFeaturesPreprocessing()
        : m_IncludeRedEdge(), m_Lambda(2.0)
    {
        m_FloatImageList = FloatImageListType::New();
        m_UInt8ImageList = UInt8ImageListType::New();
        m_BandsConcat = ConcatenateFloatImagesFilterType::New();
        m_MaskConcat = ConcatenateUInt8ImagesFilterType::New();
        m_DataSmoothingFilter = DataSmoothingFilterType::New();
        m_SpectralFeaturesFilter = CropMaskSpectralFeaturesFilterType::New();
    }

    void getRedEdgeBands(const std::unique_ptr<MetadataHelper<float, uint8_t>>& pHelper,
                                                   const TileData &td,
                                                   ImageDescriptor &descriptor) override
    {
        if (!m_IncludeRedEdge) {
            return;
        }
        std::vector<std::string> redEdgeBands = pHelper->GetRedEdgeBandNames();
        if (redEdgeBands.size() == 0) {
            return;
        }
        const std::string &narrowNirBandName = pHelper->GetNarrowNirBandName();
        if (narrowNirBandName.size() == 0) {
            return;
        }
        redEdgeBands.push_back(narrowNirBandName);

        ExtractFloatChannelFilterType::Pointer channelExtractor;

        std::vector<int> relBandsIdxs;
        MetadataHelper<float, uint8_t>::VectorImageType::Pointer img = pHelper->GetImage(redEdgeBands, &relBandsIdxs);
        img->UpdateOutputInformation();

        for (int bandIndex: relBandsIdxs) {
            channelExtractor = ExtractFloatChannelFilterType::New();
            channelExtractor->SetInput(img);
            channelExtractor->SetIndex(bandIndex);
            m_Filters->PushBack(channelExtractor);
            auto resampledBand = getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);
            descriptor.redEdgeBands.push_back(resampledBand);
        }
    }

/*
    void getSentinelRedEdgeBands(const MACCSFileMetadata &meta,
                          const TileData &td,
                          ImageDescriptor &descriptor,
                          Int16ImageReaderType::Pointer,
                          Int16ImageReaderType::Pointer reader2) override
    {
        if (!m_IncludeRedEdge) {
            return;
        }

        ExtractChannelFilterType::Pointer channelExtractor;

        int b5Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B5");
        channelExtractor = ExtractChannelFilterType::New();
        channelExtractor->SetInput(reader2->GetOutput());
        channelExtractor->SetIndex(b5Index - 1);
        m_Filters->PushBack(channelExtractor);
        auto b5Band = getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);

        int b6Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B6");
        channelExtractor = ExtractChannelFilterType::New();
        channelExtractor->SetInput(reader2->GetOutput());
        channelExtractor->SetIndex(b6Index - 1);
        m_Filters->PushBack(channelExtractor);
        auto b6Band = getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);

        int b7Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B7");
        channelExtractor = ExtractChannelFilterType::New();
        channelExtractor->SetInput(reader2->GetOutput());
        channelExtractor->SetIndex(b7Index - 1);
        m_Filters->PushBack(channelExtractor);
        auto b7Band = getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);

        int b8aIndex = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B8A");
        channelExtractor = ExtractChannelFilterType::New();
        channelExtractor->SetInput(reader2->GetOutput());
        channelExtractor->SetIndex(b8aIndex - 1);
        m_Filters->PushBack(channelExtractor);
        auto b8aBand = getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);

        descriptor.redEdgeBands.push_back(b5Band);
        descriptor.redEdgeBands.push_back(b6Band);
        descriptor.redEdgeBands.push_back(b7Band);
        descriptor.redEdgeBands.push_back(b8aBand);
    }
*/
    otb::Wrapper::FloatVectorImageType * GetOutput()
    {
        // Also build the image dates structures
        otb::SensorDataCollection sdCollection;
        int index = 0;
        std::string lastMission = "";
        size_t bands = 0;
        for (const ImageDescriptor& id : m_Descriptors) {
            if (id.mission != lastMission) {
                otb::SensorData sd;
                sd.sensorName = id.mission;
                sd.outDates = m_SensorOutDays[id.mission];
                sdCollection.push_back(sd);
                lastMission = id.mission;
            }

            auto &sd = sdCollection.back();
            int inDay = getDaysFromEpoch(id.aquisitionDate);

            sd.inDates.push_back(inDay);
            sd.bandCount = this->getBandCount(id.mission);

            for (const auto &b : id.bands) {
                m_FloatImageList->PushBack(b);
            }
            m_UInt8ImageList->PushBack(id.mask);

            if (index == 0) {
                bands = id.bands.size();
            }

            index++;
        }
        m_BandsConcat->SetInput(m_FloatImageList);
        m_MaskConcat->SetInput(m_UInt8ImageList);

        std::vector<ImageInfo> imgInfos;
        int priority = 10;
        index = 0;
        for (const auto &sd : sdCollection) {
            for (auto date : sd.inDates) {
                ImageInfo ii(index++, date, priority);
                imgInfos.push_back(ii);
            }
            priority--;
        }
        std::sort(imgInfos.begin(), imgInfos.end(), [](const ImageInfo& o1, const ImageInfo& o2) {
            return (o1.day < o2.day) || ((o1.day == o2.day) && (o1.priority > o2.priority));
        });

        // count the number of output images and create the out days file
        std::vector<int> od;
        int lastDay = -1;
        for (auto& imgInfo : imgInfos) {
            if (lastDay != imgInfo.day) {
                od.push_back(imgInfo.day);
                lastDay = imgInfo.day;
            }
        }

        bool hasRedEdge = false;
        if (m_IncludeRedEdge) {
            m_RedEdgeBandConcat->UpdateOutputInformation();
            m_RedEdgeMaskConcat->UpdateOutputInformation();

            m_RedEdgeDataSmoothingFilter = DataSmoothingFilterType::New();

            std::vector<ImageInfo> reImgInfos;
            index = 0;
            size_t reBands = 0;
            for (const ImageDescriptor& id : m_Descriptors) {
                if (id.mission == SENTINEL) {
                    if (!index) {
                        reBands = id.redEdgeBands.size();
                    }

                    reImgInfos.emplace_back(ImageInfo { index++, getDaysFromEpoch(id.aquisitionDate), 0 });
                }
            }

            m_RedEdgeDataSmoothingFilter->GetFunctor().SetBands(reBands);
            m_RedEdgeDataSmoothingFilter->GetFunctor().SetLambda(m_Lambda);
            m_RedEdgeDataSmoothingFilter->GetFunctor().SetOutputDates(od);
            m_RedEdgeDataSmoothingFilter->GetFunctor().SetInputImageInfo(reImgInfos);

            m_RedEdgeDataSmoothingFilter->SetInput1(m_RedEdgeBandConcat->GetOutput());
            m_RedEdgeDataSmoothingFilter->SetInput2(m_RedEdgeMaskConcat->GetOutput());

            m_RedEdgeDataSmoothingFilter->UpdateOutputInformation();
            m_RedEdgeDataSmoothingFilter->GetOutput()->SetNumberOfComponentsPerPixel(od.size() * reBands);

            hasRedEdge = !reImgInfos.empty();
        }


        m_DataSmoothingFilter->GetFunctor().SetBands(bands);
        m_DataSmoothingFilter->GetFunctor().SetLambda(m_Lambda);
        m_DataSmoothingFilter->GetFunctor().SetOutputDates(od);
        m_DataSmoothingFilter->GetFunctor().SetInputImageInfo(imgInfos);

        m_DataSmoothingFilter->SetInput1(m_BandsConcat->GetOutput());
        m_DataSmoothingFilter->SetInput2(m_MaskConcat->GetOutput());

        m_DataSmoothingFilter->UpdateOutputInformation();
        m_DataSmoothingFilter->GetOutput()->SetNumberOfComponentsPerPixel(od.size() * bands);

        m_SpectralFeaturesFilter->GetFunctor().SetInputDates(od);
        m_SpectralFeaturesFilter->SetInput1(m_DataSmoothingFilter->GetOutput());

        if (hasRedEdge) {
            m_SpectralFeaturesFilter->SetIncludeRedEdge(true);
            m_SpectralFeaturesFilter->SetInput2(m_RedEdgeDataSmoothingFilter->GetOutput());
        } else {
            // The second input won't be accessed, but we still have to set it to something
            m_SpectralFeaturesFilter->SetInput2(m_DataSmoothingFilter->GetOutput());
        }

        return m_SpectralFeaturesFilter->GetOutput();
    }

    void SetLambda(double lambda)
    {
        m_Lambda = lambda;
    }

private:
    bool                                              m_IncludeRedEdge;
    double                                            m_Lambda;

    FloatImageListType::Pointer                       m_FloatImageList;
    UInt8ImageListType::Pointer                       m_UInt8ImageList;
    ConcatenateFloatImagesFilterType::Pointer         m_BandsConcat;
    ConcatenateUInt8ImagesFilterType::Pointer         m_MaskConcat;
    DataSmoothingFilterType::Pointer                  m_DataSmoothingFilter;
    DataSmoothingFilterType::Pointer                  m_RedEdgeDataSmoothingFilter;
    CropMaskSpectralFeaturesFilterType::Pointer       m_SpectralFeaturesFilter;

};

namespace otb
{

//  Software Guide : BeginLatex
//  Application class is defined in Wrapper namespace.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
namespace Wrapper
{
//  Software Guide : EndCodeSnippet

//  Software Guide : BeginLatex
//
//  SampleSelection class is derived from Application class.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
class SpectralFeaturesExtraction : public Application
//  Software Guide : EndCodeSnippet
{
public:
    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef SpectralFeaturesExtraction Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)
    itkTypeMacro(SpectralFeaturesExtraction, otb::Application)
    //  Software Guide : EndCodeSnippet

private:
    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and
    // example values.
    //  Software Guide : EndLatex

    void DoInit()
    {

        // Software Guide : BeginLatex
        // Application name and description are set using following methods :
        // \begin{description}
        // \item[\code{SetName()}] Name of the application.
        // \item[\code{SetDescription()}] Set the short description of the class.
        // \item[\code{SetDocName()}] Set long name of the application (that can be displayed
        // \dots).
        // \item[\code{SetDocLongDescription()}] This methods is used to describe the class.
        // \item[\code{SetDocLimitations()}] Set known limitations (threading, invalid pixel type
        // \dots) or bugs.
        // \item[\code{SetDocAuthors()}] Set the application Authors. Author List. Format : "John
        // Doe, Winnie the Pooh" \dots
        // \item[\code{SetDocSeeAlso()}] If the application is related to one another, it can be
        // mentioned.
        // \end{description}
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        SetName("SpectralFeaturesExtraction");
        SetDescription(
            "Spectral feature extraction for unsupervised crop mask.");

        SetDocName("SpectralFeaturesExtraction");
        SetDocLongDescription("Spectral feature extraction for unsupervised crop mask.");
        SetDocLimitations("None");
        SetDocSeeAlso(" ");
        //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // \code{AddDocTag()} method categorize the application using relevant tags.
        // \code{Code/ApplicationEngine/otbWrapperTags.h} contains some predefined tags defined in
        // \code{Tags} namespace.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        AddDocTag(Tags::Vector);
        //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // The input parameters:
        // - ref: Vector file containing reference data
        // - ratio: Ratio between the number of training and validation polygons per class (dafault:
        // 0.75)
        // The output parameters:
        // - tp: Vector file containing reference data for training
        // - vp: Vector file containing reference data for validation
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        AddParameter(ParameterType_InputFilenameList, "il", "Input descriptors");
        SetParameterDescription( "il", "The list of descriptors. They must be sorted by tiles." );

        AddParameter(ParameterType_OutputImage, "out", "Output Image");
        SetParameterDescription( "out", "Output image" );

        AddParameter(ParameterType_Float, "pixsize", "The size of a pixel, in meters");
        SetDefaultParameterFloat("pixsize", 10.0); // The default value is 10 meters
        SetMinimumParameterFloatValue("pixsize", 1.0);
        MandatoryOff("pixsize");

        AddParameter(ParameterType_String, "mission", "The main raster series that will be used. By default SPOT is used");
        MandatoryOff("mission");

        AddParameter(ParameterType_Float, "lambda", "Smoothing parameter of the Whittaker function");

        AddParameter(ParameterType_Empty, "rededge", "Include Sentinel-2 vegetation red edge bands");
        MandatoryOff("rededge");

        SetDefaultParameterFloat("lambda", 2);

        //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // An example commandline is automatically generated. Method
        // \code{SetDocExampleParameterValue()} is
        // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        SetDocExampleParameterValue("lambda", "2");
        //  Software Guide : EndCodeSnippet
    }

    // Software Guide : BeginLatex
    // \code{DoUpdateParameters()} is called as soon as a parameter value change. Section
    // \ref{sec:appDoUpdateParameters}
    // gives a complete description of this method.
    // Software Guide : EndLatex
    //  Software Guide :BeginCodeSnippet
    void DoUpdateParameters()
    {
    }
    //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // The algorithm consists in a random sampling without replacement of the polygons of each class
    // with
    // probability p = sample_ratio value for the training set and
    // 1 - p for the validation set.
    // Software Guide : EndLatex
    //  Software Guide :BeginCodeSnippet
    void DoExecute()
    {
        // Get the input parameters
        double lambda = GetParameterFloat("lambda");

        // Get the list of input files
        const std::vector<std::string> &descriptors = this->GetParameterStringList("il");
        if( descriptors.size()== 0 )
          {
          itkExceptionMacro("No input file set...");
          }


        // get the required pixel size
        auto pixSize = this->GetParameterFloat("pixsize");
        // get the main mission
        std::string mission = SPOT;
        if (HasValue("mission")) {
            mission = this->GetParameterString("mission");
        }

        TileData td;

        m_Preprocessor = SpectralFeaturesPreprocessing::New();
        m_Preprocessor->SetPixelSize(pixSize);
        m_Preprocessor->SetMission(mission);
        if (GetParameterEmpty("rededge")) {
            m_Preprocessor->SetIncludeRedEdge(true);
        }
        m_Preprocessor->SetLambda(lambda);

        // compute the desired size of the processed rasters
        m_Preprocessor->updateRequiredImageSize(descriptors, 0, descriptors.size(), td);
        m_Preprocessor->Build(descriptors.begin(), descriptors.end(), td);

        SetParameterOutputImage("out", m_Preprocessor->GetOutput());
    }
    //  Software Guide :EndCodeSnippet

    SpectralFeaturesPreprocessing::Pointer m_Preprocessor;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::SpectralFeaturesExtraction)
//  Software Guide :EndCodeSnippet
