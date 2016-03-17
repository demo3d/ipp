#pragma once

#include <ipp/shared.hpp>
#include <ipp/entity/component.hpp>
#include <ipp/render/mesh.hpp>
#include "material.hpp"

namespace ipp {
namespace scene {
namespace render {

/**
 * @brief Wraps a Renderable instance in to a ComponentBase container.
 */
class RenderableComponent final : public entity::ComponentT<RenderableComponent> {
private:
    std::shared_ptr<ipp::render::Mesh> _mesh;
    std::shared_ptr<Material> _material;
    ipp::render::MaterialBuffer _materialBuffer;
    ArmatureComponent* _skinningArmature;
    std::vector<ipp::render::gl::VertexDefinition> _passVertexDefinitions;

public:
    RenderableComponent(entity::Entity& entity,
                        std::shared_ptr<ipp::render::Mesh> mesh,
                        std::shared_ptr<Material> material,
                        ArmatureComponent* skinningArmature);

    /**
     * @brief Forward Render call to appropriate object based on Kind with pass/renderPass.
     */
    void render(const ipp::render::Effect::Pass& pass, RenderPass* renderPass);

    /**
     * @brief Renderable Material resource reference.
     */
    Material& getMaterial() const
    {
        return *_material;
    }

    /**
     * @brief Material buffer that stores material properties for this renderable instance.
     */
    ipp::render::MaterialBuffer& getMaterialBuffer()
    {
        return _materialBuffer;
    }

    /**
     * @brief Renderable Mesh instance, nullptr if renderable is not rendered from Mesh.
     */
    ipp::render::Mesh* getMesh() const
    {
        return _mesh.get();
    }

    /**
     * @brief ArmatureComponent used for renderable skinning, nullptr if renderable doesn't use
     * skinning.
     */
    ArmatureComponent* getSkinningArmature() const
    {
        return _skinningArmature;
    }
};
}
}
}
