#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>

#include <ipp/log.hpp>
#include <ipp/context.hpp>
#include <ipp/resource/resourcemanager.hpp>
#include <ipp/resource/packageipparchive.hpp>
#include <ipp/scene/animation/animationsystem.hpp>
#include <ipp/scene/render/rendersystem.hpp>

using namespace std;
using namespace std::chrono;
using namespace glm;
using namespace ipp;
using namespace ipp::resource;
using namespace ipp::scene;
using namespace ipp::scene::render;
using namespace ipp::scene::camera;
using namespace ipp::scene::animation;

void cameraMouseScroll(GLFWwindow* window, double offsetX, double offsetY);
void cameraMousePos(GLFWwindow* window, double positionX, double positionY);
void cameraWindowResize(GLFWwindow* window, int width, int height);
void cameraKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods);
void cameraInit(GLFWwindow* window);

unique_ptr<Context> context;
unique_ptr<Scene> sceneInstance;
milliseconds lastFrame;

int main(int argc, char* const argv[])
{
    IVL_LOG(Info, "Initializing GLFW");
    if (!glfwInit()) {
        return -1;
    }

    IVL_LOG(Info, "Creating GLFW window");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_SAMPLES, 8);

    auto window = glfwCreateWindow(1024, 720, "Player", nullptr, nullptr);
    if (!window) {
        IVL_LOG(Error, "Failed to create a GLFW  window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    IVL_LOG(Info, "Creating Resource manager");
    context = make_unique<Context>(json{{"render", {{"shadowMapTechnique", "PCF"}}}});
    ResourceManager& resourceManager = context->getResourceManager();
    resourceManager.registerPackage(make_unique<PackageIPPArchive>(
        resourceManager, "rtts/shared", "resources/rtts/shared.ipparch"));
    resourceManager.registerPackage(make_unique<PackageIPPArchive>(resourceManager, "rtts/demo",
                                                                   "resources/rtts/demo.ipparch"));

    assert(argc > 1);
    string sceneResourcePath = argv[1];

    IVL_LOG(Info, "Creating Scene {}", sceneResourcePath);
    sceneInstance = context->createScene(sceneResourcePath);
    cameraInit(window);
    auto animationSystem = sceneInstance->getMessageLoop().findSystem<AnimationSystem>();
    sceneInstance->getMessageLoop().enqueueCommandT<AnimationSystem::PlayCommand>(
        0, static_cast<uint32_t>(animationSystem->getDuration().count()));

    IVL_LOG(Info, "Scene {} loaded", sceneResourcePath);
    lastFrame = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());

    while (!glfwWindowShouldClose(window)) {
        auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
        auto delta = now - lastFrame;
        lastFrame = now;
        sceneInstance->getMessageLoop().enqueueCommandT<AnimationSystem::UpdateCommand>(
            static_cast<uint32_t>(delta.count()));
        sceneInstance->update();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

    glfwDestroyWindow(window);
    glfwTerminate();

    sceneInstance = nullptr;

    return 0;
}
