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
 
#include <limits>

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "otbVectorImage.h"

#include "ContinuousColorMappingFilter.hxx"
#include "otbStreamingStatisticsVectorImageFilter.h"
#include <boost/algorithm/string.hpp>

namespace otb
{

namespace Wrapper
{


class ContinuousColorMapping : public Application
{
public:

    typedef otb::StreamingStatisticsVectorImageFilter<FloatVectorImageType> StreamingStatisticsVectorImageFilterType;

    typedef ContinuousColorMapping Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(ContinuousColorMapping, otb::Application)

private:

    void DoInit() ITK_OVERRIDE
    {
        SetName("ContinuousColorMapping");
        SetDescription("Applies a color ramp to an image");

        SetDocName("ContinuousColorMapping");
        SetDocLongDescription(
            "Applies a color ramp to an image");
        SetDocLimitations("None");
        SetDocAuthors("LNI");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Vector);

        AddParameter(ParameterType_InputImage, "in", "The input image");
        AddParameter(ParameterType_OutputImage, "out", "The output image");
        AddParameter(ParameterType_InputFilename, "map", "The color mapping");

        AddParameter(ParameterType_StringList, "bandidx", "Specifies the band index(es) to be used from the input image");
        MandatoryOff("bandidx");

        AddParameter(ParameterType_Int, "rgbimg", "Specifies if the input image has at least 3 bands that will be translated to RGB");
        SetDefaultParameterInt("rgbimg", 0);
        MandatoryOff("rgbimg");

        AddParameter(ParameterType_Int, "isrange", "Specifies if the lines in the file contain ranges or single values");
        SetDefaultParameterInt("isrange", 1);
        MandatoryOff("isrange");

        SetDocExampleParameterValue("in", "in.tif");
        SetDocExampleParameterValue("out", "out.tif");
        SetDocExampleParameterValue("map", "ramp.map");
    }

    void DoUpdateParameters() ITK_OVERRIDE
    {
    }

    void DoExecute() ITK_OVERRIDE
    {
        const auto &file = GetParameterString("map");
        std::ifstream mapFile(file);
        if (!mapFile) {
            itkExceptionMacro("Unable to open " << file);
        }

        bool bIsRGBImage = (GetParameterInt("rgbimg") != 0);
        const auto &in = GetParameterFloatVectorImage("in");
        in->UpdateOutputInformation();
        int nbBands = in->GetNumberOfComponentsPerPixel();

        std::vector<int> bandIdx;
        if(HasValue("bandidx")) {
            std::vector<std::string> bandIdxStr = GetParameterStringList("bandidx");
            if(bIsRGBImage && bandIdxStr.size() != 3) {
                itkExceptionMacro("Invalid number of bandidx " << bandIdxStr.size() << " when rgbimg is set!");
            }
            if(bandIdxStr.size() > 0 && bandIdxStr.size() <= 3) {
                for(unsigned int i = 0; i<bandIdxStr.size(); i++) {
                    bandIdx.emplace_back(std::stoi(bandIdxStr[i]));
                    if(bandIdx[i] < 0 || bandIdx[i] >= nbBands) {
                        itkExceptionMacro("Band idx at position " << i << " is invalid. Number of bands is " << nbBands);
                    }
                }
            }
        } else {
            if(bIsRGBImage) {
                bandIdx.emplace_back(0);
                bandIdx.emplace_back(1);
                bandIdx.emplace_back(2);
            } else {
                bandIdx.emplace_back(0);
            }
        }

        bool bIsRangeLineRepresentation = (GetParameterInt("isrange") != 0);
        auto &&ramp = bIsRGBImage ? ReadRGBColorMap(in, mapFile) :
                                    (bIsRangeLineRepresentation ? ReadColorMap(mapFile) :
                                                                  ReadSimpleLut(mapFile));

        m_Filter = ContinuousColorMappingFilter::New();
        m_Filter->SetInput(in);

        m_Filter->GetFunctor().SetRamp(std::move(ramp));
        m_Filter->GetFunctor().SetBandIndex(bandIdx);
        m_Filter->GetFunctor().SetIsRGBImg(bIsRGBImage);

        m_Filter->UpdateOutputInformation();

        SetParameterOutputImagePixelType("out", ImagePixelType_uint8);
        SetParameterOutputImage("out", m_Filter->GetOutput());
    }

    static Ramp ReadSimpleLut(std::istream &mapFile)
    {
        Ramp ramp;

        float min;
        uint32_t rMin, gMin, bMin;
        // if we have single values, the range is from min to min+1
        // and the RGB max values are the same as the minumum values
        std::string line;
        while (std::getline(mapFile, line))
        {
            boost::trim_left(line);
            if (line[0] != '#')
            {
                std::istringstream iss(line);
                //while the iss is a number
                if (iss >> min >> rMin >> gMin >> bMin)
                {
                    itk::RGBPixel<uint8_t> minColor, maxColor;
                    minColor[0] = static_cast<uint8_t>(rMin);
                    minColor[1] = static_cast<uint8_t>(gMin);
                    minColor[2] = static_cast<uint8_t>(bMin);
                    maxColor[0] = static_cast<uint8_t>(rMin);
                    maxColor[1] = static_cast<uint8_t>(gMin);
                    maxColor[2] = static_cast<uint8_t>(bMin);

                    ramp.emplace_back(min, min+1, minColor, maxColor);
                }
            }
        }

        return ramp;
    }

    static Ramp ReadColorMap(std::istream &mapFile)
    {
        Ramp ramp;

        float min, max;
        uint32_t rMin, gMin, bMin, rMax, gMax, bMax;
        while (mapFile >> min >> max >> rMin >> gMin >> bMin >> rMax >> gMax >> bMax)
        {
            itk::RGBPixel<uint8_t> minColor, maxColor;
            minColor[0] = static_cast<uint8_t>(rMin);
            minColor[1] = static_cast<uint8_t>(gMin);
            minColor[2] = static_cast<uint8_t>(bMin);
            maxColor[0] = static_cast<uint8_t>(rMax);
            maxColor[1] = static_cast<uint8_t>(gMax);
            maxColor[2] = static_cast<uint8_t>(bMax);

            ramp.emplace_back(min, max, minColor, maxColor);
        }

        return ramp;
    }

    Ramp ReadRGBColorMap(FloatVectorImageType::Pointer in, std::istream &mapFile)
    {
        Ramp ramp;

        StreamingStatisticsVectorImageFilterType::Pointer stats = StreamingStatisticsVectorImageFilterType::New();
        stats->SetInput(in);
        stats->Update();
        // we are interested only in the maximum as the minimum will be considered always 0
        auto pixelMaxVals = stats->GetMaximum();

        float min, max;
        size_t i = 0;
        uint32_t rMin, rMax;
        while (mapFile >> min >> max >> rMin >> rMax)
        {
            itk::RGBPixel<uint8_t> minColor, maxColor;
            minColor[0] = static_cast<uint8_t>(rMin);
            maxColor[0] = static_cast<uint8_t>(rMax);
            // get the minimum between the maximum value specified in map file and
            // the maximum value found in image
            if(i < pixelMaxVals.Size()) {
                max = std::min(max, (float)pixelMaxVals[i]);
            }
            ramp.emplace_back(min, max, minColor, maxColor);
            i++;
        }
        if(ramp.size() < 3) {
            itkExceptionMacro("Invalid number of rows in map file. It should be at least 3 but we found only " << ramp.size());
        }

        return ramp;
    }


    ContinuousColorMappingFilter::Pointer m_Filter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ContinuousColorMapping)
