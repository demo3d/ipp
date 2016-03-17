#pragma once

#include <ipp/shared.hpp>
#include "package.hpp"
#include "resourcebuffermemory.hpp"

namespace ipp {
namespace resource {

/**
 * @brief Directory implementation of Package.
 *
 * Resource paths are simply interpreted as file paths relative to package root directory.
 */
class PackageFileSystem final : public Package {
private:
    std::string _rootPath;

public:
    PackageFileSystem(ResourceManager& resourceManager, std::string name, std::string rootPath);

    /**
     * @brief Read resource data.
     */
    std::unique_ptr<ResourceBuffer> requestResourceData(const std::string& resourcePath) override;

    /**
     * @brief Package root folder path (absolute).
     */
    const std::string& getRootPath() const
    {
        return _rootPath;
    }
};
}
}
