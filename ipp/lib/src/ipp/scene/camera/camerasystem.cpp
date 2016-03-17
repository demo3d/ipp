#include <ipp/scene/camera/camerasystem.hpp>
#include <ipp/scene/node/nodesystem.hpp>
#include <ipp/scene/render/rendersystem.hpp>
#include <ipp/loop/messageloop.hpp>
#include <ipp/entity/world.hpp>

using namespace std;
using namespace glm;
using namespace ipp::loop;
using namespace ipp::entity;
using namespace ipp::scene::node;
using namespace ipp::scene::camera;
using namespace ipp::scene::render;

template <>
const string CameraSystem::SetActiveTypeCommand::CommandTypeName =
    "SceneCameraActiveTypeSetCommand";
template <>
const string CameraSystem::ActiveTypeUpdatedEvent::EventTypeName =
    "SceneCameraActiveTypeUpdatedEvent";
template <>
const string SystemT<CameraSystem>::SystemTypeName = "SceneCameraSystem";

vector<SystemBase*> CameraSystem::initialize()
{
    registerCommandT<SetActiveTypeCommand>();
    registerEventT<ActiveTypeUpdatedEvent>();

    _userControlledCamera = getMessageLoop().findSystem<CameraUserControlledSystem>();
    _nodeCamera = getMessageLoop().findSystem<CameraNodeSystem>();
    _activeCamera = _userControlledCamera;

    IVL_LOG(Trace, "Camera system initialized");
    return {_userControlledCamera, _nodeCamera};
}

void CameraSystem::onMessage(const Message& message)
{
    if (auto cameraTypeMessage = getCommandData<SetActiveTypeCommand>(message)) {
        auto cameraType = *cameraTypeMessage;
        if (_activeCamera == nullptr || _activeCamera->getCameraType() == cameraType) {
            return;
        }
        switch (cameraType) {
            case CameraType::UserControlled:
                _userControlledCamera->resetView(_activeCamera->getViewPosition(),
                                                 _activeCamera->getViewDirection(),
                                                 _activeCamera->getViewUp());
                _activeCamera = _userControlledCamera;
                break;

            case CameraType::EntityComponent:
                _activeCamera = _nodeCamera;
                break;
        }

        dispatchEventT<ActiveTypeUpdatedEvent>(
            static_cast<ipp::schema::message::camera::CameraType>(_activeCamera->getCameraType()));

        return;
    }
}

CameraSystem::CameraSystem(MessageLoop& messageLoop)
    : SystemT<CameraSystem>(messageLoop)
    , _activeCamera{nullptr}
{
}
