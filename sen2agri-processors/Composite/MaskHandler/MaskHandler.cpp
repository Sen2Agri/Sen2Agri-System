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
//    INPUTS: {input image}, {xml file desc}
//    OUTPUTS: {output image NDVI}
//  Software Guide : EndCommandLineArgs


//  Software Guide : BeginLatex
//
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"

#include "itkBinaryFunctorImageFilter.h"

#include "MaskExtractorFilter.hxx"

#include <vector>
#include "MaskHandlerFunctor.h"
#include "MetadataHelperFactory.h"
#include "otbImageToVectorImageCastFilter.h"
#include "GlobalDefs.h"

namespace otb
{

namespace Wrapper
{
namespace Functor
{
    template< class TInput, class TOutput>
    class MaskHandlerFunctor
    {
    public:
        MaskHandlerFunctor()
        {
        }

        MaskHandlerFunctor& operator =(const MaskHandlerFunctor& )
        {
            return *this;
        }

        bool operator!=( const MaskHandlerFunctor & ) const
        {
            return true;
        }
        bool operator==( const MaskHandlerFunctor &  ) const
        {
            return false;
        }

        TOutput operator()( const TInput & A)
        {
            TOutput var(3);
            var.Fill(-10000);
            var[0] = (A == IMG_FLG_CLOUD);
            var[1] = (A == IMG_FLG_WATER);
            var[2] = (A == IMG_FLG_SNOW);
            return var;
        }
    };
}

class MaskHandler : public Application
{
public:    

    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef MaskHandler Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)

    itkTypeMacro(MaskHandler, otb::Application)
    //  Software Guide : EndCodeSnippet

    typedef otb::ImageFileReader<Int16VectorImageType>                          ReaderType;
    typedef otb::ImageList<Int16ImageType>                                      ImageListType;
    typedef ImageListToVectorImageFilter<ImageListType, Int16VectorImageType >  ListConcatenerFilterType;
//    typedef MaskHandlerFunctor <Int16VectorImageType::PixelType,
//                                    Int16VectorImageType::PixelType, Int16VectorImageType::PixelType> MaskHandlerFunctorType;

//    typedef itk::BinaryFunctorImageFilter< Int16VectorImageType, Int16VectorImageType,
//                                            Int16VectorImageType, MaskHandlerFunctorType > FunctorFilterType;

//    typedef otb::MaskExtractorFilter<Int16VectorImageType, Int16VectorImageType> MaskExtractorFilterType;

//    typedef otb::ImageToVectorImageCastFilter<Int16ImageType, Int16VectorImageType> VectorCastFilterType;

      typedef Functor::MaskHandlerFunctor <Int16ImageType::PixelType,
                                  Int16VectorImageType::PixelType>              MaskHandlerFunctorType;

    typedef itk::UnaryFunctorImageFilter<Int16ImageType,Int16VectorImageType, MaskHandlerFunctorType > MaskHandlerFilterType;


private:

    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
    //  Software Guide : EndLatex




    void DoInit()
    {
        SetName("MaskHandler");
        SetDescription("Extracts Cloud, Water and Snow masks from _div.tif and _sat.tif SPOT files ");

        SetDocName("MaskHandler");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("AG");
        SetDocSeeAlso(" ");        
        AddDocTag(Tags::Vector);        
        AddParameter(ParameterType_String, "xml", "General xml input file for L2A");
        AddParameter(ParameterType_Int, "sentinelres", "Resolution for which the masks sould be handled, SENTINEL-S2 only");
        MandatoryOff("sentinelres");
        SetDefaultParameterInt("sentinelres", 10);

        AddParameter(ParameterType_OutputImage, "out", "Out file for cloud, water and snow mask");

    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {
        //m_castFilter = VectorCastFilterType::New();
        const std::string &inXml = GetParameterAsString("xml");
        auto factory = MetadataHelperFactory::New();

        int resolution = 10;
        if(HasValue("sentinelres")) {
            resolution = GetParameterInt("sentinelres");
        }
        m_pHelper = factory->GetMetadataHelper<short>(inXml);
        std::string missionName = m_pHelper->GetMissionName();
        if((missionName.find(SENTINEL_MISSION_STR) != std::string::npos) &&
           !HasValue("sentinelres")) {
           itkExceptionMacro("In case of SENTINEL-S2, 'sentinelres' parameter with resolution as 10 or 20 meters should be provided");
        }

        m_MaskExtractor = MaskHandlerFilterType::New();
//        m_ReaderCloud = ReaderType::New();
//        m_ReaderWaterSnow = ReaderType::New();
//        if(missionName.find(SPOT4_MISSION_STR) != std::string::npos ||
//                (missionName.find(SPOT5_MISSION_STR) != std::string::npos)) {
//            m_MaskExtractor->SetBitsMask(0, 1, 2);
//        } else if ((missionName.find(LANDSAT_MISSION_STR) != std::string::npos) ||
//                   (missionName.find(SENTINEL_MISSION_STR) != std::string::npos)) {
//            m_MaskExtractor->SetBitsMask(0, 0, 5);
//        } else {
//            itkExceptionMacro("Unknown mission: " + missionName);
//        }
//        m_ReaderCloud->SetFileName(m_pHelper->GetCloudImageFileName());
//        m_ReaderWaterSnow->SetFileName(m_pHelper->GetWaterImageFileName());

        m_MaskExtractor->SetFunctor(m_Functor);
        Int16ImageType::Pointer img = m_pHelper->GetMasksImage((MasksFlagType)(MSK_CLOUD|MSK_WATER|MSK_SNOW), false, resolution);
        img->UpdateOutputInformation();
        m_MaskExtractor->SetInput(img);
        m_MaskExtractor->UpdateOutputInformation();
        m_MaskExtractor->GetOutput()->SetNumberOfComponentsPerPixel(3);
        m_MaskExtractor->UpdateOutputInformation();

        SetParameterOutputImagePixelType("out", ImagePixelType_int16);
        SetParameterOutputImage("out", m_MaskExtractor->GetOutput());

//        m_castFilter->SetInput(m_pHelper->GetMasksImage((MasksFlagType)(MSK_CLOUD|MSK_WATER|MSK_SNOW), false));
//        SetParameterOutputImage("out", m_castFilter->GetOutput());

        return;
    }

    ReaderType::Pointer                 m_ReaderCloud;
    ReaderType::Pointer                 m_ReaderWaterSnow ;
    //FunctorFilterType::Pointer      m_SpotMaskHandlerFunctor;
    MaskHandlerFunctorType                      m_Functor;
    MaskHandlerFilterType::Pointer              m_MaskExtractor;
    std::unique_ptr<MetadataHelper<short>> m_pHelper;

    //VectorCastFilterType::Pointer m_castFilter;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::MaskHandler)



