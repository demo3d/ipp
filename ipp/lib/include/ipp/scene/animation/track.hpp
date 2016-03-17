#pragma once

#include <ipp/shared.hpp>
#include "action.hpp"

namespace ipp {
namespace scene {
namespace animation {

/**
 * @brief Track is a collection of animation Strips that bind animation Actions to targets.
 *
 * Strips cannot overlap in the track timeline, only one Action will be applied to Target
 * from Track at any time.
 */
class Track final : public NonCopyable {
public:
    /**
     * @brief Binds animation Action to Track target with specified time.
     */
    class Strip {
    private:
        std::string _name;
        Milliseconds _trackOffset;
        Milliseconds _trackDuration;
        size_t _actionIndex;
        Milliseconds _actionOffset;
        Milliseconds _actionDuration;

    public:
        Strip(std::string name,
              Milliseconds trackOffset,
              Milliseconds trackDuration,
              size_t actionIndex,
              Milliseconds actionOffset,
              Milliseconds actionDuration);

        /**
         * @brief Symbolic name string, for debugging purposes.
         */
        const std::string& getName() const
        {
            return _name;
        }

        /**
         * @brief Strip start time in Track timeline
         */
        Milliseconds getTrackOffset() const
        {
            return _trackOffset;
        }

        /**
         * @brief Strip duration
         */
        Milliseconds getTrackDuration() const
        {
            return _trackDuration;
        }

        /**
         * @brief Index of Action applied to target
         */
        size_t getActionIndex() const
        {
            return _actionIndex;
        }

        /**
         * @brief Strip offset from Action start.
         */
        Milliseconds getActionOffset() const
        {
            return _actionOffset;
        }

        /**
         * @brief Strip duration in Action time.
         *
         * Action animation duration is scaled to match Strip duration when playing a Strip.
         */
        Milliseconds getActionDuration() const
        {
            return _actionDuration;
        }
    };

private:
    std::string _name;
    std::vector<Strip> _strips;
    Channel::Target _target;
    std::vector<Strip>::iterator _stripLastUsed;
    bool _muted;

public:
    Track(std::string name, std::vector<Strip> strips, Channel::Target target);

    Track(Track&& other);

    /**
     * @brief Update Track target to animation state at time specified.
     */
    void update(const std::vector<Action>& actions, Milliseconds time);

    /**
     * @brief Symbolic track name string (for debugging).
     */
    const std::string& getName() const
    {
        return _name;
    }

    /**
     * @brief Track animation strips.
     *
     * Strips are chronologically ordered and do not overlap.
     */
    const std::vector<Strip>& getStrips() const
    {
        return _strips;
    }

    /**
     * @brief Track target Entity or Scene reference.
     */
    Channel::Target getTarget() const
    {
        return _target;
    }

    /**
     * @brief Track muted state determines if Track will apply it's effects to target.
     */
    bool isMuted() const
    {
        return _muted;
    }

    /**
     * @brief Set Track muted state.
     */
    void setMuted(bool muted)
    {
        _muted = muted;
    }
};
}
}
}
