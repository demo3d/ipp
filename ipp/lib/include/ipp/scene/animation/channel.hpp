#pragma once

#include <ipp/noncopyable.hpp>
#include <ipp/entity/entity.hpp>
#include "keyframe.hpp"

namespace ipp {
namespace scene {
namespace animation {

/**
 * @brief Abstract animation channel base class
 */
class Channel : public NonCopyable {
public:
    /**
     * @brief Typed pointer to targets controlled by Channel.
     */
    struct Target {
    public:
        enum class Type { Entity = 0, Scene };

    private:
        union {
            entity::Entity* _entity;
            Scene* _scene;
        };
        Type _type;

    public:
        Target() = delete;

        Target(entity::Entity& entity)
            : _entity{&entity}
            , _type{Type::Entity}
        {
        }

        Target(Scene& scene)
            : _scene{&scene}
            , _type{Type::Scene}
        {
        }

        entity::Entity& getEntity() const
        {
            assert(_type == Type::Entity);
            return *_entity;
        }

        Scene& getScene() const
        {
            assert(_type == Type::Scene);
            return *_scene;
        }

        Type getType() const
        {
            return _type;
        }
    };

public:
    virtual ~Channel() = default;

    /**
     * @brief Update specified target to animation value at time specified.
     */
    virtual void updateTarget(Milliseconds time, Target target) = 0;
};

/**
 * @brief Template implementation for key frame based animation channel.
 *
 * Type T is target property type and K is key frame type.
 * Update function is called to update target with value obtained from interpolating two key frames.
 * Channel has a propertyPath that is passed to update function and can be used by property
 * function to determine which property of target needs to be updated.
 */
template <typename T,
          typename K,
          void (*Update)(T value, size_t propertyPath, Channel::Target target)>
class KeyFrameChannel : public Channel {
private:
    size_t _propertyPath;
    std::vector<K> _keyFrames;

public:
    KeyFrameChannel(size_t propertyPath, std::vector<K> keyFrames)
        : _propertyPath{propertyPath}
        , _keyFrames{std::move(keyFrames)}
    {
    }

    /**
     * @brief Update channel to specified time.
     */
    void updateTarget(Milliseconds time, Channel::Target target) override
    {
        auto it = _keyFrames.begin();
        auto itPrevious = it;
        while (it != _keyFrames.end() && it->getTime() < time) {
            itPrevious = it;
            ++it;
        }

        float t = 1;
        if (it == _keyFrames.end()) {
            it = itPrevious;
        }
        else {
            auto dt = time - itPrevious->getTime();
            auto duration = it->getTime() - itPrevious->getTime();
            t = static_cast<float>(dt.count()) / static_cast<float>(duration.count());
            t = std::max(0.0f, std::min(1.0f, t));
        }
        Update(itPrevious->getValueAt(*it, t), _propertyPath, target);
    }

    /**
     * @brief Channel target property path.
     */
    size_t getPropertyPath() const
    {
        return _propertyPath;
    }

    /**
     * @brief Chronologically ordered keyframe vector.
     */
    const std::vector<K>& getKeyFrames() const
    {
        return _keyFrames;
    }
};
}
}
}
