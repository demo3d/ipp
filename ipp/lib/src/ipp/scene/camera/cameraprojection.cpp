#include <ipp/scene/camera/cameraprojection.hpp>

using namespace std;
using namespace glm;
using namespace ipp::scene::camera;

CameraProjection::CameraProjection()
    : cameraShift{0}
    , sensorPixelSize{0.0914285f}
    , sensorFit{SensorFit::Auto}
    , clipNear{0.1f}
    , clipFar{100.0f}
{
}

CameraProjection::CameraProjection(
    vec2 cameraShift, float sensorPixelSize, SensorFit sensorFit, float clipNear, float clipFar)
    : cameraShift{cameraShift}
    , sensorPixelSize{sensorPixelSize}
    , sensorFit{sensorFit}
    , clipNear{clipNear}
    , clipFar{clipFar}
{
}

mat4 CameraProjection::calculateProjectionMatrix(glm::vec2 viewport)
{
    float viewportFactor;
    switch (sensorFit) {
        case SensorFit::Auto:
            viewportFactor = std::max(viewport.x, viewport.y);
            break;
        case SensorFit::Horizontal:
            viewportFactor = viewport.x;
            break;
        case SensorFit::Vertical:
            viewportFactor = viewport.y;
            break;
    }

    float pixelSize = sensorPixelSize / viewportFactor;
    vec2 offset = cameraShift * viewportFactor;

    vec2 planeMin = ((viewport * -0.5f) + offset) * pixelSize;
    vec2 planeMax = ((viewport * 0.5f) + offset) * pixelSize;

    vec2 planeDelta = planeMax - planeMin;
    float clipDelta = clipFar - clipNear;

    mat4 projection(1);
    projection[0][0] = clipNear * 2 / planeDelta.x;
    projection[1][1] = clipNear * 2 / planeDelta.y;
    projection[0][2] = (planeMax.x + planeMin.x) / planeDelta.x;
    projection[1][2] = (planeMax.y + planeMin.y) / planeDelta.y;
    projection[2][2] = -(clipFar + clipNear) / clipDelta;
    projection[3][2] = -1;
    projection[2][3] = (-2 * clipNear * clipFar) / clipDelta;

    return transpose(projection);
}
