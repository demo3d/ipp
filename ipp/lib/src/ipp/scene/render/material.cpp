#include <ipp/scene/render/material.hpp>
#include <ipp/resource/resourcemanager.hpp>
#include <ipp/context.hpp>
#include <ipp/schema/resource/scene/material_generated.h>

using namespace std;
using namespace glm;
using namespace ipp;
using namespace ipp::resource;
using namespace ipp::render;
using namespace ipp::scene::render;

template <>
const string SharedResourceT<Material>::ResourceTypeName = "SceneMaterial";

shared_ptr<MaterialEffect> getMaterialEffect(ResourceManager& resourceManager, const char* data)
{
    auto materialDefinition = schema::resource::scene::GetMaterialDefinition(data);
    return resourceManager.requestSharedResource<MaterialEffect>(
        materialDefinition->effectResourcePath()->str(),
        resourceManager.getContext().getConfiguration());
}

Material::Material(std::unique_ptr<resource::ResourceBuffer> data)
    : SharedResourceT<Material>(data->getResourceManager(), data->getResourcePath())
    , _effect{getMaterialEffect(data->getResourceManager(), data->getData())}
    , _materialBuffer{_effect}
{
    auto materialDefinition = schema::resource::scene::GetMaterialDefinition(data->getData());
    _isTransparent = materialDefinition->transparent();
    _isShadowCaster = materialDefinition->shadowCaster();

    _effect->writeShadowMapSampler(_materialBuffer, {0});
    _textureCount = 1;

    for (auto propertyData : *materialDefinition->properties()) {
        auto propertyName = propertyData->name()->str();
        auto property = _effect->findVariable(propertyName);
        switch (propertyData->value_type()) {
            case schema::resource::scene::MaterialPropertyKind_MaterialTexture: {
                auto propertyTextureData =
                    reinterpret_cast<const schema::resource::scene::MaterialTexture*>(
                        propertyData->value());
                auto textureResource =
                    data->getResourceManager().requestSharedResource<BitmapTexture2D>(
                        propertyTextureData->resourcePath()->str());
                if (propertyName == "diffuseTexture") {
                    _diffuseTexture = make_unique<TextureProperty>(propertyName, _textureCount,
                                                                   move(textureResource));
                    _effect->writeDiffuseTextureSampler(_materialBuffer,
                                                        {static_cast<GLuint>(_textureCount)});
                    _textureCount++;
                }
                else if (propertyName == "specularTexture") {
                    _specularTexture = make_unique<TextureProperty>(propertyName, _textureCount,
                                                                    move(textureResource));
                    _effect->writeSpecularTextureSampler(_materialBuffer,
                                                         {static_cast<GLuint>(_textureCount)});
                    _textureCount++;
                }
                else {
                    IVL_LOG(Warning, "Unknown texture property slot {} (texture resource : {}})",
                            propertyName, textureResource->getResourcePath());
                }
            } break;

            case schema::resource::scene::MaterialPropertyKind_MaterialFloatUniform: {
                auto propertyFloatData =
                    reinterpret_cast<const schema::resource::scene::MaterialFloatUniform*>(
                        propertyData->value());
                switch (propertyFloatData->value()->size()) {
                    case 1: {
                        _materialBuffer.setVariableValue(property, 1,
                                                         propertyFloatData->value()->data());
                    } break;
                    case 2: {
                        auto value = glm::make_vec2(propertyFloatData->value()->data());
                        _materialBuffer.setVariableValue(property, 1, &value);
                    } break;
                    case 3: {
                        auto value = glm::make_vec3(propertyFloatData->value()->data());
                        _materialBuffer.setVariableValue(property, 1, &value);
                    } break;
                    case 4: {
                        auto value = glm::make_vec4(propertyFloatData->value()->data());
                        _materialBuffer.setVariableValue(property, 1, &value);
                    } break;
                    case 16: {
                        auto value = glm::make_mat4(propertyFloatData->value()->data());
                        _materialBuffer.setVariableValue(property, 1, &value);
                    } break;
                    default: {
                        IVL_LOG_THROW_ERROR(logic_error, "Unknown uniform float vector size {}",
                                            propertyFloatData->value()->size());
                    }
                }
            } break;
            default:
                IVL_LOG_THROW_ERROR(logic_error, "Unknown material property value type {}",
                                    static_cast<int>(propertyData->value_type()));
        }
    }
}
