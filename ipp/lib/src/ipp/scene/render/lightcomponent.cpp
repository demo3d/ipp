#include <ipp/scene/render/lightcomponent.hpp>

using namespace std;
using namespace glm;
using namespace ipp::entity;
using namespace ipp::scene::render;

template <>
const string ComponentT<LightComponent>::ComponentTypeName = "SceneLightComponent";

LightComponent::LightComponent(Entity& entity, const Directional& directional)
    : ComponentT<LightComponent>(entity)
    , _lightKind{LightKind::Directional}
    , _directional{directional}
{
}
