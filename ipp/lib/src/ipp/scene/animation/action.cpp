#include <ipp/log.hpp>
#include <ipp/scene/animation/action.hpp>

using namespace std;
using namespace glm;
using namespace ipp::scene;
using namespace ipp::scene::animation;

Action::Action(int id, string name, vector<unique_ptr<Channel>> channels)
    : _id{id}
    , _name{move(name)}
    , _channels{move(channels)}
{
    IVL_LOG(Trace, "Creating action : {} id : {} channels : {}", _name, _id, _channels.size());
}

void Action::updateTarget(ipp::Milliseconds time, Channel::Target target) const
{
    for (auto channelIt = _channels.begin(); channelIt != _channels.end(); ++channelIt) {
        channelIt->get()->updateTarget(time, target);
    }
}
