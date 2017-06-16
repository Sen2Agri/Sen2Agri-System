#pragma once

#include "otbVectorImage.h"
#include "otbWrapperTypes.h"

typedef float PixelValueType;
typedef otb::VectorImage<PixelValueType, 2> ImageType;

#define MAXNDVISLOPE 0
#define MINNDVISLOPE 1
#define MAXNDVI 2
#define MINNDVI 3
#define MAXRED 4
#define INDEXSIZE 5

template <typename PixelType>
class FeaturesNoInsituFunctor
{
public:
    FeaturesNoInsituFunctor()
        : m_IncludeRedEdge()
    {
    }

    void SetIncludeRedEdge(bool includeRedEdge)
    {
        this->m_IncludeRedEdge = includeRedEdge;
    }

    bool GetIncludeRedEdge() const
    {
        return this->m_IncludeRedEdge;
    }

    PixelType operator()(const PixelType &ts, const PixelType &rets) const
    {
        // compute the number of input image
        int numImages = m_id.size();
        auto bands = ts.Size() / numImages;
        auto outputBands = m_IncludeRedEdge ? 8 : 4;

        PixelType ndvi(numImages);

        auto ok = false;
        for (int imgIndex = 0; imgIndex < numImages; imgIndex++) {
            int b1 = ts[bands * imgIndex];
            int b2 = ts[bands * imgIndex + 1];
            int b3 = ts[bands * imgIndex + 2];
            int b4 = ts[bands * imgIndex + 3];

            if (b1 != -10000 && b2 != -10000 && b3 != -10000 && b4 != -10000) {
                ok = true;

                ndvi[imgIndex] = (std::abs(b3+b2)<0.000001) ? 0 : static_cast<PixelValueType>(b3-b2)/(b3+b2);
            } else {
                ndvi[imgIndex] = -10000;
            }
        }

        // Create the output pixel
        PixelType result(INDEXSIZE * outputBands);

        if (!ok) {
            result.Fill(static_cast<PixelValueType>(-10000));
            return result;
        }

        // Compute the maximum and minimum NDVI slopes
        // Compute the maximum and minimum NDVI values
        // Compute the maximum Red value
        int index[INDEXSIZE];
        index[MAXNDVISLOPE] = -1;
        index[MINNDVISLOPE] = -1;
        index[MAXNDVI] = 0;
        index[MINNDVI] = 0;
        index[MAXRED] = 0;
        double slope = 0.0;
        double minSlope = 0.0;
        double maxSlope = 0.0;
        double minNDVI = ndvi[0];
        double maxNDVI = ndvi[0];
        double maxRed = ts[1];

        // Get the indices for the required values
        for (int i = 0; i < numImages; i++) {
            if (i > 0 && i < numImages - 1) {
                // compute the slope
                int im1 = i - 1;
                int ip1 = i + 1;
                slope = static_cast<double>(ndvi[ip1] - ndvi[im1]) / (m_id[ip1] - m_id[im1]);
                if (i == 1 || maxSlope < slope) {
                    maxSlope = slope;
                    index[MAXNDVISLOPE] = i;
                }
                if (i == 1 || minSlope > slope) {
                    minSlope = slope;
                    index[MINNDVISLOPE] = i;
                }
            }
            if (maxNDVI < ndvi[i]) {
                maxNDVI = ndvi[i];
                index[MAXNDVI] = i;
            }
            if (minNDVI > ndvi[i]) {
                minNDVI = ndvi[i];
                index[MINNDVI] = i;
            }
            if (maxRed < ts[i * bands + 1]) {
                maxRed = ts[i * bands + 1];
                index[MAXRED] = i;
            }
        }

        for (int i = 0; i < INDEXSIZE; i++) {
            for (int j = 0; j < 4; j++) {
                result[i * outputBands + j] = (index[i] == -1 ? -10000 : ts[index[i] * 4 + j]);
            }
            if (m_IncludeRedEdge) {
                for (int j = 0; j < 4; j++) {
                    result[i * outputBands + 4 + j] = (index[i] == -1 ? -10000 : rets[index[i] * 4 + j]);
                }
            }
        }

        return result;
    }

    bool operator==(const FeaturesNoInsituFunctor &other) const
    {
        return this->m_IncludeRedEdge == other.m_IncludeRedEdge && this->m_id == other.m_id;
    }

    bool operator!=(const FeaturesNoInsituFunctor a) const
    {
        return !(*this == a);
    }

    void SetInputDates(std::vector<int> inputDates)
    {
        this->m_id = inputDates;
    }

protected:
    // the days from epoch corresponding to the input series raster
    std::vector<int> m_id;
    bool m_IncludeRedEdge;
};

// Output bands:
// Without red edge bands:
// max NDVI slope B1..B4
// min NDVI slope B1..B4
// max NDVI B1..B4
// min NDVI B1..B4
// max red B1..B4
// min red B1..B4
// Otherwise:
// max NDVI slope B1..B8
// and so on
#define MAXNDVISLOPE 0
#define MINNDVISLOPE 1
#define MAXNDVI 2
#define MINNDVI 3
#define MAXRED 4

class ITK_EXPORT CropMaskSpectralFeaturesFilter
    : public itk::BinaryFunctorImageFilter<ImageType, ImageType, ImageType, FeaturesNoInsituFunctor<ImageType::PixelType>>
{
public:
    typedef CropMaskSpectralFeaturesFilter Self;
    typedef itk::BinaryFunctorImageFilter<ImageType, ImageType, ImageType, FeaturesNoInsituFunctor<ImageType::PixelType>> Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Macro defining the type*/
    itkTypeMacro(CropMaskSpectralFeaturesFilter, SuperClass)

    void SetIncludeRedEdge(bool includeRedEdge)
    {
        this->GetFunctor().SetIncludeRedEdge(includeRedEdge);
        this->Modified();
    }

    bool GetIncludeRedEdge() const
    {
        return this->GetFunctor().GetIncludeRedEdge();
    }

protected:
    CropMaskSpectralFeaturesFilter() { }

    void GenerateOutputInformation()
    {
        Superclass::GenerateOutputInformation();

        if (GetIncludeRedEdge()) {
            this->GetOutput()->SetNumberOfComponentsPerPixel(INDEXSIZE * 8);
        } else {
            this->GetOutput()->SetNumberOfComponentsPerPixel(INDEXSIZE * 4);
        }
    }

private:
    CropMaskSpectralFeaturesFilter(const Self &); // purposely not implemented
    void operator=(const Self &);                 // purposely not implemented
};
