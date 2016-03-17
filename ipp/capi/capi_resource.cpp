#include <ipp/context.hpp>
#include <ipp/resource/packagefilesystem.hpp>
#include <ipp/resource/packageipparchive.hpp>
#include <ipp/log.hpp>
#include "capi.hpp"

using namespace std;
using namespace ipp;
using namespace ipp::resource;

extern "C" {

/**
 * @brief Register a file system package to global resource manager instance.
 */
void IVL_API_EXPORT resource_register_fs_package(Context* context,
                                                 const char* name,
                                                 const char* path)
{
    auto& resourceManager = context->getResourceManager();
    resourceManager.registerPackage(make_unique<PackageFileSystem>(resourceManager, name, path));
}

/**
 * @brief Register a IPP archive package located on path to a global resource manager instance.
 */
void IVL_API_EXPORT resource_register_ipparch_package(Context* context,
                                                      const char* name,
                                                      const char* archPath)
{
    auto& resourceManager = context->getResourceManager();
    resourceManager.registerPackage(
        make_unique<PackageIPPArchive>(resourceManager, name, archPath));
}
}
