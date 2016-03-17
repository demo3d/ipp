#pragma once

#include <ipp/shared.hpp>
#include <ipp/render/armature.hpp>
#include <ipp/entity/component.hpp>

namespace ipp {
namespace scene {
namespace render {

/**
 * @brief Armature component binds skeleton pose to a Model Mesh for skinning.
 */
class ArmatureComponent final : public entity::ComponentT<ArmatureComponent> {
private:
    std::shared_ptr<::ipp::render::Armature> _armature;
    std::vector<::ipp::render::Armature::Bone::Pose> _bonePoses;

public:
    ArmatureComponent(entity::Entity& entity, std::shared_ptr<::ipp::render::Armature> armature);

    /**
     * @brief Armature linked by component for skinning.
     */
    std::shared_ptr<::ipp::render::Armature> getArmature() const
    {
        return _armature;
    }

    /**
     * @brief Armature current skinning bone poses.
     */
    std::vector<::ipp::render::Armature::Bone::Pose>& getBonePoses()
    {
        return _bonePoses;
    }

    /**
     * @brief Armature current skinning bone poses.
     */
    const std::vector<::ipp::render::Armature::Bone::Pose>& getBonePoses() const
    {
        return _bonePoses;
    }
};
}
}
}
