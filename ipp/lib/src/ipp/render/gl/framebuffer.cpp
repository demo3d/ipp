#include <ipp/render/gl/framebuffer.hpp>
#include <ipp/log.hpp>

using namespace std;
using namespace ipp::render::gl;

bool FrameBuffer::isBound() const
{
    GLuint handle = getHandle();
    if (handle == 0) {
        return false;
    }

    GLuint boundHandle;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&boundHandle));

    return handle == boundHandle;
}

void FrameBuffer::bind() const
{
    GLuint handle = getHandle();
    assert(handle != 0);

    glBindFramebuffer(GL_FRAMEBUFFER, getHandle());
    glViewport(0, 0, _dimensions.x, _dimensions.y);
}

void FrameBuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::FrameBuffer(glm::ivec2 dimensions)
    : _dimensions{dimensions}
{
    assert(dimensions.x > 0 && dimensions.y > 0);

    GLuint handle;
    glGenFramebuffers(1, &handle);
    if (handle == 0) {
        IVL_LOG_THROW_ERROR(runtime_error, "Unable to generate GL Framebuffer object");
    }

    _handle = FrameBufferHandle(handle);
}

GLenum GetAttachmentGLEnum(FrameBuffer::Attachment attachment)
{
    switch (attachment) {
        case FrameBuffer::Attachment::Color0:
            return GL_COLOR_ATTACHMENT0;
#ifdef GL_COLOR_ATTACHMENT1_EXT
        case FrameBuffer::Attachment::Color1:
            return GL_COLOR_ATTACHMENT1_EXT;
        case FrameBuffer::Attachment::Color2:
            return GL_COLOR_ATTACHMENT2_EXT;
        case FrameBuffer::Attachment::Color3:
            return GL_COLOR_ATTACHMENT3_EXT;
#endif
        case FrameBuffer::Attachment::Depth:
            return GL_DEPTH_ATTACHMENT;
        case FrameBuffer::Attachment::Stencil:
            return GL_STENCIL_ATTACHMENT;
        default:
            throw logic_error("Unknown attachment enum ?");
    }
}

void FrameBuffer::attachRenderBuffer(Binding<FrameBuffer>& binding,
                                     Attachment attachment,
                                     std::unique_ptr<RenderBuffer> renderBuffer)
{
    binding.assertBound();

    auto handle = renderBuffer->getHandle();

    _attachedRenderBuffers[(size_t)attachment] = move(renderBuffer);
    _attachedTextures[(size_t)attachment] = nullptr;

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GetAttachmentGLEnum(attachment), GL_RENDERBUFFER,
                              handle);
}

void FrameBuffer::attachTexture2D(Binding<FrameBuffer>& binding,
                                  Attachment attachment,
                                  std::unique_ptr<Texture2D> texture)
{
    binding.assertBound();

    auto handle = texture->getHandle();

    _attachedRenderBuffers[(size_t)attachment] = nullptr;
    _attachedTextures[(size_t)attachment] = move(texture);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GetAttachmentGLEnum(attachment), GL_TEXTURE_2D, handle,
                           0);
}

FrameBuffer::Status FrameBuffer::getStatus(Binding<FrameBuffer>& binding) const
{
    binding.assertBound();

    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        case GL_FRAMEBUFFER_COMPLETE:
            return FrameBuffer::Status::Complete;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return FrameBuffer::Status::Unsupported;
        default:
            return FrameBuffer::Status::Incomplete;
    }
}

Texture2D* FrameBuffer::getAttachedTexture(Attachment attachment) const
{
    return _attachedTextures[(size_t)attachment].get();
}

RenderBuffer* FrameBuffer::getAttachedRenderBuffer(Attachment attachment) const
{
    return _attachedRenderBuffers[(size_t)attachment].get();
}

glm::ivec2 FrameBuffer::getDimensions() const
{
    return _dimensions;
}

GLuint FrameBuffer::getHandle() const
{
    return _handle.get();
}
