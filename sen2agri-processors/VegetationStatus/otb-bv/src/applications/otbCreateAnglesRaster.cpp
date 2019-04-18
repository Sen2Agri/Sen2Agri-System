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
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbStreamingResampleImageFilter.h"
#include "MetadataHelperFactory.h"
#include "ImageResampler.h"
#include "otbMetaDataKey.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "GlobalDefs.h"
#include "itkBinaryFunctorImageFilter.h"
#include "CommonFunctions.h"

#define ANGLES_GRID_SIZE    23
#define ANGLES_QUANTIFICATION_VALUE     10000

namespace otb
{

template< class TInput, class TInput2, class TOutput>
class AnglesMaskingFunctor
{
public:
    AnglesMaskingFunctor() {}
    ~AnglesMaskingFunctor() {}

  bool operator!=( const AnglesMaskingFunctor &a) const
  {
      return false;
  }
  bool operator==( const AnglesMaskingFunctor & other ) const
  {
    return !(*this != other);
  }
  inline TOutput operator()( const TInput & A, const TInput2 & B ) const
  {
        TOutput ret(A.GetSize());
        for (int i = 0; i<A.GetSize(); i++) {
            ret[i] = A[i];
            if (B[i] == 0 || B[i] == NO_DATA_VALUE) {
                ret[i] = NO_DATA_VALUE;
            }
        }
        return ret;
  }
};

namespace Wrapper
{
class CreateAnglesRaster : public Application
{
public:
    typedef CreateAnglesRaster Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    itkNewMacro(Self)

    itkTypeMacro(CreateAnglesRaster, otb::Application)

    typedef short                                                             PixelType;
    typedef otb::VectorImage<PixelType, 2>                                    ImageType;
    typedef otb::ImageFileReader<ImageType>                                   ReaderType;
    typedef FloatVectorImageType                                              AnglesImageType;

    //typedef otb::ChangeNoDataValueFilter<FloatVectorImageType,FloatVectorImageType> ChangeNoDataFilterType;

    typedef itk::BinaryFunctorImageFilter<AnglesImageType,AnglesImageType,AnglesImageType,
                    AnglesMaskingFunctor<
                        AnglesImageType::PixelType, AnglesImageType::PixelType,
                        AnglesImageType::PixelType> > AnglesMaskedOutputFilterType;
private:

    char const * NoDataValueAvailable = "NoDataValueAvailable";
    char const * NoDataValue = "NoDataValue";

    void DoInit()
    {

        SetName("CreateAnglesRaster");
        SetDescription("Creates a raster from the angles from a product");

        SetDocName("CreateAnglesRaster");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("Cosmin Udroiu");
        SetDocSeeAlso(" ");
        AddDocTag(Tags::Vector);

        AddParameter(ParameterType_String, "xml", "Product Metadata XML File");
        AddParameter(ParameterType_Int, "resampled", "Resamples to the product main resolution.");
        SetDefaultParameterInt("resampled", 0);
        MandatoryOff("resampled");
        AddParameter(ParameterType_OutputImage, "out", "Out angles image");


        SetDocExampleParameterValue("xml", "The product metadata file");
        SetDocExampleParameterValue("resampled", "Resamples the output raster to the main product resolution");
        SetDocExampleParameterValue("out", "/path/to/output_image.tif");
    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {
        const std::string &inMetadataXml = GetParameterString("xml");
        if (inMetadataXml.empty())
        {
            itkExceptionMacro("No input metadata XML set...; please set the input image");
        }

        auto factory = MetadataHelperFactory::New();
        m_pHelper = factory->GetMetadataHelper<short>(inMetadataXml);

        const std::vector<std::string> &bandNames = m_pHelper->GetBandNamesForResolution(m_pHelper->GetProductResolutions()[0]);
        std::vector<int> relBandIdxs;
        m_img = m_pHelper->GetImage(bandNames, &relBandIdxs);
        m_img->UpdateOutputInformation();


//        itk::MetaDataDictionary dict = m_defImgReader->GetOutput()->GetMetaDataDictionary();
//        std::vector<bool> flgs(1,true);
//        std::vector<double> vals(1,NO_DATA_VALUE);
//        WriteNoDataFlags(flgs, vals, dict);

//        std::vector<bool> flags;
//        std::vector<double> values;
//        ReadNoDataFlags(dict, flags, values);

        AnglesImageType::Pointer anglesImg = createAnglesBands(m_pHelper, m_pHelper->HasDetailedAngles());

        if(HasValue("resampled")) {
            bool resampled = (GetParameterInt("resampled") != 0);
            if (resampled) {
                anglesImg = resampleAnglesImage(anglesImg);
            }
        }
        anglesImg->UpdateOutputInformation();
        anglesImg->GetMetaDataDictionary();

//        std::string outImgFileName = GetParameterAsString("out");
//        outImgFileName += "?gdal:co:TILED=YES&gdal:co:COMPRESS=LZW";

//        // Create an output parameter to write the current output image
//        OutputImageParameter::Pointer paramOut = OutputImageParameter::New();
//        // Set the filename of the current output image
//        paramOut->SetFileName(outImgFileName);
//        paramOut->SetValue(anglesImg);
//        paramOut->SetPixelType(ImagePixelType_int16);
//        // Add the current level to be written
//        paramOut->InitializeWriters();
//        std::ostringstream osswriter;
//        osswriter<< "Wrinting output "<< outImgFileName;
//        AddProcess(paramOut->GetWriter(), osswriter.str());
//        paramOut->Write();


        SetParameterOutputImagePixelType("out", ImagePixelType_int16);
        SetParameterOutputImage("out", anglesImg);
    }

    AnglesImageType::Pointer createAnglesBands(const std::unique_ptr<MetadataHelper<short>> &pHelper, bool useDetailedAngles) {
        m_AnglesRaster = allocateRaster();
        m_AnglesMaskRaster = allocateRaster();
        if (useDetailedAngles) {
            writeDetailedAnglesToRaster(pHelper);
        } else {
            writeMeanAnglesToRaster(pHelper);
        }
        m_AnglesRaster->UpdateOutputInformation();
        m_AnglesMaskRaster->UpdateOutputInformation();

        return m_AnglesRaster;
    }

    AnglesImageType::Pointer allocateRaster()
    {
        auto sz = m_img->GetLargestPossibleRegion().GetSize();
        auto spacing = m_img->GetSpacing();
        const std::string &imgProjRef = m_img->GetProjectionRef();

        int width = sz[0];
        int height = sz[1];

        AnglesImageType::Pointer anglesRaster = AnglesImageType::New();

        AnglesImageType::IndexType start;
        start[0] =   0;  // first index on X
        start[1] =   0;  // first index on Y

        AnglesImageType::SizeType size;
        size[0]  = ANGLES_GRID_SIZE;  // size along X
        size[1]  = ANGLES_GRID_SIZE;  // size along Y

        AnglesImageType::RegionType region;
        region.SetSize(size);
        region.SetIndex(start);

        anglesRaster->SetRegions(region);
        anglesRaster->SetNumberOfComponentsPerPixel(3);

        AnglesImageType::SpacingType anglesRasterSpacing;
        anglesRasterSpacing[0] = (((float)width) * spacing[0]) / ANGLES_GRID_SIZE; // spacing along X
        anglesRasterSpacing[1] = (((float)height) * spacing[1]) / ANGLES_GRID_SIZE; // spacing along Y
        anglesRaster->SetSpacing(anglesRasterSpacing);
        anglesRaster->SetProjectionRef(imgProjRef);

        itk::MetaDataDictionary dict = anglesRaster->GetMetaDataDictionary();
        std::vector<bool> flgs(1,true);
        std::vector<double> vals(1,NO_DATA_VALUE);
        WriteNoDataFlags(flgs, vals, dict);
        anglesRaster->SetMetaDataDictionary(dict);

        anglesRaster->Allocate();

        return anglesRaster;
    }

    void writeMeanAnglesToRaster(const std::unique_ptr<MetadataHelper<short>> &pHelper)
    {
        double quantifValue = pHelper->GetReflectanceQuantificationValue();
        double cosSensorZenith = round(cos(pHelper->GetSensorMeanAngles().zenith * M_PI / 180) * quantifValue);
        double cosSunZenith = round(cos(pHelper->GetSolarMeanAngles().zenith * M_PI / 180) * quantifValue);
        double cosRelAzimuth = round(cos(pHelper->GetRelativeAzimuthAngle() * M_PI / 180) * quantifValue);

        for(unsigned int i = 0; i < ANGLES_GRID_SIZE; i++) {
            for(unsigned int j = 0; j < ANGLES_GRID_SIZE; j++) {
                itk::VariableLengthVector<float> vct(3);
                vct[0] = cosSensorZenith;
                vct[1] = cosSunZenith;
                vct[2] = cosRelAzimuth;

                AnglesImageType::IndexType idx;
                idx[0] = j;
                idx[1] = i;
                m_AnglesRaster->SetPixel(idx, vct);
            }
        }
    }

    void writeDetailedAnglesToRaster(const std::unique_ptr<MetadataHelper<short>> &pHelper)
    {
        const MetadataHelperAngles &solarAngles = pHelper->GetDetailedSolarAngles();
        const std::vector<MetadataHelperViewingAnglesGrid> &viewingAngles = pHelper->GetAllDetectorsDetailedViewingAngles();
        const std::vector<std::vector<double>> &sensorZenithMeanMatrix = getMeanViewingAngles(viewingAngles, true);
        const std::vector<std::vector<double>> &sensorAzimuthMeanMatrix = getMeanViewingAngles(viewingAngles, false);

        const std::vector<std::vector<double>> &sensorZenithCosMatrix = computeCosValues(sensorZenithMeanMatrix);
        const std::vector<std::vector<double>> &sunZenithCosMatrix = computeCosValues(solarAngles.Zenith.Values);
        const std::vector<std::vector<double>> &relAzimuthMatrix = computeRelativeAzimuth(solarAngles.Azimuth.Values, sensorAzimuthMeanMatrix);

        const std::vector<std::vector<short>> &sensorZenithCosMaskMatrix = extractMaskValues(sensorZenithMeanMatrix);
        const std::vector<std::vector<short>> &sunZenithCosMaskMatrix = extractMaskValues(solarAngles.Zenith.Values);
        const std::vector<std::vector<short>> &relAzimuthMaskMatrix = extractRelativeAzimuthMaskValues(solarAngles.Azimuth.Values, sensorAzimuthMeanMatrix);

        for(unsigned int i = 0; i < ANGLES_GRID_SIZE; i++) {
            const std::vector<double> &curSensorZenithLine = sensorZenithCosMatrix[i];
            const std::vector<double> &curSunZenithCosLine = sunZenithCosMatrix[i];
            const std::vector<double> &curRelAzimuthLine = relAzimuthMatrix[i];

            const std::vector<short> &curSensorZenithMaskLine = sensorZenithCosMaskMatrix[i];
            const std::vector<short> &curSunZenithCosMaskLine = sunZenithCosMaskMatrix[i];
            const std::vector<short> &curRelAzimuthMaskLine = relAzimuthMaskMatrix[i];

            for(unsigned int j = 0; j < ANGLES_GRID_SIZE; j++) {
                itk::VariableLengthVector<float> vct(3);
                vct[0] = curSensorZenithLine[j];
                vct[1] = curSunZenithCosLine[j];
                vct[2] = curRelAzimuthLine[j];

                AnglesImageType::IndexType idx;
                idx[0] = j;
                idx[1] = i;
                m_AnglesRaster->SetPixel(idx, vct);

                // set the mask image pixel
                itk::VariableLengthVector<short> vctMsk(3);
                vctMsk[0] = curSensorZenithMaskLine[j];
                vctMsk[1] = curSunZenithCosMaskLine[j];
                vctMsk[2] = curRelAzimuthMaskLine[j];
                m_AnglesMaskRaster->SetPixel(idx, vctMsk);
            }
        }
    }

    /**
     * Resamples an image according to the given current and desired resolution
     */
    AnglesImageType::Pointer resampleAnglesImage(AnglesImageType::Pointer anglesRaster) {
        auto sz = m_img->GetLargestPossibleRegion().GetSize();
        int width = sz[0];
        int height = sz[1];

        AnglesImageType::Pointer anglesImg = m_AnglesResampler.getResampler(anglesRaster, width, height)->GetOutput();
        anglesImg->UpdateOutputInformation();
        AnglesImageType::Pointer msksImg = m_AnglesResampler.getResampler(m_AnglesMaskRaster, width, height,
                                                                          Interpolator_NNeighbor)->GetOutput();
        msksImg->UpdateOutputInformation();

        m_AnglesMaskedOutputFunctor = AnglesMaskedOutputFilterType::New();
        AnglesImageType::Pointer retImg = MaskAngles(anglesImg, msksImg, m_AnglesMaskedOutputFunctor);
        retImg->UpdateOutputInformation();

        return retImg;
    }

    std::vector<std::vector<double>> getMeanViewingAngles(const std::vector<MetadataHelperViewingAnglesGrid> &viewingAngles,
                                                          bool useZenith) {
        std::vector<std::vector<double>> sumsMatrix;
        std::vector<std::vector<int>> cntNotNanMatrix;
        for (const MetadataHelperViewingAnglesGrid &grid : viewingAngles) {
            const MetadataHelperAngleList &anglesList = useZenith ? grid.Angles.Zenith : grid.Angles.Azimuth;
            for (size_t i = 0; i<anglesList.Values.size(); i++) {
                const std::vector<double> &valuesLine = anglesList.Values[i];
                if (sumsMatrix.size() == i) {
                    expandMatrices(valuesLine, sumsMatrix, cntNotNanMatrix);
                } else {
                    updateMatrices(valuesLine, i, sumsMatrix, cntNotNanMatrix);
                }
            }
        }
        return getMatrixMeanValues(sumsMatrix, cntNotNanMatrix);
    }

    void expandMatrices(const std::vector<double> &valuesLine, std::vector<std::vector<double>> &sumsMatrix,
                        std::vector<std::vector<int>> &cntNotNanMatrix) {
        std::vector<double> sumsVect;
        std::vector<int> cntNotNanVect;
        for (size_t i = 0; i<valuesLine.size(); i++) {
            if (std::isnan(valuesLine[i])) {
                sumsVect.push_back(0.0);
                cntNotNanVect.push_back(0);
            } else {
                sumsVect.push_back(valuesLine[i]);
                cntNotNanVect.push_back(1);
            }
        }
        sumsMatrix.push_back(sumsVect);
        cntNotNanMatrix.push_back(cntNotNanVect);
    }

    void updateMatrices(const std::vector<double> &valuesLine, int lineIdxInMatrix, std::vector<std::vector<double>> &sumsMatrix,
                        std::vector<std::vector<int>> &cntNotNanMatrix) {
        std::vector<double> &sumsVect = sumsMatrix[lineIdxInMatrix];
        std::vector<int> &cntNotNanVect = cntNotNanMatrix[lineIdxInMatrix];

        // Normally, the size of the values line and the sums and notNan vectors are the same
        // but if they are not, then expand the sums vector and the
        while (sumsVect.size() < valuesLine.size()) {
            sumsVect.push_back(0.0);
            cntNotNanVect.push_back(0);
        }
        // now update the lines
        for (size_t i = 0; i<valuesLine.size(); i++) {
            if (!std::isnan(valuesLine[i])) {
                sumsVect[i] += valuesLine[i];
                cntNotNanVect[i] += 1;
            }
        }
    }

    std::vector<std::vector<double>> getMatrixMeanValues(const std::vector<std::vector<double>> &sumsMatrix,
                                                         const std::vector<std::vector<int>> &nonNanMatrix) {
        //printMatrix(sumsMatrix);
        //printMatrix(nonNanMatrix);

        std::vector<std::vector<double>> retMeanValuesMatrix;
        for (size_t i = 0; i < sumsMatrix.size(); i++) {
            const std::vector<double> &sumsVect = sumsMatrix[i];
            const std::vector<int> &nonNansVect = nonNanMatrix[i];
            std::vector<double> meanValsVect;
            for (size_t j = 0; j<sumsVect.size(); j++) {
                meanValsVect.push_back((nonNansVect[j] != 0) ?
                                           (sumsVect[j]/nonNansVect[j]) :
                                           std::numeric_limits<double>::quiet_NaN());
             }
            retMeanValuesMatrix.push_back(meanValsVect);
        }

        //printMatrix(retMeanValuesMatrix);
        return retMeanValuesMatrix;
    }

    std::vector<std::vector<double>> computeCosValues(const std::vector<std::vector<double>> &meanValuesMatrix) {
        std::vector<std::vector<double>> retCosValuesMatrix;
        for (const std::vector<double> &meanValsVect : meanValuesMatrix) {
            std::vector<double> cosValsVect;
            for (double val : meanValsVect) {
                if (std::isnan(val)) {
                    cosValsVect.push_back(NO_DATA_VALUE);
                } else {
                    cosValsVect.push_back(ANGLES_QUANTIFICATION_VALUE * cos((val * M_PI ) / 180));
                }
             }
            retCosValuesMatrix.push_back(cosValsVect);
        }
        //printMatrix(retCosValuesMatrix);
        return retCosValuesMatrix;
    }

    std::vector<std::vector<double>> computeRelativeAzimuth(const std::vector<std::vector<double>> &solarAzimuthValuesMatrix,
                                                            const std::vector<std::vector<double>> &sensorAzimuthValuesMatrix) {
        std::vector<std::vector<double>> retRelAzimuthValuesMatrix;
        for (size_t i = 0; i < solarAzimuthValuesMatrix.size(); i++) {
            const std::vector<double> &solarAzimuthVect = solarAzimuthValuesMatrix[i];
            const std::vector<double> &sensorAzimuthVect = sensorAzimuthValuesMatrix[i];
            std::vector<double> relAzimuthValsVect;
            for (size_t j = 0; j<solarAzimuthVect.size(); j++) {
                if (std::isnan(solarAzimuthVect[j]) || std::isnan(sensorAzimuthVect[j])) {
                    relAzimuthValsVect.push_back(NO_DATA_VALUE);
                } else {
                    double val = solarAzimuthVect[j] - sensorAzimuthVect[j];
                    relAzimuthValsVect.push_back(ANGLES_QUANTIFICATION_VALUE * cos((val * M_PI ) / 180));
                }
             }
            retRelAzimuthValuesMatrix.push_back(relAzimuthValsVect);
        }

        return retRelAzimuthValuesMatrix;
    }

    std::vector<std::vector<short>> extractMaskValues(const std::vector<std::vector<double>> &meanValuesMatrix) {
        std::vector<std::vector<short>> retMaskValuesMatrix;
        for (const std::vector<double> &meanValsVect : meanValuesMatrix) {
            std::vector<short> maskValsVect;
            for (double val : meanValsVect) {
                if (std::isnan(val)) {
                    maskValsVect.push_back(0);
                } else {
                    maskValsVect.push_back(1);
                }
             }
            retMaskValuesMatrix.push_back(maskValsVect);
        }
        return retMaskValuesMatrix;
    }

    std::vector<std::vector<short>> extractRelativeAzimuthMaskValues(const std::vector<std::vector<double>> &solarAzimuthValuesMatrix,
                                                            const std::vector<std::vector<double>> &sensorAzimuthValuesMatrix) {
        std::vector<std::vector<short>> retMaskValuesMatrix;
        for (size_t i = 0; i < solarAzimuthValuesMatrix.size(); i++) {
            const std::vector<double> &solarAzimuthVect = solarAzimuthValuesMatrix[i];
            const std::vector<double> &sensorAzimuthVect = sensorAzimuthValuesMatrix[i];
            std::vector<short> maskValsVect;
            for (size_t j = 0; j<solarAzimuthVect.size(); j++) {
                if (std::isnan(solarAzimuthVect[j]) || std::isnan(sensorAzimuthVect[j])) {
                    maskValsVect.push_back(0);
                } else {
                    maskValsVect.push_back(1);
                }
             }
            retMaskValuesMatrix.push_back(maskValsVect);
        }

        return retMaskValuesMatrix;
    }

    AnglesImageType::Pointer MaskAngles(AnglesImageType::Pointer anglesImg, AnglesImageType::Pointer msksImg,
                                     AnglesMaskedOutputFilterType::Pointer maskingFunctor) {

        anglesImg->UpdateOutputInformation();
        msksImg->UpdateOutputInformation();

        maskingFunctor->SetInput1(anglesImg);
        maskingFunctor->SetInput2(msksImg);
        return maskingFunctor->GetOutput();
    }

    bool ReadNoDataFlags(const itk::MetaDataDictionary& dict, std::vector<bool> & flags, std::vector<double> & values)
    {
      bool ret = itk::ExposeMetaData<std::vector<bool> >(dict,NoDataValueAvailable,flags);

      if (ret)
        ret = itk::ExposeMetaData<std::vector<double> >(dict,NoDataValue,values);

      return ret;
    }

    void WriteNoDataFlags(const std::vector<bool> & flags, const std::vector<double> & values, itk::MetaDataDictionary& dict)
    {
     itk::EncapsulateMetaData<std::vector<bool> >(dict,NoDataValueAvailable,flags);
     itk::EncapsulateMetaData<std::vector<double> >(dict,NoDataValue,values);
    }

    template<typename T>
    void printMatrix(const std::vector<std::vector<T>> &matrix) {
        std::cout << "[";
        for (const std::vector<T> &curLine : matrix) {
            std::cout << "[";
            int items = 0;
            for (T val : curLine) {
                if (items == 5) {
                    std::cout << std::endl;
                    items = 0;
                }
                std::cout << " " << val << std::setprecision(7) << " ";
                items++;
             }
            std::cout << "]" << std::endl;
        }
        std::cout << "]" << std::endl;
    }

    AnglesImageType::Pointer            m_AnglesRaster;
    AnglesImageType::Pointer            m_AnglesMaskRaster;
    ImageResampler<AnglesImageType, AnglesImageType> m_AnglesResampler;
    AnglesMaskedOutputFilterType::Pointer m_AnglesMaskedOutputFunctor;

    std::unique_ptr<MetadataHelper<short>> m_pHelper;
    MetadataHelper<short>::VectorImageType::Pointer m_img;

    //ChangeNoDataFilterType::Pointer m_ChangeNoDataFilter;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::CreateAnglesRaster)


