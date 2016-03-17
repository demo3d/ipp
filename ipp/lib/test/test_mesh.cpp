#include <catch.hpp>
#include <ipp/render/mesh.hpp>
#include <ipp/resource/packagefilesystem.hpp>
#include <ipp/context.hpp>

using namespace std;
using namespace ipp;

SCENARIO("Render Mesh testing")
{
    GIVEN("Resource Manager with test package")
    {
        Context context{json{}};
        auto& resourceManager = context.getResourceManager();
        resourceManager.registerPackage(make_unique<resource::PackageFileSystem>(
            resourceManager, "ipp/unit_test", "resources/ipp/unit_test"));

        GIVEN("Cube mesh")
        {
            auto mesh = resourceManager.requestSharedResource<render::Mesh>(
                "ipp/unit_test:blends/skinned_cube.mesh");

            THEN("Check triangle count")
            {
                REQUIRE(mesh->getTriangleCount() == 12);
            }

            THEN("Check vertex definition")
            {
                auto vertexDefinition = mesh->getVertexDefinition();
                auto attributes = vertexDefinition.getAttributes();

                REQUIRE(attributes[0].definition.getName() ==
                        render::gl::VertexDefinition::AttributeName::Position);
                REQUIRE(attributes[0].location == 0);

                REQUIRE(attributes[1].definition.getName() ==
                        render::gl::VertexDefinition::AttributeName::Normal);
                REQUIRE(attributes[1].location == 1);
            }

            THEN("Try binding to context with mesh vertex definition")
            {
                auto binding = mesh->bind(mesh->getVertexDefinition());

                GLint isPositionBound;
                glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isPositionBound);
                REQUIRE(isPositionBound);

                GLint isNormalBound;
                glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isNormalBound);
                REQUIRE(isNormalBound);
            }

            THEN("Try binding to context with custom vertex definition using position only")
            {
                auto meshDefinition = mesh->getVertexDefinition();
                auto customDefinition = render::gl::VertexDefinition(
                    {meshDefinition.getAttributes()[0]}, meshDefinition.getVertexSize());

                auto binding = mesh->bind(customDefinition);

                GLint isPositionBound;
                glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isPositionBound);
                REQUIRE(isPositionBound);

                GLint isNormalBound;
                glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &isNormalBound);
                REQUIRE_FALSE(isNormalBound);
            }
        }
    }
}
