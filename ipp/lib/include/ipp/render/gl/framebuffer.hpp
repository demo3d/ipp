#pragma once

#include <ipp/shared.hpp>
#include "binding.hpp"
#include "handle.hpp"
#include "renderbuffer.hpp"
#include "rendertarget.hpp"
#include "texture2d.hpp"

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief RenderTarget that renders to GL frame buffer object (FBO)
 */
class FrameBuffer final : public RenderTarget {
public:
    /**
     * @brief Enumerates possible attachment slots to GL FBO
     * @note Color1-3 are extensions not supported on all GLES targets
     */
    enum class Attachment { Color0 = 0, Color1, Color2, Color3, Depth, Stencil };

    /**
     * @brief FBO status enumeration
     *
     * Doesn't map 1:1 with GL defined enum values grouping multiple incomplete enums because
     * they are inconsistent between implementations (for simplicity).
     */
    enum class Status { Complete, Incomplete, Unsupported };

public:
    friend class Binding<FrameBuffer>;

private:
    /**
     * @brief Return true if FrameBuffer is currently bound to context
     */
    bool isBound() const override;

    /**
     * @brief Bind FrameBuffer to context and return binding handle.
     */
    void bind() const override;

    /**
     * @brief Unbind FrameBuffer to context and return binding handle.
     */
    void unbind() const override;

    /**
     * @brief Wrapper glDeleteFramebuffers to delete single frame buffer for HandleDeleter.
     */
    static void deleteFrameBuffer(GLuint buffer)
    {
        glDeleteFramebuffers(1, &buffer);
    }

    /**
     * @brief Alias unique_ptr with custom deleter that will release GL frame buffer handle
     */
    using FrameBufferHandle = std::unique_ptr<GLuint, HandleDeleter<deleteFrameBuffer>>;

    FrameBufferHandle _handle;
    std::array<std::unique_ptr<RenderBuffer>, (size_t)Attachment::Stencil + 1>
        _attachedRenderBuffers;
    std::array<std::unique_ptr<Texture2D>, (size_t)Attachment::Stencil + 1> _attachedTextures;
    glm::ivec2 _dimensions;

public:
    FrameBuffer(glm::ivec2 dimensions);

    /**
     * @brief Attach a RenderBuffer to specified FrameBuffer attachment.
     */
    void attachRenderBuffer(Binding<FrameBuffer>& binding,
                            Attachment attachment,
                            std::unique_ptr<RenderBuffer> renderBuffer);

    /**
     * @brief Attach a Texture2D to specified FrameBuffer attachment.
     */
    void attachTexture2D(Binding<FrameBuffer>& binding,
                         Attachment attachment,
                         std::unique_ptr<Texture2D> texture);

    /**
     * @brief Return a reference to attached texture at attachment address
     * If texture is not attached at specified attachment slot texture property will be null
     */
    Texture2D* getAttachedTexture(Attachment attachment) const;

    /**
     * @brief Return a reference to attached render buffer at attachment address
     * If render buffer is not attached at specified attachment slot texture property will be null
     */
    RenderBuffer* getAttachedRenderBuffer(Attachment attachment) const;

    /**
     * @brief FrameBuffer current status.
     */
    Status getStatus(Binding<FrameBuffer>& binding) const;

    /**
     * @brief Framebuffer dimensions (width, height) in pixels
     */
    glm::ivec2 getDimensions() const override;

    /**
     * @brief Return buffer handle.
     */
    GLuint getHandle() const;
};
}
}
}
