#include <ipp/scene/animation/track.hpp>

using namespace std;
using namespace glm;
using namespace ipp;
using namespace ipp::scene;
using namespace ipp::scene::animation;

Track::Strip::Strip(string name,
                    Milliseconds trackOffset,
                    Milliseconds trackDuration,
                    size_t actionIndex,
                    Milliseconds actionOffset,
                    Milliseconds actionDuration)
    : _name{move(name)}
    , _trackOffset{trackOffset}
    , _trackDuration{trackDuration}
    , _actionIndex{actionIndex}
    , _actionOffset{actionOffset}
    , _actionDuration{actionDuration}
{
}

Track::Track(string name, std::vector<Strip> strips, Channel::Target target)
    : _name{move(name)}
    , _strips{move(strips)}
    , _target{target}
    , _stripLastUsed{_strips.end()}
{
    // empty tracks should be filtered out by exporter and are not valid input
    assert(_strips.size() > 0);
}

Track::Track(Track&& other)
    : _name{std::move(other._name)}
    , _strips{std::move(other._strips)}
    , _target{other._target}
    , _stripLastUsed{_strips.end()}
    , _muted{other._muted}
{
}

void Track::update(const std::vector<Action>& actions, Milliseconds time)
{
    // if last used strip doesn't match time find new one that matches
    if (_stripLastUsed == _strips.end() || time < _stripLastUsed->getTrackOffset() ||
        time > _stripLastUsed->getTrackOffset() + _stripLastUsed->getTrackDuration()) {
        _stripLastUsed = _strips.begin();

        while (_stripLastUsed != _strips.end() && time > _stripLastUsed->getTrackOffset()) {
            _stripLastUsed++;
        }

        if (_stripLastUsed != _strips.begin()) {
            _stripLastUsed--;
        }
    }

    // convert from track time to strip time
    auto stripTrackOffset = _stripLastUsed->getTrackOffset();
    auto stripTrackDuration = _stripLastUsed->getTrackDuration();
    auto stripTime = std::min(time - stripTrackOffset, stripTrackDuration + Milliseconds(1));

    // convert stript time to action time
    auto stripActionOffset = _stripLastUsed->getActionOffset();
    auto stripActionDuration = _stripLastUsed->getActionDuration();
    auto stripTimeDurationRatio =
        static_cast<double>(stripTime.count()) / static_cast<double>(stripTrackDuration.count());
    auto actionTime =
        stripActionOffset + chrono::milliseconds(static_cast<int64_t>(stripActionDuration.count() *
                                                                      stripTimeDurationRatio));

    auto& action = actions[_stripLastUsed->getActionIndex()];
    auto actionName = action.getName();

    // apply action animation to target
    action.updateTarget(actionTime, _target);
}
