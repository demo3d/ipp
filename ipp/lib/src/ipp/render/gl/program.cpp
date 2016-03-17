#include <ipp/render/gl/program.hpp>
#include <ipp/render/gl/error.hpp>

using namespace std;
using namespace ipp::render::gl;

ShaderProgram::AttributeBinding::AttributeBinding(VertexDefinition::AttributeName name,
                                                  size_t index,
                                                  size_t location)
    : _name{name}
    , _index{index}
    , _location{location}
{
}

ShaderProgram::AttributeBinding::AttributeBinding(const string& bindingName, size_t location)
    : _location{location}
{
#define ATTRIBUTE_NAME_PARSE(ENUM, NAME)                                                           \
    if (bindingName.find(#NAME) == 0) {                                                            \
        _name = VertexDefinition::AttributeName::ENUM;                                             \
        _index = 0;                                                                                \
        return;                                                                                    \
    }

#define ATTRIBUTE_INDEXED_NAME_PARSE(ENUM, NAME)                                                   \
    if (bindingName.find(#NAME) == 0) {                                                            \
        _name = VertexDefinition::AttributeName::ENUM;                                             \
        _index = static_cast<size_t>(bindingName[sizeof(#NAME) - 1] - '0');                        \
        return;                                                                                    \
    }

    ATTRIBUTE_NAME_PARSE(Position, position)
    ATTRIBUTE_NAME_PARSE(Normal, normal)
    ATTRIBUTE_INDEXED_NAME_PARSE(Uv, uv)
    ATTRIBUTE_INDEXED_NAME_PARSE(Color, color)
    ATTRIBUTE_NAME_PARSE(SkinningIndices, skinningIndices)
    ATTRIBUTE_NAME_PARSE(SkinningWeights, skinningWeights)
    ATTRIBUTE_NAME_PARSE(GroupIndices, groupIndices)
    ATTRIBUTE_NAME_PARSE(GroupWeights, groupWeights)
}

std::string ShaderProgram::AttributeBinding::getIdentifier() const
{
    std::string identifier;

#define ATTRIBUTE_NAME_MAP(NAME)                                                                   \
    case VertexDefinition::AttributeName::NAME:                                                    \
        identifier = #NAME;                                                                        \
        identifier[0] = static_cast<char>(tolower(identifier[0]));                                 \
        return identifier;

#define ATTRIBUTE_INDEXED_NAME_MAP(NAME)                                                           \
    case VertexDefinition::AttributeName::NAME:                                                    \
        identifier = #NAME;                                                                        \
        identifier[0] = static_cast<char>(tolower(identifier[0]));                                 \
        return identifier + to_string(_index);

    switch (_name) {
        ATTRIBUTE_NAME_MAP(Position)
        ATTRIBUTE_NAME_MAP(Normal)
        ATTRIBUTE_INDEXED_NAME_MAP(Uv)
        ATTRIBUTE_INDEXED_NAME_MAP(Color)
        ATTRIBUTE_NAME_MAP(SkinningIndices)
        ATTRIBUTE_NAME_MAP(SkinningWeights)
        ATTRIBUTE_NAME_MAP(GroupIndices)
        ATTRIBUTE_NAME_MAP(GroupWeights)
    }

#undef ATTRIBUTE_INDEXED_NAME_MAP
#undef ATTRIBUTE_NAME_MAP
}

bool ShaderProgram::isBound() const
{
    GLuint handle = getHandle();
    if (handle == 0) {
        return false;
    }

    GLuint boundProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&boundProgram));
    return handle == boundProgram;
}

void ShaderProgram::bind() const
{
    glUseProgram(getHandle());
}

void ShaderProgram::unbind() const
{
    assert(isBound());
    glUseProgram(0);
}

ShaderProgram::ShaderProgram(const vector<string>& vertexAttributes,
                             GLuint vertexShader,
                             GLuint fragmentShader)
{
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);

    for (size_t i = 0; i < vertexAttributes.size(); ++i) {
        glBindAttribLocation(programHandle, static_cast<GLuint>(i), vertexAttributes[i].c_str());
    }

    glLinkProgram(programHandle);

    int linkStatus;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        throw new GlProgramLinkError(programHandle);
    }

    _handle = ProgramHandle(programHandle);

    std::vector<char> nameBuffer;

    int attributeNameMaxLength;
    glGetProgramiv(programHandle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeNameMaxLength);
    nameBuffer.reserve(static_cast<size_t>(attributeNameMaxLength));

    int attributeCount = 0;
    glGetProgramiv(programHandle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    IVL_LOG(Trace, "Parsing shader program {} attributes", attributeCount);

    for (size_t i = 0; i < attributeCount; ++i) {
        GLint size;
        GLint nameLength;
        UniformType type;

        glGetActiveAttrib(programHandle, static_cast<GLuint>(i), attributeNameMaxLength,
                          &nameLength, &size, reinterpret_cast<GLenum*>(&type), &nameBuffer[0]);
        string name(nameBuffer.data(), nameBuffer.data() + nameLength);
        auto attribIt = find(vertexAttributes.begin(), vertexAttributes.end(), name);
        assert(attribIt != vertexAttributes.end());
        auto attribIndex = distance(vertexAttributes.begin(), attribIt);
        IVL_LOG(Trace, "Shader attribute : {} index : {}", name, attribIndex);
        _attributeBindings.emplace_back(name, attribIndex);
    }

    int uniformNameMaxLength;
    glGetProgramiv(programHandle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformNameMaxLength);
    nameBuffer.reserve(static_cast<size_t>(uniformNameMaxLength));

    int uniformCount = 0;
    glGetProgramiv(programHandle, GL_ACTIVE_UNIFORMS, &uniformCount);
    IVL_LOG(Trace, "Parsing shader program {} uniform variables", uniformCount);

    for (size_t i = 0; i < uniformCount; ++i) {
        GLint size;
        GLint nameLength;
        UniformType type;

        glGetActiveUniform(programHandle, static_cast<GLuint>(i), uniformNameMaxLength, &nameLength,
                           &size, reinterpret_cast<GLenum*>(&type), &nameBuffer[0]);
        string name(nameBuffer.data(), nameBuffer.data() + nameLength);
        // Remove array index "[0]" from name
        if (size > 1) {
            assert(name.size() > 3);
            assert(name[name.size() - 1] == ']');
            name = name.substr(0, name.size() - 3);
        }
        GLint location = glGetUniformLocation(programHandle, &name[0]);
        IVL_LOG(Trace, "Uniform variable : {} index : {} location : {} type : {} size : {}", name,
                i, location, static_cast<int>(type), size);
        _uniformBindings.emplace(
            make_pair(name, make_unique<UniformBinding>(
                                UniformVariable(name, type, static_cast<size_t>(size)), location)));
    }
    gl::GlError::assertValidateState();
}

void ShaderProgram::setUniform(Binding<ShaderProgram>& binding,
                               const UniformBinding& uniformBinding,
                               const void* source)
{
    binding.assertBound();

#define UNIFORM_PROPERTY_BIND(POSTFIX, TYPE)                                                       \
    case UniformBufferType<TYPE>::TypeEnum:                                                        \
        glUniform##POSTFIX(uniformBinding.getBindingLocation(),                                    \
                           static_cast<GLsizei>(uniformBinding.getVariable().getCount()),          \
                           reinterpret_cast<const UniformElementTypeT<TYPE>*>(source));            \
        break;
#define UNIFORM_PROPERTY_MATRIX_BIND(POSTFIX, TYPE)                                                \
    case UniformBufferType<TYPE>::TypeEnum:                                                        \
        glUniformMatrix##POSTFIX(uniformBinding.getBindingLocation(),                              \
                                 static_cast<GLsizei>(uniformBinding.getVariable().getCount()),    \
                                 false,                                                            \
                                 reinterpret_cast<const UniformElementTypeT<TYPE>*>(source));      \
        break;
    switch (uniformBinding.getVariable().getType()) {
        UNIFORM_PROPERTY_BIND(1iv, bool)
        UNIFORM_PROPERTY_BIND(2iv, glm::bvec2)
        UNIFORM_PROPERTY_BIND(3iv, glm::bvec3)
        UNIFORM_PROPERTY_BIND(4iv, glm::bvec4)

        UNIFORM_PROPERTY_BIND(1iv, GLint)
        UNIFORM_PROPERTY_BIND(2iv, glm::ivec2)
        UNIFORM_PROPERTY_BIND(3iv, glm::ivec3)
        UNIFORM_PROPERTY_BIND(4iv, glm::ivec4)

        UNIFORM_PROPERTY_BIND(1fv, GLfloat)
        UNIFORM_PROPERTY_BIND(2fv, glm::vec2)
        UNIFORM_PROPERTY_BIND(3fv, glm::vec3)
        UNIFORM_PROPERTY_BIND(4fv, glm::vec4)

        UNIFORM_PROPERTY_MATRIX_BIND(2fv, glm::mat2)
        UNIFORM_PROPERTY_MATRIX_BIND(3fv, glm::mat3)
        UNIFORM_PROPERTY_MATRIX_BIND(4fv, glm::mat4)

        UNIFORM_PROPERTY_BIND(1iv, Texture2D::UniformSampler)
    }

#undef UNIFORM_PROPERTY_BIND
#undef UNIFORM_PROPERTY_MATRIX_BIND
}

VertexDefinition ShaderProgram::getProgramVertexDefinition(const VertexDefinition& source) const
{
    vector<VertexDefinition::Attribute> attributes;
    const auto& sourceAttributes = source.getAttributes();
    for (const auto& binding : _attributeBindings) {
        vector<VertexDefinition::Attribute>::const_iterator it = sourceAttributes.begin();
        while (it != sourceAttributes.end()) {
            if (it->definition.getName() == binding.getName() &&
                it->definition.getIndex() == binding.getIndex()) {
                break;
            }
            ++it;
        }

        if (it == sourceAttributes.end()) {
            IVL_LOG_THROW_ERROR(runtime_error, "Unable to find attribute definition : {}",
                                binding.getIdentifier());
        }

        attributes.push_back(*it);
    }

    return VertexDefinition{attributes, source.getVertexSize()};
}
