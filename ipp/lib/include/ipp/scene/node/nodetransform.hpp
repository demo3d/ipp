#pragma once

#include <ipp/shared.hpp>

namespace ipp {
namespace scene {
namespace node {

/**
 * @brief Transform struct that keeps translation/rotation/scale as separate members.
 *
 * Transform properties are kept separate because animations operate on individual properties
 * independantly.
 */
struct NodeTransform {
    /**
     * @brief Enumerates Rotation modes that define how rotation is interpreted.
     */
    enum class RotationMode {
        Quaternion = 0,
        AxisAngle,
        EulerXYZ,
        EulerXZY,
        EulerYXZ,
        EulerYZX,
        EulerZXY,
        EulerZYX
    };

    glm::vec3 translation;
    glm::vec4 rotation;
    RotationMode rotationMode = RotationMode::EulerXYZ;
    glm::vec3 scale{1};

    /**
     * @brief 4x4 matrix representation of transform
     */
    operator glm::mat4() const;
};
}
}
}
