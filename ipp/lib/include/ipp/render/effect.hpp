#pragma once

#include <ipp/shared.hpp>
#include <ipp/log.hpp>
#include <ipp/resource/package.hpp>
#include "gl/uniform.hpp"
#include "gl/program.hpp"

namespace ipp {
namespace render {

/**
 * @brief Collection of render Passes that share Material Variables.
 */
class Effect : public resource::SharedResourceT<Effect> {
public:
    /**
     * @brief gl::UniformVariable paired with byte offset for the variable inside Material buffer.
     */
    class Variable {
    private:
        const gl::UniformVariable _uniformVariable;
        size_t _bufferOffset;

    public:
        Variable(gl::UniformVariable uniformVariable, size_t bufferOffset)
            : _uniformVariable{uniformVariable}
            , _bufferOffset{bufferOffset}
        {
        }

        /**
         * @brief Uniform variable this Material variable maps to.
         */
        const gl::UniformVariable& getUniformVariable() const
        {
            return _uniformVariable;
        }

        /**
         * @brief Variable memory offset (in bytes) inside of Material buffer.
         */
        size_t getBufferOffset() const
        {
            return _bufferOffset;
        }
    };

    /**
     * @brief Pass is a ShaderProgram with a map between MaterialBuffer Variable and
     *        program UniformBinding.
     *
     * Each pass has a unique name string and a renderPass string specifiying which render pass it
     * should be rendered in (eg. lighting, shadow, etc.).
     */
    class Pass : public NonCopyable {
    private:
        Effect& _effect;
        size_t _passIndex;
        std::string _name;
        std::unique_ptr<gl::ShaderProgram> _shaderProgram;
        std::unordered_map<const Variable*, const gl::ShaderProgram::UniformBinding*>
            _uniformVariableMap;

    public:
        Pass(Effect& effect,
             size_t passIndex,
             std::string name,
             std::unique_ptr<gl::ShaderProgram> shaderProgram);

        /**
         * Move constructor implemented so Pass can be stored directly in a std::vector
         * @note Don't use this to move pass outside of Effect.
         */
        Pass(Pass&& other);

        /**
         * @brief Bind effect ShaderProgram to GL context with uniforms values from Material.
         */
        gl::Binding<gl::ShaderProgram> bindWithMaterial(const MaterialBuffer& material) const;

        /**
         * @brief Pass parent Effect object.
         */
        Effect& getEffect() const
        {
            return _effect;
        }

        /**
         * @brief Index of Pass object inside parent Effect .
         */
        size_t getPassIndex() const
        {
            return _passIndex;
        }

        /**
         * @brief Unique name identifier within Effect.
         */
        const std::string& getName() const
        {
            return _name;
        }

        /**
         * @brief Shader program this Pass uses to render.
         */
        gl::ShaderProgram& getShaderProgram() const
        {
            return *_shaderProgram;
        }
    };

private:
    std::vector<std::string> _vertexAttributes;
    std::vector<Pass> _passes;
    std::unordered_map<std::string, Variable> _variables;
    size_t _bufferSize;

protected:
    /**
     * @note Protected default constructor is exposed *only* for virtual inheritance.
     */
    Effect()
        : SharedResourceT<Effect>(*static_cast<resource::ResourceManager*>(nullptr), "")
    {
        throw std::logic_error(
            "Not implemented - dummy constructor provided for virtual inheritance.");
    }

public:
    Effect(std::unique_ptr<resource::ResourceBuffer> data,
           std::string vertexHeader = "",
           std::string fragmentHeader = "");

    /**
     * @brief Return Effect Variable pointer for variable name or nullptr if not found.
     */
    const Variable* findVariable(const std::string& name) const;

    /**
     * @brief Returns a Pass with specified name or nullptr if no Pass with name found.
     */
    const Pass* findPass(const std::string& name) const;

    /**
     * @brief All Effect Passes.
     */
    const std::vector<Pass>& getPasses() const
    {
        return _passes;
    }

    /**
     * @brief Variables defined in all Effect Passes.
     */
    const std::unordered_map<std::string, Variable>& getVariables() const
    {
        return _variables;
    }

    /**
     * @brief Vertex attribute names in order they will be provided by source mesh vertex buffer.
     */
    const std::vector<std::string>& getVertexAttributes() const
    {
        return _vertexAttributes;
    }

    /**
     * @brief Buffer size required to store all effect variables.
     */
    size_t getBufferSize() const
    {
        return _bufferSize;
    }
};
}
}
