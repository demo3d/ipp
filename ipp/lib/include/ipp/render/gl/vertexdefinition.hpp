#pragma once

#include "binding.hpp"
#include <ipp/noncopyable.hpp>
#include <ipp/shared.hpp>

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief GL vertex memory layout definition.
 * VertexDefinition is used to manage vertex description GL state.
 */
class VertexDefinition {
public:
    friend class Binding<VertexDefinition>;

    /**
     * @brief Possible attribute element type
     * Attribute can be made up of multiple elements of same type.
     */
    enum class ElementType : GLenum {
        Byte = GL_BYTE,
        UByte = GL_UNSIGNED_BYTE,
        Fixed = GL_FIXED,
        Short = GL_SHORT,
        UShort = GL_UNSIGNED_SHORT,
        Float = GL_FLOAT
    };

    /**
     * @brief Attribute names are predefined as enum values instead of arbitrary strings.
     */
    enum class AttributeName : int {
        Position = 0,
        Normal,
        Uv,
        Color,
        SkinningIndices,
        SkinningWeights,
        GroupIndices,
        GroupWeights
    };

    /**
     * @brief Vertex attribute layout definition.
     */
    class AttributeDefinition {
    private:
        AttributeName _name;
        GLuint _index;
        gl::VertexDefinition::ElementType _elementType;
        GLuint _elementCount;
        GLboolean _normalized;

    public:
        AttributeDefinition(AttributeName name,
                            GLuint index,
                            gl::VertexDefinition::ElementType elementType,
                            GLuint elementCount,
                            GLboolean normalized)
            : _name{name}
            , _index{index}
            , _elementType{elementType}
            , _elementCount{elementCount}
            , _normalized{normalized}
        {
        }

        AttributeName getName() const
        {
            return _name;
        }

        GLuint getIndex() const
        {
            return _index;
        }

        gl::VertexDefinition::ElementType getElementType() const
        {
            return _elementType;
        }

        GLuint getElementCount() const
        {
            return _elementCount;
        }

        GLboolean getNormalized() const
        {
            return _normalized;
        }

        GLsizei getSize() const;
    };

    /**
     * @brief AttributeDefinition along with vertex offset/index.
     */
    struct Attribute {
        AttributeDefinition definition;
        size_t location;
        size_t offset;
    };

private:
    std::vector<Attribute> _attributes;
    size_t _vertexSize;

    /**
     * @brief Returns
     * Always returns true since there is no meaningfull way to test, implemented because
     * Binding<T> implementation expects it to be present
     */
    bool isBound() const;

    /**
     * @brief Bind vertex definition in active GL context
     */
    void bind() const;

    /**
     * @brief Unbind vertex definition in active GL context
     */
    void unbind() const;

public:
    /**
     * @brief Create a vertex definition based on a list of attributes.
     */
    VertexDefinition(std::vector<Attribute> attributes, size_t vertexSize);

    /**
     * @brief Create a veretx definition from a list of attribute definitions.
     *
     * Determines attribute layout based on attribute order and attributes are tightly packed.
     */
    VertexDefinition(const std::vector<AttributeDefinition>& attributes);

    /**
     * @brief Vertex size in bytes (including padding).
     */
    size_t getVertexSize() const
    {
        return _vertexSize;
    }

    /**
     * @brief Const access to vertex attributes.
     */
    const std::vector<Attribute>& getAttributes() const
    {
        return _attributes;
    }
};
}
}
}
