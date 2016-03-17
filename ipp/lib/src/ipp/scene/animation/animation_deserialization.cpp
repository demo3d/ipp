#include <ipp/log.hpp>
#include <ipp/scene/render/material.hpp>
#include <ipp/scene/render/renderablecomponent.hpp>
#include <ipp/scene/render/rendersystem.hpp>
#include <ipp/scene/camera/camerasystem.hpp>
#include <ipp/scene/node/nodesystem.hpp>

#include <ipp/scene/animation/keyframe.hpp>
#include <ipp/scene/animation/action.hpp>
#include <ipp/scene/animation/channel.hpp>
#include <ipp/scene/animation/track.hpp>
#include <ipp/scene/animation/sequence.hpp>
#include <ipp/scene/animation/animationsystem.hpp>

#include <ipp/scene/scene.hpp>

#include <ipp/schema/primitive_generated.h>
#include <ipp/schema/resource/scene/animation_generated.h>

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
using namespace ipp::scene::animation;

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

inline quat readQuat(const schema::primitive::Quat* data)
{
    if (data == nullptr)
        return {};
    return {data->w(), data->x(), data->y(), data->z()};
}

inline mat4 readMat4(const schema::primitive::Mat4* data)
{
    if (data == nullptr)
        return {};
    return {readVec4(&data->row0()), readVec4(&data->row1()), readVec4(&data->row2()),
            readVec4(&data->row3())};
}

/**
 * @brief Applies channel update value to target Node Component hidden property.
 */
void updateNodeHidden(bool value, size_t, Channel::Target target)
{
    auto& entity = target.getEntity();
    auto node = entity.findComponent<NodeComponent>();
    node->setHidden(value);
}

/**
 * @brief Read Action animation Channel targeting Entity Node component Hidden property.
 */
std::unique_ptr<Channel> readActionChannelNodeHidden(
    const ipp::schema::resource::scene::NodeHiddenChannel* channelData)
{
    vector<ConstKeyFrameT<bool>> keyFrames;
    for (auto keyFrameData : *channelData->keyFrames()) {
        keyFrames.push_back(ConstKeyFrameT<bool>(chrono::milliseconds(keyFrameData->time()),
                                                 keyFrameData->value()));
    }
    return make_unique<KeyFrameChannel<bool, ConstKeyFrameT<bool>, &updateNodeHidden>>(0,
                                                                                       keyFrames);
}

/**
 * @brief Applies channel update value to target Node Transform.translation property.
 *
 * Property path determines which vector component is updated.
 */
void updateTransformTranslation(float value, size_t propertyPath, Channel::Target target)
{
    auto& entity = target.getEntity();
    auto node = entity.findComponent<NodeComponent>();
    node->getTransform().translation[propertyPath] = value;
}

/**
 * @brief Applies channel update value to target Node Transform.rotation property.
 *
 * Property path determines which vector component is updated.
 */
void updateTransformRotation(float value, size_t propertyPath, Channel::Target target)
{
    auto& entity = target.getEntity();
    auto node = entity.findComponent<NodeComponent>();
    node->getTransform().rotation[propertyPath] = value;
}

/**
 * @brief Applies channel update value to target Node Transform.scale property.
 *
 * Property path determines which vector component is updated.
 */
void updateTransformScale(float value, size_t propertyPath, Channel::Target target)
{
    auto& entity = target.getEntity();
    auto node = entity.findComponent<NodeComponent>();
    node->getTransform().scale[propertyPath] = value;
}

/**
 * @brief Read Action animation Channel targeting Entity Node component Transform property.
 */
std::unique_ptr<Channel> readActionChannelNodeTransform(
    const ipp::schema::resource::scene::NodeTransformChannel* channelData)
{
    vector<FloatKeyFrame> keyFrames;
    for (auto keyFrameData : *channelData->keyFrames()) {
        FloatKeyFrame::InterpolationMode keyFrameInterpolation;

#define MAP_KEY_FRAME_INTERPOLATION_MODE(MODE)                                                     \
    case ipp::schema::resource::scene::InterpolationMode_##MODE:                                   \
        keyFrameInterpolation = FloatKeyFrame::InterpolationMode::MODE;                            \
        break;

        switch (keyFrameData->interpolationMode()) {
            MAP_KEY_FRAME_INTERPOLATION_MODE(Constant)
            MAP_KEY_FRAME_INTERPOLATION_MODE(Linear)
            MAP_KEY_FRAME_INTERPOLATION_MODE(Bezier)
            default:
                throw logic_error("Unknown float keyframe interpolation mode");
        }
#undef MAP_KEY_FRAME_INTERPOLATION_MODE

        keyFrames.push_back(FloatKeyFrame(chrono::milliseconds(keyFrameData->time()),
                                          keyFrameData->value(), keyFrameInterpolation,
                                          readVec2(&keyFrameData->bezierLeft()),
                                          readVec2(&keyFrameData->bezierRight())));
    }

#define CREATE_PROPERTY_CHANNEL(PROPERTY, PROPERTY_PATH, UPDATE)                                   \
    case ipp::schema::resource::scene::NodeTransformProperty_##PROPERTY:                           \
        return make_unique<KeyFrameChannel<float, FloatKeyFrame, &UPDATE>>(PROPERTY_PATH,          \
                                                                           keyFrames);

    switch (channelData->property()) {
        CREATE_PROPERTY_CHANNEL(TranslationX, 0, updateTransformTranslation)
        CREATE_PROPERTY_CHANNEL(TranslationY, 1, updateTransformTranslation)
        CREATE_PROPERTY_CHANNEL(TranslationZ, 2, updateTransformTranslation)

        CREATE_PROPERTY_CHANNEL(RotationEulerX, 0, updateTransformRotation)
        CREATE_PROPERTY_CHANNEL(RotationEulerY, 1, updateTransformRotation)
        CREATE_PROPERTY_CHANNEL(RotationEulerZ, 2, updateTransformRotation)

        CREATE_PROPERTY_CHANNEL(RotationQuaternionX, 0, updateTransformRotation)
        CREATE_PROPERTY_CHANNEL(RotationQuaternionY, 1, updateTransformRotation)
        CREATE_PROPERTY_CHANNEL(RotationQuaternionZ, 2, updateTransformRotation)
        CREATE_PROPERTY_CHANNEL(RotationQuaternionW, 3, updateTransformRotation)

        CREATE_PROPERTY_CHANNEL(RotationAxisAngleX, 0, updateTransformRotation)
        CREATE_PROPERTY_CHANNEL(RotationAxisAngleY, 1, updateTransformRotation)
        CREATE_PROPERTY_CHANNEL(RotationAxisAngleZ, 2, updateTransformRotation)
        CREATE_PROPERTY_CHANNEL(RotationAxisAngleAngle, 3, updateTransformRotation)

        CREATE_PROPERTY_CHANNEL(ScaleX, 0, updateTransformScale)
        CREATE_PROPERTY_CHANNEL(ScaleY, 1, updateTransformScale)
        CREATE_PROPERTY_CHANNEL(ScaleZ, 2, updateTransformScale)

        default:
            throw std::logic_error("Unknown animation node transform property");
    }

#undef CREATE_PROPERTY_CHANNEL
}

/**
 * @brief Applies channel update value to target Armature SkinningPoses property.
 *
 * Property path determines which Pose (bone) index is updated.
 */
void updateBonePose(Armature::Bone::Pose value, size_t propertyPath, Channel::Target target)
{
    auto& entity = target.getEntity();
    auto armature = entity.findComponent<ArmatureComponent>();
    armature->getBonePoses()[propertyPath] = value;
}

/**
 * @brief Read Action animation Channel targeting Entity Armature component SkinningPoses property.
 */
std::unique_ptr<Channel> readActionChannelBonePose(
    const ipp::schema::resource::scene::BonePoseChannel* channelData)
{
    vector<BonePoseKeyFrame> keyFrames;
    for (auto keyFrameData : *channelData->keyFrames()) {
        keyFrames.emplace_back(chrono::milliseconds(keyFrameData->time()),
                               readVec3(&keyFrameData->translation()),
                               readQuat(&keyFrameData->rotation()), keyFrameData->scale());
    }
    return make_unique<KeyFrameChannel<Armature::Bone::Pose, BonePoseKeyFrame, &updateBonePose>>(
        channelData->bone(), keyFrames);
}

/**
 * @brief Applies channel update value to scene active camera.
 */
void updateActiveCamera(uint32_t value, size_t propertyPath, Channel::Target target)
{
    auto& scene = target.getScene();
    scene.getMessageLoop().enqueueCommandT<CameraNodeSystem::SetActiveCommand>(value);
}

/**
 * @brief
 */
unique_ptr<Channel> readActionChannelActiveCamera(
    const schema::resource::scene::ActiveCameraChannel* channelData)
{
    vector<ConstKeyFrameT<uint32_t>> keyFrames;
    for (auto keyFrameData : *channelData->keyFrames()) {
        keyFrames.emplace_back(chrono::milliseconds(keyFrameData->time()),
                               static_cast<uint32_t>(keyFrameData->value()));
    }

    return make_unique<KeyFrameChannel<uint32_t, ConstKeyFrameT<uint32_t>, &updateActiveCamera>>(
        0, keyFrames);
}

/**
 * @brief Read animation track for specified target.
 */
Track readAnimationTrack(Channel::Target target, const schema::resource::scene::Track* trackData)
{
    vector<Track::Strip> strips;
    for (auto stripData : *trackData->strips()) {
        IVL_LOG(Trace,
                "Reading animation strip name {} offset {} duration {}, action id {} offset {} "
                "duration {}",
                stripData->name()->str(), stripData->trackOffset(), stripData->trackDuration(),
                stripData->actionIndex(), stripData->actionOffset(), stripData->actionDuration());

        strips.emplace_back(
            stripData->name()->str(), chrono::milliseconds{stripData->trackOffset()},
            chrono::milliseconds{stripData->trackDuration()}, stripData->actionIndex(),
            chrono::milliseconds{stripData->actionOffset()},
            chrono::milliseconds{stripData->actionDuration()});
    }
    return Track(trackData->name()->str(), move(strips), target);
}

/**
 * @brief Read animation tracks for entity as entity sequence.
 */
EntitySequence readAnimationEntitySequence(
    Entity& entity, const schema::resource::scene::EntitySequence* sequenceData)
{
    vector<Action> actions;
    unordered_map<int, Action*> actionIdMap;
    for (auto actionData : *sequenceData->actions()) {
        IVL_LOG(Trace, "Reading Action {} for Entity {}", entity.getName(),
                actionData->name()->str());

        vector<unique_ptr<Channel>> channels;
        for (auto channelData : *actionData->channels()) {
            std::unique_ptr<Channel> channel;
            switch (channelData->channel_type()) {
                case schema::resource::scene::ChannelKind_NodeHiddenChannel:
                    channel = readActionChannelNodeHidden(
                        reinterpret_cast<const schema::resource::scene::NodeHiddenChannel*>(
                            channelData->channel()));
                    break;

                case schema::resource::scene::ChannelKind_NodeTransformChannel:
                    channel = readActionChannelNodeTransform(
                        reinterpret_cast<const schema::resource::scene::NodeTransformChannel*>(
                            channelData->channel()));
                    break;

                case schema::resource::scene::ChannelKind_BonePoseChannel:
                    channel = readActionChannelBonePose(
                        reinterpret_cast<const schema::resource::scene::BonePoseChannel*>(
                            channelData->channel()));
                    break;

                default:
                    IVL_LOG_THROW_ERROR(logic_error, "Invalid Entity action channel {}",
                                        static_cast<int>(channelData->channel_type()));
            }
            channels.push_back(move(channel));
        }

        auto actionIndex = actions.size();
        actions.emplace_back(actionIndex, actionData->name()->str(), move(channels));
    }

    vector<Track> tracks;
    for (auto trackData : *sequenceData->tracks()) {
        IVL_LOG(Trace, "Reading Animation Track : {} for Entity : {}", entity.getName(),
                trackData->name()->str());
        tracks.emplace_back(readAnimationTrack(Channel::Target(entity), trackData));
    }

    return EntitySequence(entity, move(actions), move(tracks));
}

SceneSequence readSceneSequence(Scene& scene,
                                const schema::resource::scene::SceneSequence* sequenceData)
{
    vector<Action> actions;
    unordered_map<int, Action*> actionIdMap;
    for (auto actionData : *sequenceData->actions()) {
        vector<unique_ptr<Channel>> channels;
        for (auto channelData : *actionData->channels()) {
            std::unique_ptr<Channel> channel;
            switch (channelData->channel_type()) {
                case schema::resource::scene::ChannelKind_ActiveCameraChannel:
                    channel = readActionChannelActiveCamera(
                        reinterpret_cast<const schema::resource::scene::ActiveCameraChannel*>(
                            channelData->channel()));
                    break;

                default:
                    IVL_LOG_THROW_ERROR(logic_error, "Invalid Scene action channel {}",
                                        static_cast<int>(channelData->channel_type()));
            }
            channels.push_back(move(channel));
        }

        auto actionIndex = actions.size();
        actions.emplace_back(actionIndex, actionData->name()->str(), move(channels));
    }

    vector<Track> sceneTracks;
    for (auto trackData : *sequenceData->tracks()) {
        sceneTracks.emplace_back(readAnimationTrack(Channel::Target(scene), trackData));
    }

    return SceneSequence(scene, move(actions), move(sceneTracks));
}

/**
 * @brief Read animation sequence and actions creating AnimationSystem for scene
 */
void readAnimation(Scene& scene, const ::ipp::schema::resource::scene::Animation* animationData)
{
    vector<EntitySequence> entitySequences;
    for (auto entitySequenceData : *animationData->entitiySequences()) {
        auto entity = scene.getWorld().findEntity(entitySequenceData->entity());
        entitySequences.emplace_back(readAnimationEntitySequence(*entity, entitySequenceData));
    }

    scene.getMessageLoop().createSystem<AnimationSystem>(
        scene, readSceneSequence(scene, animationData->sceneSequence()), move(entitySequences),
        chrono::milliseconds{animationData->duration()});
}
