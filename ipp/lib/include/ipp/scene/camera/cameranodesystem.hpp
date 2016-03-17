#pragma once

#include <ipp/shared.hpp>
#include <ipp/loop/system.hpp>
#include <ipp/entity/entitygroup.hpp>
#include "cameranodecomponent.hpp"

namespace ipp {
namespace scene {
namespace camera {

/**
 * @brief Scene camera component.
 */
class CameraNodeSystem final : public Camera, public loop::SystemT<CameraNodeSystem> {
public:
    /**
     * @brief Command used to change CameraNodeSystem active camera Entity instance
     */
    typedef loop::CommandT<ipp::schema::message::camera::CameraNodeActive> SetActiveCommand;

    /**
     * @brief Event dispatched after CameraNodeSystem active Entity camera has been changed.
     */
    typedef loop::EventT<ipp::schema::message::camera::CameraNodeActive> ActiveUpdatedEvent;

    typedef entity::EntityGroupWithComponents<CameraNodeComponent, node::NodeComponent>
        CameraNodeEntityGroup;

private:
    uint32_t _defaultActiveEntityId;
    CameraNodeComponent* _active;
    CameraNodeEntityGroup* _entityGroup;

    /**
     * @brief
     */
    std::vector<SystemBase*> initialize() override;

    /**
     * @brief
     */
    void onMessage(const loop::Message& message) override;

    /**
     * @brief Update all cameras.
     */
    void onUpdate() override;

public:
    CameraNodeSystem(loop::MessageLoop& messageLoop,
                     entity::World& world,
                     uint32_t defaultActiveEntityId);

    /**
     * @brief Camera eye position.
     */
    glm::vec3 getViewPosition() override;

    /**
     * @brief Camera view direction.
     */
    glm::vec3 getViewDirection() override;

    /**
     * @brief Camera up direction.
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
     * @brief Return CameraType as EntityComponent
     */
    CameraType getCameraType() const override
    {
        return CameraType::EntityComponent;
    }

    /**
     * @brief Entity group containing all Entities with CameraComponent.
     */
    CameraNodeEntityGroup& getCameraNodeEntityGroup() const
    {
        return *_entityGroup;
    }
};
}
}
}
