#pragma once

#include <ipp/shared.hpp>
#include <ipp/loop/system.hpp>
#include <ipp/schema/primitive_generated.h>
#include <ipp/schema/message/animation_generated.h>
#include "action.hpp"
#include "track.hpp"
#include "sequence.hpp"

namespace ipp {
namespace scene {
namespace animation {

/**
 * @brief Animation system controls Scene state by sequencing animation Actions.
 */
class AnimationSystem final : public loop::SystemT<AnimationSystem> {
public:
    /**
     * @brief Empty type used as data argument for CommandT<Stop> for Stop command
     */
    struct Stop {
    };

    /**
     * @brief Re-export enumeration
     */
    enum class Status : uint32_t {
        Playing = ipp::schema::message::animation::AnimationStatus_Playing,
        Stopped = ipp::schema::message::animation::AnimationStatus_Stopped,
        Completed = ipp::schema::message::animation::AnimationStatus_Completed
    };

    /**
     * @brief Command that starts playing animation segment from start to end.
     */
    typedef loop::CommandT<ipp::schema::message::animation::AnimationPlayRange> PlayCommand;

    /**
     * @brief Command that updates animation time.
     * If animation time is outside of current play range animation is stopped.
     */
    typedef loop::CommandT<ipp::schema::message::animation::AnimationDeltaTime> UpdateCommand;

    /**
     * @brief Command that sets animation state to Stopped.
     */
    typedef loop::CommandT<Stop> StopCommand;

    /**
     * @brief Event dispatched on update after every animation state update (time or status).
     */
    typedef loop::EventT<ipp::schema::message::animation::AnimationState> StateUpdatedEvent;

private:
    Scene& _scene;

    SceneSequence _sceneSequence;
    std::vector<EntitySequence> _entitySequences;

    Milliseconds _playStart;
    Milliseconds _playEnd;
    Milliseconds _time;
    Milliseconds _duration;
    Status _status;

    /**
     * @brief System initialization implementation.
     */
    std::vector<loop::SystemBase*> initialize() override;

    /**
     * @brief Called by MessageLoop to push new messages.
     */
    void onMessage(const loop::Message& message) override;

    /**
     * @brief Called by onMessage to handle Play message.
     */
    void onPlay(Milliseconds start, Milliseconds end);

    /**
     * @brief
     */
    void onTimeUpdate(Milliseconds deltaTime);

    /**
     * @brief System update implemntation.
     */
    void onUpdate() override;

public:
    AnimationSystem(loop::MessageLoop& loop,
                    Scene& scene,
                    SceneSequence&& sceneSequence,
                    std::vector<EntitySequence> entitySequences,
                    Milliseconds duration);

    /**
     * @brief Owning Scene object.
     */
    Scene& getScene() const
    {
        return _scene;
    }

    /**
     * @brief Current animation time.
     */
    Milliseconds getTime() const
    {
        return _time;
    }

    /**
     * @brief Get animation system duration.
     */
    Milliseconds getDuration() const
    {
        return _duration;
    }

    /**
     * @brief Current animation status.
     */
    Status getStatus() const
    {
        return _status;
    }

    /**
     * @brief Currently playing animation start time
     */
    Milliseconds getPlayStart() const
    {
        return _playStart;
    }

    /**
     * @brief Currently playing animation end time
     */
    Milliseconds getPlayEnd() const
    {
        return _playEnd;
    }
};
}
}
}
