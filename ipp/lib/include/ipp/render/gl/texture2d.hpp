#pragma once

#include <ipp/shared.hpp>
#include "texture.hpp"

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief 2D TextureBindingBase implementation.
 */
class Texture2D : public Texture {
public:
    class Binding2D;

    /**
     * @brief GLuint wrapper used by UniformBuffer to bind Texture::Unit to uniform sampler.
     * Defined within Texture2D to distinquish from TextureCube::UniformSampler and allow type safe
     * unit binding.
     */
    class UniformSampler final {
    public:
        friend class Binding2D;

    private:
        GLuint _textureUnit;

    public:
        UniformSampler(Texture::Unit unit) noexcept : _textureUnit{unit - GL_TEXTURE0}
        {
        }

        UniformSampler() = delete;
        UniformSampler(const UniformSampler& other) = default;

        operator GLuint() const noexcept
        {
            return _textureUnit;
        }
    };

    /**
     * @brief Adds 2D texture specific texture operations to texture::binding<GL_TEXTURE_2D>.
     */
    class Binding2D : public Texture::Binding<GL_TEXTURE_2D, Texture2D> {
    public:
        friend class Texture2D;

    private:
        Binding2D(const Texture2D* texture, Texture::Unit unit);

    public:
        Binding2D();

        /**
         * @brief Implicit cast to uniform_sampler from unit.
         * Allows implicit conversion when setting uniform_buffer 2D sampler to binding_2d
         */
        operator UniformSampler() const noexcept
        {
            return _unit;
        }

        /**
         * @brief Call glTexImage2D on bound texture.
         */
        void texImage2D(GLint level,
                        Texture::PixelFormat internalFormat,
                        GLsizei width,
                        GLsizei height,
                        Texture::PixelFormat format,
                        Texture::PixelType type,
                        const void* source) const;

        /**
         * @brief Call glTexSubImage2D on bound texture.
         */
        void texSubImage2D(GLint level,
                           GLint xoffset,
                           GLint yoffset,
                           GLsizei width,
                           GLsizei height,
                           Texture::PixelFormat format,
                           Texture::PixelType type,
                           const void* source) const;
    };

public:
    Texture2D()
        : Texture()
    {
    }

    Binding2D bind(Texture::Unit unit) const
    {
        return Binding2D(this, unit);
    }
};
}
}
}
