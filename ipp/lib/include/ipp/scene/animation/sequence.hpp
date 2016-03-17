#pragma once

#include <ipp/shared.hpp>
#include "track.hpp"

namespace ipp {
namespace scene {
namespace animation {

/**
 * @brief Collection of Actions nad Tracks that target properties of T.
 */
template <typename T>
class Sequence {
private:
    T& _target;
    std::vector<Action> _actions;
    std::vector<Track> _tracks;

public:
    Sequence(T& target, std::vector<Action> actions, std::vector<Track> tracks)
        : _target{target}
        , _actions{std::move(actions)}
        , _tracks{std::move(tracks)}
    {
    }

    Sequence(Sequence<T>&& other)
        : _target{other._target}
        , _actions{std::move(other._actions)}
        , _tracks{std::move(other._tracks)}
    {
    }

    /**
     * @brief Update sequence tracks.
     */
    void update(Milliseconds time)
    {
        for (auto& track : _tracks) {
            track.update(_actions, time);
        }
    }

    /**
     * @brief Target T reference animated by this sequence.
     */
    T& getTarget() const
    {
        return _target;
    }

    /**
     * @brief Sequence collection of Actions that animate sequence target.
     */
    const std::vector<Action> getActions() const
    {
        return _actions;
    }

    /**
     * @brief Sequence collection of Tracks that animate sequence target.
     */
    const std::vector<Track>& getTracks() const
    {
        return _tracks;
    }
};

using EntitySequence = Sequence<entity::Entity>;
using SceneSequence = Sequence<Scene>;
}
}
}
