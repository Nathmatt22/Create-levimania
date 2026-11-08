// SPDX-License-Identifier: MIT
//
// CreateLeviLamina - Core/Rotation/Types.cpp
#include "Types.h"

namespace cl::kinetic {

BlockPos BlockPos::relative(Axis axis, int sign) const {
    switch (axis) {
    case Axis::X:
        return {x + sign, y, z};
    case Axis::Y:
        return {x, y + sign, z};
    case Axis::Z:
    default:
        return {x, y, z + sign};
    }
}

} // namespace cl::kinetic
