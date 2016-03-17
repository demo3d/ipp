#pragma once

#include <ipp/shared.hpp>
#include <ipp/render/armature.hpp>

namespace ipp {
namespace scene {
namespace animation {

/**
 * @brief A pair of data T and time in Channel timeline.
 */
template <typename T>
class ConstKeyFrameT final {
private:
    Milliseconds _time;
    T _value;

public:
    ConstKeyFrameT(Milliseconds time, T value)
        : _time{time}
        , _value{value}
    {
    }

    /**
     * @brief Key frame value.
     */
    T getValue() const
    {
        return _value;
    }

    /**
     * @brief Key frame value at time t (normalized 0-1) between this and next keyframe.
     *
     * Uses const interpolation, meaning always returns value of current keyframe.
     */
    T getValueAt(const ConstKeyFrameT<T>& next, float t) const
    {
        return _value;
    }

    /**
     * @brief Key frame time.
     */
    Milliseconds getTime() const
    {
        return _time;
    }
};

/**
 * @brief Specialized key frame type that implements bezier float interpolation mode.
 */
class FloatKeyFrame final {
public:
    enum class InterpolationMode { Constant = 0, Linear, Bezier };

private:
    Milliseconds _time;
    float _value;
    InterpolationMode _interpolationMode;
    glm::vec2 _bezierLeft;
    glm::vec2 _bezierRight;

public:
    FloatKeyFrame(Milliseconds time,
                  float value,
                  InterpolationMode interpolationMode,
                  glm::vec2 bezierLeft,
                  glm::vec2 bezierRight)
        : _time{time}
        , _value{value}
        , _interpolationMode{interpolationMode}
        , _bezierLeft{bezierLeft}
        , _bezierRight{bezierRight}
    {
    }

    /**
     * @brief Key frame time.
     */
    Milliseconds getTime() const
    {
        return _time;
    }

    /**
     * @brief Key frame value at time.
     */
    float getValue() const
    {
        return _value;
    }

    /**
     * @brief Key frame value at time t (normalized 0-1) between this and next key frame.
     *
     * Interpolation method is determined by interpolation mode for the key frame.
     */
    float getValueAt(const FloatKeyFrame& next, float t) const;

    /**
     * @brief Interpolation mode between this and next point.
     */
    InterpolationMode getInterpolationMode() const
    {
        return _interpolationMode;
    }

    /**
     * @brief Bezier interpolation left control (interpolation between previous point and this one).
     */
    glm::vec2 getBezierLeft()
    {
        return _bezierLeft;
    }

    /**
     * @brief Bezier interpolation right control (interpolation between next point and this one).
     */
    glm::vec2 getBezierRight()
    {
        return _bezierRight;
    }
};

/**
 * @brief Specialized key frame implementation that implements skeleton bone pose interpolation.
 */
class BonePoseKeyFrame {
private:
    Milliseconds _time;
    ipp::render::Armature::Bone::Pose _pose;

public:
    BonePoseKeyFrame(Milliseconds time, glm::vec3 translation, glm::quat rotation, float scale);
    /**
     * @brief Key frame time.
     */
    Milliseconds getTime() const
    {
        return _time;
    }

    /**
     * @brief Bone pose
     */
    const ipp::render::Armature::Bone::Pose& getPose() const
    {
        return _pose;
    }

    /**
     * @brief Interpolate between two bone poses and return bone space matrix
     */
    ipp::render::Armature::Bone::Pose getValueAt(const BonePoseKeyFrame& next, float t) const;
};
}
}
}
