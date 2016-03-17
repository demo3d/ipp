#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/resource/package.hpp>
#include "gl/buffer.hpp"
#include "gl/vertexdefinition.hpp"
#include "gl/program.hpp"

namespace ipp {
namespace render {

class Mesh final : public resource::SharedResourceT<Mesh> {
public:
    class Binding final : NonCopyable {
    private:
        Mesh* _mesh;
        gl::Binding<gl::ArrayBuffer> _vertexBinding;
        gl::Binding<gl::ElementArrayBuffer> _indexBinding;
        gl::Binding<gl::VertexDefinition> _vertexDefinitionBinding;

    public:
        Binding(Mesh& mesh);
        Binding(Binding&& other);

        const gl::Binding<gl::ArrayBuffer>& getVertexBinding() const
        {
            return _vertexBinding;
        }

        const gl::Binding<gl::ElementArrayBuffer>& getIndexBinding() const
        {
            return _indexBinding;
        }

        const gl::Binding<gl::VertexDefinition>& getVertexDefinitionBinding() const
        {
            return _vertexDefinitionBinding;
        }

        void assertBound() const;
    };

private:
    std::unique_ptr<gl::VertexDefinition> _vertexDefinition;
    gl::ArrayBuffer _vertexBuffer;
    gl::ElementArrayBuffer _indexBuffer;
    uint32_t _triangleCount;

public:
    Mesh(std::unique_ptr<resource::ResourceBuffer> data);

    /**
     * @brief Call glDrawElements with Mesh geometry.
     */
    void draw(Binding& meshBinding, gl::Binding<gl::ShaderProgram>& shaderProgramBinding) const;

    /**
     * @brief Mesh vertex definition.
     */
    const gl::VertexDefinition& getVertexDefinition() const
    {
        return *_vertexDefinition;
    }

    /**
     * @brief Read-only access to Mesh vertex buffer.
     */
    const gl::ArrayBuffer& getVertexBuffer() const
    {
        return _vertexBuffer;
    }

    /**
     * @brief Read-only access to Mesh index buffer.
     */
    const gl::ElementArrayBuffer& getIndexBuffer() const
    {
        return _indexBuffer;
    }

    /**
     * @brief Mesh triangle count
     */
    uint32_t getTriangleCount() const
    {
        return _triangleCount;
    }
};
}
}
