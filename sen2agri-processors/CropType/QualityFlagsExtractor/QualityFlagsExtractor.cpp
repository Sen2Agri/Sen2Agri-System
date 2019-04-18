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
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "otbVectorImage.h"
#include "otbImageList.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbBandMathImageFilter.h"

#include "otbStreamingResampleImageFilter.h"
#include "otbStreamingStatisticsImageFilter.h"
#include "otbLabelImageToVectorDataFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "otbOGRIOHelper.h"
#include "MetadataHelperFactory.h"
#include "GenericRSImageResampler.h"
#include "ImageResampler.h"
#include "GlobalDefs.h"


#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"
#define SPOT       "SPOT"

namespace Functor
{
template< class TInput, class TOutput>
class MaskStatusFlagsExtractor
{
public:
    MaskStatusFlagsExtractor() { }
    ~MaskStatusFlagsExtractor() {}
  bool operator!=( const MaskStatusFlagsExtractor & ) const
    {
    return false;
    }
  bool operator==( const MaskStatusFlagsExtractor & other ) const
    {
    return !(*this != other);
    }
  inline TOutput operator()( const TInput & A ) const
    {
      TOutput ret(4);
      // Input:
      // Bands with the Flags
      // Output:
      // 0 - number of valid land dates
      // 1 - number of water dates
      // 2 - number of snow dates
      // 3 - number of other (cloud, no data, etc.) dates
      // initialize the output vector
      for(int i = 0; i<4; i++) {
          ret[i] = 0;
      }
      for(int i = 0; i < A.Size(); i++) {
          // if cloud or saturation or the validity are invalid, then increment number of "others" dates
          if((A[i] == IMG_FLG_CLOUD) || (A[i] == IMG_FLG_SATURATION) || (A[i] == IMG_FLG_NO_DATA)) {
              ret[3] = ret[3]+1;
          } else if(A[i] == IMG_FLG_WATER) {
              // if the water mask is set, then increment the number of water dates
              ret[1] = ret[1]+1;
          } else if(A[i] == IMG_FLG_SNOW) {
              // if the snow mask is set, then increment the number of snow dates
              ret[2] = ret[2]+1;
          } else {
              // no invalid flags, increment the number of land dates
              ret[0] = ret[0]+1;
          }
      }
      return ret;
    }
};
}

typedef otb::VectorImage<short, 2>                                 ImageType;
typedef otb::Image<short, 2>                                       InternalImageType;

typedef otb::ImageList<ImageType>                                  ImageListType;
typedef otb::ImageList<InternalImageType>                          InternalImageListType;

typedef otb::ImageListToVectorImageFilter<InternalImageListType,
                                     ImageType >                   ListConcatenerFilterType;
typedef otb::MultiToMonoChannelExtractROI<ImageType::InternalPixelType,
                                     InternalImageType::PixelType> ExtractROIFilterType;
typedef otb::ObjectList<ExtractROIFilterType>                      ExtractROIFilterListType;

typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

typedef otb::StreamingResampleImageFilter<InternalImageType, InternalImageType, double>    ResampleFilterType;
typedef otb::ObjectList<ResampleFilterType>                           ResampleFilterListType;
typedef otb::BCOInterpolateImageFunction<InternalImageType,
                                            double>          BicubicInterpolationType;
typedef itk::LinearInterpolateImageFunction<InternalImageType,
                                            double>          LinearInterpolationType;
typedef itk::NearestNeighborInterpolateImageFunction<InternalImageType, double>             NearestNeighborInterpolationType;
typedef itk::IdentityTransform<double, InternalImageType::ImageDimension>      IdentityTransformType;
typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension> ScalableTransformType;
typedef ScalableTransformType::OutputVectorType                         OutputVectorType;

typedef otb::BandMathImageFilter<InternalImageType>                 BandMathImageFilterType;
typedef otb::ObjectList<BandMathImageFilterType>                    BandMathImageFilterListType;

typedef otb::StreamingStatisticsImageFilter<InternalImageType> StreamingStatisticsImageFilterType;
typedef otb::LabelImageToVectorDataFilter<InternalImageType>   LabelImageToVectorDataFilterType;

typedef itk::UnaryFunctorImageFilter<ImageType,ImageType,
                Functor::MaskStatusFlagsExtractor<
                    ImageType::PixelType,
                    ImageType::PixelType> > MaskStatusFlagsExtractorFilterType;


struct ImageDescriptor {
    // the aquisition date in format YYYYMMDD
    std::string aquisitionDate;
    // the mission name
    std::string mission;
    // this product belongs to the main mission
    bool isMain;

    // The mask for this product as computed (as FlagType)
    InternalImageType::Pointer curMask;
};

namespace otb
{
namespace Wrapper
{
class QualityFlagsExtractor : public Application
{
public:
  typedef QualityFlagsExtractor Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  itkNewMacro(Self)
;

  itkTypeMacro(QualityFlagsExtractor, otb::Application)
;


private:

  void DoInit()
  {
      SetName("QualityFlagsExtractor");
      SetDescription("Extract only the needed bands from a temporal series of images.");

      SetDocName("QualityFlagsExtractor");
      SetDocLongDescription("Extract the status flags for the CropType and CropMask processors.");
      SetDocLimitations("None");
      SetDocAuthors("CUU");
      SetDocSeeAlso(" ");
    AddDocTag(Tags::Vector);

    m_AllMasks = ListConcatenerFilterType::New();
    m_AllMasksList = InternalImageListType::New();
    m_ResamplersList = ResampleFilterListType::New();
    //m_ImageReaderList = ImageReaderListType::New();

    AddParameter(ParameterType_InputFilenameList, "il", "The xml files");
    AddParameter(ParameterType_OutputImage, "out", "A raster containing the number of dates for each pixel that have valid land values.");

    AddParameter(ParameterType_Int, "pixsize", "The size of a pixel, in meters");
    SetDefaultParameterInt("pixsize", 10); // The default value is 10 meters
    SetMinimumParameterIntValue("pixsize", 1);
    MandatoryOff("pixsize");

    AddParameter(ParameterType_String, "mission", "The main raster series that will be used. By default SENTINEL is used");
    MandatoryOff("mission");

    SetDocExampleParameterValue("il", "image1.xml image2.xml");
    SetDocExampleParameterValue("out", "fts.tif");
  }

  void DoUpdateParameters()
  {
      // Reinitialize the object
      m_AllMasks = ListConcatenerFilterType::New();
      m_AllMasksList = InternalImageListType::New();
      m_ResamplersList = ResampleFilterListType::New();
      //m_ImageReaderList = ImageReaderListType::New();
  }

  void DoExecute()
  {
      // get the required pixel size
      m_nPrimaryImgRes = this->GetParameterInt("pixsize");

      // get the main mission
      m_strPrimaryMission = SENTINEL;
      if (HasValue("mission")) {
          m_strPrimaryMission = this->GetParameterString("mission");
      }

      // Get the list of input files
      std::vector<std::string> descriptors = this->GetParameterStringList("il");

      if( descriptors.size()== 0 )
        {
        itkExceptionMacro("No input file set...");
        }

      auto factory = MetadataHelperFactory::New();
      for (const std::string& desc : descriptors) {
          m_vectHelpers.push_back(factory->GetMetadataHelper<short>(desc));
      }

      // compute the desired size of the output image
      updateRequiredImageSize(m_vectHelpers);

      // load all descriptors
      for (const std::unique_ptr<MetadataHelper<short>> &helper: m_vectHelpers) {
          ImageDescriptor id;
          ProcessMetadata(helper, id);
          m_DescriptorsList.push_back(id);
      }

      // Construct output image and mask
      BuildRasterAndMask();
  }

  void updateRequiredImageSize(const std::vector<std::unique_ptr<MetadataHelper<short>>> &vectHelpers) {
      m_primaryMissionImgWidth = -1;
      m_primaryMissionImgHeight = -1;
      m_bCutImages = false;
      for (const std::unique_ptr<MetadataHelper<short>>& helper: vectHelpers) {
          if(helper->GetMissionName().find(m_strPrimaryMission) != std::string::npos) {
              // get the image for the default resolution
                MetadataHelper<short>::VectorImageType::Pointer img = helper->GetImage(helper->GetPhysicalBandNames(), NULL, -1);
                img->UpdateOutputInformation();
//                ImageReaderType::Pointer reader = getReader(imageFile);
//                reader->UpdateOutputInformation();
                //float curRes = reader->GetOutput()->GetSpacing()[0];

                //const float scale = (float)m_nPrimaryMissionRes / curRes;
                // if we have primary mission set, then we ignore the given primary mission resolution
                // and we use the one in the primary mission product
                m_primaryMissionImgWidth = img->GetLargestPossibleRegion().GetSize()[0];// / scale;
                m_primaryMissionImgHeight = img->GetLargestPossibleRegion().GetSize()[1];// / scale;

                m_nPrimaryImgRes = img->GetSpacing()[0];

                FloatVectorImageType::PointType origin = img->GetOrigin();
                //InternalImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
                m_primaryMissionImgOrigin[0] = origin[0];// + 0.5 * spacing[0] * (scale - 1.0);
                m_primaryMissionImgOrigin[1] = origin[1];// + 0.5 * spacing[1] * (scale - 1.0);

                m_strPrMissionImgProjRef = img->GetProjectionRef();
                m_GenericRSImageResampler.SetOutputProjection(m_strPrMissionImgProjRef);
                m_bCutImages = true;

                break;
          }
      }
  }

  // Process a LANDSAT8 metadata structure and extract the needed bands and masks.
  void ProcessMetadata(const std::unique_ptr<MetadataHelper<short>> &helper, ImageDescriptor& descriptor) {

      // Extract the raster date
      descriptor.aquisitionDate = helper->GetAcquisitionDate();
      const std::string &mission = helper->GetMissionName();

      // save the mission
      if(mission.find(SENTINEL) != std::string::npos) {
          descriptor.mission = SENTINEL;
      } else if (mission.find(LANDSAT) != std::string::npos) {
        descriptor.mission = LANDSAT;
      } else if (mission.find(SPOT) != std::string::npos) {
        descriptor.mission = SPOT;
      } else {
          itkExceptionMacro("Unsupported mission " << mission);
      }
      descriptor.isMain = m_strPrimaryMission.compare(descriptor.mission) == 0;
      descriptor.curMask = cutImage(helper->GetMasksImage(ALL, false));
  }


  void BuildRasterAndMask() {
      // interpret the descriptors and extract the required bands from the atached images
      for ( const ImageDescriptor& desc : m_DescriptorsList) {
          m_AllMasksList->PushBack(desc.curMask);
      }

      m_AllMasks->SetInput(m_AllMasksList);
      m_MskStatusFlagsExtractorFunctor = MaskStatusFlagsExtractorFilterType::New();
      m_MskStatusFlagsExtractorFunctor->SetInput(m_AllMasks->GetOutput());
      m_MskStatusFlagsExtractorFunctor->UpdateOutputInformation();
      m_MskStatusFlagsExtractorFunctor->GetOutput()->SetNumberOfComponentsPerPixel(4);
      SetParameterOutputImagePixelType("out", ImagePixelType_uint16);
      SetParameterOutputImage("out", m_MskStatusFlagsExtractorFunctor->GetOutput());
  }

  // get a reader from the file path
//  ImageReaderType::Pointer getReader(const std::string& filePath) {
//      ImageReaderType::Pointer reader = ImageReaderType::New();

//      // set the file name
//      reader->SetFileName(filePath);

//      // add it to the list and return
//      m_ImageReaderList->PushBack(reader);
//      return reader;
//  }

  InternalImageType::Pointer cutImage(const InternalImageType::Pointer &img) {
      InternalImageType::Pointer retImg = img;
      img->UpdateOutputInformation();
      int curImgRes = img->GetSpacing()[0];
      const float scale = (float)m_nPrimaryImgRes / curImgRes;
      if(m_bCutImages) {
          float imageWidth = img->GetLargestPossibleRegion().GetSize()[0];
          float imageHeight = img->GetLargestPossibleRegion().GetSize()[1];

          ImageType::PointType origin = img->GetOrigin();
          ImageType::PointType imageOrigin;
          imageOrigin[0] = origin[0];
          imageOrigin[1] = origin[1];

          if((imageWidth != m_primaryMissionImgWidth) || (imageHeight != m_primaryMissionImgHeight) ||
                  (m_primaryMissionImgOrigin[0] != imageOrigin[0]) || (m_primaryMissionImgOrigin[1] != imageOrigin[1])) {

              std::string imgProjRef = img->GetProjectionRef();
              // if the projections are equal
              if(imgProjRef == m_strPrMissionImgProjRef) {
                  // use the streaming resampler
                  retImg = m_ImageResampler.getResampler(img, scale,m_primaryMissionImgWidth,
                              m_primaryMissionImgHeight,m_primaryMissionImgOrigin, Interpolator_NNeighbor)->GetOutput();
              } else {
                  // use the generic RS resampler that allows reprojecting
                  retImg = m_GenericRSImageResampler.getResampler(img, scale,m_primaryMissionImgWidth,
                              m_primaryMissionImgHeight,m_primaryMissionImgOrigin, Interpolator_NNeighbor)->GetOutput();
              }
              retImg->UpdateOutputInformation();
          }
      } else if (m_nPrimaryImgRes != curImgRes) {
          // in this case, if the main mission was not given, we just resample the image to the demanded resolution
          retImg = m_ImageResampler.getResampler(img, scale,Interpolator_NNeighbor)->GetOutput();
      }

      return retImg;
  }

  ListConcatenerFilterType::Pointer     m_AllMasks;
  ResampleFilterListType::Pointer       m_ResamplersList;
  InternalImageListType::Pointer        m_AllMasksList;
//  ImageReaderListType::Pointer          m_ImageReaderList;
  std::vector<ImageDescriptor>          m_DescriptorsList;
  std::string                           m_strPrimaryMission;
  int                                   m_nPrimaryImgRes;
  double                                m_primaryMissionImgWidth;
  double                                m_primaryMissionImgHeight;
  FloatVectorImageType::PointType       m_primaryMissionImgOrigin;
  bool                                  m_bCutImages;
  std::string                           m_strPrMissionImgProjRef;
  GenericRSImageResampler<InternalImageType, InternalImageType>  m_GenericRSImageResampler;
  ImageResampler<InternalImageType, InternalImageType>  m_ImageResampler;
  bool                                  m_bUseGenericRSResampler;

  MaskStatusFlagsExtractorFilterType::Pointer m_MskStatusFlagsExtractorFunctor;
  std::vector<std::unique_ptr<MetadataHelper<short>>> m_vectHelpers;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::QualityFlagsExtractor)


