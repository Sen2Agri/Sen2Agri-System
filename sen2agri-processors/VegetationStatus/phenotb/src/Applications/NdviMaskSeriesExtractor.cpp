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
#include "otbImageToVectorImageCastFilter.h"

#include "MetadataHelperFactory.h"
#include "GenericRSImageResampler.h"
#include "ImageResampler.h"
#include "GlobalDefs.h"
#include "itkBinaryFunctorImageFilter.h"


#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"
#define SPOT       "SPOT"

typedef otb::VectorImage<short, 2>                                 InternalVectorImageType;
typedef otb::Image<short, 2>                                       InternalImageType;
typedef otb::Image<float, 2>                                       InternalFloatImageType;
typedef otb::VectorImage<float, 2>                                 InternalFloatVectorImageType;

typedef otb::ImageList<InternalVectorImageType>                    ImageListType;
typedef otb::ImageList<InternalImageType>                          InternalImageListType;
typedef otb::ImageList<InternalFloatImageType>                     InternalFloatImageListType;

typedef otb::ImageListToVectorImageFilter<InternalImageListType,
                                     InternalVectorImageType >     ListConcatenerFilterType;
typedef otb::ImageListToVectorImageFilter<InternalFloatImageListType,
                                     InternalFloatVectorImageType > ListFloatConcatenerFilterType;

typedef otb::ImageFileReader<InternalVectorImageType>              ImageReaderType;
typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

typedef otb::ImageToVectorImageCastFilter<InternalFloatImageType, InternalFloatVectorImageType> VectorCastFilterType;

namespace otb
{
namespace Wrapper
{
class NdviMaskSeriesExtractor : public Application
{
public:
  typedef NdviMaskSeriesExtractor Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  itkNewMacro(Self)
;

  itkTypeMacro(NdviMaskSeriesExtractor, otb::Application)
;

  typedef itk::BinaryFunctorImageFilter<InternalVectorImageType,InternalImageType,InternalFloatImageType,
                  NdviBinaryFunctor<
                      InternalVectorImageType::PixelType,InternalImageType::PixelType, InternalFloatImageType::PixelType> >
                  MaskedNDVIFilterType;
  struct ImageDescriptor {
      // the aquisition date in format YYYYMMDD
      std::string aquisitionDate;
      // the mission name
      std::string mission;
      // this product belongs to the main mission
      bool isMain;

      // The mask for this product as computed (as FlagType)
      InternalImageType::Pointer curMask;
      InternalFloatImageType::Pointer ndviImg;
      MaskedNDVIFilterType::Pointer maskedNdviFunctor;
  };


private:

  void DoInit()
  {
      SetName("NdviMaskSeriesExtractor");
      SetDescription("Extract only the needed bands from a temporal series of images.");

      SetDocName("NdviMaskSeriesExtractor");
      SetDocLongDescription("Extract the status flags for the CropType and CropMask processors.");
      SetDocLimitations("None");
      SetDocAuthors("CUU");
      SetDocSeeAlso(" ");
    AddDocTag(Tags::Vector);

    m_AllMasks = ListConcatenerFilterType::New();
    m_AllMasksList = InternalImageListType::New();

    m_AllNdvis = ListFloatConcatenerFilterType::New();
    m_AllNdvisList = InternalFloatImageListType::New();

    m_ImageReaderList = ImageReaderListType::New();

    AddParameter(ParameterType_InputFilenameList, "il", "The xml files");
    AddParameter(ParameterType_OutputImage, "outndvis", "The output raster containing the ndvi series.");
    MandatoryOff("outndvis");
    AddParameter(ParameterType_OutputImage, "outmasks", "The output raster containing the masks series.");

    AddParameter(ParameterType_Int, "pixsize", "The size of a pixel, in meters");
    SetDefaultParameterInt("pixsize", 10); // The default value is 10 meters
    SetMinimumParameterIntValue("pixsize", 1);
    MandatoryOff("pixsize");

    AddParameter(ParameterType_String, "mission", "The main raster series that will be used. By default SENTINEL is used");
    MandatoryOff("mission");

    AddParameter(ParameterType_OutputFilename, "outdate", "The file containing the dates for the images");
    MandatoryOff("outdate");
    AddParameter(ParameterType_Empty, "ndh", "Information about mission name and number of xmls will not be added inside the dates files (default off)");
    MandatoryOff("ndh");

    SetDocExampleParameterValue("il", "image1.xml image2.xml");
    SetDocExampleParameterValue("out", "fts.tif");
  }

  void DoUpdateParameters()
  {
      // Reinitialize the object
      m_AllMasks = ListConcatenerFilterType::New();
      m_AllMasksList = InternalImageListType::New();

      m_AllNdvis = ListFloatConcatenerFilterType::New();
      m_AllNdvisList = InternalFloatImageListType::New();

      m_ImageReaderList = ImageReaderListType::New();
  }

  void DoExecute()
  {
      //verify is no data header (ndh) is set on true
      m_ndh = false;
      if (IsParameterEnabled("ndh")) {
          m_ndh = true;
      }

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

      // Construct output image and mask
      BuildRasterAndMask();

      // Write the dates file if configured
      writeDatesFile();
  }

  void BuildRasterAndMask() {
       for (const std::unique_ptr<MetadataHelper<short>> &pHelper: m_vectHelpers) {
          ImageDescriptor descriptor;
          // Extract the raster date
          descriptor.aquisitionDate = pHelper->GetAcquisitionDate();
          const std::string &mission = pHelper->GetMissionName();

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

          descriptor.curMask = cutImage(pHelper->GetMasksImage(ALL, false));
          descriptor.curMask->UpdateOutputInformation();

          // Create also the NDVI for this descriptor
          // the bands are 1 based
          const std::string &nirBandName = pHelper->GetNirBandName();
          const std::string &redBandName = pHelper->GetRedBandName();
          std::vector<int> relBandIdxs;
          MetadataHelper<short>::VectorImageType::Pointer img = pHelper->GetImage({redBandName, nirBandName}, &relBandIdxs);
          img->UpdateOutputInformation();

          descriptor.maskedNdviFunctor = MaskedNDVIFilterType::New();
          descriptor.maskedNdviFunctor->GetFunctor().Initialize(relBandIdxs[0], relBandIdxs[1]);
          descriptor.maskedNdviFunctor->SetInput(img);
          descriptor.maskedNdviFunctor->SetInput2(descriptor.curMask);
          descriptor.maskedNdviFunctor->UpdateOutputInformation();
          InternalFloatImageType::Pointer ndvisImg = descriptor.maskedNdviFunctor->GetOutput();
          ndvisImg->UpdateOutputInformation();
          descriptor.ndviImg = cutFloatImage(ndvisImg);
          descriptor.ndviImg->UpdateOutputInformation();

          m_DescriptorsList.push_back(descriptor);
      }

      // sort the descriptors after the aquisition date
      std::sort(m_DescriptorsList.begin(), m_DescriptorsList.end(), NdviMaskSeriesExtractor::SortMergedMetadata);

      // interpret the descriptors and extract the required bands from the atached images
//      int i = 0;
//      ImageDescriptor desc1;
      for ( const ImageDescriptor& desc : m_DescriptorsList) {
//          if (i == 0) {
//              desc1 = desc;
//            i++;
//          }
          m_AllMasksList->PushBack(desc.curMask);
          m_AllNdvisList->PushBack(desc.ndviImg);
      }

      m_AllMasks->SetInput(m_AllMasksList);
      m_AllNdvis->SetInput(m_AllNdvisList);

      SetParameterOutputImagePixelType("outndvis", ImagePixelType_float);
      SetParameterOutputImage("outndvis", m_AllNdvis->GetOutput());
//      m_castFilter = VectorCastFilterType::New();
//      m_castFilter->SetInput(desc1.ndviImg);
//      SetParameterOutputImage("outndvis", m_castFilter->GetOutput());

      SetParameterOutputImagePixelType("outmasks", ImagePixelType_uint16);
      SetParameterOutputImage("outmasks", m_AllMasks->GetOutput());
  }

  void writeDatesFile() {
      if (HasValue("outdate")) {
          std::ofstream datesFile;

          const std::string &datesFileName = GetParameterString("outdate");
          //open the file
          datesFile.open(datesFileName);
          if (!datesFile.is_open()) {
              itkExceptionMacro("Can't open dates file for writing!");
          }

          // write the main mission name
          if(!m_ndh) {
            datesFile << m_strPrimaryMission << std::endl;
          }
          std::ostringstream missionDates;
          int numDates = 0;
          for ( const ImageDescriptor& desc : m_DescriptorsList) {
              // write the date to the buffer
              missionDates << desc.aquisitionDate << std::endl;
              numDates++;
          }
          // write the data for the last mission
          if(!m_ndh) {
            datesFile << numDates << std::endl;
          }
          datesFile << missionDates.str() << std::endl;

          // close the dates file
          datesFile.close();
      }
  }

  void updateRequiredImageSize(const std::vector<std::unique_ptr<MetadataHelper<short>>> &vectHelpers) {
      m_primaryMissionImgWidth = -1;
      m_primaryMissionImgHeight = -1;
      m_bCutImages = false;
      for (const std::unique_ptr<MetadataHelper<short>>& helper: vectHelpers) {
          if(helper->GetMissionName().find(m_strPrimaryMission) != std::string::npos) {
                MetadataHelper<short>::VectorImageType::Pointer img = helper->GetImage({helper->GetRedBandName()}, NULL, -1);
                img->UpdateOutputInformation();
                //float curRes = reader->GetOutput()->GetSpacing()[0];

                //const float scale = (float)m_nPrimaryMissionRes / curRes;
                // if we have primary mission set, then we ignore the given primary mission resolution
                // and we use the one in the primary mission product
                m_primaryMissionImgWidth = img->GetLargestPossibleRegion().GetSize()[0];// / scale;
                m_primaryMissionImgHeight = img->GetLargestPossibleRegion().GetSize()[1];// / scale;

                m_nPrimaryImgRes = img->GetSpacing()[0];

                InternalFloatVectorImageType::PointType origin = img->GetOrigin();
                //InternalImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
                m_primaryMissionImgOrigin[0] = origin[0];// + 0.5 * spacing[0] * (scale - 1.0);
                m_primaryMissionImgOrigin[1] = origin[1];// + 0.5 * spacing[1] * (scale - 1.0);

                m_strPrMissionImgProjRef = img->GetProjectionRef();
                m_GenericRSImageResampler.SetOutputProjection(m_strPrMissionImgProjRef);
                m_GenericRSFloatImageResampler.SetOutputProjection(m_strPrMissionImgProjRef);

                m_bCutImages = true;

                break;
          }
      }
  }

  // get a reader from the file path
  ImageReaderType::Pointer getReader(const std::string& filePath) {
      ImageReaderType::Pointer reader = ImageReaderType::New();

      // set the file name
      reader->SetFileName(filePath);

      // add it to the list and return
      m_ImageReaderList->PushBack(reader);
      return reader;
  }

  InternalImageType::Pointer cutImage(const InternalImageType::Pointer &img) {
      InternalImageType::Pointer retImg = img;
      img->UpdateOutputInformation();
      int curImgRes = img->GetSpacing()[0];
      const float scale = (float)m_nPrimaryImgRes / curImgRes;
      if(m_bCutImages) {
          float imageWidth = img->GetLargestPossibleRegion().GetSize()[0];
          float imageHeight = img->GetLargestPossibleRegion().GetSize()[1];

          InternalImageType::PointType origin = img->GetOrigin();
          InternalImageType::PointType imageOrigin;
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
          }
      } else if (m_nPrimaryImgRes != curImgRes) {
          // in this case, if the main mission was not given, we just resample the image to the demanded resolution
          retImg = m_ImageResampler.getResampler(img, scale,Interpolator_NNeighbor)->GetOutput();
      }
      retImg->UpdateOutputInformation();

      return retImg;
  }

  InternalFloatImageType::Pointer cutFloatImage(const InternalFloatImageType::Pointer &img) {
      InternalFloatImageType::Pointer retImg = img;
      img->UpdateOutputInformation();
      int curImgRes = img->GetSpacing()[0];
      const float scale = (float)m_nPrimaryImgRes / curImgRes;
      if(m_bCutImages) {
          float imageWidth = img->GetLargestPossibleRegion().GetSize()[0];
          float imageHeight = img->GetLargestPossibleRegion().GetSize()[1];

          InternalFloatImageType::PointType origin = img->GetOrigin();
          InternalFloatImageType::PointType imageOrigin;
          imageOrigin[0] = origin[0];
          imageOrigin[1] = origin[1];

          if((imageWidth != m_primaryMissionImgWidth) || (imageHeight != m_primaryMissionImgHeight) ||
                  (m_primaryMissionImgOrigin[0] != imageOrigin[0]) || (m_primaryMissionImgOrigin[1] != imageOrigin[1])) {

              std::string imgProjRef = img->GetProjectionRef();
              // if the projections are equal
              if(imgProjRef == m_strPrMissionImgProjRef) {
                  // use the streaming resampler
                  retImg = m_FloatImageResampler.getResampler(img, scale,m_primaryMissionImgWidth,
                              m_primaryMissionImgHeight,m_primaryMissionImgOrigin, Interpolator_NNeighbor)->GetOutput();
              } else {
                  // use the generic RS resampler that allows reprojecting
                  retImg = m_GenericRSFloatImageResampler.getResampler(img, scale,m_primaryMissionImgWidth,
                              m_primaryMissionImgHeight,m_primaryMissionImgOrigin, Interpolator_NNeighbor)->GetOutput();
              }
          }
      } else if (m_nPrimaryImgRes != curImgRes) {
          // in this case, if the main mission was not given, we just resample the image to the demanded resolution
          retImg = m_FloatImageResampler.getResampler(img, scale,Interpolator_NNeighbor)->GetOutput();
      }
      retImg->UpdateOutputInformation();

      return retImg;
  }


  // Sort the descriptors based on the aquisition date
  static bool SortMergedMetadata(const ImageDescriptor& o1, const ImageDescriptor& o2) {
      return o1.aquisitionDate.compare(o2.aquisitionDate) < 0;
  }

  ListConcatenerFilterType::Pointer     m_AllMasks;
  InternalImageListType::Pointer        m_AllMasksList;

  ListFloatConcatenerFilterType::Pointer m_AllNdvis;
  InternalFloatImageListType::Pointer   m_AllNdvisList;

  ImageReaderListType::Pointer          m_ImageReaderList;
  std::vector<ImageDescriptor>          m_DescriptorsList;
  std::string                           m_strPrimaryMission;
  int                                   m_nPrimaryImgRes;
  double                                m_primaryMissionImgWidth;
  double                                m_primaryMissionImgHeight;
  InternalFloatVectorImageType::PointType m_primaryMissionImgOrigin;
  bool                                  m_bCutImages;
  std::string                           m_strPrMissionImgProjRef;
  GenericRSImageResampler<InternalImageType, InternalImageType>  m_GenericRSImageResampler;
  ImageResampler<InternalImageType, InternalImageType>  m_ImageResampler;

  GenericRSImageResampler<InternalFloatImageType, InternalFloatImageType>  m_GenericRSFloatImageResampler;
  ImageResampler<InternalFloatImageType, InternalFloatImageType>  m_FloatImageResampler;

  bool                                  m_bUseGenericRSResampler;

  std::vector<std::unique_ptr<MetadataHelper<short>>> m_vectHelpers;

  bool                                  m_ndh;

  VectorCastFilterType::Pointer m_castFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::NdviMaskSeriesExtractor)


