#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>

namespace ipp {
namespace resource {

/**
 * @brief Resource memory data buffer interface.
 *
 * Provide Resource loaders with abstract interface to access resource data (allows custom read
 * strategies like plain file preload, memory mapped files, streaming, etc.)
 *
 * Instances of this class should only be created by Package implementations.
 */
class ResourceBuffer : public NonCopyable {
private:
    Package& _package;
    std::string _resourcePath;

protected:
    ResourceBuffer(Package& package, std::string resourcePath);

public:
    virtual ~ResourceBuffer() = default;

    /**
     * @brief Return (at least) requested number of bytes (length) from source at specified offset.
     * Returned memory is owned by buffer and valid as long as buffer object is alive.
     */
    virtual const char* getData(size_t offset = 0, size_t length = 0) = 0;

    /**
     * @brief Buffer size in bytes.
     */
    virtual size_t getSize() const = 0;

    /**
     * @brief Resource path this ResourceBuffer represents.
     */
    const std::string& getResourcePath() const
    {
        return _resourcePath;
    }

    /**
     * @brief ResourceManager resource belongs to.
     */
    ResourceManager& getResourceManager() const;

    /**
     * @brief Package the resource belongs to.
     */
    Package& getPackage() const
    {
        return _package;
    }
};
}
}
