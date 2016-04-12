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
 
template <typename TInput, typename TOutput>
class CreateMaskFromValueFunctor
{
public:
    typedef typename TInput::ValueType InputValueType;

    CreateMaskFromValueFunctor() : m_NoDataValue()
    {
    }

    void SetNoDataValue(InputValueType value)
    {
        m_NoDataValue = value;
    }

    InputValueType GetNoDataValue() const
    {
        return m_NoDataValue;
    }

    TOutput operator()(const TInput &in) const
    {
        auto size = in.GetSize();
        for (decltype(size) i = 0; i < size; i++) {
            if (in[i] != m_NoDataValue) {
                return 1;
            }
        }

        return 0;
    }

private:
    InputValueType m_NoDataValue;
};
