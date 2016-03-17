#include <ipp/scene/render/materialeffect.hpp>
#include <ipp/scene/render/armaturecomponent.hpp>
#include <ipp/render/texturebitmap.hpp>
#include <ipp/render/armature.hpp>
#include <ipp/resource/resourcemanager.hpp>
#include <ipp/context.hpp>

using namespace std;
using namespace glm;
using namespace ipp::resource;
using namespace ipp::render;
using namespace ipp::render::gl;
using namespace ipp::scene::render;

template <>
const string SharedResourceT<MaterialEffect>::ResourceTypeName = "SceneMaterialEffect";
const string MaterialEffect::ResourceTypeName = SharedResourceT<MaterialEffect>::ResourceTypeName;

std::string getShaderShadowMapHeader(const json& configuration)
{
    ostringstream header;
    header << "#define SHADOW_MAP_TECHNIQUE_NONE 0 \n"
           << "#define SHADOW_MAP_TECHNIQUE_PCF 1\n"
           << "#define SHADOW_MAP_TECHNIQUE_VSF 2 \n";
    auto configurationRender = configuration["render"];
    string shadowMapTechnique = configurationRender["shadowMapTechnique"];
    if (shadowMapTechnique == "") {
        shadowMapTechnique = "NONE";
    }
    std::transform(
        shadowMapTechnique.begin(), shadowMapTechnique.end(), shadowMapTechnique.begin(),
        [](char ch) { return std::use_facet<std::ctype<char>>(std::locale()).toupper(ch); });

    header << "#define SHADOW_MAP_TECHNIQUE SHADOW_MAP_TECHNIQUE_" << shadowMapTechnique << "\n";

    return header.str();
}

std::string getVertexShaderHeaderFromConfiguration(const json& configuration)
{
    ostringstream header;
    header << getShaderShadowMapHeader(configuration);
    return header.str();
}

string getFragmentShaderHeaderFromConfiguration(const json& configuration)
{
    ostringstream header;
    header << getShaderShadowMapHeader(configuration);
    return header.str();
}

MaterialEffect::MaterialEffect(unique_ptr<ResourceBuffer> data, const json& configuration)
    : ipp::render::Effect{move(data), getVertexShaderHeaderFromConfiguration(configuration),
                          getFragmentShaderHeaderFromConfiguration(configuration)}
{
    _viewPosition = findVariable("viewPosition");
    _viewDirection = findVariable("viewDirection");
    _worldMatrix = findVariable("worldMatrix");
    _viewMatrix = findVariable("viewMatrix");
    _projectionMatrix = findVariable("projectionMatrix");
    _viewProjectionMatrix = findVariable("viewProjectionMatrix");
    _worldViewMatrix = findVariable("worldViewMatrix");
    _worldViewProjectionMatrix = findVariable("worldViewProjectionMatrix");
    _normalMatrix = findVariable("normalMatrix");

    _skinningMatrices = findVariable("skinningMatrices");

    _diffuseColor = findVariable("diffuseColor");
    _diffuseIntensity = findVariable("diffuseIntensity");
    _diffuseAmbientIntensity = findVariable("diffuseAmbientIntensity");

    _specularColor = findVariable("specularColor");
    _specularIntensity = findVariable("specularIntensity");
    _specularHardness = findVariable("specularHardness");

    _diffuseTextureSampler = findVariable("diffuseTextureSampler");
    _specularTextureSampler = findVariable("specularTextureSampler");
    _normalTextureSampler = findVariable("normalTextureSampler");

    _lightViewProjectionMatrix = findVariable("lightViewProjectionMatrix");
    _lightShadowMapMatrix = findVariable("lightShadowMapMatrix");
    _lightDirection = findVariable("lightDirection");
    _lightColor = findVariable("lightColor");
    _lightAmbientDiffuseIntensity = findVariable("lightAmbientDiffuseIntensity");
    _shadowMapSampler = findVariable("shadowMapSampler");

    _lightDirectionalPass = findPass("lightDirectional");
    _shadowMapDirectionalPass = findPass("shadowMapDirectional");
    _particlePass = findPass("particle");
}

mat4 MaterialEffect::readWorld(const MaterialBuffer& material) const
{
    mat4 result;
    material.getVariableValue(_worldMatrix, 1, &result);
    return result;
}

mat4 MaterialEffect::readView(const MaterialBuffer& material) const
{
    mat4 result;
    material.getVariableValue(_viewMatrix, 1, &result);
    return result;
}

mat4 MaterialEffect::readProjection(const MaterialBuffer& material) const
{
    mat4 result;
    material.getVariableValue(_projectionMatrix, 1, &result);
    return result;
}

mat4 MaterialEffect::readWorldView(const MaterialBuffer& material) const
{
    mat4 result;
    material.getVariableValue(_worldViewMatrix, 1, &result);
    return result;
}

mat4 MaterialEffect::readWorldViewProjection(const MaterialBuffer& material) const
{
    mat4 result;
    material.getVariableValue(_worldViewProjectionMatrix, 1, &result);
    return result;
}

void MaterialEffect::writeWorldViewProjection(MaterialBuffer& material,
                                              const vec3& viewPosition,
                                              const vec3& viewDirection,
                                              const mat4& world,
                                              const mat4& view,
                                              const mat4& projection,
                                              const mat4& viewProjection,
                                              const mat4& worldView,
                                              const mat4& worldViewProjection,
                                              const mat3& normalMatirx) const
{
    material.setVariableValue(_viewPosition, 1, &viewPosition);
    material.setVariableValue(_viewDirection, 1, &viewDirection);
    material.setVariableValue(_worldMatrix, 1, &world);
    material.setVariableValue(_viewMatrix, 1, &view);
    material.setVariableValue(_projectionMatrix, 1, &projection);
    material.setVariableValue(_viewProjectionMatrix, 1, &viewProjection);
    material.setVariableValue(_worldViewMatrix, 1, &worldView);
    material.setVariableValue(_worldViewProjectionMatrix, 1, &worldViewProjection);

    auto normalMatrix4 = mat4(normalMatirx);
    material.setVariableValue(_normalMatrix, 1, &normalMatrix4);
}

void MaterialEffect::writeSkinningMatrices(MaterialBuffer& material,
                                           const ArmatureComponent* armatureComponent) const
{
    if (_skinningMatrices == nullptr) {
        return;
    }

    auto& bones = armatureComponent->getArmature()->getBones();
    auto& bonePoses = armatureComponent->getBonePoses();

    // thread local array of matrices used to store bone parent transforms since bones are
    // topologically sorted
    static vector<mat4> poseMatrixCache;
    poseMatrixCache.clear();
    poseMatrixCache.reserve(bonePoses.size());

    // instead of using an intermediate array of vec4 for typed setter pack skinning matrices
    // directly in to material buffer memory
    vec4* skinningMatricesUniforms = reinterpret_cast<vec4*>(material.getVariableBuffer() +
                                                             _skinningMatrices->getBufferOffset());
    size_t skinningIndex = 0;
    size_t boneCount = bones.size();
    for (size_t boneIndex = 0; boneIndex < boneCount; boneIndex++) {
        auto& bone = bones[boneIndex];
        auto parentIndex = bone.getParentIndex();
        mat4 poseMatrix = static_cast<mat4>(bonePoses[boneIndex]);
        if (parentIndex >= 0) {
            poseMatrix = poseMatrixCache[parentIndex] * poseMatrix;
        }
        poseMatrixCache.push_back(poseMatrix);

        if (bone.isDeform()) {
            poseMatrix = poseMatrix * bone.getInverseBindPose();
            for (size_t i = 0; i < 3; i++) {
                for (size_t j = 0; j < 4; j++) {
                    skinningMatricesUniforms[skinningIndex * 3 + i][j] = poseMatrix[j][i];
                }
            }
            ++skinningIndex;
        }
    }
}

vec3 MaterialEffect::readDiffuseColor(const MaterialBuffer& material) const
{
    vec3 result;
    material.getVariableValue(_diffuseColor, 1, &result);
    return result;
}

float MaterialEffect::readDiffuseIntensity(const ipp::render::MaterialBuffer& material) const
{
    float result;
    material.getVariableValue(_diffuseIntensity, 1, &result);
    return result;
}

float MaterialEffect::readDiffuseAmbientIntensity(const ipp::render::MaterialBuffer& material) const
{
    float result;
    material.getVariableValue(_diffuseAmbientIntensity, 1, &result);
    return result;
}

void MaterialEffect::writeDiffuse(MaterialBuffer& material,
                                  const vec3& color,
                                  float intensity,
                                  float ambientIntensity)
{
    material.setVariableValue(_diffuseColor, 1, &color);
    material.setVariableValue(_diffuseIntensity, 1, &intensity);
    material.setVariableValue(_diffuseAmbientIntensity, 1, &ambientIntensity);
}

glm::vec3 MaterialEffect::readSpecularColor(const ipp::render::MaterialBuffer& material) const
{
    vec3 result;
    material.getVariableValue(_specularColor, 1, &result);
    return result;
}

float MaterialEffect::readSpecularIntensity(const ipp::render::MaterialBuffer& material) const
{
    float result;
    material.getVariableValue(_specularIntensity, 1, &result);
    return result;
}

float MaterialEffect::readSpecularHardness(const ipp::render::MaterialBuffer& material) const
{
    float result;
    material.getVariableValue(_specularHardness, 1, &result);
    return result;
}

void MaterialEffect::writeSpecular(MaterialBuffer& material,
                                   const vec3& color,
                                   float intensity,
                                   float hardness)
{
    material.setVariableValue(_specularColor, 1, &color);
    material.setVariableValue(_specularIntensity, 1, &intensity);
    material.setVariableValue(_specularHardness, 1, &hardness);
}
void MaterialEffect::writeDiffuseTextureSampler(MaterialBuffer& material,
                                                Texture2D::UniformSampler sampler) const
{
    material.setVariableValue(_diffuseTextureSampler, 1, &sampler);
}

void MaterialEffect::writeSpecularTextureSampler(ipp::render::MaterialBuffer& material,
                                                 Texture2D::UniformSampler sampler) const
{
    material.setVariableValue(_specularTextureSampler, 1, &sampler);
}

void MaterialEffect::writeNormalTextureSampler(MaterialBuffer& material,
                                               Texture2D::UniformSampler sampler) const
{
    material.setVariableValue(_normalTextureSampler, 1, &sampler);
}
vec3 MaterialEffect::readLightDirection(const MaterialBuffer& material) const
{
    vec3 result;
    material.getVariableValue(_lightDirection, 1, &result);
    return result;
}

vec3 MaterialEffect::readLightColor(const MaterialBuffer& material) const
{
    vec3 result;
    material.getVariableValue(_lightColor, 1, &result);
    return result;
}

float MaterialEffect::readLightAmbientDiffuseIntensity(const MaterialBuffer& material) const
{
    float result;
    material.getVariableValue(_lightViewProjectionMatrix, 1, &result);
    return result;
}

mat4 MaterialEffect::readLightViewProjectionMatrix(const MaterialBuffer& material) const
{
    mat4 result;
    material.getVariableValue(_lightViewProjectionMatrix, 1, &result);
    return result;
}

mat4 MaterialEffect::readLightShadowMapMatrix(const MaterialBuffer& material) const
{
    mat4 result;
    material.getVariableValue(_lightShadowMapMatrix, 1, &result);
    return result;
}

void MaterialEffect::writeLightDirectional(MaterialBuffer& material,
                                           const vec3& direction,
                                           const vec3& color,
                                           float ambientDiffuseIntensity,
                                           const mat4& lightViewProjectionMatrix,
                                           const mat4& lightShadowMapMatrix) const
{
    material.setVariableValue(_lightDirection, 1, &direction);
    material.setVariableValue(_lightColor, 1, &color);
    material.setVariableValue(_lightAmbientDiffuseIntensity, 1, &ambientDiffuseIntensity);
    material.setVariableValue(_lightViewProjectionMatrix, 1, &lightViewProjectionMatrix);
    material.setVariableValue(_lightShadowMapMatrix, 1, &lightShadowMapMatrix);
}

void MaterialEffect::writeShadowMapSampler(MaterialBuffer& material,
                                           gl::Texture2D::UniformSampler sampler) const
{
    material.setVariableValue(_shadowMapSampler, 1, &sampler);
}
