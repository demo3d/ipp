#pragma once

#include <ipp/entity/entitygroup.hpp>
#include <ipp/loop/system.hpp>
#include <ipp/render/gl/texture2d.hpp>
#include <ipp/render/gl/framebuffer.hpp>
#include <ipp/render/gl/backbuffer.hpp>
#include <ipp/shared.hpp>
#include <ipp/scene/camera/camerasystem.hpp>
#include <ipp/schema/primitive_generated.h>
#include <ipp/schema/message/render_generated.h>
#include "renderablecomponent.hpp"
#include "armaturecomponent.hpp"
#include "lightcomponent.hpp"

namespace ipp {
namespace scene {
namespace render {

class RenderSystem final : public loop::SystemT<RenderSystem> {
public:
    /**
     * @brief Command to update render target viewport size
     */
    using ViewportResizeCommand = loop::CommandT<ipp::schema::message::render::RenderViewportSize>;

    /**
     * @brief Event dispatched after render target viewport has been resized
     */
    using ViewportResizedEvent = loop::EventT<ipp::schema::message::render::RenderViewportSize>;

    /**
     * @brief Command to update render clear flags
     */
    using ClearFlagsSetCommand = loop::CommandT<ipp::schema::message::render::RenderClearFlags>;

    /**
     * @brief Event dispatched after render target viewport has been resized
     */
    using ClearFlagsUpdatedEvent = loop::EventT<ipp::schema::message::render::RenderClearFlags>;

    /**
     * @brief EntityGroup for RenderableComponent entities with associated NodeComponent
     */
    using RenderableGroup =
        entity::EntityGroupWithComponents<RenderableComponent, node::NodeComponent>;

    /**
     * @brief EntityGroup for LightComponent entities with associated NodeComponent.
     */
    using LightGroup = entity::EntityGroupWithComponents<LightComponent, node::NodeComponent>;

private:
    camera::CameraSystem* _cameraSystem;
    glm::ivec2 _viewportDimensions;
    RenderableGroup* _renderableEntities;
    LightGroup* _lightEntities;

    /**
     * @brief Initialize render system
     */
    std::vector<loop::SystemBase*> initialize() override;

    /**
     * @brief Listen for RenderSystem events.
     */
    void onMessage(const loop::Message& message) override;

    /**
     * @brief Update Node tree matrices to reflect transform changes.
     *
     * Updates Node and CameraBlender components.
     */
    void onUpdate() override;

    /**
     * @brief Renders a directional light to front buffer.
     */
    void renderDirectionalLight(
        const LightComponent::Directional& light,
        const std::vector<std::tuple<float, RenderableComponent*>>& renderables,
        const glm::mat4& lightViewProjectionMatrix);

    /**
     * @brief Render particles to front buffer.
     */
    void renderParticles(const std::vector<std::tuple<float, RenderableComponent*>>& renderables);

public:
    RenderSystem(loop::MessageLoop& messageLoop, entity::World& scene);

    /**
     * @brief EntityGroup containing all Entities with Renderable/Node components.
     */
    RenderableGroup& getRenderableEntityGroup() const
    {
        return *_renderableEntities;
    }

    /**
     * @brief Return scene viewport dimensions.
     */
    const glm::ivec2& getViewportDimensions() const
    {
        return _viewportDimensions;
    }
};
}
}
}
