#include <ipp/render/gl/renderbuffer.hpp>
#include <ipp/render/gl/error.hpp>
#include <ipp/log.hpp>

using namespace std;
using namespace ipp::render::gl;

bool RenderBuffer::isBound() const
{
    GLuint handle = getHandle();
    if (handle == 0) {
        return false;
    }

    GLuint boundHandle;
    glGetIntegerv(GL_RENDERBUFFER_BINDING, reinterpret_cast<GLint*>(&boundHandle));

    return handle == boundHandle;
}

void RenderBuffer::bind() const
{
    glBindRenderbuffer(GL_RENDERBUFFER, getHandle());
}

void RenderBuffer::unbind() const
{
    assert(isBound());
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::RenderBuffer()
{
    GLuint handle;
    glGenRenderbuffers(1, &handle);
    if (handle == 0) {
        IVL_LOG_THROW_ERROR(runtime_error, "Unable to generate GL Renderbuffer object");
    }
    _handle = RenderBufferHandle(handle);
}

void RenderBuffer::setStorage(Binding<RenderBuffer>& binding,
                              InternalFormat internalFormat,
                              glm::ivec2 dimensions)
{
    binding.assertBound();
    glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(internalFormat), dimensions.x,
                          dimensions.y);
    gl::GlError::assertValidateState();
}