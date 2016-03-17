#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include "binding.hpp"
#include "handle.hpp"
#include "shader.hpp"
#include "uniform.hpp"
#include "vertexdefinition.hpp"

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief Wraps GL shader program object handle and provides typed access to uniform variables.
 */
class ShaderProgram final {
public:
    friend class Binding<ShaderProgram>;

    class AttributeBinding final {
    private:
        VertexDefinition::AttributeName _name;
        size_t _index;
        size_t _location;

    public:
        AttributeBinding(VertexDefinition::AttributeName name, size_t index, size_t location);

        /**
         * @brief Parse attribute binding name in to AttributeName enum and index value.
         */
        AttributeBinding(const std::string& bindingName, size_t location);

        VertexDefinition::AttributeName getName() const
        {
            return _name;
        }

        /**
         * @brief Attribute index from 0, >0 for attributes that can have multiple values (eg. uv)
         */
        size_t getIndex() const
        {
            return _index;
        }

        /**
         * @brief Attribute program binding location.
         */
        size_t getLocation() const
        {
            return _location;
        }

        std::string getIdentifier() const;
    };

    /**
     * @brief UniformVariable paired with it's binding location inside of GL shader program.
     */
    class UniformBinding final : public NonCopyable {
    private:
        UniformVariable _variable;
        GLint _bindingLocation;

    public:
        UniformBinding(UniformVariable variable, GLint bindingLocation)
            : _variable{variable}
            , _bindingLocation{bindingLocation}
        {
        }

        /**
         * @brief Variable definition.
         */
        const UniformVariable& getVariable() const
        {
            return _variable;
        }

        /**
         * @brief Variable binding location in GL shader program object.
         */
        GLint getBindingLocation() const
        {
            return _bindingLocation;
        }
    };

private:
    /**
     * Wrap glDeleteProgram because it can be implemented in differently between platforms
     * and they don't play well with HandleDeleter template.
     */
    static void deleteProgram(GLuint handle)
    {
        glDeleteProgram(handle);
    }

    /**
     * @brief Alias unique_ptr with custom deleter that will release GL shader program handle
     */
    using ProgramHandle = std::unique_ptr<void, HandleDeleter<deleteProgram>>;

    /**
     * @brief Check if binding object is valid and program is currently bound to context.
     */
    bool isBound() const;

    /**
     * @brief Bind ShaderProgram to GL context
     */
    void bind() const;

    /**
     * @brief Unbind ShaderProgram from context
     */
    void unbind() const;

    ProgramHandle _handle;
    std::vector<AttributeBinding> _attributeBindings;
    std::unordered_map<std::string, std::unique_ptr<UniformBinding>> _uniformBindings;

public:
    ShaderProgram(const std::vector<std::string>& vertexAttributes,
                  GLuint vertexShader,
                  GLuint fragmentShader);

    /**
     * @brief Set uniform variable specified by binding to value from source.
     */
    void setUniform(Binding<ShaderProgram>& binding,
                    const UniformBinding& uniformBinding,
                    const void* source);

    GLuint getHandle() const
    {
        return _handle.get();
    }

    /**
     * @brief Shader program defined list of used UniformBindings.
     */
    const std::unordered_map<std::string, std::unique_ptr<UniformBinding>>& getUniformBindings()
        const
    {
        return _uniformBindings;
    }

    /**
     * @brief Return a VertexDefinition object that maps source vertex definition to program.
     * @note Source vertex definition must contain all attributes used by vertex.
     */
    VertexDefinition getProgramVertexDefinition(const VertexDefinition& source) const;

    /**
     * @brief Resource specified list of shader vertex attribute names.
     */
    const std::vector<AttributeBinding>& getAttributeBindings() const
    {
        return _attributeBindings;
    }
};
}
}
}
