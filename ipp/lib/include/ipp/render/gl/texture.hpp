#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include "handle.hpp"
#include "error.hpp"

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief 2D exture
 */
class Texture : public NonCopyable {
public:
    enum class MinFilter : GLenum {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
        NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
    };

    enum class MagFilter : GLenum { Nearest = GL_NEAREST, Linear = GL_LINEAR };

    enum class WrapCoordinate : GLenum { S = GL_TEXTURE_WRAP_S, T = GL_TEXTURE_WRAP_T };

    enum class Wrap : GLenum {
        ClampToEdge = GL_CLAMP_TO_EDGE,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        Repeat = GL_REPEAT
    };

    /**
     * @brief GL texture pixel format
     */
    enum class PixelFormat : GLenum {
        Alpha = GL_ALPHA,
        Luminance = GL_LUMINANCE,
        LuminanceAlpha = GL_LUMINANCE_ALPHA,
        Rgb = GL_RGB,
        Rgba = GL_RGBA,
        DepthComponent = GL_DEPTH_COMPONENT
    };

    /**
     * @brief GL texture pixel type
     */
    enum class PixelType : GLenum {
        UByte = GL_UNSIGNED_BYTE,
        UShort565 = GL_UNSIGNED_SHORT_5_6_5,
        UShort4444 = GL_UNSIGNED_SHORT_4_4_4_4,
        UShort5551 = GL_UNSIGNED_SHORT_5_5_5_1,
        UShort = GL_UNSIGNED_SHORT,
        UInt = GL_UNSIGNED_INT
    };

    /**
     * @brief Texture binding target.
     * Initialized from a number between 0 to max textures and converted to appropriate
     * GL_TEXTURE{N} GLenum value
     */
    class Unit final {
    private:
        GLenum _textureUnit;

    public:
        Unit() = delete;
        Unit(GLuint unitNumber);

        operator GLenum() const noexcept
        {
            return _textureUnit;
        }

        /**
         * @brief Activate texture unit in GL context
         */
        void activate() const noexcept;

        /**
         * @brief Query GL context to see if texture unit is currently active
         */
        bool isActive() const noexcept;
    };

    /**
     *
     */
    template <GLenum Target, typename TextureType>
    class Binding : public NonCopyable {
    protected:
        const TextureType* _texture;
        gl::Texture::Unit _unit;

        Binding()
            : _texture{nullptr}
            , _unit{0}
        {
        }

        Binding(const TextureType* texture, gl::Texture::Unit unit)
            : _texture{texture}
            , _unit{unit}
        {
            _unit.activate();
            glBindTexture(Target, _texture->getHandle());
            GlError::assertValidateState();
        }

        void assertBound() const
        {
            if (!isBound()) {
                throw GlLogicError("Texture must be bound for this call to succeed.");
            }
        }

    public:
        Binding(Binding<Target, TextureType>&& other)
            : _texture{other._texture}
            , _unit{other._unit}
        {
            other._texture = nullptr;
        }

        ~Binding()
        {
            if (_texture != nullptr) {
                _unit.activate();
#ifdef IVL_DEBUG_BUILD
                if (!isBound()) {
                    throw GlLogicError(
                        "Cannot unbind Texture from target when texture is not bound to it.");
                }
#endif
                glBindTexture(Target, 0);
            }
        }

        /**
         * @brief Move assign operator moves binding from one object to another.
         */
        Binding<Target, TextureType>& operator=(Binding<Target, TextureType>&& other)
        {
            _texture = other._texture;
            _unit = other._unit;
            other._texture = nullptr;
            return *this;
        }

        /**
         * @brief Check if binding is bound to TextureUnit.
         * @note Binding must be active or GlLogicError will be thrown
         */
        bool isBound() const
        {
            _unit.activate();
            if (_texture == nullptr) {
                throw GlLogicError(
                    "Texture binding object moved from this instance call not valid.");
            }
            GLuint activeTexture;
            GLenum targetBinding;
            switch (Target) {
                case GL_TEXTURE_2D:
                    targetBinding = GL_TEXTURE_BINDING_2D;
                    break;
                case GL_TEXTURE_CUBE_MAP:
                    targetBinding = GL_TEXTURE_BINDING_CUBE_MAP;
                    break;
                default:
                    throw std::logic_error("Unknown texture binding target ?");
            }
            glGetIntegerv(targetBinding, reinterpret_cast<GLint*>(&activeTexture));
            return activeTexture == _texture->getHandle();
        }

        /**
         * @brief Generate texture mipmap levels for texture.
         * @note Texture must be currently active and bound to context for the call to succeed.
         */
        void generateMipmap() const
        {
            assertBound();
            glGenerateMipmap(Target);
        }

        /**
         * @brief Set texture minification filter.
         * @note Texture must be currently active and bound to context for the call to succeed.
         */
        void setMinFilter(gl::Texture::MinFilter filter)
        {
            assertBound();
            glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(filter));
        }

        /**
         * @brief Query GL context for texture minification filter.
         * @note Texture must be currently active and bound to context for the call to succeed.
         */
        gl::Texture::MinFilter getMinFilter() const
        {
            assertBound();
            gl::Texture::MinFilter result;
            glGetTexParameteriv(Target, GL_TEXTURE_MIN_FILTER, reinterpret_cast<GLint*>(&result));
            return result;
        }

        /**
         * @brief Set texture maginifcation filter.
         * @note Texture must be currently active and bound to context for the call to succeed.
         */
        void setMagFilter(gl::Texture::MagFilter filter)
        {
            assertBound();
            glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(filter));
        }

        /**
         * @brief Query GL context for texture magnification filter.
         * @note Texture must be currently active and bound to context for the call to succeed.
         */
        gl::Texture::MagFilter getMagFilter() const
        {
            assertBound();
            Texture::MagFilter result;
            glGetTexParameteriv(Target, GL_TEXTURE_MAG_FILTER, reinterpret_cast<GLint*>(&result));
            return result;
        }

        /**
         * @brief Set texture wrap parameter for coord.
         * @note Texture must be currently active and bound to context for the call to succeed.
         */
        void setWrap(gl::Texture::WrapCoordinate coord, gl::Texture::Wrap wrap)
        {
            assertBound();
            glTexParameteri(Target, static_cast<GLenum>(coord), static_cast<GLint>(wrap));
        }

        /**
         * @brief Query GL context for texture wrap parameter of coord.
         * @note Texture must be currently active and bound to context for the call to succeed.
         */
        gl::Texture::Wrap getWrap(Texture::WrapCoordinate coord) const
        {
            assertBound();
            gl::Texture::Wrap result;
            glGetTexParameteriv(Target, static_cast<GLenum>(coord),
                                reinterpret_cast<GLint*>(&result));
            return result;
        }

        /**
         * @brief Texture object that is bound trough this Binding object.
         */
        const TextureType* getTexture() const
        {
            return _texture;
        }

        /**
         * @brief Unit texture is bound to trough this object.
         */
        gl::Texture::Unit getUnit() const
        {
            return _unit;
        }
    };

private:
    /**
     * @brief Wrapper glDeleteTextures to delete single texture for HandleDeleter.
     */
    static void deleteTexture(GLuint texture)
    {
        glDeleteTextures(1, &texture);
    }

    /**
     * @brief Alias unique_ptr with custom deleter that will release GL texture handle
     */
    using TextureHandle = std::unique_ptr<GLuint, HandleDeleter<deleteTexture>>;

    TextureHandle _handle;

protected:
    Texture();

public:
    GLuint getHandle() const
    {
        return _handle.get();
    }
};
}
}
}
