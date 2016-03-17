#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include "resourcebuffer.hpp"
#include "resource.hpp"

namespace ipp {
namespace resource {

/**
 * @brief Loads resource file in to a std::string and provides const access.
 */
class StringResource : public SharedResourceT<StringResource> {
private:
    std::string _data;

public:
    StringResource(std::unique_ptr<ResourceBuffer> data);

    /**
     * @brief Resource data string const access.
     */
    const std::string& getData() const
    {
        return _data;
    }
};
}
}
