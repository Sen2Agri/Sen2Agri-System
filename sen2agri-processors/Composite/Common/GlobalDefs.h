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
 
#ifndef GLOBALDEFS_H
#define GLOBALDEFS_H

#define DEFAULT_QUANTIFICATION_VALUE   1000
#define NO_DATA_VALUE               -10000

#define WEIGHT_NO_DATA_VALUE        -10000

#define EPSILON         0.0001f
#define NO_DATA_EPSILON EPSILON

typedef enum {IMG_FLG_NO_DATA=0, IMG_FLG_CLOUD=1, IMG_FLG_SNOW=2, IMG_FLG_WATER=3, IMG_FLG_LAND=4, IMG_FLG_CLOUD_SHADOW=5, IMG_FLG_SATURATION=6} FlagType;

typedef enum
{
  Interpolator_NNeighbor,
  Interpolator_Linear,
  Interpolator_BCO
} Interpolator_Type;

template<typename T>
struct HasSizeMethod
{
    template<typename U, unsigned int (U::*)() const> struct SFINAE {};
    template<typename U> static char Test(SFINAE<U, &U::Size>*);
    template<typename U> static int Test(...);
    static const bool Has = sizeof(Test<T>(0)) == sizeof(char);
};

// Translates the SHORT reflectances in the input image into FLOAT, taking into account
// the quantification value
template< class TInput, class TOutput>
class ShortToFloatTranslationFunctor
{
public:
    ShortToFloatTranslationFunctor() {}
    ~ShortToFloatTranslationFunctor() {}
    void Initialize(float fQuantificationVal, float fNoDataVal = NO_DATA_VALUE) {
        m_fQuantifVal = fQuantificationVal;
        m_fNoDataVal = fNoDataVal;
  }

  bool operator!=( const ShortToFloatTranslationFunctor &a) const
  {
      return (this->m_fQuantifVal != a.m_fQuantifVal) || (this->m_fNoDataVal != a.m_fNoDataVal);
  }
  bool operator==( const ShortToFloatTranslationFunctor & other ) const
  {
    return !(*this != other);
  }
  inline TOutput operator()( const TInput & A )
  {
      return HandleMultiSizeInput(A,
             std::integral_constant<bool, HasSizeMethod<TInput>::Has>());
  }

  inline TOutput HandleMultiSizeInput(const TInput & A, std::true_type)
  {
      TOutput ret(A.Size());
      for(int i = 0; i<A.Size(); i++) {
           if(fabs(A[i] - m_fNoDataVal) < NO_DATA_EPSILON) {
               ret[i] = m_fNoDataVal;
           } else {
                ret[i] = static_cast< float >(static_cast< float >(A[i]))/m_fQuantifVal;
           }
      }

      return ret;
  }

  inline TOutput HandleMultiSizeInput(const TInput & A, std::false_type)
  {
        TOutput ret;
        if(fabs(A - m_fNoDataVal) < NO_DATA_EPSILON) {
            ret = m_fNoDataVal;
        } else {
            ret = static_cast< float >(static_cast< float >(A))/m_fQuantifVal;
        }

      return ret;
  }

private:
  int m_fQuantifVal;
  int m_fNoDataVal;
};

// Translates the FLOAT reflectances in the input image into SHORT, taking into account
// the quantification value
template< class TInput, class TOutput>
class FloatToShortTranslationFunctor
{
public:
    FloatToShortTranslationFunctor() {}
    ~FloatToShortTranslationFunctor() {}
    void Initialize(float fQuantificationVal, float fNoDataVal = 0, bool bSetNegValsToNoData = false) {
        m_fQuantifVal = fQuantificationVal;
        m_fNoDataVal = fNoDataVal;
        m_bSetNegValsToNoData = bSetNegValsToNoData;
  }

  bool operator!=( const FloatToShortTranslationFunctor &a) const
  {
      return ((this->m_fQuantifVal != a.m_fQuantifVal) || (this->m_fNoDataVal != a.m_fNoDataVal) || (this->m_bSetNegValsToNoData != a.bSetNegValsToZero));
  }
  bool operator==( const FloatToShortTranslationFunctor & other ) const
  {
    return !(*this != other);
  }
  inline TOutput operator()( const TInput & A )
  {
      return HandleMultiSizeInput(A,
             std::integral_constant<bool, HasSizeMethod<TInput>::Has>());
  }

  inline TOutput HandleMultiSizeInput(const TInput & A, std::true_type)
  {
      bool bRetComputed = false;
      TOutput ret(A.Size());
      for(int i = 0; i<A.Size(); i++) {
           // if close to NO_DATA, set the result to no_data
           if(fabs(A[i] - m_fNoDataVal) < NO_DATA_EPSILON) {
               ret[i] = m_fNoDataVal;
           } else {
                bRetComputed = false;
                if(m_bSetNegValsToNoData) {
                    // If negative but still close to 0, set the value to 0
                    if(fabs(A[i]) < EPSILON) {
                        ret[i] = 0;
                        bRetComputed = true;
                    } else if (A[i] < 0){
                        // Otherwise if negative, set the value to NO_DATA
                        ret[i] = m_fNoDataVal;
                        bRetComputed = true;
                    }
                }
                if (!bRetComputed) {
                    ret[i] = static_cast< unsigned short >(A[i] * m_fQuantifVal);
                }
           }
      }

      return ret;
  }
  inline TOutput HandleMultiSizeInput(const TInput & A, std::false_type)
  {
      TOutput ret;
      if(fabs(A - m_fNoDataVal) < NO_DATA_EPSILON) {
        ret = m_fNoDataVal;
      } else {
          if(m_bSetNegValsToNoData && (A < 0)) {
              ret = 0;
          } else {
              ret = static_cast< unsigned short >(A * m_fQuantifVal);
          }
      }

      return ret;
  }
private:
  int m_fQuantifVal;
  int m_fNoDataVal;
  bool m_bSetNegValsToNoData;
};


#endif //GLOBALDEFS_H
