#include <catch.hpp>
#include <ipp/resource/packagefilesystem.hpp>
#include <ipp/resource/packageipparchive.hpp>
#include <ipp/context.hpp>

using namespace std;
using namespace ipp;
using namespace ipp::resource;

class TestResource final : public SharedResourceT<TestResource> {
private:
    string _data;

public:
    TestResource(unique_ptr<ResourceBuffer> data)
        : SharedResourceT(data->getResourceManager(), data->getResourcePath())
        , _data{data->getData(), data->getSize()}
    {
    }

    const string& getData() const
    {
        return _data;
    }
};

template <>
const string SharedResourceT<TestResource>::ResourceTypeName = "TestResource";

SCENARIO("Resource tests")
{
    GIVEN("Resource Manager")
    {
        Context context{json{}};
        auto& resourceManager = context.getResourceManager();
        GIVEN("File system test package")
        {
            resourceManager.registerPackage(make_unique<PackageFileSystem>(
                resourceManager, "ipp/unit_test", "resources/ipp/unit_test"));

            WHEN("Loading test resource file")
            {
                auto resource = resourceManager.requestSharedResource<TestResource>(
                    "ipp/unit_test:custom/test.txt");
                THEN("File contents must be a simple_test string")
                {
                    auto data = resource->getData();
                    REQUIRE(data.find("simple_test") == 0);
                }

                THEN("Reloading a file must return same instance as original.")
                {
                    REQUIRE(resource ==
                            resourceManager.requestSharedResource<TestResource>(
                                "ipp/unit_test:custom/test.txt"));
                }

                THEN("Requestiong invalid resource must raise an exception.")
                {
                    REQUIRE_THROWS(
                        resourceManager.requestSharedResource<TestResource>("wrong_format"));
                    REQUIRE_THROWS(
                        resourceManager.requestSharedResource<TestResource>("foo:invalid.txt"));
                    REQUIRE_THROWS(resourceManager.requestSharedResource<TestResource>(
                        "ipp/unit_test:custom/invalid.txt"));
                }
            }
        }

        GIVEN("Archive test package")
        {
            resourceManager.registerPackage(make_unique<PackageIPPArchive>(
                resourceManager, "ipp/unit_test", "resources/ipp/unit_test.ipparch"));

            WHEN("Loading test resource file")
            {
                auto resource = resourceManager.requestSharedResource<TestResource>(
                    "ipp/unit_test:custom/test.txt");
                THEN("File contents must be a simple_test string")
                {
                    auto data = resource->getData();
                    REQUIRE(data.find("simple_test") == 0);
                }

                THEN("Reloading a file must return same instance as original.")
                {
                    REQUIRE(resource ==
                            resourceManager.requestSharedResource<TestResource>(
                                "ipp/unit_test:custom/test.txt"));
                }

                THEN("Requestiong invalid resource must raise an exception.")
                {
                    REQUIRE_THROWS(
                        resourceManager.requestSharedResource<TestResource>("wrong_format"));
                    REQUIRE_THROWS(
                        resourceManager.requestSharedResource<TestResource>("foo:invalid.txt"));
                    REQUIRE_THROWS(resourceManager.requestSharedResource<TestResource>(
                        "ipp/unit_test:custom/invalid.txt"));
                }
            }
        }
    }
}
