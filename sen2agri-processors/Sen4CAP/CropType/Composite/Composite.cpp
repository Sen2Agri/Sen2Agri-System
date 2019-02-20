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
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbWrapperInputImageListParameter.h"

#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbWrapperTypes.h"
#include "otbObjectList.h"
#include "otbVectorImage.h"
#include "otbImageList.h"
#include "otbImageListToImageFilter.h"

#include "otbImageToGenericRSOutputParameters.h"
#include "otbGenericRSResampleImageFilter.h"

#include "itkLinearInterpolateImageFunction.h"
#include "otbBCOInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "otbExtractROI.h"

#include "otbGeographicalDistance.h"

namespace otb
{

template <class TInputImageType, class TOutputImageType>
class ITK_EXPORT CompositeFilter
  : public ImageListToImageFilter<TInputImageType, TInputImageType>
{
public:
  /** Standard typedefs */
  typedef CompositeFilter                           Self;
  typedef ImageListToImageFilter
      <TInputImageType, TInputImageType>            Superclass;
  typedef itk::SmartPointer<Self>                   Pointer;
  typedef itk::SmartPointer<const Self>             ConstPointer;

  typedef TInputImageType                           InputImageType;
  typedef typename InputImageType::Pointer          InputImagePointerType;
  typedef typename InputImageType::PixelType        InputPixelType;
  typedef ImageList<InputImageType>                 InputImageListType;
  typedef TOutputImageType                          OutputImageType;
  typedef typename TOutputImageType::PixelType      OutputPixelType;
  typedef typename OutputImageType::Pointer         OutputImagePointerType;
  typedef double                                    PrecisionType;


  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename InputImageType::RegionType  InputImageRegionType;


  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(CompositeFilter, ImageListToImageFilter);

  itkGetMacro(NoDataValue, InputPixelType);
  itkSetMacro(NoDataValue, InputPixelType);
  itkGetMacro(UseNoDataValue, bool);
  itkSetMacro(UseNoDataValue, bool);
  itkBooleanMacro(UseNoDataValue);

protected:
  /** Constructor */
  CompositeFilter();
  /** Destructor */
  ~CompositeFilter() override {}

  void GenerateInputRequestedRegion() override;
  void GenerateOutputInformation() override;
  void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, itk::ThreadIdType threadId) override;

  /**PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const override;


private:
  CompositeFilter(const Self &) = delete;
  void operator =(const Self&) = delete;

  InputPixelType            m_NoDataValue;
  bool                      m_UseNoDataValue;
};

template <class TInputImageType, class TOutputImageType>
CompositeFilter<TInputImageType, TOutputImageType>
::CompositeFilter()
 : m_NoDataValue(),
   m_UseNoDataValue()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);
}

template <class TInputImageType, class TOutputImageType>
void
CompositeFilter<TInputImageType, TOutputImageType>
::GenerateInputRequestedRegion(void)
{
  auto inputPtr = this->GetInput();
  for (auto inputListIt = inputPtr->Begin(); inputListIt != inputPtr->End(); ++inputListIt)
    {
    inputListIt.Get()->SetRequestedRegion(this->GetOutput()->GetRequestedRegion());
    }
}

template <class TInputImageType, class TOutputImageType>
void
CompositeFilter<TInputImageType, TOutputImageType>
::GenerateOutputInformation()
{
  if (this->GetOutput())
    {
    if (this->GetInput()->Size() > 0)
      {
      this->GetOutput()->CopyInformation(this->GetInput()->GetNthElement(0));
      this->GetOutput()->SetLargestPossibleRegion(this->GetInput()->GetNthElement(0)->GetLargestPossibleRegion());
      this->GetOutput()->SetNumberOfComponentsPerPixel(1);
      }
    }
}

template <class TInputImageType, class TOutputImageType>
void
CompositeFilter<TInputImageType, TOutputImageType>
::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, itk::ThreadIdType threadId)
{
  auto inputPtr = this->GetInput();
  auto inputImages = this->GetInput()->Size();

  OutputImagePointerType  outputPtr = this->GetOutput();

  typedef itk::ImageRegionConstIteratorWithIndex<InputImageType> InputIteratorType;
  typedef itk::ImageRegionIteratorWithIndex<OutputImageType>     OutputIteratorType;

  itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  OutputIteratorType outputIt(outputPtr, outputRegionForThread);
  outputIt.GoToBegin();

  std::vector<InputIteratorType> inputIts;
  inputIts.reserve(inputImages);
  for (auto inputListIt = inputPtr->Begin(); inputListIt != inputPtr->End(); ++inputListIt)
    {
    InputIteratorType inputIt(inputListIt.Get(), outputRegionForThread);
    inputIts.emplace_back(std::move(inputIt));
    inputIts.back().GoToBegin();
    }

  InputPixelType zero = m_UseNoDataValue ? m_NoDataValue : 0;

  OutputPixelType outPix;
  PrecisionType sum;
  int count;

  while (!outputIt.IsAtEnd())
    {
    sum = 0;
    count = 0;

    for (auto &it : inputIts)
      {
      const auto &inPix = it.Get();
      if ((!m_UseNoDataValue || inPix != m_NoDataValue) && !std::isnan(inPix))
        {
        sum += inPix;
        count++;
        }
      ++it;
      }

    if (count > 0)
      {
      outPix = static_cast<OutputPixelType>(sum / count);
      }
      else
      {
      outPix = 0;
      }

    outputIt.Set(outPix);
    ++outputIt;
    progress.CompletedPixel();
    }
}

/**
 * PrintSelf Method
 */
template <class TInputImageType, class TOutputImageType>
void
CompositeFilter<TInputImageType, TOutputImageType>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

enum
{
  Interpolator_BCO,
  Interpolator_NNeighbor,
  Interpolator_Linear
};

const float DefaultGridSpacingMeter = 4.0;

namespace Wrapper
{
class Composite : public Application
{
public:
    typedef Composite Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(Composite, otb::Application);

    typedef FloatImageType                                                InputImageType;
    typedef FloatImageType                                                OutputImageType;
    typedef InputImageType::PixelType                                     InputPixelType;
    typedef OutputImageType::PixelType                                    OutputPixelType;
    typedef otb::ImageList<InputImageType>                                ImageListType;
    typedef otb::ImageFileReader<InputImageType>                          ReaderType;
    typedef otb::ObjectList<ReaderType>                                   ReaderListType;
    typedef otb::ExtractROI
                <InputPixelType, OutputPixelType>                         ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                         ExtractROIListType;
    typedef otb::CompositeFilter
                <InputImageType, OutputImageType>                         CompositeFilterType;
    /** Generic Remote Sensor Resampler */
    typedef otb::GenericRSResampleImageFilter<InputImageType,
                                              InputImageType>             ResampleFilterType;

    /** Interpolators typedefs*/
    typedef itk::LinearInterpolateImageFunction<InputImageType,
                                                double>              LinearInterpolationType;
    typedef itk::NearestNeighborInterpolateImageFunction<InputImageType,
                                                         double>     NearestNeighborInterpolationType;
    typedef otb::BCOInterpolateImageFunction<InputImageType>         BCOInterpolationType;

private:
    void DoInit() override
    {
        SetName("Composite");
        SetDescription("Computes a composite of multiple images");

        SetDocName("Composite");
        SetDocLongDescription("Computes a mean composite of multiple images.");
        SetDocLimitations("None");
        SetDocAuthors("LN");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Raster);

        AddParameter(ParameterType_InputFilenameList, "il", "Input images");
        SetParameterDescription("il", "The list of input images");

        AddParameter(ParameterType_Float, "bv", "Background value");
        SetParameterDescription("bv", "Background value to ignore in computation.");
        SetDefaultParameterFloat("bv", 0.);
        MandatoryOff("bv");

        AddParameter(ParameterType_OutputImage, "out", "Output image");
        SetParameterDescription("out", "Output image.");

        AddParameter(ParameterType_Group, "srcwin", "Source window");
        SetParameterDescription("srcwin","This group of parameters allows one to define a source image window to process.");

        AddParameter(ParameterType_Float, "srcwin.ulx", "Upper Left X");
        SetParameterDescription("srcwin.ulx","Cartographic X coordinate of upper-left corner (meters for cartographic projections, degrees for geographic ones)");

        AddParameter(ParameterType_Float, "srcwin.uly", "Upper Left Y");
        SetParameterDescription("srcwin.uly","Cartographic Y coordinate of upper-left corner (meters for cartographic projections, degrees for geographic ones)");

        AddParameter(ParameterType_Float, "srcwin.lrx", "Lower right X");
        SetParameterDescription("srcwin.lrx","Cartographic X coordinate of lower-right corner (meters for cartographic projections, degrees for geographic ones)");

        AddParameter(ParameterType_Float, "srcwin.lry", "Lower right Y");
        SetParameterDescription("srcwin.lry","Cartographic Y coordinate of lower-right corner (meters for cartographic projections, degrees for geographic ones)");

        MandatoryOff("srcwin.ulx");
        MandatoryOff("srcwin.uly");
        MandatoryOff("srcwin.lrx");
        MandatoryOff("srcwin.lry");

        // Add the output parameters in a group
        AddParameter(ParameterType_Group, "outputs", "Output Image Grid");
        SetParameterDescription("outputs","This group of parameters allows one to define the grid on which the input image will be resampled.");

        // Upper left point coordinates
        AddParameter(ParameterType_Float, "outputs.ulx", "Upper Left X");
        SetParameterDescription("outputs.ulx","Cartographic X coordinate of upper-left corner (meters for cartographic projections, degrees for geographic ones)");

        AddParameter(ParameterType_Float, "outputs.uly", "Upper Left Y");
        SetParameterDescription("outputs.uly","Cartographic Y coordinate of the upper-left corner (meters for cartographic projections, degrees for geographic ones)");

        // Size of the output image
        AddParameter(ParameterType_Int, "outputs.sizex", "Size X");
        SetParameterDescription("outputs.sizex","Size of projected image along X (in pixels)");

        AddParameter(ParameterType_Int, "outputs.sizey", "Size Y");
        SetParameterDescription("outputs.sizey","Size of projected image along Y (in pixels)");

        // Spacing of the output image
        AddParameter(ParameterType_Float, "outputs.spacingx", "Pixel Size X");
        SetParameterDescription("outputs.spacingx","Size of each pixel along X axis (meters for cartographic projections, degrees for geographic ones)");

        // Add the output parameters in a group
        AddParameter(ParameterType_Group, "outputs", "Output Image Grid");
        SetParameterDescription("outputs", "This group of parameters allows one to define the grid on which the input images will be resampled.");

        AddParameter(ParameterType_Float, "outputs.spacingy", "Pixel Size Y");
        SetParameterDescription("outputs.spacingy","Size of each pixel along Y axis (meters for cartographic projections, degrees for geographic ones)");

        // Lower right point coordinates
        AddParameter(ParameterType_Float, "outputs.lrx", "Lower right X");
        SetParameterDescription("outputs.lrx","Cartographic X coordinate of the lower-right corner (meters for cartographic projections, degrees for geographic ones)");

        AddParameter(ParameterType_Float, "outputs.lry", "Lower right Y");
        SetParameterDescription("outputs.lry","Cartographic Y coordinate of the lower-right corner (meters for cartographic projections, degrees for geographic ones)");

        DisableParameter("outputs.lrx");
        DisableParameter("outputs.lry");

        MandatoryOff("outputs.ulx");
        MandatoryOff("outputs.uly");
        MandatoryOff("outputs.spacingx");
        MandatoryOff("outputs.spacingy");
        MandatoryOff("outputs.sizex");
        MandatoryOff("outputs.sizey");
        MandatoryOff("outputs.lrx");
        MandatoryOff("outputs.lry");

        // Existing ortho image that can be used to compute size, origin and spacing of the output
        AddParameter(ParameterType_InputImage, "ref", "Model ortho-image");
        SetParameterDescription("ref","A model ortho-image that can be used to compute size, origin and spacing of the output");

        // Interpolators
        AddParameter(ParameterType_Choice,   "interpolator", "Interpolation");
        AddChoice("interpolator.bco",    "Bicubic interpolation");
        AddParameter(ParameterType_Radius, "interpolator.bco.radius", "Radius for bicubic interpolation");
        SetParameterDescription("interpolator.bco.radius","This parameter allows one to control the size of the bicubic interpolation filter. If the target pixel size is higher than the input pixel size, increasing this parameter will reduce aliasing artifacts.");
        SetParameterDescription("interpolator","This group of parameters allows one to define how the input image will be interpolated during resampling.");
        AddChoice("interpolator.nn",     "Nearest Neighbor interpolation");
        SetParameterDescription("interpolator.nn","Nearest neighbor interpolation leads to poor image quality, but it is very fast.");
        AddChoice("interpolator.linear", "Linear interpolation");
        SetParameterDescription("interpolator.linear","Linear interpolation leads to average image quality but is quite fast");
        SetDefaultParameterInt("interpolator.bco.radius", 2);

        AddParameter(ParameterType_Group,"opt","Speed optimization parameters");
        SetParameterDescription("opt","This group of parameters allows optimization of processing time.");

        // Displacement Field Spacing
        AddParameter(ParameterType_Float, "opt.gridspacing", "Resampling grid spacing");
        SetDefaultParameterFloat("opt.gridspacing", DefaultGridSpacingMeter);
        SetParameterDescription("opt.gridspacing",
                                "Resampling is done according to a coordinate mapping deformation grid, "
                                "whose pixel size is set by this parameter, and "
                                "expressed in the coordinate system of the output image "
                                "The closer to the output spacing this parameter is, "
                                "the more precise will be the ortho-rectified image,"
                                "but increasing this parameter will reduce processing time.");
        MandatoryOff("opt.gridspacing");

        AddRAMParameter();

        SetDocExampleParameterValue("il", "image1.tif image2.tif");
        SetDocExampleParameterValue("ref", "reference.tif");
        SetDocExampleParameterValue("out", "output.tif");
    }

    void DoUpdateParameters() override
    {
        // Make all the parameters in this mode mandatory
        MandatoryOff("outputs.ulx");
        MandatoryOff("outputs.uly");
        MandatoryOff("outputs.spacingx");
        MandatoryOff("outputs.spacingy");
        MandatoryOff("outputs.sizex");
        MandatoryOff("outputs.sizey");
        MandatoryOff("outputs.lrx");
        MandatoryOff("outputs.lry");

        // Disable the parameters
        DisableParameter("outputs.ulx");
        DisableParameter("outputs.uly");
        DisableParameter("outputs.spacingx");
        DisableParameter("outputs.spacingy");
        DisableParameter("outputs.sizex");
        DisableParameter("outputs.sizey");
        DisableParameter("outputs.lrx");
        DisableParameter("outputs.lry");

        if (!HasValue("ref"))
        {
            return;
        }

        auto inOrtho = GetParameterImage("ref");

        ResampleFilterType::OriginType orig = inOrtho->GetOrigin();
        ResampleFilterType::SpacingType spacing = inOrtho->GetSpacing();
        ResampleFilterType::SizeType size = inOrtho->GetLargestPossibleRegion().GetSize();
        m_OutputProjectionRef = inOrtho->GetProjectionRef();

        SetParameterInt("outputs.sizex",size[0]);
        SetParameterInt("outputs.sizey",size[1]);
        SetParameterFloat("outputs.spacingx",spacing[0]);
        SetParameterFloat("outputs.spacingy",spacing[1]);
        SetParameterFloat("outputs.ulx",orig[0] - 0.5 * spacing[0]);
        SetParameterFloat("outputs.uly",orig[1] - 0.5 * spacing[1]);
        // Update lower right
        SetParameterFloat("outputs.lrx",GetParameterFloat("outputs.ulx") + GetParameterFloat("outputs.spacingx") * static_cast<double>(GetParameterInt("outputs.sizex")));
        SetParameterFloat("outputs.lry",GetParameterFloat("outputs.uly") + GetParameterFloat("outputs.spacingy") * static_cast<double>(GetParameterInt("outputs.sizey")));

        if (!HasUserValue("opt.gridspacing"))
          {
          // Update opt.gridspacing
          // In case output coordinate system is WG84,
          if (m_OutputProjectionRef == otb::GeoInformationConversion::ToWKT(4326))
            {
            // How much is 4 meters in degrees ?
            typedef itk::Point<float,2> FloatPointType;
            FloatPointType point1, point2;

            typedef otb::GeographicalDistance<FloatPointType> GeographicalDistanceType;
            GeographicalDistanceType::Pointer geoDistanceCalculator = GeographicalDistanceType::New();

            // center
            point1[0] = GetParameterFloat("outputs.ulx") + GetParameterFloat("outputs.spacingx") * GetParameterInt("outputs.sizex") / 2;
            point1[1] = GetParameterFloat("outputs.uly") + GetParameterFloat("outputs.spacingy") * GetParameterInt("outputs.sizey") / 2;

            // center + [1,0]
            point2[0] = point1[0] + GetParameterFloat("outputs.spacingx");
            point2[1] = point1[1];
            double xgroundspacing = geoDistanceCalculator->Evaluate(point1, point2);
            otbAppLogINFO( "Output X ground spacing in meter = " << xgroundspacing );

            // center + [0,1]
            point2[0] = point1[0];
            point2[1] = point1[1] + GetParameterFloat("outputs.spacingy");
            double ygroundspacing = geoDistanceCalculator->Evaluate(point1, point2);
            otbAppLogINFO( "Output Y ground spacing in meter = " << ygroundspacing );

            double xgridspacing = DefaultGridSpacingMeter * GetParameterFloat("outputs.spacingx") / xgroundspacing;
            double ygridspacing = DefaultGridSpacingMeter * GetParameterFloat("outputs.spacingy") / ygroundspacing;

            otbAppLogINFO( << DefaultGridSpacingMeter << " meters in X direction correspond roughly to "
                           << xgridspacing << " degrees" );
            otbAppLogINFO( << DefaultGridSpacingMeter << " meters in Y direction correspond roughly to "
                           << ygridspacing << " degrees" );

            // Use the smallest spacing (more precise grid)
            double optimalSpacing = std::min( std::abs(xgridspacing), std::abs(ygridspacing) );
            otbAppLogINFO( "Setting grid spacing to " << optimalSpacing );
            SetParameterFloat("opt.gridspacing",optimalSpacing);
            }
          else
            {
            // Use the smallest spacing (more precise grid)
            double optimalSpacing = std::min( std::abs(spacing[0]), std::abs(spacing[1]) );
            otbAppLogINFO( "Setting grid spacing to " << optimalSpacing );
            SetParameterFloat("opt.gridspacing",optimalSpacing);
            }
          }
    }

    void DoExecute() override
    {
        const auto &inImages = GetParameterStringList("il");

        m_ReprojectedImages = ImageListType::New();
        m_Readers = ReaderListType::New();

        auto first = true;
        for (const auto &file : inImages)
        {
            auto reader = ReaderType::New();
            reader->SetFileName(file);
            reader->UpdateOutputInformation();
            m_Readers->PushBack(reader);

            const auto inImage = reader->GetOutput();

//            itk::Point<float, 2> ulp;
//            itk::Point<float, 2> lrp;
//            ulp[0] = GetParameterFloat("srcwin.ulx");
//            ulp[1] = GetParameterFloat("srcwin.uly");
//            lrp[0] = GetParameterFloat("srcwin.lrx");
//            lrp[1] = GetParameterFloat("srcwin.lry");

//            InputImageType::IndexType uli;
//            InputImageType::IndexType lri;
//            inImage->TransformPhysicalPointToIndex(ulp, uli);
//            inImage->TransformPhysicalPointToIndex(lrp, lri);

//            std::cout << uli << '\n';
//            std::cout << lri << '\n';

//            auto extractROIFilter = ExtractROIFilterType::New();
//            extractROIFilter->SetInput(inImage);
//            extractROIFilter->SetStartX(uli[0]);
//            extractROIFilter->SetStartY(uli[1]);
//            extractROIFilter->SetSizeX(lri[0] - uli[0] + 1);
//            extractROIFilter->SetSizeY(lri[1] - uli[1] + 1);
//            m_ExtractROIFilters->PushBack(extractROIFilter);
//            extractROIFilter->UpdateOutputInformation();
//            m_ReprojectedImages->PushBack(extractROIFilter->GetOutput());

            // Resampler Instantiation
            auto resampleFilter = ResampleFilterType::New();
            m_ResampleFilters.emplace_back(resampleFilter);
            resampleFilter->SetInput(inImage);

            // Set the output projection Ref
            resampleFilter->SetInputProjectionRef(inImage->GetProjectionRef());
            resampleFilter->SetInputKeywordList(inImage->GetImageKeywordlist());
            resampleFilter->SetOutputProjectionRef(m_OutputProjectionRef);

            // Check size
            if (GetParameterInt("outputs.sizex") <= 0 || GetParameterInt("outputs.sizey") <= 0)
              {
              otbAppLogCRITICAL("Wrong value : negative size : ("<<GetParameterInt("outputs.sizex")<<" , "<<GetParameterInt("outputs.sizey")<<")");
              }

            //Check spacing sign
            if (GetParameterFloat("outputs.spacingy") > 0.)
              {
              otbAppLogWARNING(<<"Wrong value for outputs.spacingy: Pixel size along Y axis should be negative, (outputs.spacingy=" <<GetParameterFloat("outputs.spacingy") << ")" )
              }

            // Get Interpolator
            switch ( GetParameterInt("interpolator") )
              {
              case Interpolator_Linear:
              {
              if (first)
                {
                otbAppLogINFO(<< "Using linear interpolator");
                }
              LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
              resampleFilter->SetInterpolator(interpolator);
              }
              break;
              case Interpolator_NNeighbor:
              {
              if (first)
                {
                otbAppLogINFO(<< "Using nn interpolator");
                }
              NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
              resampleFilter->SetInterpolator(interpolator);
              }
              break;
              case Interpolator_BCO:
              {
              if (first)
                {
                  otbAppLogINFO(<< "Using BCO interpolator");
                }
              BCOInterpolationType::Pointer interpolator = BCOInterpolationType::New();
              interpolator->SetRadius(GetParameterInt("interpolator.bco.radius"));
              resampleFilter->SetInterpolator(interpolator);
              }
              break;
              }

            // Set Output information
            ResampleFilterType::SizeType size;
            size[0] = GetParameterInt("outputs.sizex");
            size[1] = GetParameterInt("outputs.sizey");
            resampleFilter->SetOutputSize(size);

            ResampleFilterType::SpacingType spacing;
            spacing[0] = GetParameterFloat("outputs.spacingx");
            spacing[1] = GetParameterFloat("outputs.spacingy");
            resampleFilter->SetOutputSpacing(spacing);

            ResampleFilterType::OriginType origin;
            origin[0] = GetParameterFloat("outputs.ulx") + 0.5 * GetParameterFloat("outputs.spacingx");
            origin[1] = GetParameterFloat("outputs.uly") + 0.5 * GetParameterFloat("outputs.spacingy");
            resampleFilter->SetOutputOrigin(origin);

            // Build the default pixel
            InputImageType::PixelType defaultValue = GetParameterFloat("bv");
            resampleFilter->SetEdgePaddingValue(defaultValue);

            if (first)
              {
              otbAppLogINFO("Generating output with size = " << size);
              otbAppLogINFO("Generating output with pixel spacing = " << spacing);
              otbAppLogINFO("Generating output with origin = " << origin);
              otbAppLogINFO("Area outside input image bounds will have a pixel value of " << defaultValue);
              }

            // Displacement Field spacing
            ResampleFilterType::SpacingType gridSpacing;
            if (IsParameterEnabled("opt.gridspacing"))
              {
              gridSpacing[0] = GetParameterFloat("opt.gridspacing");
              gridSpacing[1] = -GetParameterFloat("opt.gridspacing");

              if ( GetParameterFloat( "opt.gridspacing" ) == 0 )
                {
                otbAppLogFATAL( "opt.gridspacing must be different from 0 " );
                }

              // Predict size of deformation grid
              ResampleFilterType::SpacingType deformationGridSize;
              deformationGridSize[0] = static_cast<ResampleFilterType::SpacingType::ValueType >(std::abs(
                  GetParameterInt("outputs.sizex") * GetParameterFloat("outputs.spacingx") / GetParameterFloat("opt.gridspacing") ));
              deformationGridSize[1] = static_cast<ResampleFilterType::SpacingType::ValueType>(std::abs(
                  GetParameterInt("outputs.sizey") * GetParameterFloat("outputs.spacingy") / GetParameterFloat("opt.gridspacing") ));
              if (first)
                {
                otbAppLogINFO("Using a deformation grid with a physical spacing of " << GetParameterFloat("opt.gridspacing"));
                otbAppLogINFO("Using a deformation grid of size " << deformationGridSize);
                }

              if (deformationGridSize[0] * deformationGridSize[1] == 0)
                {
                otbAppLogFATAL("Deformation grid degenerated (size of 0). "
                    "You shall set opt.gridspacing appropriately. "
                    "opt.gridspacing units are the same as outputs.spacing units");
                }

              if (std::abs(GetParameterFloat("opt.gridspacing")) < std::abs(GetParameterFloat("outputs.spacingx"))
                   || std::abs(GetParameterFloat("opt.gridspacing")) < std::abs(GetParameterFloat("outputs.spacingy")) )
                {
                otbAppLogWARNING("Spacing of deformation grid should be at least equal to "
                    "spacing of output image. Otherwise, computation time will be slow, "
                    "and precision of output will not be better. "
                    "You shall set opt.gridspacing appropriately. "
                    "opt.gridspacing units are the same as outputs.spacing units");
                }

              resampleFilter->SetDisplacementFieldSpacing(gridSpacing);
              m_ReprojectedImages->PushBack(resampleFilter->GetOutput());
              }
          first = false;
        }

        m_CompositeFilter = CompositeFilterType::New();
        m_CompositeFilter->SetInput(m_ReprojectedImages);

        // Resampler Instantiation
//        auto resampleFilter = ResampleFilterType::New();
//        m_ResampleFilters.emplace_back(resampleFilter);
//        m_CompositeFilter->UpdateOutputInformation();
//        resampleFilter->SetInput(m_CompositeFilter->GetOutput());

//        // Set the output projection Ref
//        resampleFilter->SetInputProjectionRef(m_CompositeFilter->GetOutput()->GetProjectionRef());
//        resampleFilter->SetInputKeywordList(m_CompositeFilter->GetOutput()->GetImageKeywordlist());
//        resampleFilter->SetOutputProjectionRef(m_OutputProjectionRef);

//        // Check size
//        if (GetParameterInt("outputs.sizex") <= 0 || GetParameterInt("outputs.sizey") <= 0)
//          {
//          otbAppLogCRITICAL("Wrong value : negative size : ("<<GetParameterInt("outputs.sizex")<<" , "<<GetParameterInt("outputs.sizey")<<")");
//          }

//        //Check spacing sign
//        if (GetParameterFloat("outputs.spacingy") > 0.)
//          {
//          otbAppLogWARNING(<<"Wrong value for outputs.spacingy: Pixel size along Y axis should be negative, (outputs.spacingy=" <<GetParameterFloat("outputs.spacingy") << ")" )
//          }

//        // Get Interpolator
//        switch ( GetParameterInt("interpolator") )
//          {
//          case Interpolator_Linear:
//          {
//          if (first)
//            {
//            otbAppLogINFO(<< "Using linear interpolator");
//            }
//          LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
//          resampleFilter->SetInterpolator(interpolator);
//          }
//          break;
//          case Interpolator_NNeighbor:
//          {
//          if (first)
//            {
//            otbAppLogINFO(<< "Using nn interpolator");
//            }
//          NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
//          resampleFilter->SetInterpolator(interpolator);
//          }
//          break;
//          case Interpolator_BCO:
//          {
//          if (first)
//            {
//              otbAppLogINFO(<< "Using BCO interpolator");
//            }
//          BCOInterpolationType::Pointer interpolator = BCOInterpolationType::New();
//          interpolator->SetRadius(GetParameterInt("interpolator.bco.radius"));
//          resampleFilter->SetInterpolator(interpolator);
//          }
//          break;
//          }

//        // Set Output information
//        ResampleFilterType::SizeType size;
//        size[0] = GetParameterInt("outputs.sizex");
//        size[1] = GetParameterInt("outputs.sizey");
//        resampleFilter->SetOutputSize(size);

//        ResampleFilterType::SpacingType spacing;
//        spacing[0] = GetParameterFloat("outputs.spacingx");
//        spacing[1] = GetParameterFloat("outputs.spacingy");
//        resampleFilter->SetOutputSpacing(spacing);

//        ResampleFilterType::OriginType origin;
//        origin[0] = GetParameterFloat("outputs.ulx") + 0.5 * GetParameterFloat("outputs.spacingx");
//        origin[1] = GetParameterFloat("outputs.uly") + 0.5 * GetParameterFloat("outputs.spacingy");
//        resampleFilter->SetOutputOrigin(origin);

//        // Build the default pixel
//        InputImageType::PixelType defaultValue = GetParameterFloat("bv");
//        resampleFilter->SetEdgePaddingValue(defaultValue);

//        if (first)
//          {
//          otbAppLogINFO("Generating output with size = " << size);
//          otbAppLogINFO("Generating output with pixel spacing = " << spacing);
//          otbAppLogINFO("Generating output with origin = " << origin);
//          otbAppLogINFO("Area outside input image bounds will have a pixel value of " << defaultValue);
//          }

//        // Displacement Field spacing
//        ResampleFilterType::SpacingType gridSpacing;
//        if (IsParameterEnabled("opt.gridspacing"))
//          {
//          gridSpacing[0] = GetParameterFloat("opt.gridspacing");
//          gridSpacing[1] = -GetParameterFloat("opt.gridspacing");

//          if ( GetParameterFloat( "opt.gridspacing" ) == 0 )
//            {
//            otbAppLogFATAL( "opt.gridspacing must be different from 0 " );
//            }

//          // Predict size of deformation grid
//          ResampleFilterType::SpacingType deformationGridSize;
//          deformationGridSize[0] = static_cast<ResampleFilterType::SpacingType::ValueType >(std::abs(
//              GetParameterInt("outputs.sizex") * GetParameterFloat("outputs.spacingx") / GetParameterFloat("opt.gridspacing") ));
//          deformationGridSize[1] = static_cast<ResampleFilterType::SpacingType::ValueType>(std::abs(
//              GetParameterInt("outputs.sizey") * GetParameterFloat("outputs.spacingy") / GetParameterFloat("opt.gridspacing") ));
//          if (first)
//            {
//            otbAppLogINFO("Using a deformation grid with a physical spacing of " << GetParameterFloat("opt.gridspacing"));
//            otbAppLogINFO("Using a deformation grid of size " << deformationGridSize);
//            }

//          if (deformationGridSize[0] * deformationGridSize[1] == 0)
//            {
//            otbAppLogFATAL("Deformation grid degenerated (size of 0). "
//                "You shall set opt.gridspacing appropriately. "
//                "opt.gridspacing units are the same as outputs.spacing units");
//            }

//          if (std::abs(GetParameterFloat("opt.gridspacing")) < std::abs(GetParameterFloat("outputs.spacingx"))
//               || std::abs(GetParameterFloat("opt.gridspacing")) < std::abs(GetParameterFloat("outputs.spacingy")) )
//            {
//            otbAppLogWARNING("Spacing of deformation grid should be at least equal to "
//                "spacing of output image. Otherwise, computation time will be slow, "
//                "and precision of output will not be better. "
//                "You shall set opt.gridspacing appropriately. "
//                "opt.gridspacing units are the same as outputs.spacing units");
//            }

//          resampleFilter->SetDisplacementFieldSpacing(gridSpacing);
////              m_ReprojectedImages->PushBack(resampleFilter->GetOutput());
//          }

        if (HasValue("bv"))
          {
          m_CompositeFilter->UseNoDataValueOn();
          m_CompositeFilter->SetNoDataValue(GetParameterFloat("bv"));
          }

        // Output Image
        SetParameterOutputImage("out", m_CompositeFilter->GetOutput());
//        SetParameterOutputImage("out", resampleFilter->GetOutput());
    }

    ReaderListType::Pointer                      m_Readers;
    ExtractROIListType::Pointer                  m_ExtractROIFilters;
    std::vector<ResampleFilterType::Pointer>     m_ResampleFilters;
    ImageListType::Pointer                       m_ReprojectedImages;
    CompositeFilterType::Pointer                 m_CompositeFilter;
    std::string                                  m_OutputProjectionRef;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::Composite)
