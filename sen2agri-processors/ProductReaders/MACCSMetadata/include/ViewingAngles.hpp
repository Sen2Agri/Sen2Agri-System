#pragma once

#include <vector>

#include "MACCSMetadata.hpp"

#if (defined(WIN32) || defined(_WIN32))
#  define MACCS_COMPUTE_VIEWING_ANGLES __declspec(dllexport)
#else
#  define MACCS_COMPUTE_VIEWING_ANGLES
#endif

std::vector<MACCSBandViewingAnglesGrid> MACCS_COMPUTE_VIEWING_ANGLES ComputeViewingAngles(const std::vector<CommonViewingAnglesGrid> &angleGrids);
