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
 
#define BOOST_TEST_MODULE TestTemporalResampling
#include <boost/test/unit_test.hpp>

#include <itkVariableLengthVector.h>

#include <TemporalResampling.hxx>

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

BOOST_AUTO_TEST_CASE(TestBandsSimple)
{
    std::vector<SensorData> sd = { { "Sensor 1", { 0, 5, 10, 15, 20 }, { 0, 5, 10, 15, 20 } } };

    auto image = makeItkVector({ 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 });
    auto mask = makeItkVector({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
    GapFillingFunctor<itk::VariableLengthVector<float>, itk::VariableLengthVector<uint8_t>> functor(sd, 0, 2);

    auto result = functor(image, mask);
    BOOST_REQUIRE_EQUAL(result.Size(), 10);
    BOOST_CHECK_EQUAL(result[0], 2);
    BOOST_CHECK_EQUAL(result[1], 3);
    BOOST_CHECK_EQUAL(result[2], 4);
    BOOST_CHECK_EQUAL(result[3], 5);
    BOOST_CHECK_EQUAL(result[4], 6);
    BOOST_CHECK_EQUAL(result[5], 7);
    BOOST_CHECK_EQUAL(result[6], 8);
    BOOST_CHECK_EQUAL(result[7], 9);
    BOOST_CHECK_EQUAL(result[8], 10);
    BOOST_CHECK_EQUAL(result[9], 11);
}

BOOST_AUTO_TEST_CASE(TestInterpolationSymmetric)
{
    std::vector<SensorData> sd = { { "Sensor 1", { 0, 5, 10, 15, 20 }, { 0, 5, 10, 15, 20 } } };

    auto image = makeItkVector({ 2, 5, 6, 9, 10 });
    auto mask = makeItkVector({ 0, 0, 1, 0, 0 });
    GapFillingFunctor<itk::VariableLengthVector<float>, itk::VariableLengthVector<uint8_t>> functor(sd, 0, 1);

    auto result = functor(image, mask);
    BOOST_REQUIRE_EQUAL(result.Size(), 5);
    BOOST_CHECK_EQUAL(result[0], 2);
    BOOST_CHECK_EQUAL(result[1], 5);
    BOOST_CHECK_EQUAL(result[2], 7);
    BOOST_CHECK_EQUAL(result[3], 9);
    BOOST_CHECK_EQUAL(result[4], 10);
}

BOOST_AUTO_TEST_CASE(TestInterpolationAsymmetric)
{
    std::vector<SensorData> sd = { { "Sensor 1", { 0, 5, 10, 15, 20 }, { 0, 5, 11, 15, 20 } } };

    auto image = makeItkVector({ 2, 5, 6, 9, 10 });
    auto mask = makeItkVector({ 0, 0, 1, 0, 0 });
    GapFillingFunctor<itk::VariableLengthVector<float>, itk::VariableLengthVector<uint8_t>> functor(sd, 0, 1);

    auto result = functor(image, mask);
    BOOST_REQUIRE_EQUAL(result.Size(), 5);
    BOOST_CHECK_EQUAL(result[0], 2);
    BOOST_CHECK_EQUAL(result[1], 5);
    BOOST_CHECK_CLOSE(result[2], image[1] * 0.4 + image[3] * 0.6, 1);
    BOOST_CHECK_EQUAL(result[3], 9);
    BOOST_CHECK_EQUAL(result[4], 10);
}

BOOST_AUTO_TEST_CASE(TestInterpolationEdges)
{
    std::vector<SensorData> sd = { { "Sensor 1", { 0, 5, 10, 15, 20 }, { 0, 5, 10, 15, 20 } } };

    auto image = makeItkVector({ 2, 5, 6, 9, 10 });
    auto mask = makeItkVector({ 1, 0, 0, 0, 1 });
    GapFillingFunctor<itk::VariableLengthVector<float>, itk::VariableLengthVector<uint8_t>> functor(sd, 0, 1);

    auto result = functor(image, mask);
    BOOST_REQUIRE_EQUAL(result.Size(), 5);
    BOOST_CHECK_EQUAL(result[0], 5);
    BOOST_CHECK_EQUAL(result[1], 5);
    BOOST_CHECK_EQUAL(result[2], 6);
    BOOST_CHECK_EQUAL(result[3], 9);
    BOOST_CHECK_EQUAL(result[4], 9);
}

BOOST_AUTO_TEST_CASE(TestInterpolationNoData)
{
    std::vector<SensorData> sd = { { "Sensor 1", { 0, 5, 10, 15, 20 }, { 0, 5, 10, 15, 20 } } };

    auto image = makeItkVector({ 2, 5, 6, 9, 10 });
    auto mask = makeItkVector({ 1, 1, 1, 1, 1 });
    GapFillingFunctor<itk::VariableLengthVector<float>, itk::VariableLengthVector<uint8_t>> functor(sd, 0, 1);

    auto result = functor(image, mask);
    BOOST_REQUIRE_EQUAL(result.Size(), 5);
    BOOST_CHECK_EQUAL(result[0], NOVALUEPIXEL);
    BOOST_CHECK_EQUAL(result[1], NOVALUEPIXEL);
    BOOST_CHECK_EQUAL(result[2], NOVALUEPIXEL);
    BOOST_CHECK_EQUAL(result[3], NOVALUEPIXEL);
    BOOST_CHECK_EQUAL(result[4], NOVALUEPIXEL);
}

BOOST_AUTO_TEST_CASE(TestMultipleSensors)
{
    std::vector<SensorData> sd = { { "Sensor 1", { 0, 5, 10, 15, 20 }, { 0, 5, 11, 15, 20 } },
                                   { "Sensor 2", { 0, 5, 10, 15, 20 }, { 0, 5, 10, 15, 20 } } };

    auto image = makeItkVector({ 10, 11, 12, 13, 14, 20, 21, 22, 23, 24 });
    auto mask = makeItkVector({ 0, 0, 1, 0, 0, 1, 0, 0, 0, 1 });
    GapFillingFunctor<itk::VariableLengthVector<float>, itk::VariableLengthVector<uint8_t>> functor(sd, 0, 1);

    auto result = functor(image, mask);
    BOOST_REQUIRE_EQUAL(result.Size(), 10);
    BOOST_CHECK_EQUAL(result[0], 10);
    BOOST_CHECK_EQUAL(result[1], 11);
    BOOST_CHECK_CLOSE(result[2], image[1] * 0.4 + image[3] * 0.6, 1);
    BOOST_CHECK_EQUAL(result[3], 13);
    BOOST_CHECK_EQUAL(result[4], 14);
    BOOST_CHECK_EQUAL(result[5], 21);
    BOOST_CHECK_EQUAL(result[6], 21);
    BOOST_CHECK_EQUAL(result[7], 22);
    BOOST_CHECK_EQUAL(result[8], 23);
    BOOST_CHECK_EQUAL(result[9], 23);
}

BOOST_AUTO_TEST_CASE(TestInterleaving)
{
    std::vector<SensorData> sd = { { "Sensor 1", { 0, 10, 20 }, { 5, 15 } },
                                   { "Sensor 2", { 5, 15 }, { 0, 10, 20 } } };

    auto image = makeItkVector({ 20, 30, 40, 50, 60 });
    auto mask = makeItkVector({ 0, 0, 0, 0, 0 });
    GapFillingFunctor<itk::VariableLengthVector<float>, itk::VariableLengthVector<uint8_t>> functor(sd, 0, 1);

    auto result = functor(image, mask);
    BOOST_REQUIRE_EQUAL(result.Size(), 5);
    BOOST_CHECK_EQUAL(result[0], 25);
    BOOST_CHECK_EQUAL(result[1], 35);
    BOOST_CHECK_EQUAL(result[2], 50);
    BOOST_CHECK_EQUAL(result[3], 55);
    BOOST_CHECK_EQUAL(result[4], 60);
}
