#include <ipp/render/gl/texture.hpp>
#include <ipp/render/gl/error.hpp>

using namespace std;
using namespace ipp::render::gl;

Texture::Unit::Unit(GLuint unitNumber)
    : _textureUnit{GL_TEXTURE0 + unitNumber}
{
}

void Texture::Unit::activate() const noexcept
{
    glActiveTexture(_textureUnit);
}

bool Texture::Unit::isActive() const noexcept
{
    GLuint activeUnit;
    glGetIntegerv(GL_ACTIVE_TEXTURE, reinterpret_cast<GLint*>(&activeUnit));
    return _textureUnit == activeUnit;
}

Texture::Texture()
{
    GLuint nativeHandle;
    glGenTextures(1, &nativeHandle);
    if (nativeHandle == 0) {
        throw GlError("Unable to create GL texture handle");
    }
    _handle = TextureHandle(nativeHandle);
}
