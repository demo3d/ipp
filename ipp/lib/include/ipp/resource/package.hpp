#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/log.hpp>
#include "resourcebuffer.hpp"
#include "resource.hpp"

namespace ipp {
namespace resource {

/**
 * @brief Abstract factory for ResourceBuffer objects from a specific resource package.
 * Resource packages are named groups of resource data objects.
 */
class Package : public NonCopyable {
private:
    ResourceManager& _resourceManager;
    const std::string _name;

public:
    Package(ResourceManager& resourceManager, std::string name)
        : _resourceManager{resourceManager}
        , _name{std::move(name)}
    {
    }

    virtual ~Package() = default;

    /**
     * @param packageResourcePath package relative resource path (no package prefix)
     * @return Resource data buffer for resource at specified path.
     */
    virtual std::unique_ptr<ResourceBuffer> requestResourceData(
        const std::string& packageResourcePath) = 0;

    /**
     * @brief Owning ResourceManager object.
     */
    ResourceManager& getResourceManager() const
    {
        return _resourceManager;
    }

    /**
     * @brief Name associated with Package inside resourceManager.
     */
    const std::string& getName() const
    {
        return _name;
    }
};
}
}
