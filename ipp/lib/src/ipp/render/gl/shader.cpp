#include <ipp/log.hpp>
#include <ipp/render/gl/shader.hpp>

using namespace std;
using namespace ipp::render::gl;

ShaderHandle ipp::render::gl::compileShader(ShaderKind kind,
                                            const std::vector<std::string>& sources)
{
    IVL_LOG(Trace, "Compiling GL shader");

    GLuint handle = glCreateShader(static_cast<GLenum>(kind));
    if (handle == 0) {
        throw GlError("Unable to create GL Shader handle");
    }

    IVL_LOG(Trace, "GL shaders loaded");
    std::vector<const char*> sourcePtrs;
    std::vector<GLint> sourceLengths;
    for (auto& source : sources) {
        sourcePtrs.push_back(source.c_str());
        sourceLengths.push_back(source.length());
    }

    glShaderSource(handle, sourcePtrs.size(), sourcePtrs.data(), sourceLengths.data());
    IVL_LOG(Trace, "Source ptr lengths {}", sourcePtrs.size());
    gl::GlError::assertValidateState();

    glCompileShader(handle);
    IVL_LOG(Trace, "GL shader compiled");

    int shaderCompileStatus;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &shaderCompileStatus);
    if (shaderCompileStatus == GL_FALSE) {
        std::string sourceCombined;
        for (auto& source : sources) {
            sourceCombined = sourceCombined + source;
        }
        throw GlShaderCompilationError(handle, sourceCombined);
    }
    IVL_LOG(Trace, "GL shader compiled successfully");
    gl::GlError::assertValidateState();

    return ShaderHandle{handle};
}
