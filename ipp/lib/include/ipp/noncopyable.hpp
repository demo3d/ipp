#pragma once

#include <ipp/shared.hpp>

namespace ipp {

/**
 * @brief Base class that disables copy constructor/operator.
 */
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    ~NonCopyable() = default;

    NonCopyable& operator=(const NonCopyable&) = delete;
};
}
