#include <ipp/resource/resourcemanager.hpp>
#include <ipp/entity/world.hpp>
#include <ipp/render/effect.hpp>

#include <ipp/scene/render/material.hpp>
#include <ipp/scene/render/renderablecomponent.hpp>
#include <ipp/scene/render/rendersystem.hpp>
#include <ipp/scene/camera/camerasystem.hpp>
#include <ipp/scene/node/nodesystem.hpp>

#include <ipp/scene/scene.hpp>

#include <ipp/schema/primitive_generated.h>
#include <ipp/schema/resource/scene/animation_generated.h>
#include <ipp/schema/resource/scene/scene_generated.h>

using namespace std;
using namespace glm;
using namespace ipp;
using namespace ipp::resource;
using namespace ipp::entity;
using namespace ipp::render;
using namespace ipp::scene;
using namespace ipp::scene::render;
using namespace ipp::scene::camera;
using namespace ipp::scene::node;

inline vec2 readVec2(const schema::primitive::Vec2* data)
{
    if (data == nullptr)
        return {};
    return {data->x(), data->y()};
}

inline vec3 readVec3(const schema::primitive::Vec3* data)
{
    if (data == nullptr)
        return {};
    return {data->x(), data->y(), data->z()};
}

inline vec4 readVec4(const schema::primitive::Vec4* data)
{
    if (data == nullptr)
        return {};
    return {data->x(), data->y(), data->z(), data->w()};
}

inline mat4 readMat4(const schema::primitive::Mat4* data)
{
    if (data == nullptr)
        return {};
    return {readVec4(&data->row0()), readVec4(&data->row1()), readVec4(&data->row2()),
            readVec4(&data->row3())};
}

void readNodeComponent(Entity& entity,
                       Node& rootNode,
                       const schema::resource::scene::NodeComponent* nodeData)
{
    auto componentNode = entity.createComponent<NodeComponent>();

    auto& nodeTransform = componentNode->getTransform();

#define MAP_ROTATION_MODE(MODE)                                                                    \
    case schema::resource::scene::NodeRotationMode_##MODE:                                         \
        nodeTransform.rotationMode = NodeTransform::RotationMode::MODE;                            \
        break;

    switch (nodeData->transform()->rotationMode()) {
        MAP_ROTATION_MODE(Quaternion)
        MAP_ROTATION_MODE(AxisAngle)
        MAP_ROTATION_MODE(EulerXYZ)
        MAP_ROTATION_MODE(EulerXZY)
        MAP_ROTATION_MODE(EulerYXZ)
        MAP_ROTATION_MODE(EulerYZX)
        MAP_ROTATION_MODE(EulerZXY)
        MAP_ROTATION_MODE(EulerZYX)
        default:
            throw logic_error("Unknown rotation mode");
    }

#undef MAP_ROTATION_MODE

    nodeTransform.translation = readVec3(&nodeData->transform()->translation());
    nodeTransform.rotation = readVec4(&nodeData->transform()->rotation());
    nodeTransform.scale = readVec3(&nodeData->transform()->scale());

    auto parentId = nodeData->parentEntity();
    auto parentEntity = entity.getWorld().findEntity(parentId);
    auto& parentNode = parentEntity ? *parentEntity->findComponent<NodeComponent>() : rootNode;
    auto transformParentingInverseMatrix =
        transpose(readMat4(nodeData->transformParentingInverseMatrix()));
    parentNode.addChild(componentNode, transformParentingInverseMatrix);
}

void readCameraComponent(Entity& entity, const schema::resource::scene::CameraComponent* cameraData)
{
    CameraProjection::SensorFit sensorFit = CameraProjection::SensorFit::Auto;

#define MAP_CAMERA_SENSOR_FIT(ENUM)                                                                \
    case schema::resource::scene::SensorFit_##ENUM:                                                \
        sensorFit = CameraProjection::SensorFit::ENUM;                                             \
        break;

    switch (cameraData->sensorFit()) {
        MAP_CAMERA_SENSOR_FIT(Auto)
        MAP_CAMERA_SENSOR_FIT(Horizontal)
        MAP_CAMERA_SENSOR_FIT(Vertical)
    }

#undef MAP_CAMERA_SENSOR_FIT

    entity.createComponent<CameraNodeComponent>(
        CameraProjection{readVec2(cameraData->cameraShift()), cameraData->pixelSize(), sensorFit,
                         cameraData->clipNear(), cameraData->clipFar()});
}

void readLightComponent(Entity& entity, const schema::resource::scene::LightComponent* lightData)
{
    switch (lightData->light_type()) {
        case schema::resource::scene::LightKind_DirectionalLight: {
            auto directionalLightData =
                reinterpret_cast<const schema::resource::scene::DirectionalLight*>(
                    lightData->light());
            auto direction = readVec3(directionalLightData->direction());
            auto color = readVec3(directionalLightData->color());
            auto ambientDiffuseIntensity = directionalLightData->ambientDiffuseIntensity();

            entity.createComponent<LightComponent>(
                LightComponent::Directional{direction, color, ambientDiffuseIntensity});
        } break;

        default:
            IVL_LOG_THROW_ERROR(logic_error, "Unknown light kind");
    }
}

void readArmatureComponent(Entity& entity,
                           ResourceManager& resourceManager,
                           const schema::resource::scene::ArmatureComponent* armatureData)
{
    auto armatureResource = resourceManager.requestSharedResource<Armature>(
        armatureData->armatureResourcePath()->str());
    entity.createComponent<ArmatureComponent>(armatureResource);
}

/**
 * @brief Read Entity Component Model and attach it to entity
 *
 * Loads Mesh and Skeleton (if used) from resourcePackage.
 */
void readModelComponent(Entity& entity,
                        ResourceManager& resourceManager,
                        const schema::resource::scene::ModelComponent* modelData)
{
    IVL_LOG(Trace, "Model Material : {}", modelData->materialResourcePath()->str());
    auto material =
        resourceManager.requestSharedResource<Material>(modelData->materialResourcePath()->str());
    auto mesh = resourceManager.requestSharedResource<Mesh>(modelData->meshResourcePath()->str());

    auto armatureId = modelData->armatureEntity();
    ArmatureComponent* armatureComponent = nullptr;
    if (armatureId != 0) {
        armatureComponent =
            entity.getWorld().findEntity(armatureId)->findComponent<ArmatureComponent>();
    }

    entity.createComponent<RenderableComponent>(mesh, material, armatureComponent);
}

void readScene(Scene& scene,
               ResourceManager& resourceManager,
               const schema::resource::scene::Scene* sceneData)
{
    auto& world = scene.getWorld();
    auto& messageLoop = scene.getMessageLoop();
    auto& nodeSystem = messageLoop.createSystem<NodeSystem>();
    auto& rootNode = nodeSystem.getRootNode();

    for (auto entityData : *sceneData->entities()) {
        auto entity = world.createEntity(entityData->id(), entityData->name()->str());

        IVL_LOG(Trace, "Deserializing entity : {} with Id : {}", entity->getName(),
                entity->getId());

        for (auto componentData : *entityData->components()) {
            switch (componentData->component_type()) {
                case schema::resource::scene::ComponentKind_NodeComponent:
                    IVL_LOG(Trace, "Deserializing Entity ({}) Node component", entity->getName());
                    readNodeComponent(
                        *entity, rootNode,
                        reinterpret_cast<const schema::resource::scene::NodeComponent*>(
                            componentData->component()));
                    break;

                case schema::resource::scene::ComponentKind_CameraComponent:
                    IVL_LOG(Trace, "Deserializing Entity ({}) Camera component.",
                            entity->getName());
                    readCameraComponent(
                        *entity, reinterpret_cast<const schema::resource::scene::CameraComponent*>(
                                     componentData->component()));
                    break;

                case schema::resource::scene::ComponentKind_LightComponent:
                    IVL_LOG(Trace, "Deserializing Entity ({}) Light component.", entity->getName());
                    readLightComponent(
                        *entity, reinterpret_cast<const schema::resource::scene::LightComponent*>(
                                     componentData->component()));
                    break;

                case schema::resource::scene::ComponentKind_ArmatureComponent:
                    IVL_LOG(Trace, "Deserializing Entity ({}) Armature component.",
                            entity->getName());
                    readArmatureComponent(
                        *entity, resourceManager,
                        reinterpret_cast<const schema::resource::scene::ArmatureComponent*>(
                            componentData->component()));
                    break;

                case schema::resource::scene::ComponentKind_ModelComponent:
                    IVL_LOG(Trace, "Deserializing Entity ({}) Model component.", entity->getName());
                    readModelComponent(
                        *entity, resourceManager,
                        reinterpret_cast<const schema::resource::scene::ModelComponent*>(
                            componentData->component()));
                    break;

                default:
                    IVL_LOG_THROW_ERROR(logic_error, "Unknown component kind");
            }
        }
    }

    messageLoop.createSystem<CameraSystem>();
    messageLoop.createSystem<CameraUserControlledSystem>();
    messageLoop.createSystem<CameraNodeSystem>(scene.getWorld(), sceneData->initialActiveCamera());
}
