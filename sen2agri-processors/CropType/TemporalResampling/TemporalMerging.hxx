#pragma once

#include "itkUnaryFunctorImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"

template <typename PixelType>
class TemporalMergingFunctor
{
public:
    PixelType operator()(const PixelType &pix) const
    {
        PixelType result(numOutputImages * bands);

        for (int i = 0; i < numOutputImages; i++) {
            int idx = outputIndices[i];
            for (int j = 0; j < bands; j++) {
                result[i * bands + j] = pix[idx * bands + j];
            }
        }
        return result;
    }

    void SetNumberOfBands(int bands)
    {
        this->bands = bands;
    }

    void SetOutputIndices(std::vector<int> outputIndices)
    {
        this->outputIndices = std::move(outputIndices);

        numOutputImages = this->outputIndices.size();
    }

private:
    int bands;
    int numOutputImages;
    std::vector<int> outputIndices;
};

template <typename TImage>
using TemporalMergingFilter = itk::UnaryFunctorImageFilter < TImage,
      TImage, TemporalMergingFunctor<typename TImage::PixelType>;

struct ImageInfo
{
    int index;
    int day;
    int priority;

    ImageInfo(int index, int day, int priority) noexcept : index(index),
                                                           day(day),
                                                           priority(priority)
    {
    }
};
