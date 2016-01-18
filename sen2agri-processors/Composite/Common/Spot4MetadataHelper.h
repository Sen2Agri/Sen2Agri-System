#ifndef SPOT4METADATAHELPER_H
#define SPOT4METADATAHELPER_H

#include "MetadataHelper.h"
#include "ResamplingBandExtractor2.h"

#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"
typedef itk::SPOT4MetadataReader                                   SPOT4MetadataReaderType;
#include "itkUnaryFunctorImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkTernaryFunctorImageFilter.h"

class Spot4MetadataHelper : public MetadataHelper
{
    template< class TInput1, class TOutput>
    class MaskHandlerFunctor
    {
    public:
        MaskHandlerFunctor(){}
        void Initialize(MasksFlagType nMaskFlags, bool binarizeResult) { m_MaskFlags = nMaskFlags; m_bBinarizeResult = binarizeResult;}
        MaskHandlerFunctor& operator =(const MaskHandlerFunctor& copy) {
            m_MaskFlags=copy.m_MaskFlags;
            m_bBinarizeResult = copy.m_bBinarizeResult;
            return *this;
        }
        bool operator!=( const MaskHandlerFunctor & other) const { return true; }
        bool operator==( const MaskHandlerFunctor & other ) const { return !(*this != other); }
        TOutput operator()( const TInput1 & A) {
            return Spot4MetadataHelper::computeGlobalMaskPixelValue(A, m_MaskFlags, m_bBinarizeResult);
        }
    protected:
        MasksFlagType m_MaskFlags;
        bool m_bBinarizeResult;
    };

    template< class TInput1, class TInput2, class TOutput>
    class BinaryMaskHandlerFunctor : public MaskHandlerFunctor<TInput1, TOutput>
    {
    public:
        BinaryMaskHandlerFunctor(){}
        TOutput operator()( const TInput1 & A , const TInput2 & B) {
            return Spot4MetadataHelper::computeGlobalMaskPixelValue(A, B, this->m_MaskFlags, this->m_bBinarizeResult);
        }
    };

    template< class TInput1, class TInput2, class TInput3, class TOutput>
    class TernaryMaskHandlerFunctor : public MaskHandlerFunctor<TInput1, TOutput>
    {
    public:
        TernaryMaskHandlerFunctor(){}
        TOutput operator()( const TInput1 & A , const TInput2 & B, const TInput3 & C) {
            return  Spot4MetadataHelper::computeGlobalMaskPixelValue(A, B, C, this->m_MaskFlags, this->m_bBinarizeResult);
        }
    };

public:
    typedef MaskHandlerFunctor<MetadataHelper::SingleBandShortImageType::PixelType,
                                    MetadataHelper::SingleBandShortImageType::PixelType>        MaskHandlerFunctorType;
    typedef itk::UnaryFunctorImageFilter< MetadataHelper::SingleBandShortImageType,
                                          MetadataHelper::SingleBandShortImageType,
                                          MaskHandlerFunctorType >                              MaskHandlerFilterType;
    typedef BinaryMaskHandlerFunctor<MetadataHelper::SingleBandShortImageType::PixelType,
                                    MetadataHelper::SingleBandShortImageType::PixelType,
                                    MetadataHelper::SingleBandShortImageType::PixelType>        BinaryMaskHandlerFunctorType;
    typedef itk::BinaryFunctorImageFilter< MetadataHelper::SingleBandShortImageType,
                                          MetadataHelper::SingleBandShortImageType,
                                          MetadataHelper::SingleBandShortImageType,
                                          BinaryMaskHandlerFunctorType >                        BinaryMaskHandlerFilterType;
    typedef TernaryMaskHandlerFunctor<MetadataHelper::SingleBandShortImageType::PixelType,
                                        MetadataHelper::SingleBandShortImageType::PixelType,
                                        MetadataHelper::SingleBandShortImageType::PixelType,
                                        MetadataHelper::SingleBandShortImageType::PixelType>    TernaryMaskHandlerFunctorType;
    typedef itk::TernaryFunctorImageFilter< MetadataHelper::SingleBandShortImageType,
                                          MetadataHelper::SingleBandShortImageType,
                                          MetadataHelper::SingleBandShortImageType,
                                          MetadataHelper::SingleBandShortImageType,
                                          TernaryMaskHandlerFunctorType >                       TernaryMaskHandlerFilterType;

    Spot4MetadataHelper();

    const char * GetNameOfClass() { return "Spot4MetadataHelper"; }

    virtual std::string GetBandName(unsigned int nIdx, bool bRelativeIdx=true);
    // for Spot we have only one resolution
    virtual int GetRelativeBandIndex(unsigned int nAbsBandIdx) { return nAbsBandIdx; }

    virtual MetadataHelper::SingleBandShortImageType::Pointer GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult);

    static int computeGlobalMaskPixelValue(int val1, int val2, int val3, MasksFlagType nMaskFlags, bool binarizeResult);
    static int computeGlobalMaskPixelValue(int val1, int val2, MasksFlagType nMaskFlags, bool binarizeResult);
    static int computeGlobalMaskPixelValue(int val1, MasksFlagType nMaskFlags, bool binarizeResult);

protected:
    virtual bool DoLoadMetadata();

    std::string DeriveFileNameFromImageFileName(const std::string& replacement);

    std::string getImageFileName();
    std::string getAotFileName();
    std::string getCloudFileName();
    std::string getWaterFileName();
    std::string getSnowFileName();
    std::string getSaturationFileName();

    std::unique_ptr<SPOT4Metadata> m_metadata;

    ResamplingBandExtractor2<short> m_bandsExtractor;

    MaskHandlerFunctorType m_maskHandlerFunctor;
    MaskHandlerFilterType::Pointer m_maskHandlerFilter;
    BinaryMaskHandlerFunctorType m_binaryMaskHandlerFunctor;
    BinaryMaskHandlerFilterType::Pointer m_binaryMaskHandlerFilter;
    TernaryMaskHandlerFunctorType m_ternaryMaskHandlerFunctor;
    TernaryMaskHandlerFilterType::Pointer m_ternaryMaskHandlerFilter;

};

#endif // SPOT4METADATAHELPER_H
