#ifndef SPOT4METADATAHELPER_H
#define SPOT4METADATAHELPER_H

#include "MetadataHelper.h"
#include "ResamplingBandExtractor2.h"

#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"
#include "itkNaryFunctorImageFilter.h"

typedef itk::SPOT4MetadataReader                                   SPOT4MetadataReaderType;

class Spot4MetadataHelper : public MetadataHelper
{
    template< class TInput, class TOutput>
    class NaryMaskHandlerFunctor
    {
    public:
        NaryMaskHandlerFunctor(){}
        void Initialize(MasksFlagType nMaskFlags, bool binarizeResult) { m_MaskFlags = nMaskFlags; m_bBinarizeResult = binarizeResult;}
        NaryMaskHandlerFunctor& operator =(const NaryMaskHandlerFunctor& copy) {
            m_MaskFlags=copy.m_MaskFlags;
            m_bBinarizeResult = copy.m_bBinarizeResult;
            return *this;
        }
        bool operator!=( const NaryMaskHandlerFunctor & a) const { return   (this->m_MaskFlags != a.m_MaskFlags) ||
                                                                            (this->m_bBinarizeResult != a.m_bBinarizeResult) ;}
        bool operator==( const NaryMaskHandlerFunctor & a ) const { return !(*this != a); }

        TOutput operator()( const std::vector< TInput > & B) {
            switch (B.size())
            {
            case 1:
                if((m_MaskFlags & MSK_CLOUD) != 0) {
                    if(B[0] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_CLOUD;
                } else if((m_MaskFlags & MSK_SAT) != 0) {
                    if(B[0] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_SATURATION;
                } else {
                    // in this case we have div file (snow, water etc)
                    if((B[0] & 0x01) != 0) return m_bBinarizeResult ? 1 : IMG_FLG_NO_DATA;
                    if(((m_MaskFlags & MSK_WATER) != 0) && ((B[0] & 0x02) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_WATER;
                    if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[0] & 0x04) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_SNOW;
                }
                break;
            case 2:
                // we have values only for 2 of them and they are the first two
                // we determine them based on the flag type
                if((m_MaskFlags & MSK_CLOUD) != 0) {
                    if(B[0] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_CLOUD;
                    if((m_MaskFlags & MSK_SAT) != 0) {
                        // in this case in the second value val2 we have saturation
                        if(B[1] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_SATURATION;
                    } else {
                        // in this case we have div file (snow, water etc)
                        if((B[1] & 0x01) != 0) return m_bBinarizeResult ? 1 : IMG_FLG_NO_DATA;
                        if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x02) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_WATER;
                        if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[1] & 0x04) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_SNOW;
                    }
                } else {
                    // it means that we have in val1 and val2 the value from div file and from saturation
                    if((B[0] & 0x01) != 0) return m_bBinarizeResult ? 1 : IMG_FLG_NO_DATA;
                    if(((m_MaskFlags & MSK_WATER) != 0) && ((B[0] & 0x02) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_WATER;
                    if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[0] & 0x04) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_SNOW;

                    if(B[1] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_SATURATION;
                }
                break;
            case 3:
                if((B[1] & 0x01) != 0) return m_bBinarizeResult ? 1 : IMG_FLG_NO_DATA;
                if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x02) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_WATER;
                if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[1] & 0x04) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_SNOW;

                // if we have cloud,
                if(B[0] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_CLOUD;
                if(B[2] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_SATURATION;
            }
            return m_bBinarizeResult ? 0 : IMG_FLG_LAND;
        }
    private:
        MasksFlagType m_MaskFlags;
        bool m_bBinarizeResult;
    };


public:
    typedef NaryMaskHandlerFunctor<MetadataHelper::SingleBandShortImageType::PixelType,
                                        MetadataHelper::SingleBandShortImageType::PixelType>    NaryMaskHandlerFunctorType;
    typedef itk::NaryFunctorImageFilter< MetadataHelper::SingleBandShortImageType,
                                        MetadataHelper::SingleBandShortImageType,
                                        NaryMaskHandlerFunctorType>                             NaryFunctorImageFilterType;

    Spot4MetadataHelper();

    const char * GetNameOfClass() { return "Spot4MetadataHelper"; }

    virtual std::string GetBandName(unsigned int nIdx, bool bRelativeIdx=true);
    // for Spot we have only one resolution
    virtual int GetRelativeBandIndex(unsigned int nAbsBandIdx) { return nAbsBandIdx; }

    virtual MetadataHelper::SingleBandShortImageType::Pointer GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult);

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

    NaryMaskHandlerFunctorType m_maskHandlerFunctor;
    NaryFunctorImageFilterType::Pointer m_maskHandlerFilter;

};

#endif // SPOT4METADATAHELPER_H
