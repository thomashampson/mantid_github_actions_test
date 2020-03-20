#pragma once

#include "MantidGeometry/DllConfig.h"
#include <Eigen/Core>

namespace Mantid {
namespace Geometry {

class ComponentInfo;
class DetectorInfo;
namespace ComponentInfoBankHelpers {

MANTID_GEOMETRY_DLL bool isDetectorFixedInBank(const ComponentInfo &compInfo,
                                               const size_t detIndex);
MANTID_GEOMETRY_DLL bool isSaveableBank(const ComponentInfo &compInfo,
                                        const DetectorInfo &detInfo,
                                        const size_t idx);

MANTID_GEOMETRY_DLL bool isAncestorOf(const ComponentInfo &compInfo,
                                      const size_t possibleAncestor,
                                      const size_t current);
MANTID_GEOMETRY_DLL Eigen::Vector3d
offsetFromAncestor(const Mantid::Geometry::ComponentInfo &compInfo,
                   const size_t ancestorIdx, const size_t currentIdx);
} // namespace ComponentInfoBankHelpers
} // namespace Geometry
} // namespace Mantid
