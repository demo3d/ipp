#pragma once

#include <ipp/shared.hpp>

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief Custom std::unique_ptr Deleter that holds and releases GL handles (GLuint type).
 */
template <void (*func)(GLuint)>
struct HandleDeleter {
    struct pointer {
        GLuint handle;

        pointer(std::nullptr_t = nullptr)
            : handle(0)
        {
        }

        pointer(GLuint handle)
            : handle{handle}
        {
        }

        operator GLuint() const
        {
            return handle;
        }

        friend bool operator==(pointer a, pointer b)
        {
            return a.handle == b.handle;
        }

        friend bool operator!=(pointer a, pointer b)
        {
            return a.handle != b.handle;
        }
    };

    void operator()(GLuint p) const
    {
        func(p);
    }
};
}
}
}
