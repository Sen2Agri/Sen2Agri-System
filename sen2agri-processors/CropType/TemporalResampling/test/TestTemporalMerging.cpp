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
 
#define BOOST_TEST_MODULE TestTemporalMerging
#include <boost/test/unit_test.hpp>

#include <itkVariableLengthVector.h>

#include <TemporalMerging.hxx>

template <typename T>
itk::VariableLengthVector<T> makeItkVector(std::initializer_list<T> data)
{
    itk::VariableLengthVector<T> result(data.size());
    size_t pos = 0;
    for (auto t : data) {
        result[pos++] = t;
    }
    return result;
}

BOOST_AUTO_TEST_CASE(TestSingleBand)
{
    std::vector<ImageInfo> ii = {
        { 0, 0, 0 }, { 1, 1, 0 }, { 2, 2, 0 }, { 3, 3, 0 }, { 4, 4, 0 }, { 5, 5, 0 }
    };
    auto image = makeItkVector({ 1, 2, 3, 4, 5, 6 });
    auto mask = makeItkVector({ 0, 0, 0, 0, 0, 0 });

    std::sort(ii.begin(), ii.end(), [](const ImageInfo &i1, const ImageInfo &i2) {
        return std::make_tuple(i1.day, -i1.priority) < std::make_tuple(i2.day, -i2.priority);
    });

    TemporalMergingFunctor<itk::VariableLengthVector<float> > functor(ii, 6, 1);
    auto result = functor(image, mask);
    BOOST_REQUIRE_EQUAL(result.Size(), 6);
    BOOST_CHECK_EQUAL(result[0], 1);
    BOOST_CHECK_EQUAL(result[1], 2);
    BOOST_CHECK_EQUAL(result[2], 3);
    BOOST_CHECK_EQUAL(result[3], 4);
    BOOST_CHECK_EQUAL(result[4], 5);
    BOOST_CHECK_EQUAL(result[5], 6);
}

BOOST_AUTO_TEST_CASE(TestReorder)
{
    std::vector<ImageInfo> ii = {
        { 0, 0, 0 }, { 1, 2, 0 }, { 2, 4, 0 }, { 3, 1, 0 }, { 4, 3, 0 }, { 5, 5, 0 }
    };
    auto image = makeItkVector({ 1, 2, 3, 4, 5, 6 });
    auto mask = makeItkVector({ 0, 0, 0, 0, 0, 0 });

    std::sort(ii.begin(), ii.end(), [](const ImageInfo &i1, const ImageInfo &i2) {
        return std::make_tuple(i1.day, -i1.priority) < std::make_tuple(i2.day, -i2.priority);
    });

    TemporalMergingFunctor<itk::VariableLengthVector<float> > functor(ii, 6, 1);
    auto result = functor(image, mask);
    BOOST_REQUIRE_EQUAL(result.Size(), 6);
    BOOST_CHECK_EQUAL(result[0], 1);
    BOOST_CHECK_EQUAL(result[1], 4);
    BOOST_CHECK_EQUAL(result[2], 2);
    BOOST_CHECK_EQUAL(result[3], 5);
    BOOST_CHECK_EQUAL(result[4], 3);
    BOOST_CHECK_EQUAL(result[5], 6);
}
