#include <ipp/scene/camera/camerausercontrolledsystem.hpp>
#include <ipp/log.hpp>
#include <limits>

using namespace std;
using namespace ipp::loop;
using namespace ipp::scene::camera;

template <>
const string CameraUserControlledSystem::MoveCommand::CommandTypeName =
    "SceneCameraUserControlledMoveCommand";
template <>
const string CameraUserControlledSystem::RotatePolarCommand::CommandTypeName =
    "SceneCameraUserControlledRotatePolarCommand";
template <>
const string CameraUserControlledSystem::RotationStartCommand::CommandTypeName =
    "SceneCameraUserControlledRotationArcballStartCommand";
template <>
const string CameraUserControlledSystem::RotationUpdateCommand::CommandTypeName =
    "SceneCameraUserControlledRotationArcballUpdateCommand";
template <>
const string CameraUserControlledSystem::ZoomCommand::CommandTypeName =
    "SceneCameraUserControlledZoomCommand";
template <>
const string CameraUserControlledSystem::StateSetCommand::CommandTypeName =
    "SceneCameraUserControlledStateSetCommand";
template <>
const string CameraUserControlledSystem::LimitsSetCommand::CommandTypeName =
    "SceneCameraUserControlledLimitsSetCommand";
template <>
const string CameraUserControlledSystem::StateUpdatedEvent::EventTypeName =
    "SceneCameraUserControlledStateUpdatedEvent";
template <>
const string CameraUserControlledSystem::LimitsUpdatedEvent::EventTypeName =
    "SceneCameraUserControlledLimitsUpdatedEvent";
template <>
const string SystemT<CameraUserControlledSystem>::SystemTypeName =
    "SceneCameraUserControlledSystem";

vector<SystemBase*> CameraUserControlledSystem::initialize()
{
    registerCommandT<MoveCommand>();
    registerCommandT<RotatePolarCommand>();
    registerCommandT<RotationStartCommand>();
    registerCommandT<RotationUpdateCommand>();
    registerCommandT<ZoomCommand>();

    registerCommandT<StateSetCommand>();
    registerCommandT<LimitsSetCommand>();

    registerEventT<StateUpdatedEvent>();
    registerEventT<LimitsUpdatedEvent>();

    IVL_LOG(Trace, "CameraUserControlled system initialized");

    return {};
}

void CameraUserControlledSystem::onMessage(const Message& message)
{
    if (auto move = getCommandData<MoveCommand>(message)) {
        onMove({move->deltaPosition().x(), move->deltaPosition().y(), move->deltaPosition().z()});
        return;
    }

    if (auto rotation = getCommandData<RotatePolarCommand>(message)) {
        onRotatePolar(rotation->deltaLatitude(), rotation->deltaLongitude());
        return;
    }

    if (auto rotation = getCommandData<RotationStartCommand>(message)) {
        onRotationArcballStart({rotation->point().x(), rotation->point().y()});
        return;
    }

    if (auto rotation = getCommandData<RotationUpdateCommand>(message)) {
        onRotationArcballUpdate({rotation->point().x(), rotation->point().y()});
        return;
    }

    if (auto zoom = getCommandData<ZoomCommand>(message)) {
        onZoom(zoom->distance());
        return;
    }

    if (auto state = getCommandData<StateSetCommand>(message)) {
        onStateSet(*state);
        return;
    }

    if (auto limits = getCommandData<LimitsSetCommand>(message)) {
        onLimitsSet(*limits);
        return;
    }
}

void CameraUserControlledSystem::onMove(glm::vec3 delta)
{
    glm::vec3 offset = _viewDirection * delta.z + _viewUp * delta.y +
                       (glm::cross(_viewUp, _viewDirection) * delta.x);
    glm::vec3 target = _viewTarget + offset;
    if (glm::clamp(target, _viewTargetMin, _viewTargetMax) != target) {
        return;
    }
    _viewTarget = target;
    _viewPosition += offset;

    updateView();
}

void CameraUserControlledSystem::onRotatePolar(float deltaLatitude, float deltaLongitude)
{
    deltaLatitude = glm::radians(deltaLatitude);
    deltaLongitude = glm::radians(deltaLongitude);
    glm::vec3 right = glm::cross(_viewUp, _viewDirection);
    glm::mat3 rotation = (glm::mat3{glm::axisAngleMatrix(right, deltaLatitude)} *
                          glm::mat3{glm::axisAngleMatrix(_viewUp, deltaLongitude)}) *
                         glm::mat3{right, glm::cross(_viewDirection, right), _viewDirection};

    glm::vec3 positionToTarget = _viewPosition - _viewTarget;
    positionToTarget = rotation[2] * glm::length(positionToTarget);
    glm::vec3 viewPosition = _viewTarget - positionToTarget;
    if (glm::clamp(viewPosition, _viewPositionMin, _viewPositionMax) != viewPosition) {
        return;
    }

    float newUpAngle = glm::dot(glm::vec3{0, 0, 1}, -rotation[2]);
    float oldUpAngle = glm::dot(glm::vec3{0, 0, 1}, -_viewDirection);
    if ((newUpAngle < _rotationPolarUpAngleCosMin && newUpAngle < oldUpAngle) ||
        (newUpAngle > _rotationPolarUpAngleCosMax && newUpAngle > oldUpAngle)) {
        return;
    }

    _viewUp = glm::vec3{0, 0, 1};
    _viewPosition = viewPosition;

    updateView();
}

glm::vec3 projectArcballPoint(glm::vec2 screenPoint)
{
    glm::vec3 arcballPoint{screenPoint.x, screenPoint.y, 0};
    float screenPointLenthSquared = glm::length2(screenPoint);
    if (screenPointLenthSquared <= 1) {
        arcballPoint.z = sqrt(1 - screenPointLenthSquared);
    }
    else {
        arcballPoint = glm::normalize(arcballPoint);
    }
    return arcballPoint;
}

void CameraUserControlledSystem::onRotationArcballStart(glm::vec2 point)
{
    _rotationPrevious = point;
}

void CameraUserControlledSystem::onRotationArcballUpdate(glm::vec2 point)
{
    glm::vec3 arcballPointA = projectArcballPoint(_rotationPrevious),
              arcballPointB = projectArcballPoint(point);

    if (glm::distance(arcballPointB, arcballPointA) > 0.001) {
        glm::vec3 right = glm::cross(_viewUp, _viewDirection);

        glm::vec3 rotationNormal = glm::normalize(glm::cross(arcballPointA, arcballPointB));
        glm::mat3 rotation = glm::toMat3(glm::angleAxis(
            acos(glm::clamp(glm::dot(arcballPointA, arcballPointB), -1.0f, 1.0f)), rotationNormal));
        rotation = glm::mat3{right, glm::cross(_viewDirection, right), _viewDirection} * rotation;

        glm::vec3 positionToTarget = _viewPosition - _viewTarget;
        positionToTarget = rotation[2] * glm::length(positionToTarget);
        glm::vec3 viewPosition = _viewTarget - positionToTarget;
        if (glm::clamp(viewPosition, _viewPositionMin, _viewPositionMax) != viewPosition) {
            return;
        }
        _viewPosition = viewPosition;
        _viewUp = rotation[1];
        _rotationPrevious = point;

        updateView();
    }
}

void CameraUserControlledSystem::onZoom(float deltaDistance)
{
    glm::vec3 viewPosition = _viewPosition - _viewDirection * deltaDistance;
    float viewDistance = glm::length(viewPosition);
    if (glm::clamp(viewPosition, _viewPositionMin, _viewPositionMax) != viewPosition ||
        glm::clamp(viewDistance, _viewDistanceMin, _viewDistanceMax) != viewDistance) {
        return;
    }
    _viewPosition = viewPosition;
    updateView();
}

void CameraUserControlledSystem::onStateSet(
    const ipp::schema::message::camera::CameraUserControlledState& state)
{
    _viewPosition = {state.position().x(), state.position().y(), state.position().z()};
    _viewTarget = {state.target().x(), state.target().y(), state.target().z()};
    _viewUp = {state.up().x(), state.up().y(), state.up().z()};

    updateProjection();
    updateView();
}

void CameraUserControlledSystem::onLimitsSet(
    const ipp::schema::message::camera::CameraUserControlledLimits& limits)
{

    _viewTargetMin = {limits.targetMin().x(), limits.targetMin().y(), limits.targetMin().z()};
    _viewTargetMax = {limits.targetMax().x(), limits.targetMax().y(), limits.targetMax().z()};

    _viewPositionMin = {limits.positionMin().x(), limits.positionMin().y(),
                        limits.positionMin().z()};
    _viewPositionMax = {limits.positionMax().x(), limits.positionMax().y(),
                        limits.positionMax().z()};

    _viewDistanceMin = limits.distanceMin();
    _viewDistanceMax = limits.distanceMax();

    _rotationPolarUpAngleCosMin = limits.rotationPolarUpAngleCosMin();
    _rotationPolarUpAngleCosMax = limits.rotationPolarUpAngleCosMax();

    dispatchEventT<LimitsUpdatedEvent>(
        ipp::schema::primitive::Vec3{_viewTargetMin.x, _viewTargetMin.y, _viewTargetMin.z},
        ipp::schema::primitive::Vec3{_viewTargetMax.x, _viewTargetMax.y, _viewTargetMax.z},
        ipp::schema::primitive::Vec3{_viewPositionMin.x, _viewPositionMin.y, _viewPositionMin.z},
        ipp::schema::primitive::Vec3{_viewPositionMax.x, _viewPositionMax.y, _viewPositionMax.z},
        _viewDistanceMin, _viewDistanceMax, _rotationPolarUpAngleCosMin,
        _rotationPolarUpAngleCosMax);

    updateView();
}

void CameraUserControlledSystem::updateView()
{
    _viewTarget = glm::clamp(_viewTarget, _viewTargetMin, _viewTargetMax);

    glm::vec3 positionToTarget = _viewTarget - _viewPosition;
    float distance = glm::length(positionToTarget);
    positionToTarget /= distance;
    distance = glm::clamp(distance, _viewDistanceMin, _viewDistanceMax);

    _viewPosition = _viewTarget - positionToTarget * distance;
    _viewPosition = glm::clamp(_viewPosition, _viewPositionMin, _viewPositionMax);
    _viewDirection = glm::normalize(_viewTarget - _viewPosition);

    _viewMatrix = glm::lookAt(_viewPosition, _viewTarget, _viewUp);

    // TODO: Add collision detection for camera eye
    dispatchEventT<StateUpdatedEvent>(
        ipp::schema::primitive::Vec3{_viewPosition.x, _viewPosition.y, _viewPosition.z},
        ipp::schema::primitive::Vec3{_viewTarget.x, _viewTarget.y, _viewTarget.z},
        ipp::schema::primitive::Vec3{_viewUp.x, _viewUp.y, _viewUp.z});
}

void CameraUserControlledSystem::updateProjection()
{
    if (_projectionViewport == glm::vec2{0}) {
        _projectionMatrixCache = glm::mat4(1);
        return;
    }

    _projectionMatrixCache = _projection.calculateProjectionMatrix(_projectionViewport);
}

CameraUserControlledSystem::CameraUserControlledSystem(MessageLoop& messageLoop)
    : SystemT<CameraUserControlledSystem>(messageLoop)
    , _viewUp{0, 1, 0}
    , _viewPosition{0, 1, 2}
    , _viewTarget{0, 1, 0}
    , _viewMatrix{1}
    , _viewDistanceMin{1}
    , _viewDistanceMax{numeric_limits<float>::max()}
    , _viewPositionMin{numeric_limits<float>::lowest()}
    , _viewPositionMax{numeric_limits<float>::max()}
    , _viewTargetMin{numeric_limits<float>::lowest()}
    , _viewTargetMax{numeric_limits<float>::max()}
    , _rotationPrevious{0, 0}
    , _rotationPolarUpAngleCosMin{-0.8f}
    , _rotationPolarUpAngleCosMax{0.8f}
    , _projectionViewport{0}
{
}

void CameraUserControlledSystem::resetView(const glm::vec3& position,
                                           const glm::vec3& direction,
                                           const glm::vec3& up)
{
    _viewPosition = position;
    _viewTarget = _viewPosition + (direction * glm::dot(direction, -_viewPosition));
    _viewDirection = glm::normalize(_viewTarget - _viewPosition);
    _viewUp = up;

    updateView();
}

glm::vec3 CameraUserControlledSystem::getViewPosition()
{
    return _viewPosition;
}

glm::vec3 CameraUserControlledSystem::getViewDirection()
{
    return _viewDirection;
}

glm::vec3 CameraUserControlledSystem::getViewUp()
{
    return _viewUp;
}

const glm::mat4& CameraUserControlledSystem::getView()
{
    return _viewMatrix;
}

const glm::mat4& CameraUserControlledSystem::getProjection(int width, int height)
{
    if (_projectionViewport != glm::vec2{width, height}) {
        _projectionViewport = glm::vec2{width, height};
        _projectionMatrixCache = _projection.calculateProjectionMatrix(_projectionViewport);
    }

    return _projectionMatrixCache;
}
