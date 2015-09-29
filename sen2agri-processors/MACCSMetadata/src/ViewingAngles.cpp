#include <stdexcept>
#include <map>

#include "ViewingAngles.hpp"

static void checkDimensions(size_t expectedHeight,
                            size_t expectedWidth,
                            const std::vector<std::vector<double>> &grid)
{
    if (grid.size() != expectedHeight) {
        throw std::runtime_error("The angle grids must have the same height");
    }

    for (const auto &row : grid) {
        if (row.size() != expectedWidth) {
            throw std::runtime_error("The angle grids must have the same width");
        }
    }
}

static std::vector<std::vector<double>> makeGrid(size_t height, size_t width)
{
    std::vector<std::vector<double>> r(height);

    for (auto &row : r) {
        row.resize(width);
    }

    return r;
}

std::vector<MACCSBandViewingAnglesGrid>
ComputeViewingAngles(const std::vector<MACCSViewingAnglesGrid> &angleGrids)
{
    if (angleGrids.empty() || angleGrids.front().Angles.Zenith.Values.empty()) {
        return {};
    }

    const auto &firstGrid = angleGrids.front().Angles.Zenith.Values;
    size_t width = firstGrid.front().size(), height = firstGrid.size();
    std::map<std::string, MACCSBandViewingAnglesGrid> resultGrids;
    auto endResultGrids = std::end(resultGrids);
    for (const auto &grid : angleGrids) {
        checkDimensions(height, width, grid.Angles.Zenith.Values);
        checkDimensions(height, width, grid.Angles.Azimuth.Values);

        auto it = resultGrids.find(grid.BandId);
        if (it == endResultGrids) {
            endResultGrids.insert(grid.BandId, makeGrid(height, width));
        }
    }

    for (auto &resultGrid : resultGrids) {
        for (size_t j = 0; j < height; j++) {
            for (size_t i = 0; i < width; i++) {
                auto zenith = std::numeric_limits<double>::quiet_nan();
                auto azimuth = std::numeric_limits<double>::quiet_nan();

                for (const auto &grid : angleGrids) {
                    if (std::isnan(zenith)) {
                        zenith = grid.Angles.Zenith.Values[j][i];
                    }
                    if (std::isnan(azimuth)) {
                        azimuth = grid.Angles.Azimuth.Values[j][i];
                    }
                }

                resultGrid.Angles.Zenith.Values[j][i] = zenith;
                resultGrid.Angles.Azimuth.Values[j][i] = azimuth;
            }
        }
    }

    std::vector<MACCSBandViewingAnglesGrid> result;
    for (const auto &grid : resultGrids) {
        result.emplace_back(grid.first, grid.second);
    }
    return result;
}
