#include <ipp/scene/animation/keyframe.hpp>

using namespace std;
using namespace glm;
using namespace ipp::scene::animation;

float FloatKeyFrame::getValueAt(const FloatKeyFrame& next, float t) const
{
    switch (_interpolationMode) {
        case InterpolationMode::Constant:
            return _value;
        case InterpolationMode::Linear:
        case InterpolationMode::Bezier:
            // TODO: Actually implement bezier interpolation
            return _value * (1 - t) + next._value * t;
    }
}

BonePoseKeyFrame::BonePoseKeyFrame(ipp::Milliseconds time,
                                   vec3 translation,
                                   quat rotation,
                                   float scale)
    : _time{time}
    , _pose{translation, rotation, scale}
{
}

ipp::render::Armature::Bone::Pose BonePoseKeyFrame::getValueAt(const BonePoseKeyFrame& next,
                                                               float t) const
{
    auto translation = mix(_pose.translation, next._pose.translation, t);
    auto rotation = slerp(_pose.rotation, next._pose.rotation, t);
    auto scale = mix(_pose.scale, next._pose.scale, t);

    return {translation, rotation, scale};
}
