#define GLFW_INCLUDE_ES2
#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_COLOUR_NONE

#include <GLFW/glfw3.h>
#include <catch.hpp>
#include <ipp/shared.hpp>

using namespace std;

int main(int argc, char* const argv[])
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);

    auto window = glfwCreateWindow(640, 480, "Unit Test application", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    int result = Catch::Session().run(argc, argv);

    glfwDestroyWindow(window);
    glfwTerminate();

    return result;
}
