#pragma once

#include <ipp/shared.hpp>
#include <ipp/render/armature.hpp>
#include <ipp/entity/component.hpp>
#include "armaturecomponent.hpp"

namespace ipp {
namespace scene {
namespace render {

/**
 * @brief Armature component binds skeleton pose to a Model Mesh for skinning.
 */
class LightComponent final : public entity::ComponentT<LightComponent> {
public:
    enum class LightKind { Directional };

    struct Directional {
        glm::vec3 direction;
        glm::vec3 color;
        float ambientDiffuseIntensity;
    };

private:
    LightKind _lightKind;
    Directional _directional;

public:
    LightComponent(entity::Entity& entity, const Directional& directional);

    /**
     * @brief Const directional light data.
     */
    const Directional& getDirectional() const
    {
        assert(_lightKind == LightKind::Directional);
        return _directional;
    }

    /**
     * @brief Const directional light data.
     */
    Directional& getDirectional()
    {
        assert(_lightKind == LightKind::Directional);
        return _directional;
    }

    /**
     * @brief LightComponent light kind.
     */
    LightKind getLightKind() const
    {
        return _lightKind;
    }
};
}
}
}
