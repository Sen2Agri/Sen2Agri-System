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
 
#ifndef MASKHANDLER_H
#define MASKHANDLER_H


#define MASK_CLOUD      0x01
#define CLOUD_BIT_POS   0
#define MASK_WATER      0x02
#define WATER_BIT_POS   1
#define MASK_SNOW       0x04
#define SNOW_BIT_POS    2

template< class TInput1, class TInput2, class TOutput>
class MaskHandlerFunctor
{
public:
    MaskHandlerFunctor()
    {
        m_CloudMask = -1;
        m_WaterMask = -1;
        m_SnowMask = -1;
    }

    MaskHandlerFunctor& operator =(const MaskHandlerFunctor& copy)
    {
        return *this;
    }

    bool operator!=( const MaskHandlerFunctor & other) const
    {
        return true;
    }
    bool operator==( const MaskHandlerFunctor & other ) const
    {
        return false;
    }

    TOutput operator()( const TInput1 & A , const TInput2 & B)
    {
        TOutput var(3);
        var.Fill(-10000);
        if(m_CloudMask > -1 && m_CloudMask < 8)
            var[0] = ((A[0]) & m_MasksArray[m_CloudMask]) >> m_CloudMask;
        if(m_WaterMask > -1 && m_WaterMask < 8)
            var[1] = ((B[0]) & m_MasksArray[m_WaterMask]) >> m_WaterMask;
        if(m_SnowMask > -1 && m_SnowMask < 8)
            var[2] = ((B[0]) & m_MasksArray[m_SnowMask]) >> m_SnowMask;
        return var;
    }   
    void SetBitsMask(int cloudMask, int waterMask, int snowMask)
    {
        m_CloudMask = cloudMask;
        m_WaterMask = waterMask;
        m_SnowMask = snowMask;

    }

private:
    int m_MasksArray[8] = {0x01, 0x02, 0x04, 0x08, 0x16, 0x32, 0x64, 0x128};
    int m_CloudMask, m_WaterMask, m_SnowMask;
};

#endif // SPOTMASKHANDLER_H
