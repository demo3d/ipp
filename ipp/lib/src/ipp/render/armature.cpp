#include <ipp/log.hpp>
#include <ipp/render/armature.hpp>
#include <ipp/schema/primitive_generated.h>
#include <ipp/schema/resource/render/armature_generated.h>

using namespace std;
using namespace glm;
using namespace ipp;
using namespace ipp::render;
using namespace ipp::resource;

template <>
const string SharedResourceT<Armature>::ResourceTypeName = "RenderArmature";

Armature::Bone::Pose::Pose()
    : translation{0, 0, 0}
    , rotation{}
    , scale{1}
{
}

Armature::Bone::Pose::Pose(vec3 translation, quat rotation, float scale)
    : translation{translation}
    , rotation{rotation}
    , scale{scale}
{
}

Armature::Bone::Pose::operator mat4() const
{
    return translate(translation) * toMat4(rotation) * ::glm::scale(vec3(scale));
}

Armature::Bone::Bone(string name,
                     size_t index,
                     int parentIndex,
                     bool isDeform,
                     const vec3& bindTranslation,
                     const quat& bindRotation,
                     float bindScale)
    : _name{move(name)}
    , _index{index}
    , _parentIndex{parentIndex}
    , _isDeform{isDeform}
    , _bindPose{bindTranslation, bindRotation, bindScale}
    , _inverseBindPose{inverse(static_cast<mat4>(_bindPose))}
{
    IVL_LOG(Trace, "Bone : {} with parent {} translation [{}] rotation [{}] scale [{}]", _name,
            parentIndex, glm::to_string(bindTranslation),
            glm::to_string(vec4(bindRotation.x, bindRotation.y, bindRotation.z, bindRotation.w)),
            bindScale);
}

Armature::Armature(unique_ptr<ResourceBuffer> data)
    : SharedResourceT<Armature>(data->getResourceManager(), data->getResourcePath())
{
    auto armatureData = schema::resource::render::armature::GetArmature(data->getData());

    for (auto boneData : *armatureData->bones()) {
        _bones.emplace_back(
            boneData->name()->str(), _bones.size(), boneData->parentIndex(),
            boneData->isDeformation(),
            vec3{boneData->bindPose()->translation().x(), boneData->bindPose()->translation().y(),
                 boneData->bindPose()->translation().z()},
            quat{boneData->bindPose()->rotation().w(), boneData->bindPose()->rotation().x(),
                 boneData->bindPose()->rotation().y(), boneData->bindPose()->rotation().z()},
            boneData->bindPose()->scale());
    }
}
