#include <catch.hpp>
#include <ipp/render/effect.hpp>
#include <ipp/resource/packagefilesystem.hpp>
#include <ipp/context.hpp>

using namespace std;
using namespace ipp;

#define REQUIRE_UNIFORM(NAME, ENUM, COUNT)                                                         \
    auto NAME##It = effectVariables.find(#NAME);                                                   \
    REQUIRE(NAME##It != effectVariables.end());                                                    \
    auto NAME = NAME##It->second;                                                                  \
    REQUIRE(NAME.getUniformVariable().getType() == render::gl::UniformType::ENUM);                 \
    REQUIRE(NAME.getUniformVariable().getCount() == COUNT);

#define REQUIRE_ATTRIBUTE(NAME, INDEX)                                                             \
    auto attribute##NAME##INDEX =                                                                  \
        find_if(attributes.begin(), attributes.end(), [](auto& attribute) {                        \
            return attribute.getName() == render::gl::VertexDefinition::AttributeName::NAME &&     \
                   attribute.getIndex() == INDEX;                                                  \
        });                                                                                        \
    REQUIRE(attribute##NAME##INDEX != attributes.end());

SCENARIO("Render Effect Testing")
{
    GIVEN("Resource Manager with test package")
    {
        Context context{json{}};
        auto& resourceManager = context.getResourceManager();
        resourceManager.registerPackage(make_unique<resource::PackageFileSystem>(
            resourceManager, "ipp/unit_test", "resources/ipp/unit_test"));

        GIVEN("Basic effect")
        {
            auto effect = resourceManager.requestSharedResource<render::Effect>(
                "ipp/unit_test:materials/basic.effect");

            THEN("Effect must contain one pass")
            {
                REQUIRE(effect->getPasses().size() == 1);
            }

            THEN("Effect must contain expected variables")
            {
                auto effectVariables = effect->getVariables();
                REQUIRE_UNIFORM(uniformMat4, Mat4, 1)
                REQUIRE_UNIFORM(uniformVec4, FVec4, 1)
                REQUIRE_UNIFORM(uniformFloat, Float, 1)
                REQUIRE_UNIFORM(uniformVec3Array, FVec3, 3)
                REQUIRE_UNIFORM(fragmentUniformInt, Int, 1)
            }

            THEN("Basic Pass must contain expected attributes")
            {
                auto pass = effect->findPass("basic");
                REQUIRE(pass != nullptr);

                auto attributes = pass->getShaderProgram().getAttributeBindings();
                REQUIRE_ATTRIBUTE(Position, 0)
                REQUIRE_ATTRIBUTE(Normal, 0)
            }
        }

        GIVEN("UV effect")
        {
            auto effect = resourceManager.requestSharedResource<render::Effect>(
                "ipp/unit_test:materials/uv.effect");

            THEN("Effect must contain one pass")
            {
                REQUIRE(effect->getPasses().size() == 1);
            }

            THEN("Pass must contain expected attributes")
            {
                auto pass = effect->findPass("uv");
                REQUIRE(pass != nullptr);

                auto attributes = pass->getShaderProgram().getAttributeBindings();

                REQUIRE_ATTRIBUTE(Position, 0)
                REQUIRE_ATTRIBUTE(Normal, 0)
                REQUIRE_ATTRIBUTE(Uv, 0)
                REQUIRE_ATTRIBUTE(Uv, 1)
            }
        }
    }
}

#undef REQUIRE_UNIFORM
#undef REQUIRE_ATTRIBUTE
