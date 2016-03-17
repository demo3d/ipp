#include <ipp/scene/render/renderablecomponent.hpp>

using namespace std;
using namespace ipp::resource;
using namespace ipp::entity;
using namespace ipp::render;
using namespace ipp::scene::render;

template <>
const string ComponentT<RenderableComponent>::ComponentTypeName = "SceneRenderableComponent";

RenderableComponent::RenderableComponent(Entity& entity,
                                         shared_ptr<Mesh> mesh,
                                         shared_ptr<Material> material,
                                         ArmatureComponent* skinningArmature)
    : ComponentT<RenderableComponent>(entity)
    , _mesh{move(mesh)}
    , _material{move(material)}
    , _materialBuffer{_material->getMaterialBuffer()}
    , _skinningArmature{skinningArmature}
{
    auto& passes = _material->getEffect().getPasses();
    for (size_t i = 0; i < passes.size(); ++i) {
        assert(passes[i].getPassIndex() == i);
        _passVertexDefinitions.push_back(
            passes[i].getShaderProgram().getProgramVertexDefinition(_mesh->getVertexDefinition()));
    }
}

void RenderableComponent::render(const Effect::Pass& pass, RenderPass* renderPass)
{
    auto passBinding = pass.bindWithMaterial(_materialBuffer);
    Mesh::Binding meshBinding{*_mesh};
    _mesh->draw(meshBinding, passBinding);
}
