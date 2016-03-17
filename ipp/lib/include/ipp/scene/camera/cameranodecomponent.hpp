#pragma once

#include <ipp/shared.hpp>
#include <ipp/entity/component.hpp>
#include "camera.hpp"
#include "cameraprojection.hpp"

namespace ipp {
namespace scene {
namespace camera {

/**
 * @brief Scene camera component.
 */
class CameraNodeComponent final : public ::ipp::entity::ComponentT<CameraNodeComponent> {
private:
    glm::mat4 _nodeTransform;
    glm::mat4 _view;
    bool _viewDirty;

    CameraProjection _projection;
    glm::vec2 _projectionViewport;
    glm::mat4 _projectionMatrixCache;

private:
    void updateView();

public:
    CameraNodeComponent(entity::Entity& entity, CameraProjection projection);

    /**
     * @brief Updates camera node transform matrix from which the view matrix is calculated
     */
    void updateFromNodeTransform(const glm::mat4& transform);

    /**
     * @brief Camera eye position.
     */
    glm::vec3 getViewPosition();

    /**
     * @brief Camera view direction.
     */
    glm::vec3 getViewDirection();

    /**
     * @brief Camera up direction.
     */
    glm::vec3 getViewUp();

    /**
     * @brief Get camera view matrix
     */
    const glm::mat4& getView();

    /**
     * @brief Get camera projection matrix
     */
    const glm::mat4& getProjection(int width, int height);
};
}
}
}
