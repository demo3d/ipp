#include <ipp/resource/resourcemanager.hpp>
#include <ipp/resource/stringresource.hpp>
#include <ipp/render/effect.hpp>
#include <ipp/render/materialbuffer.hpp>
#include <ipp/schema/resource/render/effect_generated.h>

using namespace std;
using namespace ipp::render;
using namespace ipp::resource;

template <>
const string SharedResourceT<Effect>::ResourceTypeName = "RenderEffect";

Effect::Pass::Pass(Effect& effect,
                   size_t passIndex,
                   string name,
                   unique_ptr<gl::ShaderProgram> shaderProgram)
    : _effect{effect}
    , _passIndex{passIndex}
    , _name{move(name)}
    , _shaderProgram{move(shaderProgram)}
{
    IVL_LOG(Trace, "Creating Pass : {} for Effect : {}", _name, effect.getResourcePath());
    auto& effectVariables = effect.getVariables();
    for (auto& uniform : _shaderProgram->getUniformBindings()) {
        auto variableIt = effectVariables.find(uniform.first);
        if (variableIt != effectVariables.end()) {
            // TODO: Compare Effect UniformVariable to ShaderProgram one.
            IVL_LOG(Trace, "Adding Variable : {} to Pass : {}",
                    variableIt->second.getUniformVariable().getName(), _name);
            _uniformVariableMap[&variableIt->second] = uniform.second.get();
        }
    }
}

Effect::Pass::Pass(Effect::Pass&& other)
    : _effect{other._effect}
    , _passIndex{other._passIndex}
    , _name{move(other._name)}
    , _shaderProgram{move(other._shaderProgram)}
    , _uniformVariableMap{move(other._uniformVariableMap)}
{
}

gl::Binding<gl::ShaderProgram> Effect::Pass::bindWithMaterial(const MaterialBuffer& material) const
{
    assert(_shaderProgram != nullptr);

    gl::Binding<gl::ShaderProgram> binding{*_shaderProgram};
    gl::GlError::assertValidateState();

    auto sourceBuffer = static_cast<const uint8_t*>(material.getVariableBuffer());
    for (auto uniform : _uniformVariableMap) {
        _shaderProgram->setUniform(binding, *uniform.second,
                                   sourceBuffer + uniform.first->getBufferOffset());
    }
    gl::GlError::assertValidateState();

    return binding;
}

Effect::Effect(unique_ptr<ResourceBuffer> data, string vertexHeader, string fragmentHeader)
    : SharedResourceT<Effect>(data->getResourceManager(), data->getResourcePath())
{
    _bufferSize = 0;
    auto effectData = schema::resource::render::effect::GetEffect(data->getData());

    for (const auto& vertexAttribute : *effectData->vertexAttributes()) {
        _vertexAttributes.emplace_back(vertexAttribute->c_str());
    }

    for (const auto& passData : *effectData->passes()) {
        auto passName = passData->name()->str();
        IVL_LOG(Trace, "Creating ShaderProgram for Effect {} Pass {}", getResourcePath(), passName);

        vector<string> vertexSources{vertexHeader + "\n", passData->vertexHeader()->str()};
        for (auto vertexResourcePath : *passData->vertexResourcePaths()) {
            auto resource = data->getResourceManager().requestSharedResource<StringResource>(
                vertexResourcePath->str());
            vertexSources.push_back(resource->getData());
        }
        auto vertexShader = gl::compileShader(gl::ShaderKind::Vertex, vertexSources);

        vector<string> fragmentSources{fragmentHeader + "\n", passData->fragmentHeader()->str()};
        for (auto fragmentResourcePath : *passData->fragmentResourcePaths()) {
            auto resource = data->getResourceManager().requestSharedResource<StringResource>(
                fragmentResourcePath->str());
            fragmentSources.push_back(resource->getData());
        }
        auto fragmentShader = gl::compileShader(gl::ShaderKind::Fragment, fragmentSources);

        auto shaderProgram = make_unique<gl::ShaderProgram>(_vertexAttributes, vertexShader.get(),
                                                            fragmentShader.get());

        for (const auto& uniform : shaderProgram->getUniformBindings()) {
            if (_variables.find(uniform.first) != _variables.end()) {
                // TODO: Check for conflicting variable types
                continue;
            }
            IVL_LOG(Info, "Adding Effect Variable : {}", uniform.first);
            _variables.emplace(string(uniform.first),
                               Variable{uniform.second->getVariable(), _bufferSize});
            _bufferSize += uniform.second->getVariable().getSize();
        }

        _passes.emplace_back(*this, _passes.size(), passName, move(shaderProgram));
    }
}

const Effect::Variable* Effect::findVariable(const string& name) const
{
    auto& materialVariables = getVariables();
    auto materialIt = materialVariables.find(name);
    if (materialIt == materialVariables.end()) {
        return nullptr;
    }
    return &materialIt->second;
}

const Effect::Pass* Effect::findPass(const string& name) const
{
    for (const auto& pass : _passes) {
        if (pass.getName() == name) {
            return &pass;
        }
    }
    return nullptr;
}
