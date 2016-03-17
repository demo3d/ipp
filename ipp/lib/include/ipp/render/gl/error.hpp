#pragma once

#include <ipp/shared.hpp>

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief Generic GL runtime error that captures error state trough glGetError.
 */
class GlError : public std::runtime_error {
private:
    GLenum _errorCode;

public:
    GlError(const std::string& message);

    GLenum getErrorCode() const
    {
        return _errorCode;
    }

    /**
     * @brief Check glGetError and raise GlError if it's not 0.
     * @note Only enabled in debug builds for perofrmance reasons.
     */
    static void assertValidateState();
};

/**
 * @brief GL operation call was not valid.
 * Used to signal incorrect context setup for a call (not a runtime GL error).
 */
class GlLogicError : public std::logic_error {
public:
    GlLogicError(std::string message)
        : std::logic_error(message)
    {
    }
};

/**
 * Exception that captures GL shader compilation error message and shader source file name.
 */
class GlShaderCompilationError : public std::runtime_error {
private:
    std::string _source;
    std::string _errorMessage;
    std::string _message;

public:
    GlShaderCompilationError(GLuint shaderHandle, const std::string& source);

    virtual const char* what() const throw() override
    {
        return _message.c_str();
    }

    const std::string& getShaderSource() const
    {
        return _source;
    }

    const std::string& getErrorMessage() const
    {
        return _errorMessage;
    }
};

/**
 * Exception that captures GL program link error message.
 */
class GlProgramLinkError : public std::runtime_error {
private:
    std::string _errorMessage;
    std::string _message;

public:
    GlProgramLinkError(GLuint programHandle);

    virtual const char* what() const throw()
    {
        return _message.c_str();
    }

    const std::string& getErrorMessage() const
    {
        return _errorMessage;
    }
};
}
}
}
