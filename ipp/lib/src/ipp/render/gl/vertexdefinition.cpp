#include <ipp/log.hpp>
#include <ipp/render/gl/error.hpp>
#include <ipp/render/gl/vertexdefinition.hpp>

using namespace std;
using namespace ipp::render::gl;

GLsizei VertexDefinition::AttributeDefinition::getSize() const
{
    switch (_elementType) {
        case VertexDefinition::ElementType::Byte:
        case VertexDefinition::ElementType::UByte:
            return _elementCount;
        case VertexDefinition::ElementType::Fixed:
        case VertexDefinition::ElementType::Short:
        case VertexDefinition::ElementType::UShort:
            return _elementCount * 2;
        case VertexDefinition::ElementType::Float:
            return _elementCount * 4;
        default:
            IVL_LOG_THROW_ERROR(logic_error,
                                "Unknown vertex_definition attribute element_type : {0}",
                                (GLenum)_elementType);
    }
}

bool VertexDefinition::isBound() const
{
    return true;
}

void VertexDefinition::bind() const
{
    for (auto& attribute : _attributes) {
        glEnableVertexAttribArray(attribute.location);
        glVertexAttribPointer(attribute.location, attribute.definition.getElementCount(),
                              (GLenum)attribute.definition.getElementType(),
                              attribute.definition.getNormalized(), getVertexSize(),
                              reinterpret_cast<const void*>(attribute.offset));
    }
}

void VertexDefinition::unbind() const
{
    for (auto& attribute : _attributes) {
        glDisableVertexAttribArray(attribute.location);
    }
}

VertexDefinition::VertexDefinition(vector<Attribute> attributes, size_t vertexSize)
    : _attributes{move(attributes)}
    , _vertexSize{vertexSize}
{
}

VertexDefinition::VertexDefinition(const vector<AttributeDefinition>& attributeDefinitions)
{
    _attributes.reserve(attributeDefinitions.size());
    _vertexSize = 0;
    size_t index = 0;
    for (auto& attribute : attributeDefinitions) {
        _attributes.push_back({attribute, index, _vertexSize});
        _vertexSize += attribute.getSize();
        index++;
    }
}
