#pragma once

#include <ipp/shared.hpp>
#include <ipp/schema/primitive_generated.h>
#include <ipp/schema/message/camera_generated.h>

namespace ipp {
namespace scene {
namespace camera {

/**
 * @brief Re-export CameraType enum
 */
enum class CameraType : uint32_t {
    UserControlled = ipp::schema::message::camera::CameraType::CameraType_UserControlled,
    EntityComponent = ipp::schema::message::camera::CameraType::CameraType_EntityComponent
};

/**
 * @brief Abstract Camera interface.
 */
class Camera {
public:
    virtual ~Camera() = default;

    /**
     * @brief Camera Eye position.
     */
    virtual glm::vec3 getViewPosition() = 0;

    /**
     * @brief Camera view direction (normal from eye towards view target).
     */
    virtual glm::vec3 getViewDirection() = 0;

    /**
     * @brief Camera view up direction.
     */
    virtual glm::vec3 getViewUp() = 0;

    /**
     * @brief Get camera view matrix
     */
    virtual const glm::mat4& getView() = 0;

    /**
     * @brief Get camera projection matrix for specified viewport dimensions
     */
    virtual const glm::mat4& getProjection(int width, int height) = 0;

    /**
     * @brief Return CameraType for camera implementation
     */
    virtual CameraType getCameraType() const = 0;
};
}
}
}
