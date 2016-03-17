#pragma once

#include <ipp/shared.hpp>
#include "resourcebuffer.hpp"

namespace ipp {
namespace resource {

/**
 * @brief ResourceBuffer that holds a memory buffer in std::vector and provides read-only access.
 */
class ResourceBufferMemory final : public ResourceBuffer {
private:
    std::vector<char> _buffer;

public:
    ResourceBufferMemory(Package& package, std::string resourcePath, std::vector<char> buffer);
    const char* getData(size_t offset = 0, size_t length = 0) override;
    size_t getSize() const override;
};
}
}
