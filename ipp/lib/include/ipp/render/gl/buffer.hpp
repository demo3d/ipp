#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/log.hpp>
#include "binding.hpp"
#include "error.hpp"
#include "handle.hpp"

namespace ipp {
namespace render {
namespace gl {

enum class BufferKind : GLenum { Array = GL_ARRAY_BUFFER, ElementArray = GL_ELEMENT_ARRAY_BUFFER };

enum class BufferUsage : GLenum {
    StaticDraw = GL_STATIC_DRAW,
    StreamDraw = GL_STREAM_DRAW,
    DynamicDraw = GL_DYNAMIC_DRAW
};

/**
 * @brief GL buffer class wrapper.
 * Creates a buffer handle on construction and releases it in destructor.
 */
template <BufferKind Kind>
class Buffer final : public NonCopyable {
public:
    friend class Binding<Buffer<Kind>>;
    /**
    * @brief Re-export Kind template parameter.
    */
    static const BufferKind BufferKindT = Kind;

    /**
     * @brief Return buffer handle for buffer bound to BufferKind in context
     */
    static GLuint GetBoundHandle()
    {
        GLuint boundBuffer = 0;
        switch (Kind) {
            case BufferKind::Array:
                glGetIntegerv(GL_ARRAY_BUFFER_BINDING, reinterpret_cast<GLint*>(&boundBuffer));
                return boundBuffer;
            case BufferKind::ElementArray:
                glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING,
                              reinterpret_cast<GLint*>(&boundBuffer));
                return boundBuffer;
            default:
                throw std::logic_error("Invalid BufferKind value");
        }
    }

private:
    /**
     * @brief Wrapper glDeleteBuffers to delete single buffer for HandleDeleter.
     */
    static void deleteBuffer(GLuint p)
    {
        glDeleteBuffers(1, &p);
    }

    /**
     * @brief Alias unique_ptr with custom deleter that will release GL buffer handle
     */
    using BufferHandle = std::unique_ptr<void, HandleDeleter<deleteBuffer>>;

    BufferHandle _handle;

    /**
     * @brief Check if buffer is bound to Target in current GL context.
     * @note This function is a sanity check, with correct API usage as long as
     *       object is alive buffer must be bound to Target.
     */
    bool isBound() const noexcept
    {
        GLuint handle = getHandle();
        if (handle == 0) {
            return false;
        }

        return handle == GetBoundHandle();
    }

    void bind() const
    {
        assert(GetBoundHandle() == 0);

        glBindBuffer(static_cast<GLenum>(Kind), getHandle());
        GlError::assertValidateState();
    }

    void unbind() const
    {
        assert(isBound());
        glBindBuffer(static_cast<GLenum>(Kind), 0);
    }

public:
    Buffer()
    {
        GLuint nativeHandle;
        glGenBuffers(1, &nativeHandle);
        if (nativeHandle == 0) {
            throw GlError("Unable to create GL Buffer handle");
        }
        _handle = std::unique_ptr<void, HandleDeleter<deleteBuffer>>(nativeHandle);
    }

    /**
     * @brief Copy data to buffer.
     */
    void bufferData(Binding<Buffer<Kind>>& binding,
                    GLuint size,
                    const void* data,
                    BufferUsage bufferUsage = BufferUsage::StaticDraw) const
    {
        binding.assertBound();
        glBufferData(static_cast<GLenum>(Kind), size, data, static_cast<GLenum>(bufferUsage));
        GlError::assertValidateState();
    }

    /**
     * @brief Query GL context for buffer size.
     */
    GLuint getBufferSize(Binding<Buffer<Kind>>& binding) const
    {
        binding.assertBound();
        GLuint size;
        glGetBufferParameteriv(static_cast<GLenum>(Kind), GL_BUFFER_SIZE,
                               reinterpret_cast<GLint*>(&size));
        GlError::assertValidateState();
        return size;
    }

    /**
     * @brief Query GL context for buffer usage.
     */
    BufferUsage getBufferUsage(Binding<Buffer<Kind>>& binding) const
    {
        binding.assertBound();
        BufferUsage usage;
        glGetBufferParameteriv(static_cast<GLenum>(Kind), GL_BUFFER_SIZE,
                               reinterpret_cast<GLint*>(&usage));
        GlError::assertValidateState();
        return usage;
    }

    /**
     * @brief Return buffer GL handle
     */
    GLuint getHandle() const
    {
        return _handle.get();
    }
};

using ArrayBuffer = Buffer<BufferKind::Array>;
using ElementArrayBuffer = Buffer<BufferKind::ElementArray>;
}
}
}
