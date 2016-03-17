#pragma once

#include <ipp/shared.hpp>
#include "package.hpp"
#include "resourcebuffermemory.hpp"

namespace ipp {
namespace resource {

/**
 * @brief IPP archive format implementation of Package.
 *
 * Resource paths are interpreted as file paths inside of archive.
 */
class PackageIPPArchive final : public Package {
private:
    struct FileReference {
        uint32_t size;
        uint32_t offset;
    };

    std::unordered_map<std::string, FileReference> _files;
    std::string _archivePath;

public:
    PackageIPPArchive(ResourceManager& resourceManager, std::string name, std::string archivePath);

    /**
     * @brief Package archive path (absolute).
     */
    const std::string& getArchivePath() const
    {
        return _archivePath;
    }

    /**
     * @brief Create ResourceBuffer from resourcePath by reading the data to memory from Archive.
     */
    std::unique_ptr<ResourceBuffer> requestResourceData(const std::string& resourcePath) override;
};
}
}
