#pragma once

#include <ipp/shared.hpp>
#include "channel.hpp"

namespace ipp {
namespace scene {
namespace animation {

class Action final : public NonCopyable {
private:
    int _id;
    std::string _name;
    std::vector<std::unique_ptr<Channel>> _channels;

public:
    Action(int id, std::string name, std::vector<std::unique_ptr<Channel>> channels);

    Action(Action&& other)
        : _id{other._id}
        , _name{std::move(other._name)}
        , _channels{std::move(other._channels)}
    {
    }

    void updateTarget(Milliseconds time, Channel::Target target) const;

    int getId() const
    {
        return _id;
    }

    const std::string& getName() const
    {
        return _name;
    }

    const std::vector<std::unique_ptr<Channel>>& getChannels() const
    {
        return _channels;
    }
};
}
}
}
