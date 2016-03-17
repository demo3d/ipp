#include <ipp/scene/camera/cameranodesystem.hpp>
#include <ipp/scene/node/nodesystem.hpp>
#include <ipp/entity/world.hpp>

using namespace std;
using namespace glm;
using namespace ipp::loop;
using namespace ipp::entity;
using namespace ipp::scene::node;
using namespace ipp::scene::camera;

template <>
const string CameraNodeSystem::SetActiveCommand::CommandTypeName =
    "SceneCameraNodeActiveSetCommand";
template <>
const string CameraNodeSystem::ActiveUpdatedEvent::EventTypeName =
    "SceneCameraNodeActiveUpdatedEvent";
template <>
const string SystemT<CameraNodeSystem>::SystemTypeName = "SceneCameraNodeSystem";

vector<SystemBase*> CameraNodeSystem::initialize()
{
    registerCommandT<SetActiveCommand>();
    registerEventT<ActiveUpdatedEvent>();

    auto nodeSystem = getMessageLoop().findSystem<NodeSystem>();

    IVL_LOG(Trace, "CameraNode system initialized");
    return {nodeSystem};
}

void CameraNodeSystem::onMessage(const Message& message)
{
    if (auto cameraEntity = getCommandData<SetActiveCommand>(message)) {
        uint32_t cameraEntityId = cameraEntity->entityId();
        assert(cameraEntityId != 0);
        IVL_LOG(Info, "Updating Camera system Active Entity ID to {}", cameraEntityId);

        auto& cameraEntities = _entityGroup->getEntities();
        auto cameraEntityIt =
            find_if(cameraEntities.begin(), cameraEntities.end(), [&](auto& cameraEntity) {
                return std::get<0>(cameraEntity)->getId() == cameraEntityId;
            });

        if (cameraEntityIt == cameraEntities.end()) {
            IVL_LOG(Error, "Unable to find Camera Entity with ID {}", cameraEntityId);
            return;
        }

        _active = std::get<1>(*cameraEntityIt);
        dispatchEventT<ActiveUpdatedEvent>(cameraEntityId);
        return;
    }
}

void CameraNodeSystem::onUpdate()
{
    for (auto& cameraEntity : _entityGroup->getEntities()) {
        auto camera = get<1>(cameraEntity);
        auto node = get<2>(cameraEntity);
        camera->updateFromNodeTransform(node->getTransformMatrix());
    }

    if (_active == nullptr && _defaultActiveEntityId != 0) {
        auto& cameraEntities = _entityGroup->getEntities();
        auto cameraEntityIt =
            find_if(cameraEntities.begin(), cameraEntities.end(), [&](auto& cameraEntity) {
                return std::get<0>(cameraEntity)->getId() == _defaultActiveEntityId;
            });

        if (cameraEntityIt == cameraEntities.end()) {
            return;
        }
        _active = std::get<1>(*cameraEntityIt);
        dispatchEventT<ActiveUpdatedEvent>(_defaultActiveEntityId);
    }
}

CameraNodeSystem::CameraNodeSystem(MessageLoop& messageLoop,
                                   World& world,
                                   uint32_t defaultActiveEntityId)
    : SystemT<CameraNodeSystem>(messageLoop)
    , _defaultActiveEntityId{defaultActiveEntityId}
    , _active{nullptr}
{
    _entityGroup = world.createEntityObserver<CameraNodeSystem::CameraNodeEntityGroup>();
}

glm::vec3 CameraNodeSystem::getViewPosition()
{
    if (_active == nullptr) {
        return glm::vec3{0, 0, 0};
    }
    return _active->getViewPosition();
}

glm::vec3 CameraNodeSystem::getViewDirection()
{
    if (_active == nullptr) {
        return glm::vec3{0, -1, 0};
    }
    return _active->getViewDirection();
}

glm::vec3 CameraNodeSystem::getViewUp()
{
    if (_active == nullptr) {
        return glm::vec3{0, 0, 1};
    }
    return _active->getViewUp();
}

const glm::mat4& CameraNodeSystem::getView()
{
    static const glm::mat4 _identity{1};
    if (_active == nullptr) {
        return _identity;
    }
    return _active->getView();
}

const glm::mat4& CameraNodeSystem::getProjection(int width, int height)
{
    static const glm::mat4 _identity{1};
    if (_active == nullptr) {
        return _identity;
    }
    return _active->getProjection(width, height);
}
