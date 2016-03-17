#include <ipp/render/gl/texture2d.hpp>
#include <ipp/render/gl/error.hpp>

using namespace std;
using namespace ipp::render::gl;

Texture2D::Binding2D::Binding2D(const Texture2D* texture, Texture::Unit unit)
    : Binding(texture, unit)
{
}

Texture2D::Binding2D::Binding2D()
    : Binding()
{
}

void Texture2D::Binding2D::texImage2D(GLint level,
                                      Texture::PixelFormat internalFormat,
                                      GLsizei width,
                                      GLsizei height,
                                      Texture::PixelFormat format,
                                      Texture::PixelType type,
                                      const void* source) const
{
    if (!isBound()) {
        throw GlLogicError("Texture must be bound for this call to succeed.");
    }
    glTexImage2D(GL_TEXTURE_2D, level, static_cast<GLenum>(internalFormat), width, height, 0,
                 static_cast<GLenum>(format), static_cast<GLenum>(type), source);
    GlError::assertValidateState();
}

void Texture2D::Binding2D::texSubImage2D(GLint level,
                                         GLint xoffset,
                                         GLint yoffset,
                                         GLsizei width,
                                         GLsizei height,
                                         Texture::PixelFormat format,
                                         Texture::PixelType type,
                                         const void* source) const
{
    if (!isBound()) {
        throw GlLogicError("Texture must be bound for this call to succeed.");
    }
    glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset, width, height,
                    static_cast<GLenum>(format), static_cast<GLenum>(type), source);
    GlError::assertValidateState();
}
