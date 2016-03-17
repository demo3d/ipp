#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include "binding.hpp"
#include "handle.hpp"

namespace ipp {
namespace render {
namespace gl {

class RenderBuffer final : public NonCopyable {
public:
    friend class Binding<RenderBuffer>;

    /**
     * @brief Render target valid pixel formats
     */
    enum class InternalFormat {
        DepthComponent16 = GL_DEPTH_COMPONENT16,
        StencilIndex8 = GL_STENCIL_INDEX8
    };

private:
    /**
     * @brief Wrap glDeleteRenderbuffers to only delete a single handle for HandleDeleter
     */
    static void deleteRenderBuffer(GLuint buffer)
    {
        glDeleteRenderbuffers(1, &buffer);
    }

    /**
     * @brief Alias unique_ptr with custom deleter that will release GL render buffer handle
     */
    using RenderBufferHandle = std::unique_ptr<GLuint, HandleDeleter<deleteRenderBuffer>>;

    RenderBufferHandle _handle;

    /**
     * @brief Bind RenderBuffer and return binding handle.
     */
    bool isBound() const;

    /**
     * @brief Bind RenderBuffer and return binding handle.
     */
    void bind() const;

    /**
     * @brief Unbind RenderBuffer from context.
     */
    void unbind() const;

public:
    RenderBuffer();

    /**
     * @brief
     */
    void setStorage(Binding<RenderBuffer>& binding,
                    InternalFormat internalFormat,
                    glm::ivec2 dimensions);

    /**
     * @brief RenderBuffer object GL handle
     */
    GLuint getHandle() const
    {
        return _handle.get();
    }
};
}
}
}
