#pragma once

#include "texture2d.hpp"
#include <ipp/log.hpp>
#include <ipp/shared.hpp>

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief Enumerate all UniformVariable types
 */
enum class UniformType : GLenum {
    Bool = GL_BOOL,
    BVec2 = GL_BOOL_VEC2,
    BVec3 = GL_BOOL_VEC3,
    BVec4 = GL_BOOL_VEC4,

    Int = GL_INT,
    IVec2 = GL_INT_VEC2,
    IVec3 = GL_INT_VEC3,
    IVec4 = GL_INT_VEC4,

    Float = GL_FLOAT,
    FVec2 = GL_FLOAT_VEC2,
    FVec3 = GL_FLOAT_VEC3,
    FVec4 = GL_FLOAT_VEC4,

    Mat2 = GL_FLOAT_MAT2,
    Mat3 = GL_FLOAT_MAT3,
    Mat4 = GL_FLOAT_MAT4,

    Sampler2D = GL_SAMPLER_2D,
    SamplerCube = GL_SAMPLER_CUBE
};

/**
 * Template mapping from T to uniform_type and BufferType trough specializations.
 * Specialization must be provided for T for default implementation of Uniform<T>.
 */
template <typename T>
struct UniformBufferType {
};

#define UNIFORM_BUFFER_TYPE(TYPE, BUFFER_TYPE, ELEMENT_TYPE, TYPE_ENUM)                            \
    template <>                                                                                    \
    struct UniformBufferType<TYPE> {                                                               \
        typedef BUFFER_TYPE BufferType;                                                            \
        typedef ELEMENT_TYPE ElementType;                                                          \
        static const UniformType TypeEnum = UniformType::TYPE_ENUM;                                \
    };

UNIFORM_BUFFER_TYPE(bool, int, GLint, Bool)
UNIFORM_BUFFER_TYPE(glm::bvec2, glm::ivec2, GLint, BVec2)
UNIFORM_BUFFER_TYPE(glm::bvec3, glm::ivec3, GLint, BVec3)
UNIFORM_BUFFER_TYPE(glm::bvec4, glm::ivec4, GLint, BVec4)

UNIFORM_BUFFER_TYPE(int, int, GLint, Int)
UNIFORM_BUFFER_TYPE(glm::ivec2, glm::ivec2, GLint, IVec2)
UNIFORM_BUFFER_TYPE(glm::ivec3, glm::ivec3, GLint, IVec3)
UNIFORM_BUFFER_TYPE(glm::ivec4, glm::ivec4, GLint, IVec4)

UNIFORM_BUFFER_TYPE(float, float, GLfloat, Float)
UNIFORM_BUFFER_TYPE(glm::vec2, glm::vec2, GLfloat, FVec2)
UNIFORM_BUFFER_TYPE(glm::vec3, glm::vec3, GLfloat, FVec3)
UNIFORM_BUFFER_TYPE(glm::vec4, glm::vec4, GLfloat, FVec4)

UNIFORM_BUFFER_TYPE(glm::mat2, glm::mat2, GLfloat, Mat2)
UNIFORM_BUFFER_TYPE(glm::mat3, glm::mat3, GLfloat, Mat3)
UNIFORM_BUFFER_TYPE(glm::mat4, glm::mat4, GLfloat, Mat4)

UNIFORM_BUFFER_TYPE(Texture2D::UniformSampler, GLuint, GLint, Sampler2D)

#undef UNIFORM_TYPE

/**
 * @brief Re-export UniformBufferType<T>::BufferType.
 * Buffer type represents type used to store uniform variable in buffer memory (to match GL layout)
 */
template <typename T>
using UniformBufferTypeT = typename UniformBufferType<T>::BufferType;

/**
 * @brief Re-export UniformBufferType<T>::ElementType
 */
template <typename T>
using UniformElementTypeT = typename UniformBufferType<T>::ElementType;

/**
 * Cast value T to it's uniform buffer type value (UniformBufferType<T>::BufferType).
 */
template <typename T>
UniformBufferTypeT<T> uniform_cast(const T& source)
{
    return static_cast<UniformBufferTypeT<T>>(source);
}

/**
 * Cast from uniform buffer value (UniformBufferType<T>::BufferType) to value of type T.
 */
template <typename T>
T uniform_cast(const UniformBufferTypeT<T>& source)
{
    return static_cast<T>(source);
}

/**
 * @brief Information describing a GL program uniform variable.
 */
class UniformVariable {
private:
    std::string _name;
    UniformType _type;
    size_t _count;

public:
    UniformVariable(std::string name, UniformType type, size_t count)
        : _name{std::move(name)}
        , _type{type}
        , _count{count}
    {
    }

    /**
     * @brief Variable name
     */
    const std::string& getName() const
    {
        return _name;
    }

    /**
     * @brief Uniform value type
     */
    UniformType getType() const
    {
        return _type;
    }

    /**
     * @brief Number of values this uniform variable can contain (> 1 means array)
     */
    size_t getCount() const
    {
        return _count;
    }

    /**
     * @brief Return variable size (element type size * count).
     */
    size_t getSize() const
    {
        return _count * getElementSize(_type);
    }

    /**
     * @brief Byte size of UniformType element.
     */
    static size_t getElementSize(UniformType type)
    {
        switch (type) {
            case UniformType::Bool:
            case UniformType::Float:
            case UniformType::Int:
            case UniformType::Sampler2D:
            case UniformType::SamplerCube:
                return 4;
            case UniformType::FVec2:
            case UniformType::IVec2:
            case UniformType::BVec2:
                return 8;
            case UniformType::FVec3:
            case UniformType::IVec3:
            case UniformType::BVec3:
                return 12;
            case UniformType::FVec4:
            case UniformType::IVec4:
            case UniformType::BVec4:
                return 16;
            case UniformType::Mat2:
                return 16;
            case UniformType::Mat3:
                return 36;
            case UniformType::Mat4:
                return 64;
            default:
                IVL_LOG_THROW_ERROR(std::logic_error, "Unknown Uniform Type : {0}", (int)type);
        }
    }
};
}
}
}
