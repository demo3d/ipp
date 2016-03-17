#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#include <ipp/log.hpp>
#include <ipp/scene/scene.hpp>
#include <ipp/resource/resourcemanager.hpp>
#include <ipp/resource/packagefilesystem.hpp>
#include <ipp/scene/animation/animationsystem.hpp>
#include <ipp/scene/render/rendersystem.hpp>

using namespace std;
using namespace std::chrono;
using namespace glm;
using namespace ipp::resource;
using namespace ipp::scene;
using namespace ipp::scene::render;
using namespace ipp::scene::camera;
using namespace ipp::scene::animation;

static const double CameraZoomSpeed = 0.2;
static const double CameraRotationSpeed = 0.5;
static const double CameraMoveSpeed = 0.05;

static bool cameraRotationArcball = false;

extern shared_ptr<Scene> sceneInstance;

void cameraMouseScroll(GLFWwindow* window, double offsetX, double offsetY)
{
    sceneInstance->getMessageLoop().enqueueCommandT<CameraUserControlledSystem::ZoomCommand>(
        static_cast<float>(offsetY * CameraZoomSpeed));
}

/**
 * @brief Mouse move callback that updates camera position and rotation.
 */
void cameraMousePos(GLFWwindow* window, double positionX, double positionY)
{
    static dvec2 cursorPosition(0);
    auto cursorPositionPrevious = cursorPosition;
    cursorPosition = dvec2(positionX, positionY);
    static bool rotationDragging = false;

    if (cameraRotationArcball) {
        ivec2 windowSize;
        glfwGetWindowSize(window, &windowSize.x, &windowSize.y);

        vec2 cursorScreenPos = {(cursorPosition.x / windowSize.x) * 2 - 1,
                                -((cursorPosition.y / windowSize.y) * 2 - 1)};
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            if (!rotationDragging) {
                rotationDragging = true;
                sceneInstance->getMessageLoop()
                    .enqueueCommandT<CameraUserControlledSystem::RotationStartCommand>(
                        ipp::schema::primitive::Vec2{cursorScreenPos.x, cursorScreenPos.y});
            }
            else {
                sceneInstance->getMessageLoop()
                    .enqueueCommandT<CameraUserControlledSystem::RotationUpdateCommand>(
                        ipp::schema::primitive::Vec2{cursorScreenPos.x, cursorScreenPos.y});
            }
        }
        else {
            rotationDragging = false;
        }
    }
    else {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            if (!rotationDragging) {
                rotationDragging = true;
            }
            else {
                auto rotationDelta =
                    (cursorPositionPrevious - cursorPosition) * CameraRotationSpeed;
                sceneInstance->getMessageLoop()
                    .enqueueCommandT<CameraUserControlledSystem::RotatePolarCommand>(
                        static_cast<float>(rotationDelta.y), static_cast<float>(rotationDelta.x));
            }
        }
        else {
            rotationDragging = false;
        }
    }

    static bool moveDragging = false;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
        if (!moveDragging) {
            moveDragging = true;
        }
        else {
            dvec2 moveDelta = (cursorPositionPrevious - cursorPosition) * CameraMoveSpeed;
            sceneInstance->getMessageLoop()
                .enqueueCommandT<CameraUserControlledSystem::MoveCommand>(
                    ipp::schema::primitive::Vec3{static_cast<float>(moveDelta.x),
                                                 static_cast<float>(moveDelta.y), 0});
        }
    }
    else {
        moveDragging = true;
    }
}

void cameraWindowResize(GLFWwindow* window, int width, int height)
{
    sceneInstance->getMessageLoop().enqueueCommandT<RenderSystem::ViewportResizeCommand>(
        static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

void cameraReset()
{
    sceneInstance->getMessageLoop().enqueueCommandT<CameraUserControlledSystem::StateSetCommand>(
        ipp::schema::primitive::Vec3{3, 0, 1}, ipp::schema::primitive::Vec3{0, 0, 1},
        ipp::schema::primitive::Vec3{0, 0, 1});

    sceneInstance->getMessageLoop().enqueueCommandT<CameraUserControlledSystem::LimitsSetCommand>(
        ipp::schema::primitive::Vec3{-2, -2, 0}, ipp::schema::primitive::Vec3{2, 2, 2},
        ipp::schema::primitive::Vec3{-7, -7, 0}, ipp::schema::primitive::Vec3{7, 7, 7}, 1.0f, 10.0f,
        -0.8, 0.8);
}

void cameraKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto animationSystem = sceneInstance->getMessageLoop().findSystem<AnimationSystem>();

    float deltaZoom = CameraZoomSpeed;
    float deltaMoveX = CameraMoveSpeed;
    float deltaMoveY = CameraMoveSpeed;
    float deltaMoveZ = CameraMoveSpeed;

    switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;

        case GLFW_KEY_Q:
            cameraRotationArcball = true;
            break;
        case GLFW_KEY_E:
            cameraRotationArcball = false;
            break;

        case GLFW_KEY_C:
            sceneInstance->getMessageLoop().enqueueCommandT<CameraSystem::SetActiveTypeCommand>(
                CameraType::EntityComponent);
            break;
        case GLFW_KEY_V:
            sceneInstance->getMessageLoop().enqueueCommandT<CameraSystem::SetActiveTypeCommand>(
                CameraType::UserControlled);
            break;

        case GLFW_KEY_I:
            sceneInstance->getMessageLoop().enqueueCommandT<AnimationSystem::PlayCommand>(
                0, static_cast<uint32_t>(animationSystem->getDuration().count()));
            break;
        case GLFW_KEY_P: {
            uint32_t time = 0;
            switch (animationSystem->getStatus()) {
                case AnimationSystem::Status::Playing:
                    return;
                case AnimationSystem::Status::Stopped:
                    time = static_cast<uint32_t>(animationSystem->getTime().count());
                    break;
                default:
                    time = 0;
                    break;
            }
            sceneInstance->getMessageLoop().enqueueCommandT<AnimationSystem::PlayCommand>(
                time, static_cast<uint32_t>(animationSystem->getDuration().count()));
        } break;
        case GLFW_KEY_O:
            sceneInstance->getMessageLoop().enqueueCommandT<AnimationSystem::StopCommand>();
            break;

        case GLFW_KEY_R:
            cameraReset();
            break;

        case GLFW_KEY_PAGE_UP:
            deltaZoom *= -1;
        case GLFW_KEY_PAGE_DOWN:
            sceneInstance->getMessageLoop()
                .enqueueCommandT<CameraUserControlledSystem::ZoomCommand>(deltaZoom);
            break;

        case GLFW_KEY_S:
            deltaMoveZ *= -1;
        case GLFW_KEY_W:
            sceneInstance->getMessageLoop()
                .enqueueCommandT<CameraUserControlledSystem::MoveCommand>(
                    ipp::schema::primitive::Vec3{0, 0, deltaMoveZ});
            break;
        case GLFW_KEY_D:
            deltaMoveX *= -1;
        case GLFW_KEY_A:
            sceneInstance->getMessageLoop()
                .enqueueCommandT<CameraUserControlledSystem::MoveCommand>(
                    ipp::schema::primitive::Vec3{deltaMoveX, 0, 0});
            break;
    }
}

void cameraInit(GLFWwindow* window)
{
    ivec2 windowSize;
    glfwGetWindowSize(window, &windowSize.x, &windowSize.y);

    sceneInstance->getMessageLoop().enqueueCommandT<RenderSystem::ViewportResizeCommand>(
        static_cast<uint32_t>(windowSize.x), static_cast<uint32_t>(windowSize.y));

    glfwSetCursorPosCallback(window, &cameraMousePos);
    glfwSetScrollCallback(window, &cameraMouseScroll);
    glfwSetWindowSizeCallback(window, &cameraWindowResize);
    glfwSetKeyCallback(window, &cameraKeyInput);

    cameraReset();
}
