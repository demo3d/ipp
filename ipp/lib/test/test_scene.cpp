#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>
#include <catch.hpp>
#include <ipp/context.hpp>
#include <ipp/resource/packagefilesystem.hpp>
#include <ipp/scene/scene.hpp>
#include <ipp/scene/render/rendersystem.hpp>

using namespace std;
using namespace ipp;
using namespace ipp::resource;
using namespace ipp::scene;
using namespace ipp::scene::render;

SCENARIO("Scene loading")
{
    GIVEN("Resource manager and test package")
    {
        Context context{json{}};
        ResourceManager& resourceManager = context.getResourceManager();
        resourceManager.registerPackage(make_unique<PackageFileSystem>(
            resourceManager, "ipp/unit_test", "resources/ipp/unit_test"));

        GIVEN("Basic scene")
        {
            auto basicScene = context.createScene("ipp/unit_test:blends/core.scene");

            basicScene->getMessageLoop().enqueueCommandT<RenderSystem::ViewportResizeCommand>(640,
                                                                                              480);
            basicScene->update();
        }
    }
}
