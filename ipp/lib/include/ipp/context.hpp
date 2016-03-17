#pragma once

#include <ipp/shared.hpp>
#include <ipp/resource/resourcemanager.hpp>
#include <ipp/scene/scene.hpp>

namespace ipp {

class Context final : public NonCopyable {
private:
    json _configuration;
    resource::ResourceManager _resourceManager;

public:
    Context(json configuration);

    /**
     * @brief Create a new Scene instance from resourcePath
     */
    std::unique_ptr<scene::Scene> createScene(const std::string& resourcePath);

    /**
     * @brief Configuration JSON object
     */
    const json& getConfiguration() const
    {
        return _configuration;
    }

    /**
     * @brief Context ResourceManager instance.
     */
    resource::ResourceManager& getResourceManager()
    {
        return _resourceManager;
    }
};
}
