#include <vector>

#include <itkImage.h>
#include <itkRGBPixel.h>

#include <otbWrapperTypes.h>

template<typename T>
itk::RGBPixel<float> Lerp(float t, const itk::RGBPixel<T> &p1, const itk::RGBPixel<T> &p2)
{
    itk::RGBPixel<T> result;
    result[0] = p1[0] * (1.0f - t) + p2[0] * t;
    result[1] = p1[1] * (1.0f - t) + p2[1] * t;
    result[2] = p1[2] * (1.0f - t) + p2[2] * t;
    return result;
}

template<typename T>
itk::RGBPixel<T> Round(const itk::RGBPixel<float> &p)
{
    itk::RGBPixel<T> result;
    result[0] = p[0] + 0.5f;
    result[1] = p[1] + 0.5f;
    result[2] = p[2] + 0.5f;
    return result;
}

struct RampEntry
{
    float min;
    float max;
    itk::RGBPixel<uint8_t> minColor;
    itk::RGBPixel<uint8_t> maxColor;

    RampEntry(float min, float max, itk::RGBPixel<uint8_t> minColor, itk::RGBPixel<uint8_t> maxColor)
        : min(min), max(max), minColor(minColor), maxColor(maxColor)
    {
    }
};

typedef std::vector<RampEntry> Ramp;

class ContinuousColorMappingFunctor
{
public:
    typedef otb::Wrapper::FloatImageType InputImageType;
    typedef InputImageType::PixelType InputPixelType;

    typedef itk::RGBPixel<uint8_t> OutputPixelType;
    typedef itk::Image<OutputPixelType> OutputImageType;

    ContinuousColorMappingFunctor()
    {
    }

    void SetRamp(std::vector<RampEntry> ramp)
    {
        m_Ramp = std::move(ramp);
    }

    const Ramp & GetNoDataValue() const
    {
        return m_Ramp;
    }

    OutputPixelType operator()(const InputPixelType &in) const
    {
        OutputPixelType result;
        result.Fill(0);

        for (const auto &entry : m_Ramp)
        {
            if (in >= entry.min && in < entry.max)
            {
                float t = (in - entry.min) / (entry.max - entry.min);
                result = Round<uint8_t>(Lerp(t, entry.minColor, entry.maxColor));
                break;
            }
        }

        return result;
    }

private:
    Ramp m_Ramp;
};
