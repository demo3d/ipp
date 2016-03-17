#pragma once

#include <ipp/shared.hpp>

namespace ipp {
namespace scene {
namespace camera {

/**
 * @brief Blender defined camera projection parameters
 *
 * TODO: Implement orthographic projection
 */
struct CameraProjection {
public:
    enum class SensorFit { Auto = 0, Horizontal = 1, Vertical = 2 };

public:
    glm::vec2 cameraShift;
    float sensorPixelSize;
    SensorFit sensorFit;
    float clipNear;
    float clipFar;

public:
    /**
     * @brief Default values for blender camera
     */
    CameraProjection();

    /**
     * @brief Initialize CameraProjection with all projection values specified
     */
    CameraProjection(glm::vec2 cameraShift,
                     float sensorPixelSize,
                     SensorFit sensorFit,
                     float clipNear,
                     float clipFar);

    /**
     * @brief Calculate projection matrix from projection parameters and viewport dimensions
     *
     * Uses Blender defined projection camera formula to convert projection parameters
     * to a valid projection matrix.
     */
    glm::mat4 calculateProjectionMatrix(glm::vec2 viewport);
};
}
}
}
