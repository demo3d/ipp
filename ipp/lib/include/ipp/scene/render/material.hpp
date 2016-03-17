#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/render/materialbuffer.hpp>
#include <ipp/render/texturebitmap.hpp>
#include "materialeffect.hpp"

namespace ipp {
namespace scene {
namespace render {

/**
 * @brief Resource that creates ipp::render::MaterialBuffer instances
 */
class Material : public resource::SharedResourceT<Material> {
public:
    /**
     * @brief Defines property that binds textureResource -> textureUnit for this Material
     */
    struct TextureProperty {
        TextureProperty(std::string propertyName,
                        ipp::render::gl::Texture2D::Unit textureUnit,
                        std::shared_ptr<::ipp::render::BitmapTexture2D> textureResource)
            : propertyName{std::move(propertyName)}
            , textureUnit{textureUnit}
            , textureResource{std::move(textureResource)}
        {
        }

        std::string propertyName;
        ipp::render::gl::Texture2D::Unit textureUnit;
        std::shared_ptr<::ipp::render::BitmapTexture2D> textureResource;
    };

private:
    size_t _textureCount;
    std::unique_ptr<TextureProperty> _diffuseTexture;
    std::unique_ptr<TextureProperty> _specularTexture;
    std::shared_ptr<MaterialEffect> _effect;
    ipp::render::MaterialBuffer _materialBuffer;
    bool _isTransparent;
    bool _isShadowCaster;

public:
    Material(std::unique_ptr<resource::ResourceBuffer> data);

    /**
     * @brief MaterialBuffer with default Material values.
     * Copy MaterialBuffer to create a new mutable instance with same initial values.
     */
    const ipp::render::MaterialBuffer& getMaterialBuffer() const
    {
        return _materialBuffer;
    }

    /**
     * @brief Transparent objects are rendered after opaque objects with depth write disabled in
     * back-to-front order.
     */
    bool isTransparent() const
    {
        return _isTransparent;
    }

    /**
     * @brief Shadow caster objects get rendered in to shadow map and cast shadows on other objects.
     */
    bool isShadowCaster() const
    {
        return _isShadowCaster;
    }

    /**
     * @brief Diffuse texture, nullptr if material doesn't have a diffuse texture.
     */
    TextureProperty* getDiffuseTexture() const
    {
        return _diffuseTexture.get();
    }

    /**
     * @brief Specular texture, nullptr if material doesn't have a specular texture.
     */
    TextureProperty* getSpecularTexture() const
    {
        return _specularTexture.get();
    }

    /**
     * @brief
     */
    size_t getTextureCount() const
    {
        return _textureCount;
    }

    /**
     * @brief
     */
    const MaterialEffect& getEffect() const
    {
        return *_effect;
    }
};
}
}
}
