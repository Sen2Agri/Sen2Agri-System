#pragma once

#include <vector>

#include "MACCSMetadata.hpp"

std::vector<MACCSBandViewingAnglesGrid> ComputeViewingAngles(const std::vector<CommonViewingAnglesGrid> &angleGrids);
