#ifndef SPOTMASKHANDLER_H
#define SPOTMASKHANDLER_H


#define MASK_CLOUD      0x01
#define CLOUD_BIT_POS   0
#define MASK_WATER      0x02
#define WATER_BIT_POS   1
#define MASK_SNOW       0x04
#define SNOW_BIT_POS    2

template< class TInput1, class TInput2, class TOutput>
class SpotMaskHandlerFunctor
{
public:
    SpotMaskHandlerFunctor()
    {

    }

    SpotMaskHandlerFunctor& operator =(const SpotMaskHandlerFunctor& copy)
    {
        return *this;
    }

    bool operator!=( const SpotMaskHandlerFunctor & other) const
    {
        return (*this != other);
    }
    bool operator==( const SpotMaskHandlerFunctor & other ) const
    {
        return !(*this != other);
    }

    TOutput operator()( const TInput1 & A , const TInput2 & B)
    {
        TOutput var(3);

        var[0] = ((A[0]) & MASK_CLOUD) >> CLOUD_BIT_POS;
        var[1] = ((B[0]) & MASK_WATER) >> WATER_BIT_POS;
        var[2] = ((B[0]) & MASK_SNOW) >> SNOW_BIT_POS;
        return var;
    }   
};

#endif // SPOTMASKHANDLER_H
