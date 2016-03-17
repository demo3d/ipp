#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/render/materialbuffer.hpp>
#include <ipp/render/texturebitmap.hpp>

namespace ipp {
namespace scene {
namespace render {

/**
 * ipp::render::Effect object can contain arbitrary uniform variables and passes.
 * Instead of searching variable/pass every frame this object caches references to variables and
 * passes used by RenderSystem in named/typed properties and exposes setters/getters.
 */
class MaterialEffect : public virtual ipp::render::Effect {
public:
    MaterialEffect(std::unique_ptr<resource::ResourceBuffer> data, const json& configuration);

    /**
     * @brief Override base ResourceTypeName
     */
    const std::string& getResourceTypeName() override
    {
        return ResourceTypeName;
    }

    /**
     * @brief Override base ResourceTypeName.
     */
    static const std::string ResourceTypeName;

private:
    const ipp::render::Effect::Variable* _viewPosition;
    const ipp::render::Effect::Variable* _viewDirection;
    const ipp::render::Effect::Variable* _worldMatrix;
    const ipp::render::Effect::Variable* _viewMatrix;
    const ipp::render::Effect::Variable* _projectionMatrix;
    const ipp::render::Effect::Variable* _viewProjectionMatrix;
    const ipp::render::Effect::Variable* _worldViewMatrix;
    const ipp::render::Effect::Variable* _worldViewProjectionMatrix;
    const ipp::render::Effect::Variable* _normalMatrix;

public:
    //----------------------------------------------------------------------------------------------
    // Transform properties
    //----------------------------------------------------------------------------------------------
    glm::mat4 readWorld(const ipp::render::MaterialBuffer& material) const;
    glm::mat4 readView(const ipp::render::MaterialBuffer& material) const;
    glm::mat4 readProjection(const ipp::render::MaterialBuffer& material) const;
    glm::mat4 readWorldView(const ipp::render::MaterialBuffer& material) const;
    glm::mat4 readWorldViewProjection(const ipp::render::MaterialBuffer& material) const;

    void writeWorldViewProjection(ipp::render::MaterialBuffer& material,
                                  const glm::vec3& viewPosition,
                                  const glm::vec3& viewDirection,
                                  const glm::mat4& world,
                                  const glm::mat4& view,
                                  const glm::mat4& projection,
                                  const glm::mat4& viewProjection,
                                  const glm::mat4& worldView,
                                  const glm::mat4& worldViewProjection,
                                  const glm::mat3& normalMatirx) const;

private:
    const ipp::render::Effect::Variable* _skinningMatrices;

public:
    //----------------------------------------------------------------------------------------------
    // Skinning properties
    //----------------------------------------------------------------------------------------------
    void writeSkinningMatrices(ipp::render::MaterialBuffer& material,
                               const ArmatureComponent* armatureComponent) const;

private:
    const ipp::render::Effect::Variable* _diffuseColor;
    const ipp::render::Effect::Variable* _diffuseIntensity;
    const ipp::render::Effect::Variable* _diffuseAmbientIntensity;
    const ipp::render::Effect::Variable* _specularColor;
    const ipp::render::Effect::Variable* _specularIntensity;
    const ipp::render::Effect::Variable* _specularHardness;

public:
    //----------------------------------------------------------------------------------------------
    // Color properties
    //----------------------------------------------------------------------------------------------
    glm::vec3 readDiffuseColor(const ipp::render::MaterialBuffer& material) const;

    float readDiffuseIntensity(const ipp::render::MaterialBuffer& material) const;

    float readDiffuseAmbientIntensity(const ipp::render::MaterialBuffer& material) const;

    void writeDiffuse(ipp::render::MaterialBuffer& material,
                      const glm::vec3& color,
                      float intensity,
                      float ambientIntensity);

    glm::vec3 readSpecularColor(const ipp::render::MaterialBuffer& material) const;

    float readSpecularIntensity(const ipp::render::MaterialBuffer& material) const;

    float readSpecularHardness(const ipp::render::MaterialBuffer& material) const;

    void writeSpecular(ipp::render::MaterialBuffer& material,
                       const glm::vec3& color,
                       float intensity,
                       float hardness);

private:
    const ipp::render::Effect::Variable* _diffuseTextureSampler;
    const ipp::render::Effect::Variable* _specularTextureSampler;
    const ipp::render::Effect::Variable* _normalTextureSampler;

public:
    //----------------------------------------------------------------------------------------------
    // Texture properties
    //----------------------------------------------------------------------------------------------
    void writeDiffuseTextureSampler(ipp::render::MaterialBuffer& material,
                                    ipp::render::gl::Texture2D::UniformSampler sampler) const;

    void writeSpecularTextureSampler(ipp::render::MaterialBuffer& material,
                                     ipp::render::gl::Texture2D::UniformSampler sampler) const;

    void writeNormalTextureSampler(ipp::render::MaterialBuffer& material,
                                   ipp::render::gl::Texture2D::UniformSampler sampler) const;

private:
    const ipp::render::Effect::Variable* _lightViewProjectionMatrix;
    const ipp::render::Effect::Variable* _lightShadowMapMatrix;
    const ipp::render::Effect::Variable* _lightDirection;
    const ipp::render::Effect::Variable* _lightColor;
    const ipp::render::Effect::Variable* _lightAmbientDiffuseIntensity;
    const ipp::render::Effect::Variable* _shadowMapSampler;

public:
    //----------------------------------------------------------------------------------------------
    // Light properties
    //----------------------------------------------------------------------------------------------
    glm::vec3 readLightDirection(const ipp::render::MaterialBuffer& material) const;
    glm::vec3 readLightColor(const ipp::render::MaterialBuffer& material) const;
    float readLightAmbientDiffuseIntensity(const ipp::render::MaterialBuffer& material) const;
    glm::mat4 readLightViewProjectionMatrix(const ipp::render::MaterialBuffer& material) const;
    glm::mat4 readLightShadowMapMatrix(const ipp::render::MaterialBuffer& material) const;

    void writeLightDirectional(ipp::render::MaterialBuffer& material,
                               const glm::vec3& direction,
                               const glm::vec3& color,
                               float ambientDiffuseIntensity,
                               const glm::mat4& lightViewProjectionMatrix,
                               const glm::mat4& ligthShadowMapMatrix) const;

    void writeShadowMapSampler(ipp::render::MaterialBuffer& material,
                               ipp::render::gl::Texture2D::UniformSampler sampler) const;

private:
    const ipp::render::Effect::Pass* _particlePass;
    const ipp::render::Effect::Pass* _lightDirectionalPass;
    const ipp::render::Effect::Pass* _shadowMapDirectionalPass;

public:
    //----------------------------------------------------------------------------------------------
    // Effect passes for material, skipped if null
    //----------------------------------------------------------------------------------------------
    /**
     * @brief Renders object to a shadow map using directional light
     */
    const ipp::render::Effect::Pass* getShadowMapDirectionalPass() const
    {
        return _shadowMapDirectionalPass;
    }

    /**
     * @brief Renders object to front buffer using directional light for shading
     * Shadow map is provided by shadowMapDirectional pass rendered for each directional light
     */
    const ipp::render::Effect::Pass* getLightDirectionalPass() const
    {
        return _lightDirectionalPass;
    }

    /**
     * @brief Renders object to front buffer using directional light for shading
     *
     */
    const ipp::render::Effect::Pass* getLightDirectionalPassTransparent() const
    {
        return _lightDirectionalPass;
    }

    /**
     * @brief Renders object to front buffer with no shading/occlusion
     */
    const ipp::render::Effect::Pass* getParticlePass() const
    {
        return _particlePass;
    }
};
}
}
}
