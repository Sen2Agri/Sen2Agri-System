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
  Program:   otb-bv
  Language:  C++

  Copyright (c) CESBIO. All rights reserved.

  See otb-bv-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkTernaryFunctorImageFilter.h"

#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbBandMathImageFilter.h"
#include "MetadataHelperFactory.h"
#include "GlobalDefs.h"
#include "ImageResampler.h"
#include "GenericRSImageResampler.h"
#include "otbMultiToMonoChannelExtractROI.h"

#include <vector>

#include "dateUtils.h"
#include "phenoFunctions.h"
#include "otbProfileReprocessing.h"
#include "MetadataHelperFactory.h"

//VectorType date_vect1 = {37, 57, 77, 92, 102, 107, 112, 117, 122, 127, 132, 137, 147, 152, 157, 162, 167};
//VectorType ts1 = {0, 0, 0, 0, 0.316361, 0.320108, 0.284682, 0.287518, 0.291294, 0.300072, 0, 0, 0, 1.21381, 1.18754, 0, 0};
//VectorType ets1 = {};
//VectorType msks1 = {2, 2, 2, 1, 4, 4, 4, 4, 4, 4, 1, 1, 1, 4, 4, 1, 1};

namespace otb
{
int date_to_doy(std::string& date_str)
{
  return pheno::doy(pheno::make_date(date_str));
}

namespace Functor
{
}


namespace Wrapper
{

typedef enum {ALGO_LOCAL = 0, ALGO_FIT} ALGO_TYPE;

template< class TInput1, class TInput2, class TInput3, class TOutput>
class ProfileReprocessingFunctor
{
public:
  ProfileReprocessingFunctor() {}
  ~ProfileReprocessingFunctor() {}
  bool operator!=(const ProfileReprocessingFunctor &a) const
  {
      return (this->date_vect != a.date_vect) || (this->algoType != a.algoType) ||
              (this->bwr != a.bwr) || (this->fwr != a.fwr);
  }

  bool operator==(const ProfileReprocessingFunctor & other) const
  {
    return !( *this != other );
  }

  void SetDates(VectorType &idDates)
  {
      date_vect = idDates;
  }

  void SetAlgoType(ALGO_TYPE algo)
  {
      algoType = algo;
  }

  void SetBwr(size_t inBwr) {
      bwr = inBwr;
  }

  void SetFwr(size_t inFwr) {
      fwr = inFwr;
  }

  void SetGenerateAll(bool bGenAll) {
      m_bGenAll = bGenAll;
  }


  inline TOutput operator()(const TInput1 & A,
                            const TInput2 & B,
                            const TInput3 & C) const
  {
      //itk::VariableLengthVector vect;
    int nbBvElems = A.GetNumberOfElements();

    VectorType ts(nbBvElems);
    VectorType ets(nbBvElems);
    VectorType msks(nbBvElems);
    int i;
    for(i = 0; i<nbBvElems; i++) {
        ts[i] = A[i];
        ets[i] = B[i];
        msks[i] = C[i];
    }

    VectorType out_bv_vec{};
    VectorType out_flag_vec{};

    if(algoType == ALGO_LOCAL) {
        std::tie(out_bv_vec, out_flag_vec) =
            smooth_time_series_local_window_with_error(date_vect, ts, ets,msks,
                                                       bwr, fwr);
    } else {
      std::tie(out_bv_vec, out_flag_vec) =
        //fit_csdm_2(date_vect1, ts1, ets1, msks1);
        fit_csdm_2(date_vect, ts, ets, msks);
    }
    if(m_bGenAll) {
        TOutput result(2*nbBvElems);
        i = 0;
        for(i = 0; i < nbBvElems; i++) {
            result[i] = out_bv_vec[i];
        }
        for(int j = 0; j < nbBvElems; j++) {
            result[i+j] = out_flag_vec[j];
        }
        return result;
    } else {
        // if not necessary to generate all, then return only the last value
        TOutput result(2);
        result[0] = out_bv_vec[nbBvElems-1];
        result[1] = out_flag_vec[nbBvElems-1];
        return result;
    }
  }
private:
  // input dates vector
  VectorType date_vect;
  ALGO_TYPE algoType;
  size_t bwr;
  size_t fwr;
  bool m_bGenAll;
};

template< class TInput1, class TInput2, class TOutput>
class ProfileReprocessingNoErrEstFunctor : public ProfileReprocessingFunctor<TInput1,TInput1,TInput2,TOutput>
{
public:
  ProfileReprocessingNoErrEstFunctor() {}
  ~ProfileReprocessingNoErrEstFunctor() {}

  inline TOutput operator()(const TInput1 & A,
                            const TInput2 & B) const
  {
    TInput1 C = A;
    int nbBvElems = A.GetNumberOfElements();
    int i;
    for(i = 0; i<nbBvElems; i++) {
        C[i] = 0;
    }
    return ProfileReprocessingFunctor<TInput1,TInput1,TInput2,TOutput>::operator ()(A, C, B);
  }
};

class ProfileReprocessing : public Application
{
public:
  /** Standard class typedefs. */
  typedef ProfileReprocessing               Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  typedef short                                                             ShortPixelType;
  typedef otb::Image<ShortPixelType, 2>                                     ShortImageType;

  typedef float                                   PixelType;
  typedef FloatVectorImageType                    InputImageType;
  typedef FloatVectorImageType                    OutImageType;

  typedef ProfileReprocessingFunctor <InputImageType::PixelType,
                                    InputImageType::PixelType,
                                    InputImageType::PixelType,
                                    OutImageType::PixelType>                FunctorType;
    typedef ProfileReprocessingNoErrEstFunctor <InputImageType::PixelType,
                                      InputImageType::PixelType,
                                      OutImageType::PixelType>                BinaryFunctorType;

  typedef itk::TernaryFunctorImageFilter<InputImageType,
                                        InputImageType,
                                        InputImageType,
                                        OutImageType, FunctorType> TernaryFilterType;
    typedef itk::BinaryFunctorImageFilter<InputImageType,
                                          InputImageType,
                                          OutImageType, BinaryFunctorType> BinaryFilterType;

    typedef FloatVectorImageType                    ImageType;
    typedef otb::Image<float, 2>                    InternalImageType;
    typedef otb::ImageFileReader<ImageType>         ReaderType;
    typedef otb::ImageFileWriter<ImageType>         WriterType;
    typedef otb::ImageList<InternalImageType>       ImageListType;
    typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>       VectorImageToImageListType;
    typedef otb::ImageListToVectorImageFilter<ImageListType, ImageType>       ImageListToVectorImageFilterType;
    typedef otb::ObjectList<ImageListToVectorImageFilterType>                   ImageListToVectorImageFilterListType;

    typedef otb::ImageFileReader<ImageType>         ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>        ImageReaderListType;
    typedef otb::ObjectList<ImageType>              ImagesListType;

    typedef otb::ObjectList<VectorImageToImageListType>    SplitFilterListType;

    typedef itk::UnaryFunctorImageFilter<ImageType,ImageType,
                    ShortToFloatTranslationFunctor<
                        ImageType::PixelType,
                        ImageType::PixelType> > DequantifyFilterType;
    typedef otb::ObjectList<DequantifyFilterType>              DeqFunctorListType;

    typedef otb::MultiToMonoChannelExtractROI<InputImageType::InternalPixelType,
                                              InternalImageType::InternalPixelType> SplitterFilterType;
  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(ProfileReprocessing, otb::Application);

private:
  void DoInit()
  {

    SetName("ProfileReprocessing");
    SetDescription("Reprocess a BV time profile.");

    // Parameters for the case when a time series is provided
    AddParameter(ParameterType_InputFilenameList, "illai", "The image files list");
    MandatoryOff("illai");
    AddParameter(ParameterType_InputFilenameList, "ilerr", "The image files list");
    MandatoryOff("ilerr");
    AddParameter(ParameterType_InputFilenameList, "ilmsks", "The image files list");
    MandatoryOff("ilmsks");
    AddParameter(ParameterType_Float, "deqval", "The de-quantification value to be used");
    SetDefaultParameterFloat("deqval", -1);
    MandatoryOff("deqval");
    AddParameter(ParameterType_String, "main", "The image from the illai that is used for the cutting other images");
    MandatoryOff("main");

    // Parameters for the case when only one product is provided that contains already the time series
    AddParameter(ParameterType_InputImage, "lai", "Input profile file.");
    SetParameterDescription( "lai", "Input file containing the profile to process. This file contains the BV estimation." );
    MandatoryOff("lai");

    AddParameter(ParameterType_InputImage, "err", "Input profile file.");
    SetParameterDescription( "err", "Input file containing the profile to process. This file contains the error." );
    MandatoryOff("err");

    AddParameter(ParameterType_InputImage, "msks", "Image containing time series mask flags.");
    SetParameterDescription( "msks", "Input file containing time series mask flags. Land is expected to be with value (4)" );
    MandatoryOff("msks");

    // NOTE: although not mandatory, either ilxml or ildates should be provided
    AddParameter(ParameterType_InputFilenameList, "ilxml", "The XML metadata files list");
    MandatoryOff("ilxml");

    AddParameter(ParameterType_StringList, "ildates", "The dates for the products");
    MandatoryOff("ildates");

    AddParameter(ParameterType_OutputImage, "opf", "Output profile file.");
    SetParameterDescription( "opf", "Filename where the reprocessed profile saved. "
                                    "This is an raster band contains the new BV estimation value for each pixel. "
                                    "The last band contains the boolean information which is 0 if the value has not been reprocessed." );
    MandatoryOff("opf");

    AddParameter(ParameterType_Choice, "algo", 
                 "Reprocessing algorithm: local, fit.");
    SetParameterDescription("algo", 
                            "Reprocessing algorithm: local uses a window around the current date, fit is a double logisting fitting of the complete profile.");

    AddChoice("algo.fit", "Double logistic fitting of the complete profile.");
    SetParameterDescription("algo.fit", "This group of parameters allows to set fit window parameters. ");

    AddChoice("algo.local", "Uses a window around the current date.");
    SetParameterDescription("algo.local", "This group of parameters allows to set local window parameters. ");

    AddParameter(ParameterType_Int, "algo.local.bwr", "Local window backward radius");
    SetParameterInt("algo.local.bwr", 2);
    SetParameterDescription("algo.local.bwr", "Backward radius of the local window. ");

    AddParameter(ParameterType_Int, "algo.local.fwr", "Local window forward radius");
    SetParameterInt("algo.local.fwr", 0);
    SetParameterDescription("algo.local.fwr", "Forward radius of the local window. ");
    MandatoryOff("algo");

    AddParameter(ParameterType_Int, "genall", "Generate LAI for all products in the time series, in one product.");
    MandatoryOff("genall");
    SetDefaultParameterInt("genall", 0);

    // Profile reprocessing splitter parameters
    // TODO: These 2 parameters are not working and should be removed.
    //      The split induce large execution times so it is not an improvement
    //       Instead, ReprocessedProfileSplitter2 should be used
//    AddParameter(ParameterType_OutputFilename, "outrlist", "File containing the list of all raster files produced.");
//    MandatoryOff("outrlist");
//    AddParameter(ParameterType_OutputFilename, "outflist", "File containing the list of all flag files produced.");
//    MandatoryOff("outflist");
    AddParameter(ParameterType_Int, "compress", "Specifies if output files should be compressed or not.");
    MandatoryOff("compress");
    SetDefaultParameterInt("compress", 0);

    m_ImageReaderList = ImageReaderListType::New();
    m_ImageSplitList = SplitFilterListType::New();
    m_deqFunctorList = DeqFunctorListType::New();
    m_imagesList = ImagesListType::New();
    m_bandsConcatteners = ImageListToVectorImageFilterListType::New();

  }

  void DoUpdateParameters()
  {
    //std::cout << "ProfileReprocessing::DoUpdateParameters" << std::endl;
  }

  void DoExecute()
  {
      FloatVectorImageType::Pointer lai_image;
      FloatVectorImageType::Pointer err_image;
      FloatVectorImageType::Pointer msks_image;
      bool hasErrImg = false;

      if (HasValue("illai") && HasValue("ilmsks")) {
          // update the width, hight, origin and projection if we have a main image
          updateRequiredImageSize();

          const std::vector<std::string> &laiImgsList = this->GetParameterStringList("illai");
          const std::vector<std::string> &msksImgsList = this->GetParameterStringList("ilmsks");
          lai_image = this->GetTimeSeriesImage(laiImgsList, false);
          msks_image = this->GetTimeSeriesImage(msksImgsList, true);
          if (HasValue("ilerr")) {
            const std::vector<std::string> &errImgsList = this->GetParameterStringList("ilerr");
            err_image = this->GetTimeSeriesImage(errImgsList, false);
            hasErrImg = true;
          }
      } else {
          lai_image = this->GetParameterImage("lai");
          msks_image = this->GetParameterImage("msks");
          if (HasValue("err")) {
            err_image = this->GetParameterImage("err");
            hasErrImg = true;
          }
      }
      unsigned int nb_dates = 0;
      std::vector<std::string> datesList;
      if (HasValue("ilxml")) {
          const std::vector<std::string> &xmlsList = this->GetParameterStringList("ilxml");
          nb_dates = xmlsList.size();
          for (const std::string &strXml : xmlsList)
          {
              MetadataHelperFactory::Pointer factory = MetadataHelperFactory::New();
              // we are interested only in the 10m resolution as we need only the date
              auto pHelper = factory->GetMetadataHelper<short>(strXml);
              datesList.push_back(pHelper->GetAcquisitionDate());
          }
      } else if (HasValue("ildates")) {
          datesList = this->GetParameterStringList("ildates");
          nb_dates = datesList.size();
      }
      if (nb_dates == 0) {
          itkExceptionMacro("Either ilxml or ildates should be provided");
      }
      // sort the dates
      std::sort (datesList.begin(), datesList.end());

      unsigned int nb_lai_bands = lai_image->GetNumberOfComponentsPerPixel();
      unsigned int nb_err_bands = hasErrImg ? err_image->GetNumberOfComponentsPerPixel() : 0;
      if((nb_lai_bands == 0) || (hasErrImg && (nb_lai_bands != nb_err_bands)) || (nb_lai_bands != nb_dates)) {
          itkExceptionMacro("Invalid number of bands or xmls: lai bands=" <<
                            nb_lai_bands << ", err bands =" <<
                            nb_err_bands << ", nb_dates=" << nb_dates);
      }

      std::vector<std::tm> dv;
      // we have dates array
      for (const std::string &strDate : datesList)
      {
          struct tm tmDate = {};
          std::string formatDate = "%Y%m%d";
          if(strDate.size() == 15 && strDate[8] == 'T') {
              formatDate = "%Y%m%dT%H%M%S";
          }
          if (strptime(strDate.c_str(), formatDate.c_str(), &tmDate) == NULL) {
              itkExceptionMacro("Invalid value for a date: " + strDate);
          }
          dv.push_back(tmDate);
      }

      auto times = pheno::tm_to_doy_list(dv);
      VectorType inDates = VectorType(dv.size());
      std::copy(std::begin(times), std::end(times), std::begin(inDates));

      size_t bwr{1};
      size_t fwr{1};
      ALGO_TYPE algoType = ALGO_LOCAL;
      std::string algo{"local"};
      if (IsParameterEnabled("algo"))
        algo = GetParameterString("algo");
      if (algo == "local")
      {
        if (IsParameterEnabled("algo.local.bwr"))
          bwr = GetParameterInt("algo.local.bwr");
        if (IsParameterEnabled("algo.local.fwr"))
          fwr = GetParameterInt("algo.local.fwr");
      } else {
          algoType = ALGO_FIT;
      }

      bool bGenerateAll = (GetParameterInt("genall") != 0);

      // compute the real BWR according to the dates
//      size_t realBwr = computeRealBwr(bwr, inDates);
//      if(realBwr > bwr) {
//          bwr = realBwr;
//      }

      //instantiate a functor with the regressor and pass it to the
      //unary functor image filter pass also the normalization values
      FunctorType *functor;
      if (hasErrImg) {
          functor = &m_ternaryFunctor;
      } else {
          functor = &m_binaryFunctor;
      }
      functor->SetDates(inDates);
      functor->SetAlgoType(algoType);
      functor->SetBwr(bwr);
      functor->SetFwr(fwr);
      functor->SetGenerateAll(bGenerateAll);

      int nTotalBands = 2;
      if(bGenerateAll) {
          nTotalBands = nb_lai_bands*2;
      }

      if (hasErrImg) {
          m_profileReprocessingFilter = TernaryFilterType::New();
          m_profileReprocessingFilter->SetFunctor(*functor);
          m_profileReprocessingFilter->SetInput1(lai_image);
          m_profileReprocessingFilter->SetInput2(err_image);
          m_profileReprocessingFilter->SetInput3(msks_image);
          m_profileReprocessingFilter->UpdateOutputInformation();
          m_profileReprocessingFilter->GetOutput()->SetNumberOfComponentsPerPixel(nTotalBands);

          //DoProfileReprocessingOutput(datesList, nTotalBands);
          SetParameterOutputImage("opf", m_profileReprocessingFilter->GetOutput());
      } else {
          m_profileReprocessingNoErrFilter = BinaryFilterType::New();
          m_profileReprocessingNoErrFilter->SetFunctor(*((BinaryFunctorType*)functor));
          m_profileReprocessingNoErrFilter->SetInput1(lai_image);
          m_profileReprocessingNoErrFilter->SetInput2(msks_image);
          m_profileReprocessingNoErrFilter->UpdateOutputInformation();
          m_profileReprocessingNoErrFilter->GetOutput()->SetNumberOfComponentsPerPixel(nTotalBands);

          //DoProfileReprocessingOutput(datesList, nTotalBands);
          SetParameterOutputImage("opf", m_profileReprocessingNoErrFilter->GetOutput());
      }
  }

  ImageType::Pointer GetTimeSeriesImage(const std::vector<std::string> &imgsList, bool bIsFlgTimeSeries) {

      float deqValue = GetParameterFloat("deqval");

      if( imgsList.size()== 0 )
      {
          itkExceptionMacro("No input files set...");
      }

      // keep the first image one that has the origin and dimmension of the main one (if it is the case)
      //imgsList = trimLeftInvalidPrds(imgsList);

      ImageListType::Pointer allBandsList = ImageListType::New();
      for (const std::string& strImg : imgsList)
      {
          ImageReaderType::Pointer reader = getReader(strImg);
          ImageType::Pointer img = reader->GetOutput();
          img->UpdateOutputInformation();

          // cut the image if we need to
          img = cutImage(img, bIsFlgTimeSeries);

          // dequantify image if we need to
          img = dequantifyImage(img, deqValue);

          m_imagesList->PushBack(img);

          VectorImageToImageListType::Pointer splitter = getSplitter(img);
          int nBands = img->GetNumberOfComponentsPerPixel();
          for(int i = 0; i<nBands; i++)
          {
              allBandsList->PushBack(splitter->GetOutput()->GetNthElement(i));
          }

      }
      ImageListToVectorImageFilterType::Pointer bandsConcat = ImageListToVectorImageFilterType::New();
      bandsConcat->SetInput(allBandsList);
      bandsConcat->UpdateOutputInformation();
      m_bandsConcatteners->PushBack(bandsConcat);
      bandsConcat->GetOutput()->UpdateOutputInformation();

      return bandsConcat->GetOutput();
  }

  // get a reader from the file path
  ImageReaderType::Pointer getReader(const std::string& filePath) {
        ImageReaderType::Pointer reader = ImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);
        reader->UpdateOutputInformation();

        // add it to the list and return
        m_ImageReaderList->PushBack(reader);
        return reader;
  }
  VectorImageToImageListType::Pointer getSplitter(const ImageType::Pointer& image) {
      VectorImageToImageListType::Pointer imgSplit = VectorImageToImageListType::New();
      imgSplit->SetInput(image);
      imgSplit->UpdateOutputInformation();
      m_ImageSplitList->PushBack( imgSplit );
      return imgSplit;
  }

  ImageType::Pointer dequantifyImage(const ImageType::Pointer &img, float deqVal) {
      if(deqVal > 0) {
          DequantifyFilterType::Pointer deqFunctor = DequantifyFilterType::New();
          m_deqFunctorList->PushBack(deqFunctor);
          deqFunctor->GetFunctor().Initialize(deqVal, 0);
          deqFunctor->SetInput(img);
          int nComponents = img->GetNumberOfComponentsPerPixel();
          ImageType::Pointer newImg = deqFunctor->GetOutput();
          newImg->SetNumberOfComponentsPerPixel(nComponents);
          newImg->UpdateOutputInformation();
          return newImg;
      }
      return img;
  }

  ImageType::Pointer cutImage(const ImageType::Pointer &img, bool bIsFlg) {
      ImageType::Pointer retImg = img;
      if(m_bCutImages) {
          float imageWidth = img->GetLargestPossibleRegion().GetSize()[0];
          float imageHeight = img->GetLargestPossibleRegion().GetSize()[1];

          ImageType::PointType origin = img->GetOrigin();
          ImageType::PointType imageOrigin;
          imageOrigin[0] = origin[0];
          imageOrigin[1] = origin[1];
          int curImgRes = img->GetSpacing()[0];
          const float scale = (float)m_nPrimaryImgRes / curImgRes;

          if((imageWidth != m_primaryMissionImgWidth) || (imageHeight != m_primaryMissionImgHeight) ||
                  (m_primaryMissionImgOrigin[0] != imageOrigin[0]) || (m_primaryMissionImgOrigin[1] != imageOrigin[1])) {

              Interpolator_Type interpolator = (bIsFlg == 0 ? Interpolator_Linear :
                                                    Interpolator_NNeighbor);
              std::string imgProjRef = img->GetProjectionRef();
              // if the projections are equal
              if(imgProjRef == m_strPrMissionImgProjRef) {
                  // use the streaming resampler
                  retImg = m_ImageResampler.getResampler(img, scale,m_primaryMissionImgWidth,
                              m_primaryMissionImgHeight,m_primaryMissionImgOrigin, interpolator)->GetOutput();
              } else {
                  // use the generic RS resampler that allows reprojecting
                  retImg = m_GenericRSImageResampler.getResampler(img, scale,m_primaryMissionImgWidth,
                              m_primaryMissionImgHeight,m_primaryMissionImgOrigin, interpolator)->GetOutput();
              }
              retImg->UpdateOutputInformation();
          }
      }

      return retImg;

  }

  void updateRequiredImageSize() {
      m_bCutImages = false;
      m_primaryMissionImgWidth = 0;
      m_primaryMissionImgHeight = 0;

      std::string mainImg;
      if (HasValue("main")) {
          mainImg = this->GetParameterString("main");
          m_bCutImages = true;
      } else {
          return;
      }

      ImageReaderType::Pointer reader = getReader(mainImg);
      m_primaryMissionImg = reader->GetOutput();
      reader->UpdateOutputInformation();
      m_primaryMissionImg->UpdateOutputInformation();

      m_primaryMissionImgWidth = m_primaryMissionImg->GetLargestPossibleRegion().GetSize()[0];
      m_primaryMissionImgHeight = m_primaryMissionImg->GetLargestPossibleRegion().GetSize()[1];

      //ImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
      m_nPrimaryImgRes = m_primaryMissionImg->GetSpacing()[0];

      ImageType::PointType origin = m_primaryMissionImg->GetOrigin();
      m_primaryMissionImgOrigin[0] = origin[0];
      m_primaryMissionImgOrigin[1] = origin[1];

      m_strPrMissionImgProjRef = m_primaryMissionImg->GetProjectionRef();
      m_GenericRSImageResampler.SetOutputProjection(m_strPrMissionImgProjRef);
  }

  // dynamically compute the bwr in order to use a BWR as dates and not as images
  int computeRealBwr(size_t bwr, const VectorType &inDates)
  {
      size_t newBwr = 1;
      size_t realBwr = 0;
      for (size_t i = 1; i<inDates.size(); i++) {
          if (realBwr == bwr+1) {
              newBwr--;
              break;
          }
          if (inDates[i] != inDates[i - 1] ||
              (i == inDates.size() - 1)) {
              realBwr++;
          }
          newBwr++;
      }

      return (newBwr - 1);
  }

  // Profile reprocessing variables
  TernaryFilterType::Pointer m_profileReprocessingFilter;
  BinaryFilterType::Pointer m_profileReprocessingNoErrFilter;
  FunctorType m_ternaryFunctor;
  BinaryFunctorType m_binaryFunctor;

  // Time series builder variables
  ImageReaderListType::Pointer                m_ImageReaderList;
  SplitFilterListType::Pointer                m_ImageSplitList;
  ImageListToVectorImageFilterListType::Pointer   m_bandsConcatteners;
  DeqFunctorListType::Pointer                 m_deqFunctorList;
  ImagesListType::Pointer                     m_imagesList;

  float                                 m_pixSize;
  double                                m_primaryMissionImgWidth;
  double                                m_primaryMissionImgHeight;
  ImageType::PointType                  m_primaryMissionImgOrigin;
  int                                   m_nPrimaryImgRes;
  bool m_bCutImages;
  ImageResampler<ImageType, ImageType>  m_ImageResampler;
  GenericRSImageResampler<ImageType, ImageType>  m_GenericRSImageResampler;

  ImageType::Pointer m_primaryMissionImg;
  std::string m_strPrMissionImgProjRef;

  // Profile reprocessing splitter variables
  SplitterFilterType::Pointer        m_Filter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ProfileReprocessing)
