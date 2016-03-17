#include <ipp/scene/camera/cameranodecomponent.hpp>

using namespace std;
using namespace glm;
using namespace ipp::resource;
using namespace ipp::entity;
using namespace ipp::render;
using namespace ipp::scene;
using namespace ipp::scene::camera;

template <>
const string ComponentT<CameraNodeComponent>::ComponentTypeName = "SceneCameraComponent";

void CameraNodeComponent::updateView()
{
    _view = inverse(_nodeTransform);
    _viewDirty = false;
}

CameraNodeComponent::CameraNodeComponent(Entity& entity, CameraProjection projection)
    : ComponentT<CameraNodeComponent>(entity)
    , _projection{projection}
    , _projectionViewport{0}
{
}

void CameraNodeComponent::updateFromNodeTransform(const mat4& nodeTransform)
{
    _nodeTransform = nodeTransform;
    _viewDirty = true;
}

glm::vec3 CameraNodeComponent::getViewPosition()
{
    return vec3(_nodeTransform[3]);
}

glm::vec3 CameraNodeComponent::getViewDirection()
{
    return vec3(_nodeTransform[2]);
}

glm::vec3 CameraNodeComponent::getViewUp()
{
    return vec3(_nodeTransform[1]);
}

const glm::mat4& CameraNodeComponent::getView()
{
    if (_viewDirty) {
        updateView();
    }

    return _view;
}

const mat4& CameraNodeComponent::getProjection(int width, int height)
{
    if (_projectionViewport != vec2{width, height}) {
        _projectionViewport = vec2{width, height};
        _projectionMatrixCache = _projection.calculateProjectionMatrix(_projectionViewport);
    }

    return _projectionMatrixCache;
}
