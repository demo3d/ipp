#include <ipp/scene/render/armaturecomponent.hpp>

using namespace std;
using namespace ipp::resource;
using namespace ipp::entity;
using namespace ipp::render;
using namespace ipp::scene::render;
using namespace ipp::scene::animation;

template <>
const string ComponentT<ArmatureComponent>::ComponentTypeName = "SceneArmatureComponent";

ArmatureComponent::ArmatureComponent(Entity& entity, shared_ptr<Armature> armature)
    : ComponentT<ArmatureComponent>(entity)
    , _armature{move(armature)}
{
    _bonePoses.reserve(_armature->getBones().size());
    for (size_t i = 0; i < _armature->getBones().size(); ++i) {
        _bonePoses.push_back(_armature->getBones()[i].getBindPose());
    }
}
