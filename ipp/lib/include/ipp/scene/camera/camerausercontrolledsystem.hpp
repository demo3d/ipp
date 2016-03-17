#pragma once

#include <ipp/shared.hpp>
#include <ipp/loop/system.hpp>
#include <ipp/scene/camera/camera.hpp>
#include <ipp/schema/primitive_generated.h>
#include <ipp/schema/message/camera_generated.h>
#include "cameraprojection.hpp"

namespace ipp {
namespace scene {
namespace camera {

/**
 * @brief User controlled camera using yaw/pitch/distance/target parameters
 */
class CameraUserControlledSystem final : public Camera,
                                         public loop::SystemT<CameraUserControlledSystem> {
public:
    typedef loop::CommandT<ipp::schema::message::camera::CameraUserControlledMove> MoveCommand;
    typedef loop::CommandT<ipp::schema::message::camera::CameraUserControlledRotationPolar>
        RotatePolarCommand;
    typedef loop::CommandT<ipp::schema::message::camera::CameraUserControlledRotationArcballStart>
        RotationStartCommand;
    typedef loop::CommandT<ipp::schema::message::camera::CameraUserControlledRotationArcballUpdate>
        RotationUpdateCommand;
    typedef loop::CommandT<ipp::schema::message::camera::CameraUserControlledZoom> ZoomCommand;
    typedef loop::CommandT<ipp::schema::message::camera::CameraUserControlledState> StateSetCommand;
    typedef loop::CommandT<ipp::schema::message::camera::CameraUserControlledLimits>
        LimitsSetCommand;
    typedef loop::EventT<ipp::schema::message::camera::CameraUserControlledState> StateUpdatedEvent;
    typedef loop::EventT<ipp::schema::message::camera::CameraUserControlledLimits>
        LimitsUpdatedEvent;

private:
    glm::vec3 _viewUp;
    glm::vec3 _viewPosition;
    glm::vec3 _viewDirection;
    glm::vec3 _viewTarget;
    glm::mat4 _viewMatrix;

    float _viewDistanceMin;
    float _viewDistanceMax;
    glm::vec3 _viewPositionMin;
    glm::vec3 _viewPositionMax;
    glm::vec3 _viewTargetMin;
    glm::vec3 _viewTargetMax;

    glm::vec2 _rotationPrevious;

    float _rotationPolarUpAngleCosMin;
    float _rotationPolarUpAngleCosMax;

    CameraProjection _projection;
    glm::vec2 _projectionViewport;
    glm::mat4 _projectionMatrixCache;

    /**
     * @brief
     */
    std::vector<SystemBase*> initialize() override;

    /**
     * @brief
     */
    void onMessage(const loop::Message& message) override;

    /**
     * @brief Move camera target by delta x/y/z relative to eye rotation
     */
    void onMove(glm::vec3 delta);

    /**
     * @brief Rotate view position arround view target by polar delta angles
     *
     * Polar rotation rotates arround fixed up axis (0,0,1) to provide an intuitive
     * up/down and left/right rotation interface.
     *
     * @note Polar rotation is limited by view direction angle to global up axis (0,0,1)
     *       cosine value (viewDir dot (0,0,1)) specified in _rotationPolarUpAngleCosMin/Max
     */
    void onRotatePolar(float deltaLatitude, float deltaLongitude);

    /**
     * @brief Provide the inital mouse point for Arcball pull rotation
     */
    void onRotationArcballStart(glm::vec2 point);

    /**
     * @brief Rotates view position arround view target using angle between two points on arcball
     *
     * Arcball points are provided by point and previous point and are -1/1 range x/y coordinates
     * on unit circle that represents the rotation arcball.
     */
    void onRotationArcballUpdate(glm::vec2 point);

    /**
     * @brief Zoom camera by delta distance
     */
    void onZoom(float deltaDistance);

    /**
     * @brief Reset camera state
     */
    void onStateSet(const ipp::schema::message::camera::CameraUserControlledState& state);

    /**
     * @brief Update camera limits
     */
    void onLimitsSet(const ipp::schema::message::camera::CameraUserControlledLimits& limits);

    /**
     * @brief Recalculate camera view matrix
     */
    void updateView();

    /**
     * @brief Recalculate camera projection matrix
     */
    void updateProjection();

public:
    CameraUserControlledSystem(loop::MessageLoop& messageLoop);

    /**
     * @brief Reset camera eye position and calculate a
     */
    void resetView(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up);

    /**
     * @brief Get camera eye position
     */
    glm::vec3 getViewPosition() override;

    /**
     * @brief Get camera eye looking direction
     */
    glm::vec3 getViewDirection() override;

    /**
     * @brief Get camera up direction
     */
    glm::vec3 getViewUp() override;

    /**
     * @brief Get camera view matrix
     */
    const glm::mat4& getView() override;

    /**
     * @brief Get camera projection matrix
     */
    const glm::mat4& getProjection(int width, int height) override;

    /**
     * @brief Return CameraType as UserControlled
     */
    CameraType getCameraType() const override
    {
        return CameraType::UserControlled;
    }
};
}
}
}
