#include <ipp/context.hpp>
#include <ipp/scene/camera/camerasystem.hpp>
#include <ipp/scene/animation/animationsystem.hpp>
#include <ipp/scene/scene.hpp>
#include "capi.hpp"

using namespace std;
using namespace ipp;
using namespace ipp::resource;
using namespace ipp::scene;

extern "C" {

vector<unique_ptr<Context>> contexts;
vector<unique_ptr<Scene>> scenes;

/**
 * @brief Create a new Context instance.
 */
Context* IVL_API_EXPORT context_create(const char* configuration)
{
    auto context = make_unique<Context>(json::parse(configuration));
    auto contextPtr = context.get();
    contexts.emplace_back(move(context));
    return contextPtr;
}

/**
 * @brief Destroy a Context object created by context_create.
 * @note Will also destroy any Scene instance associated with Context.
 */
void IVL_API_EXPORT context_destroy(Context* context)
{
    auto contextIt = find_if(contexts.begin(), contexts.end(),
                             [context](const auto& elem) { return elem.get() == context; });
    if (contextIt != contexts.end()) {
        remove_if(scenes.begin(), scenes.end(),
                  [context](const auto& scene) { return &scene->getContext() == context; });
        contexts.erase(contextIt);
    }
}

/**
 * @brief Create a new instance of Scene trough Context and return a pointer to instance.
 * @note Reference must be released trough context_scene_destroy(reference).
 */
Scene* IVL_API_EXPORT context_scene_create(Context* context, const char* resourcePath)
{
    auto scene = context->createScene(resourcePath);
    auto scenePtr = scene.get();
    scenes.emplace_back(move(scene));
    return scenePtr;
}

/**
 * @brief Destroy a Scene handle created by context_scene_create.
 * @note Once this function completes scene pointer is invalid.
 */
void IVL_API_EXPORT context_scene_destroy(Scene* scene)
{
    auto sceneIt = find_if(scenes.begin(), scenes.end(),
                           [scene](const auto& other) { return other.get() == scene; });
    if (sceneIt != scenes.end()) {
        scenes.erase(sceneIt);
    }
}

/**
 * @brief Return Scene MessageLoop instance pointer.
 */
loop::MessageLoop* IVL_API_EXPORT context_scene_get_loop(Scene* scene)
{
    return &scene->getMessageLoop();
}
}
