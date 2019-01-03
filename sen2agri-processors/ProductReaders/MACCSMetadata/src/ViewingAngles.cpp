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
 
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>

#include "ViewingAngles.hpp"

static void checkDimensions(std::string expectedColumnUnit,
                            std::string expectedColumnStep,
                            std::string expectedRowUnit,
                            std::string expectedRowStep,
                            size_t expectedHeight,
                            size_t expectedWidth,
                            const CommonAngleList &grid)
{
    if (grid.ColumnUnit != expectedColumnUnit) {
        throw std::runtime_error("The angle grids must have the same column unit");
    }
    if (grid.ColumnStep != expectedColumnStep) {
        throw std::runtime_error("The angle grids must have the same column step");
    }
    if (grid.RowUnit != expectedRowUnit) {
        throw std::runtime_error("The angle grids must have the same row unit");
    }
    if (grid.RowStep != expectedRowStep) {
        throw std::runtime_error("The angle grids must have the same row step");
    }

    if (grid.Values.size() != expectedHeight) {
        throw std::runtime_error("The angle grids must have the same height");
    }

    for (const auto &row : grid.Values) {
        if (row.size() != expectedWidth) {
            throw std::runtime_error("The angle grids must have the same width");
        }
    }
}

static std::vector<std::vector<double> > makeGrid(size_t height, size_t width)
{
    std::vector<std::vector<double> > r(height);

    for (auto &row : r) {
        row.resize(width);
    }

    return r;
}

std::vector<MACCSBandViewingAnglesGrid>
ComputeViewingAngles(const std::vector<CommonViewingAnglesGrid> &angleGrids)
{
    if (angleGrids.empty() || angleGrids.front().Angles.Zenith.Values.empty()) {
        return {};
    }

    const auto &firstGrid = angleGrids.front().Angles.Zenith;
    auto columnUnit = firstGrid.ColumnUnit;
    auto columnStep = firstGrid.ColumnStep;
    auto rowUnit = firstGrid.RowUnit;
    auto rowStep = firstGrid.RowStep;
    auto width = firstGrid.Values.front().size();
    auto height = firstGrid.Values.size();
    std::map<std::string, int> bandPos;
    std::map<std::string, MACCSBandViewingAnglesGrid> resultGrids;
    auto endBandPos = std::end(bandPos);
    auto endResultGrids = std::end(resultGrids);
    for (const auto &grid : angleGrids) {
        checkDimensions(columnUnit, columnStep, rowUnit, rowStep, height, width, grid.Angles.Zenith);
        checkDimensions(columnUnit, columnStep, rowUnit, rowStep, height, width, grid.Angles.Azimuth);

        auto itBandPos = bandPos.find(grid.BandId);
        if (itBandPos == endBandPos) {
            bandPos.emplace(grid.BandId, bandPos.size());
        }

        auto it = resultGrids.find(grid.BandId);
        if (it == endResultGrids) {
            resultGrids.emplace(
                grid.BandId,
                MACCSBandViewingAnglesGrid{ grid.BandId,
                                            { { columnUnit, columnStep, rowUnit, rowStep, makeGrid(height, width) },
                                              { columnUnit, columnStep, rowUnit, rowStep, makeGrid(height, width) } } });
        }
    }

    for (auto &resultGrid : resultGrids) {
        for (size_t j = 0; j < height; j++) {
            for (size_t i = 0; i < width; i++) {
                auto zenith = std::numeric_limits<double>::quiet_NaN();
                auto azimuth = std::numeric_limits<double>::quiet_NaN();

                for (const auto &grid : angleGrids) {
                    if (grid.BandId == resultGrid.first) {
                        auto z = grid.Angles.Zenith.Values[j][i];
                        auto a = grid.Angles.Azimuth.Values[j][i];

                        if (!std::isnan(z)) {
                            zenith = z;
                        }
                        if (!std::isnan(a)) {
                            azimuth = a;
                        }
                    }
                }

                resultGrid.second.Angles.Zenith.Values[j][i] = zenith;
                resultGrid.second.Angles.Azimuth.Values[j][i] = azimuth;
            }
        }
    }

    std::vector<MACCSBandViewingAnglesGrid> result(resultGrids.size());
    for (const auto &grid : resultGrids) {
        result[bandPos[grid.first]] = grid.second;
    }
    return result;
}
