#include <ipp/context.hpp>
#include <ipp/scene/scene.hpp>
#include <ipp/scene/node/nodesystem.hpp>
#include <ipp/scene/camera/camerasystem.hpp>
#include <ipp/scene/animation/animationsystem.hpp>
#include <ipp/scene/render/rendersystem.hpp>
#include <ipp/schema/primitive_generated.h>
#include <ipp/schema/resource/scene/animation_generated.h>
#include <ipp/schema/resource/scene/scene_generated.h>

using namespace std;
using namespace glm;
using namespace ipp;
using namespace ipp::resource;
using namespace ipp::entity;
using namespace ipp::loop;
using namespace ipp::render;
using namespace ipp::scene;
using namespace ipp::scene::render;
using namespace ipp::scene::animation;
using namespace ipp::scene::node;
using namespace ipp::scene::camera;

void readAnimation(Scene& scene, const schema::resource::scene::Animation* animationData);

void readScene(Scene& scene,
               ResourceManager& resourceManager,
               const schema::resource::scene::Scene* sceneData);

Scene::Scene(Context& context, std::unique_ptr<resource::ResourceBuffer> data)
    : _context{context}
    , _resourcePath{data->getResourcePath()}
{
    auto sceneData = schema::resource::scene::GetScene(data->getData());

    IVL_LOG(Info, "Deserializing Scene {} with {} entities", _resourcePath,
            sceneData->entities()->size());

    readScene(*this, context.getResourceManager(), sceneData);
    readAnimation(*this, sceneData->animation());
    getMessageLoop().createSystem<RenderSystem>(getWorld());

    IVL_LOG(Info, "Scene entities deserialization successfull");

    _messageLoop.initialize();
    IVL_LOG(Info, "Scene successfully initialized");
}

void Scene::update()
{
    _messageLoop.update();
}
