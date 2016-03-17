#pragma once

#include <ipp/log.hpp>
#include <ipp/shared.hpp>
#include <ipp/resource/package.hpp>
#include "handle.hpp"
#include "error.hpp"

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief GL shader types enumeration.
 */
enum class ShaderKind : GLenum { Vertex = GL_VERTEX_SHADER, Fragment = GL_FRAGMENT_SHADER };

/**
 * Wrap glDeleteShader because it can be implemented in differently between platforms
 * and they don't play well with HandleDeleter template.
 */
inline void deleteShader(GLuint handle)
{
    glDeleteShader(handle);
}

/**
 * @brief Alias unique_ptr with custom deleter that will release GL shader handle
 */
using ShaderHandle = std::unique_ptr<void, HandleDeleter<deleteShader>>;

/**
 * @brief Compile shader sources in to a shader file.
 */
ShaderHandle compileShader(ShaderKind kind, const std::vector<std::string>& sources);
}
}
}
