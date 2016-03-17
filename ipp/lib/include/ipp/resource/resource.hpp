#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include "resourcebuffer.hpp"

namespace ipp {
namespace resource {

/**
 * @brief Base class for *shared* resource types.
 *
 * Resource objects are held in std::shared_ptr references and will be unloaded when no references
 * are held. Resource Package holds a std::weak_ptr reference to ResourceBase and returns an
 * existing resource object if it's already loaded when requested.
 *
 * Resources are created trough Package::getResource<T> method and every implementation must provide
 * a public constructor with (Package*, string, unique_ptr<ResourceBuffer>) signature.
 *
 * Every resource implementation must also expose a public static std::string ResourceTypeName
 * containing a unique type identifier string for ResourceBase (for debugging and validation).
 *
 * Override destructor to handle resource unloading.
 */
class SharedResourceBase : public NonCopyable {
protected:
    SharedResourceBase(ResourceManager& resourceManager, std::string resourcePath)
        : _resourceManager{resourceManager}
        , _resourcePath{std::move(resourcePath)}
    {
    }

private:
    ResourceManager& _resourceManager;
    const std::string _resourcePath;

public:
    virtual ~SharedResourceBase() = default;

    /**
     * @brief Unique type identifier string of ResourceBase implementation (for debugging).
     */
    virtual const std::string& getResourceTypeName() = 0;

    /**
     * @brief Resource unique path in resource manager.
     * Resource path is structured as : <package/name>:<resource/path>, package name is unique
     * Package name string in ResourceManager and resource/path is a relative resource path inside
     * of Package specified by name.
     */
    const std::string& getResourcePath() const
    {
        return _resourcePath;
    }

    /**
     * @brief Resource manager this Resource belongs to.
     */
    ResourceManager& getResourceManager() const
    {
        return _resourceManager;
    }
};

/**
 * @brief Generic SharedResourceBase implementation.
 *
 * T is implementation/deriving type.
 */
template <typename T>
class SharedResourceT : public SharedResourceBase {
public:
    SharedResourceT(ResourceManager& resourceManager, std::string resourcePath)
        : SharedResourceBase(resourceManager, resourcePath)
    {
    }

    /**
     * @brief Returns ResourceT<T>::ResourceTypeName
     */
    const std::string& getResourceTypeName() override
    {
        return SharedResourceT<T>::ResourceTypeName;
    }

    /**
     * @brief Static string type name identifier, must be implemented for every T.
     */
    static const std::string ResourceTypeName;
};
}
}
