#include <ipp/scene/render/rendersystem.hpp>
#include <ipp/scene/node/nodesystem.hpp>
#include <ipp/scene/node/nodecomponent.hpp>
#include <ipp/scene/camera/camerasystem.hpp>
#include <ipp/scene/animation/animationsystem.hpp>
#include <ipp/loop/messageloop.hpp>
#include <ipp/render/gl/error.hpp>
#include <ipp/entity/world.hpp>

using namespace std;
using namespace glm;
using namespace ipp::loop;
using namespace ipp::entity;
using namespace ipp::render;
using namespace ipp::scene::render;
using namespace ipp::scene::node;
using namespace ipp::scene::animation;

template <>
const string RenderSystem::ViewportResizeCommand::CommandTypeName =
    "SceneRenderViewportResizeCommand";
template <>
const string RenderSystem::ClearFlagsSetCommand::CommandTypeName =
    "SceneRenderClearFlagsSetCommand";
template <>
const string RenderSystem::ViewportResizedEvent::EventTypeName = "SceneRenderViewportResizedEvent";
template <>
const string RenderSystem::ClearFlagsUpdatedEvent::EventTypeName =
    "SceneRenderClearFlagsUpdatedEvent";
template <>
const string SystemT<RenderSystem>::SystemTypeName = "SceneRenderSystem";

vector<SystemBase*> RenderSystem::initialize()
{
    registerCommandT<ViewportResizeCommand>();
    registerEventT<ViewportResizedEvent>();

    _cameraSystem = getMessageLoop().findSystem<camera::CameraSystem>();
    auto nodeSystem = getMessageLoop().findSystem<NodeSystem>();
    auto animationSystem = getMessageLoop().findSystem<AnimationSystem>();

    IVL_LOG(Trace, "Render system initialized");
    return {nodeSystem, _cameraSystem, animationSystem};
}

void RenderSystem::onMessage(const Message& message)
{
    if (auto viewportDimensions = getCommandData<ViewportResizeCommand>(message)) {
        _viewportDimensions.x = static_cast<int>(viewportDimensions->width());
        _viewportDimensions.y = static_cast<int>(viewportDimensions->height());

        dispatchEventT<ViewportResizedEvent>(viewportDimensions->width(),
                                             viewportDimensions->height());
        return;
    }
}

void RenderSystem::renderDirectionalLight(
    const LightComponent::Directional& light,
    const vector<tuple<float, RenderableComponent*>>& renderables,
    const mat4& lightViewProjectionMatrix)
{

    static const auto biasMatrix = mat4(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
                                        1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

    auto lightShadowMapMatrix = biasMatrix * lightViewProjectionMatrix;

    // render shadow map
    /*
    {
        auto frameBufferBinding =
            _shadowMapFrameBuffer.bind({{0, 0, _viewportDimensions.x, _viewportDimensions.y}});
        assert(frameBufferBinding.getStatus() == gl::FrameBuffer::Status::Complete);
        glClearDepthf(1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render backfaces in to shadowmap for less self-occlusion artifacts
        glCullFace(GL_FRONT);
        for (auto& distanceRenderable : renderables) {
            auto renderable = get<1>(distanceRenderable);
            auto& material = renderable->getMaterial();
            auto& effect = material.getEffect();
            auto& materialBuffer = renderable->getMaterialBuffer();

            effect.writeLightDirectional(materialBuffer, light.direction, light.color,
                                         light.ambientDiffuseIntensity, lightViewProjectionMatrix,
                                         lightShadowMapMatrix);

            if (material.isShadowCaster()) {
                if (auto shadowMapDirectionalPass = effect.getShadowMapDirectionalPass()) {
                    renderable->render(*shadowMapDirectionalPass, nullptr);
                }
            }
        }

        // Restore culling state
        glCullFace(GL_BACK);
    }
    */
    // render light to front buffer
    {
        // auto shadowMapBinding = _shadowMapTexture.bind({0});
        for (auto& distanceRenderable : renderables) {
            auto renderable = get<1>(distanceRenderable);
            auto& material = renderable->getMaterial();
            auto& effect = material.getEffect();
            auto& materialBuffer = renderable->getMaterialBuffer();
            effect.writeLightDirectional(materialBuffer, light.direction, light.color,
                                         light.ambientDiffuseIntensity, lightViewProjectionMatrix,
                                         lightShadowMapMatrix);

            if (auto lightDirectionalPass = effect.getLightDirectionalPass()) {
                gl::Texture2D::Binding2D diffuseBinding;
                if (auto diffuseTexture = material.getDiffuseTexture()) {
                    diffuseBinding =
                        diffuseTexture->textureResource->bind(diffuseTexture->textureUnit);
                }
                renderable->render(*lightDirectionalPass, nullptr);
            }
        }
    }
}

void RenderSystem::renderParticles(const vector<tuple<float, RenderableComponent*>>& renderables)
{
    for (auto& distanceRenderable : renderables) {
        auto renderable = get<1>(distanceRenderable);
        auto& material = renderable->getMaterial();
        auto& effect = material.getEffect();

        if (auto particlePass = effect.getParticlePass()) {
            renderable->render(*particlePass, nullptr);
        }
    }
}

void RenderSystem::onUpdate()
{
    // render light pass
    auto& camera = _cameraSystem->getActiveCamera();

    auto cameraViewMatrix = camera.getView();
    auto cameraProjectionMatrix =
        camera.getProjection(_viewportDimensions.x, _viewportDimensions.y);
    auto cameraViewProjectionMatrix = cameraProjectionMatrix * cameraViewMatrix;
    vec3 cameraViewPosition = camera.getViewPosition();
    vec3 cameraViewDirection = camera.getViewPosition();

    // build render queue and from world renderable node entities
    static std::vector<std::tuple<float, RenderableComponent*>> renderQueue;
    renderQueue.clear();
    for (auto& renderableEntity : _renderableEntities->getEntities()) {
        RenderableComponent* renderable = get<1>(renderableEntity);
        NodeComponent* node = get<2>(renderableEntity);

        if (node->isHidden()) {
            continue;
        }

        const MaterialEffect& effect = renderable->getMaterial().getEffect();
        MaterialBuffer& materialBuffer = renderable->getMaterialBuffer();

        mat4 worldMatrix = node->getTransformMatrix();
        mat3 normalMatrix = transpose(inverse(mat3(worldMatrix)));
        mat4 worldViewMatrix = cameraViewMatrix * worldMatrix;
        mat4 worldViewProjectionMatrix = cameraViewProjectionMatrix * worldMatrix;
        effect.writeWorldViewProjection(materialBuffer, cameraViewPosition, cameraViewDirection,
                                        worldMatrix, cameraViewMatrix, cameraProjectionMatrix,
                                        cameraViewProjectionMatrix, worldViewMatrix,
                                        worldViewProjectionMatrix, normalMatrix);

        ArmatureComponent* armature = renderable->getSkinningArmature();
        if (armature) {
            auto name = get<0>(renderableEntity)->getName();
            effect.writeSkinningMatrices(materialBuffer, armature);
        }

        auto nodeEyeDistance = length2(node->getTransform().translation - cameraViewPosition);
        renderQueue.emplace_back(nodeEyeDistance, renderable);
    }

    // sort render queue by distance from camera
    sort(renderQueue.begin(), renderQueue.end(),
         [](const auto& a, const auto& b) { return get<0>(a) < get<0>(b); });
    glViewport(0, 0, _viewportDimensions.x, _viewportDimensions.y);
    // clear default target backbuffer
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glClearDepthf(1);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glClearColor(1, 1, 1, 1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render directional light from camera
    {
        mat4 lightProjectionMatrix = glm::ortho<float>(-2.1f, 2.1f, -2.1f, 2.1f, -1, 20);
        mat4 lightViewMatrix = lookAt(normalize(cameraViewPosition), vec3(0, 0, 0), vec3(0, 0, 1));
        mat4 lightViewProjectionMatrix = lightProjectionMatrix * lightViewMatrix;

        LightComponent::Directional cameraLight{normalize(cameraViewPosition), vec3{0.8, 0.8, 0.8},
                                                0.2f};
        renderDirectionalLight(cameraLight, renderQueue, lightViewProjectionMatrix);
    }

    // render particle pass
    renderParticles(renderQueue);
}

RenderSystem::RenderSystem(MessageLoop& messageLoop, World& world)
    : SystemT<RenderSystem>(messageLoop)
{
    _renderableEntities = world.createEntityObserver<RenderSystem::RenderableGroup>();
    _lightEntities = world.createEntityObserver<RenderSystem::LightGroup>();

    /*
    {
        auto textureBinding = _shadowMapTexture.bind({0});
        textureBinding.texImage2D(
            0, gl::Texture::PixelFormat::DepthComponent, static_cast<GLsizei>(_shadowMapDimensions),
            static_cast<GLsizei>(_shadowMapDimensions), gl::Texture::PixelFormat::DepthComponent,
            gl::Texture::PixelType::UShort, nullptr);

        textureBinding.setWrap(gl::Texture::WrapCoordinate::S, gl::Texture::Wrap::ClampToEdge);
        textureBinding.setWrap(gl::Texture::WrapCoordinate::T, gl::Texture::Wrap::ClampToEdge);
        textureBinding.setMinFilter(gl::Texture::MinFilter::Linear);
        textureBinding.setMagFilter(gl::Texture::MagFilter::Linear);
        auto frameBufferBinding =
            _shadowMapFrameBuffer.bind({{0, 0, _viewportDimensions.x, _viewportDimensions.y}});
        frameBufferBinding.attachTexture2D(gl::FrameBuffer::Attachment::DepthAttachment,
                                           _shadowMapTexture, 0);

        auto frameBufferStatus = frameBufferBinding.getStatus();
        if (frameBufferStatus != gl::FrameBuffer::Status::Complete) {
            IVL_LOG_THROW_ERROR(runtime_error, "Unable to create shadow map FrameBuffer object.");
        }
    }
    */
}
