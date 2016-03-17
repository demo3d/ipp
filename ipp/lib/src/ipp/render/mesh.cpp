#include <ipp/resource/resourcemanager.hpp>
#include <ipp/render/gl/error.hpp>
#include <ipp/render/mesh.hpp>
#include <ipp/schema/resource/render/vertex_generated.h>
#include <ipp/schema/resource/render/mesh_generated.h>

using namespace std;
using namespace ipp;
using namespace ipp::render;
using namespace ipp::resource;

template <>
const string SharedResourceT<Mesh>::ResourceTypeName = "RenderMesh";

gl::VertexDefinition::ElementType GetElementTypeGLEnum(
    schema::resource::render::vertex::ElementType elementType)
{
#define ELEMENT_TYPE_MAP(TYPE)                                                                     \
    case schema::resource::render::vertex::ElementType::ElementType_##TYPE:                        \
        return gl::VertexDefinition::ElementType::TYPE;

    switch (elementType) {
        ELEMENT_TYPE_MAP(Byte)
        ELEMENT_TYPE_MAP(UByte)
        ELEMENT_TYPE_MAP(Short)
        ELEMENT_TYPE_MAP(UShort)
        ELEMENT_TYPE_MAP(Float)
        ELEMENT_TYPE_MAP(Fixed)
        default:
            throw logic_error("Invalid Vertex ElementType !");
    }

#undef ELEMENT_TYPE_MAP
}

gl::VertexDefinition::AttributeName GetAttributeNameGLEnum(
    schema::resource::render::vertex::AttributeName attributeName)
{
#define ATTRIBUTE_NAME_MAP(NAME)                                                                   \
    case schema::resource::render::vertex::AttributeName_##NAME:                                   \
        return gl::VertexDefinition::AttributeName::NAME;

    switch (attributeName) {
        ATTRIBUTE_NAME_MAP(Position)
        ATTRIBUTE_NAME_MAP(Normal)
        ATTRIBUTE_NAME_MAP(Uv)
        ATTRIBUTE_NAME_MAP(Color)
        ATTRIBUTE_NAME_MAP(SkinningIndices)
        ATTRIBUTE_NAME_MAP(SkinningWeights)
        ATTRIBUTE_NAME_MAP(GroupIndices)
        ATTRIBUTE_NAME_MAP(GroupWeights)
        default:
            throw logic_error("Invalid Vertex AttributeName !");
    }

#undef ATTRIB_NAME_MAP
}

Mesh::Binding::Binding(Mesh& mesh)
    : _mesh{&mesh}
    , _vertexBinding{mesh.getVertexBuffer()}
    , _indexBinding{mesh.getIndexBuffer()}
    , _vertexDefinitionBinding{mesh.getVertexDefinition()}
{
}

Mesh::Binding::Binding(Binding&& other)
    : _mesh{other._mesh}
    , _vertexBinding{move(other._vertexBinding)}
    , _indexBinding{move(other._indexBinding)}
    , _vertexDefinitionBinding{move(other._vertexDefinitionBinding)}
{
    other._mesh = nullptr;
}
void Mesh::Binding::assertBound() const
{
    _vertexBinding.assertBound();
    _indexBinding.assertBound();
    _vertexDefinitionBinding.assertBound();
}

Mesh::Mesh(unique_ptr<ResourceBuffer> data)
    : SharedResourceT<Mesh>(data->getResourceManager(), data->getResourcePath())
{
    auto meshData = schema::resource::render::mesh::GetMesh(data->getData());
    _triangleCount = meshData->triangleCount();
    auto vertexData = meshData->vertices();

    IVL_LOG(Info, "Loading Mesh from resource : {} with {} triangles.", getResourcePath(),
            _triangleCount);

    // load vertex definition
    size_t vertexSize = 0;
    vector<gl::VertexDefinition::AttributeDefinition> vertexAttributes;
    for (const auto attrib : *vertexData->attributes()) {
        IVL_LOG(Trace, "Mesh vertex attribute definition : {}, index : {}, element type : {}, "
                       "element count : {}, normalized : {}",
                static_cast<int>(attrib->name()), attrib->index(),
                static_cast<int>(GetElementTypeGLEnum(attrib->elementType())),
                attrib->elementCount(), attrib->normalized());
        gl::VertexDefinition::AttributeDefinition attributeDefinition(
            GetAttributeNameGLEnum(attrib->name()), attrib->index(),
            GetElementTypeGLEnum(attrib->elementType()), attrib->elementCount(),
            attrib->normalized());
        vertexAttributes.push_back(attributeDefinition);
        vertexSize += static_cast<size_t>(attributeDefinition.getSize());
    }
    _vertexDefinition = make_unique<gl::VertexDefinition>(vertexAttributes);
    IVL_LOG(Trace, "Mesh vertex size : {}", vertexSize);

    // load vertex buffer
    {
        gl::Binding<gl::ArrayBuffer> binding{_vertexBuffer};
        _vertexBuffer.bufferData(binding, vertexData->buffer()->size(),
                                 vertexData->buffer()->data());
    }
    gl::GlError::assertValidateState();
    IVL_LOG(Trace, "Vertex buffer created - size : {}, number of vertices : {}",
            vertexData->buffer()->size(), vertexData->buffer()->size() / vertexSize);

    // load index buffer
    {
        gl::Binding<gl::ElementArrayBuffer> binding{_indexBuffer};
        _indexBuffer.bufferData(binding, meshData->indices()->size(), meshData->indices()->data());
    }
    gl::GlError::assertValidateState();

    IVL_LOG(Trace, "Index buffer created - size : {}, index count : {}, triangle count : {}",
            meshData->indices()->size(), meshData->indices()->size() / 2,
            meshData->indices()->size() / (3 * 2));
}

void Mesh::draw(Binding& meshBinding, gl::Binding<gl::ShaderProgram>& shaderProgram) const
{
    meshBinding.assertBound();
    shaderProgram.assertBound();

    glDrawElements(GL_TRIANGLES, static_cast<GLint>(_triangleCount * 3), GL_UNSIGNED_SHORT, 0);
}
