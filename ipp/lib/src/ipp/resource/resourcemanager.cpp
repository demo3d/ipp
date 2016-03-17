#include <ipp/resource/resourcemanager.hpp>

using namespace std;
using namespace ipp;
using namespace ipp::resource;

ResourceManager::ResourceManager(Context& context)
    : _context{context}
    , _packages{}
{
}

unique_ptr<ResourceBuffer> ResourceManager::requestResourceData(const string& resourcePath) const
{
    IVL_LOG(Info, "Requesting resource {} from ResourceManager.", resourcePath);
    auto packageSeparator = resourcePath.find(':');
    if (packageSeparator > resourcePath.length() - 1) {
        IVL_LOG_THROW_ERROR(
            std::runtime_error,
            "Unable to package parse package resource path : {}, "
            "expected package formated as : '<package-name>:/package/resource/path'",
            resourcePath);
    }

    auto packageName = resourcePath.substr(0, packageSeparator);
    auto packageResourcePath = resourcePath.substr(packageSeparator + 1);

    auto package = findPackage(packageName);
    if (package == nullptr) {
        IVL_LOG_THROW_ERROR(std::runtime_error,
                            "Unable to find package : {} from package resource path : {}",
                            packageName, packageResourcePath);
    }

    return package->requestResourceData(resourcePath);
}

void ResourceManager::registerPackage(unique_ptr<Package> package)
{
    if (_packages.find(package->getName()) != _packages.end()) {
        IVL_LOG_THROW_ERROR(std::logic_error,
                            "Resource package with name {} already registered in ResourceManager",
                            package->getName());
    }

    _packages[package->getName()] = std::move(package);
}

Package* ResourceManager::findPackage(const string& name) const
{
    auto packageIt = _packages.find(name);
    if (packageIt == _packages.end()) {
        IVL_LOG(Info, "Requested package {} not found in ResourceManager", name);
        return nullptr;
    }

    return packageIt->second.get();
}
