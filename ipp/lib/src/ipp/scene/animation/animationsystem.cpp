#include <ipp/log.hpp>
#include <ipp/loop/messageloop.hpp>
#include <ipp/scene/animation/channel.hpp>
#include <ipp/scene/animation/animationsystem.hpp>
#include <ipp/scene/node/nodesystem.hpp>
#include <ipp/scene/camera/camerasystem.hpp>

using namespace std;
using namespace std::chrono;
using namespace glm;
using namespace ipp;
using namespace ipp::loop;
using namespace ipp::scene;
using namespace ipp::scene::animation;

template <>
const string AnimationSystem::PlayCommand::CommandTypeName = "SceneAnimationPlayCommand";
template <>
const string AnimationSystem::UpdateCommand::CommandTypeName = "SceneAnimationUpdateCommand";
template <>
const string AnimationSystem::StopCommand::CommandTypeName = "SceneAnimationStopCommand";
template <>
const string AnimationSystem::StateUpdatedEvent::EventTypeName = "SceneAnimationStateUpdatedEvent";

template <>
const string SystemT<AnimationSystem>::SystemTypeName = "SceneAnimationSystem";

vector<SystemBase*> AnimationSystem::initialize()
{
    registerCommandT<PlayCommand>();
    registerCommandT<UpdateCommand>();
    registerCommandT<StopCommand>();
    registerEventT<StateUpdatedEvent>();

    IVL_LOG(Trace, "Animation system initialized");
    return {};
}

void AnimationSystem::onMessage(const Message& message)
{
    if (auto update = getCommandData<UpdateCommand>(message)) {
        onTimeUpdate(chrono::milliseconds{update->deltaTime()});
        return;
    }

    if (auto play = getCommandData<PlayCommand>(message)) {
        onPlay(milliseconds(play->start()), milliseconds(play->end()));
        return;
    }

    if (getCommandData<StopCommand>(message)) {
        _status = Status::Stopped;
        dispatchEventT<StateUpdatedEvent>(
            static_cast<uint32_t>(_time.count()),
            static_cast<ipp::schema::message::animation::AnimationStatus>(_status));
        return;
    }
}

void AnimationSystem::onPlay(Milliseconds start, Milliseconds end)
{
    _playStart = start;
    _playEnd = end == Milliseconds(0) ? _duration : end;
    _time = start;
    IVL_LOG(Trace, "Animation system received Play event from {} to {}, current status : {}",
            _playStart.count(), _playEnd.count(), static_cast<int>(_status));

    _status = Status::Playing;
}

void AnimationSystem::onTimeUpdate(Milliseconds deltaTime)
{
    if (_status == Status::Playing) {
        // only advance timer if status is playing
        _time += deltaTime;

        if (_time > _playEnd) {
            _time = _playEnd;
        }
    }
}

void AnimationSystem::onUpdate()
{
    // don't update animation system if animation isn't playing
    if (_status != Status::Playing) {
        return;
    }

    // update scene sequence
    _sceneSequence.update(_time);

    // update entity sequences
    for (auto& entitySequence : _entitySequences) {
        entitySequence.update(_time);
    }

    // if play end is reached update status to Completed
    if (_time == _playEnd) {
        _status = Status::Completed;
    }
    dispatchEventT<StateUpdatedEvent>(
        static_cast<uint32_t>(_time.count()),
        static_cast<ipp::schema::message::animation::AnimationStatus>(_status));
}

AnimationSystem::AnimationSystem(ipp::loop::MessageLoop& loop,
                                 Scene& scene,
                                 SceneSequence&& sceneSequence,
                                 vector<EntitySequence> entitySequences,
                                 Milliseconds duration)
    : ipp::loop::SystemT<AnimationSystem>(loop)
    , _scene{scene}
    , _sceneSequence{move(sceneSequence)}
    , _entitySequences{move(entitySequences)}
    , _time(0)
    , _duration{duration}
    , _status(Status::Stopped)
{
}
