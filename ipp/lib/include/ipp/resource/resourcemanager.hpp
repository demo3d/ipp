#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/log.hpp>
#include "package.hpp"

namespace ipp {
namespace resource {

/**
 * @brief Collection of named resource Package objects.
 *
 * ResourceManager is the root object under which Package instances can search for references to
 * other Package instances.
 */
class ResourceManager final {
public:
    friend class ipp::Context;

private:
    Context& _context;
    std::unordered_map<std::string, std::weak_ptr<SharedResourceBase>> _resources;
    std::unordered_map<std::string, std::unique_ptr<Package>> _packages;

    ResourceManager(Context& context);

public:
    /**
     * @brief Transfer ownership of resource Package object to ResourceManager.
     */
    void registerPackage(std::unique_ptr<Package> package);

    /**
     * @brief Find a resource package by name or return nullptr if no package with name specified.
     */
    Package* findPackage(const std::string& name) const;

    /**
     * @brief Request resource data from specified resource path.
     */
    std::unique_ptr<ResourceBuffer> requestResourceData(const std::string& resourcePath) const;

    /**
     * @brief Get a shared resource by fully qualified resource path.
     *
     * Type T must be a SharedResourceBase implementation that has a public constructor with
     * constructor signature : (unique_ptr<ResourceBuffer>, ...)
     *
     * If the resource with specified path and type T exists in package existing instance is
     * returned, otherwise a new instance is created using this Package, resourcePath,
     * ResourceBuffer created by Package for resourcePath and an optional list of params forwarded
     * from this method call.
     *
     * Raises an exception if ResourceBuffer for resourcePath cannot be created or if resource with
     * same resourcePath but different type T exists in Package.
     *
     * Fully qualified packageResourcePath string must be '<package-name>:/package/resource/path'.
     * Finds <package-name> in ResourceManager and calls :
     *     requestResourceData('/pcakge/resource/path', ...).
     *
     * Rasies exception if packageResourcePath is not properly formated, if requested package
     * not found or if package->getResource fails.
     */
    template <typename T, typename... Params>
    std::shared_ptr<T> requestSharedResource(const std::string& resourcePath, Params&&... params)
    {
        auto resourceIt = _resources.find(resourcePath);
        if (resourceIt != _resources.end() && !resourceIt->second.expired()) {
            if (T::ResourceTypeName != resourceIt->second.lock()->getResourceTypeName()) {
                IVL_LOG_THROW_ERROR(
                    std::runtime_error,
                    "Resource with path {}, already exists in resource manager with type ({}) "
                    "not matchign requested type {}.",
                    resourcePath, resourceIt->second.lock()->getResourceTypeName(),
                    T::ResourceTypeName);
            }

            IVL_LOG(Trace,
                    "Requested resource {} already loaded as type {} returning existing object",
                    resourcePath, T::ResourceTypeName);

            auto existingResourcePtr = std::dynamic_pointer_cast<T>(resourceIt->second.lock());
            if (existingResourcePtr == nullptr) {
                IVL_LOG_THROW_ERROR(
                    std::logic_error,
                    "Existing resource {} matches requested type name {} but failed dynamic_cast ?",
                    resourcePath, T::ResourceTypeName);
            }

            return existingResourcePtr;
        }

        IVL_LOG(Trace, "Requested resource {} not currently loaded, creating a new {} instance.",
                resourcePath, T::ResourceTypeName);

        auto resourcePtr =
            std::make_shared<T>(requestResourceData(resourcePath), std::forward<Params>(params)...);
        _resources[resourcePath] = resourcePtr;
        return resourcePtr;
    }

    /**
     * @brief Parent Context instance that owns this ResourceManager object.
     */
    Context& getContext()
    {
        return _context;
    }
};
}
}
