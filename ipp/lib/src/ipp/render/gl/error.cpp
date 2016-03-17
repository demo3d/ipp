#include <ipp/log.hpp>
#include <ipp/render/gl/error.hpp>

using namespace std;
using namespace ipp::render::gl;

GlError::GlError(const string& message)
    : runtime_error(message)
{
    _errorCode = glGetError();
}

void GlError::assertValidateState()
{
#ifdef IVL_DEBUG_BUILD
    GLenum errorCode = glGetError();
    if (errorCode != 0) {
        IVL_LOG_THROW_ERROR(GlError, "GL error state invalid : {0:0X}.", (int)errorCode);
    }
#endif
}

GlShaderCompilationError::GlShaderCompilationError(GLuint shaderHandle, const string& source)
    : runtime_error("")
    , _source{source}
{
    int errorMessageLength;
    glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &errorMessageLength);
    _errorMessage.reserve(errorMessageLength);
    glGetShaderInfoLog(shaderHandle, errorMessageLength, &errorMessageLength, &_errorMessage[0]);
    _message = IVL_LOG(Error, "Error compiling GL shader message : {}", _errorMessage);
    IVL_LOG(Error, "Shader source : \n-------------------------------------------\n{}", source);
}

GlProgramLinkError::GlProgramLinkError(GLuint programHandle)
    : runtime_error("")
{
    int errorMessageLength;
    glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &errorMessageLength);
    _errorMessage.reserve(errorMessageLength);
    glGetProgramInfoLog(programHandle, errorMessageLength, &errorMessageLength, &_errorMessage[0]);
    _message = IVL_LOG(Error, "GL Program link error : ", _errorMessage);
}
