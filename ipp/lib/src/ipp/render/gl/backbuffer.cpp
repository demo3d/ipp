#include <ipp/render/gl/backbuffer.hpp>

using namespace std;
using namespace ipp;
using namespace ipp::render::gl;

bool BackBuffer::isBound() const
{
    GLuint boundHandle;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&boundHandle));

    return boundHandle == 0;
}

void BackBuffer::bind() const
{
    assert(_dimensions.x > 0 && _dimensions.y > 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _dimensions.x, _dimensions.y);
}

void BackBuffer::unbind() const
{
}

BackBuffer::BackBuffer()
    : _dimensions{0, 0}
{
}

glm::ivec2 BackBuffer::getDimensions() const
{
    return _dimensions;
}

void BackBuffer::setDimensions(glm::ivec2 dimensions)
{
    _dimensions = dimensions;
}