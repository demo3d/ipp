#include <ipp/scene/node/nodetransform.hpp>

using namespace std;
using namespace glm;
using namespace ipp::scene::node;

NodeTransform::operator mat4() const
{
    auto translationMatrix = glm::translate(translation);

    mat4 rotationMatrix;
    if (rotationMode == RotationMode::Quaternion) {
        rotationMatrix = toMat4(quat(rotation.w, rotation.x, rotation.y, rotation.z));
    }
    else if (rotationMode == RotationMode::AxisAngle) {
        auto axis = vec3(rotation);
        rotationMatrix = axisAngleMatrix(axis, rotation.w);
    }
    else {
        auto rotationEulerX = eulerAngleX(rotation.x);
        auto rotationEulerY = eulerAngleY(rotation.y);
        auto rotationEulerZ = eulerAngleZ(rotation.z);
        switch (rotationMode) {
            case RotationMode::EulerXYZ:
                rotationMatrix = rotationEulerZ * rotationEulerY * rotationEulerX;
                break;
            case RotationMode::EulerXZY:
                rotationMatrix = rotationEulerY * rotationEulerZ * rotationEulerX;
                break;
            case RotationMode::EulerYXZ:
                rotationMatrix = rotationEulerZ * rotationEulerX * rotationEulerY;
                break;
            case RotationMode::EulerYZX:
                rotationMatrix = rotationEulerX * rotationEulerZ * rotationEulerY;
                break;
            case RotationMode::EulerZXY:
                rotationMatrix = rotationEulerY * rotationEulerX * rotationEulerZ;
                break;
            case RotationMode::EulerZYX:
                rotationMatrix = rotationEulerX * rotationEulerY * rotationEulerZ;
                break;
            default:
                throw logic_error("Unknown node transform rotation mode");
        }
    }

    auto scaleMatrix = glm::scale(scale);

    return translationMatrix * rotationMatrix * scaleMatrix;
}
