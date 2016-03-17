#include <catch.hpp>
#include <ipp/entity/world.hpp>
#include <ipp/entity/entitygroup.hpp>

using namespace std;
using namespace ipp::entity;

template <int N>
class DummyComponent : public ComponentT<DummyComponent<N>> {
public:
    DummyComponent(Entity& entity)
        : ComponentT<DummyComponent<N>>(entity)
    {
    }

    static const string ComponentTypeName;
};

template <int N>
class DummyComponentGroup final : public EntityGroupWithComponents<DummyComponent<N>> {
private:
    /**
     * @brief Called by onEntityComponentUpdate when entity gets added to entity group.
     */
    virtual void onGroupEntityAdded(Entity& entity)
    {
        matchingComponents.push_back(entity.findComponent<DummyComponent<N>>());
    }

    /**
     * @brief Called by onEntityComponentUpdate before entity gets removed from entity group.
     */
    virtual void onGroupEntityRemoved(Entity& entity)
    {
        auto it = find_if(matchingComponents.begin(), matchingComponents.end(),
                          [&entity](auto component) { return &component->getEntity() == &entity; });
        REQUIRE(it != matchingComponents.end());
        matchingComponents.erase(it);
    }

public:
    DummyComponentGroup(World& world)
        : EntityGroupWithComponents<DummyComponent<N>>(world)
    {
    }

    std::vector<DummyComponent<N>*> matchingComponents;
    static const string SystemTypeName;
};

typedef DummyComponent<1> ComponentA;
typedef DummyComponent<2> ComponentB;
typedef DummyComponent<3> ComponentC;

template <>
const string ComponentA::ComponentTypeName = "DummyComponentA";
template <>
const string ComponentB::ComponentTypeName = "DummyComponentB";
template <>
const string ComponentC::ComponentTypeName = "DummyComponentC";

typedef DummyComponentGroup<1> GroupA;
typedef DummyComponentGroup<2> GroupB;
typedef DummyComponentGroup<3> GroupC;

template <>
const string GroupA::SystemTypeName = "DummyComponentGroupA";
template <>
const string GroupB::SystemTypeName = "DummyComponentGroupB";
template <>
const string GroupC::SystemTypeName = "DummyComponentGroupC";

SCENARIO("World test")
{
    GIVEN("World")
    {
        World world;

        THEN("Creating Entity with ID 0 must fail")
        {
            REQUIRE_THROWS(world.createEntity(0, "Fail"));
        }

        GIVEN("Entities")
        {
            auto entityA = world.createEntity(1, "EntityA");
            auto entityB = world.createEntity(2, "EntityB");
            auto entityC = world.createEntity(3, "EntityC");

            THEN("Querying entities by ID must work")
            {
                REQUIRE(world.findEntity(1) == entityA);
                REQUIRE(world.findEntity(2) == entityB);
                REQUIRE(world.findEntity(3) == entityC);
            }

            THEN("Querying entities by name must work")
            {
                REQUIRE(world.findEntity("EntityA") == entityA);
                REQUIRE(world.findEntity("EntityB") == entityB);
                REQUIRE(world.findEntity("EntityC") == entityC);
            }

            THEN("Inserting entities with same name and/or ID must fail")
            {
                REQUIRE_THROWS(world.createEntity(1, "EntityA"));
                REQUIRE_THROWS(world.createEntity(2, "Foo"));
                REQUIRE_THROWS(world.createEntity(100, "EntityC"));
            }

            THEN("Querying for nonexisting components must return nullptr")
            {
                REQUIRE(entityA->findComponent<ComponentA>() == nullptr);
                REQUIRE(entityA->findComponent<ComponentB>() == nullptr);
                REQUIRE(entityA->findComponent<ComponentB>() == nullptr);
                REQUIRE(entityB->findComponent<ComponentB>() == nullptr);
                REQUIRE(entityC->findComponent<ComponentC>() == nullptr);
            }

            GIVEN("Components")
            {
                auto componentA = entityA->createComponent<ComponentA>();
                auto componentB = entityB->createComponent<ComponentB>();
                auto componentC = entityC->createComponent<ComponentC>();

                THEN("Creating the same component twice must fail")
                {
                    REQUIRE_THROWS(entityA->createComponent<ComponentA>());
                }

                GIVEN("Entity groups")
                {
                    auto groupA = world.createEntityObserver<GroupA>();
                    auto groupB = world.createEntityObserver<GroupB>();
                    auto groupC = world.createEntityObserver<GroupC>();

                    THEN("Entities/components must be registered in coresponding groups")
                    {
                        REQUIRE(groupA->matchingComponents.size() == 1);
                        REQUIRE(groupA->matchingComponents[0] == componentA);
                        REQUIRE(groupB->matchingComponents.size() == 1);
                        REQUIRE(groupB->matchingComponents[0] == componentB);
                        REQUIRE(groupC->matchingComponents.size() == 1);
                        REQUIRE(groupC->matchingComponents[0] == componentC);
                    }

                    GIVEN("Extra components")
                    {
                        auto componentAB = entityA->createComponent<ComponentB>();
                        auto componentAC = entityA->createComponent<ComponentC>();

                        THEN("Entities/components must be registered in coresponding groups")
                        {
                            REQUIRE(groupA->matchingComponents.size() == 1);
                            REQUIRE(groupB->matchingComponents.size() == 2);
                            REQUIRE(groupB->matchingComponents[1] == componentAB);
                            REQUIRE(groupC->matchingComponents.size() == 2);
                            REQUIRE(groupC->matchingComponents[1] == componentAC);
                        }

                        WHEN("Removing extra components")
                        {
                            entityA->removeComponent(componentAB);
                            entityA->removeComponent(componentAC);

                            THEN("Entities/components must be unregistered in coresponding groups")
                            {
                                REQUIRE(groupA->matchingComponents.size() == 1);
                                REQUIRE(groupB->matchingComponents.size() == 1);
                                REQUIRE(groupC->matchingComponents.size() == 1);
                            }

                            THEN("Query for removed component must return nullptr")
                            {
                                REQUIRE(entityA->findComponent<ComponentB>() == nullptr);
                                REQUIRE(entityA->findComponent<ComponentC>() == nullptr);
                            }
                        }
                    }
                }
            }
        }
    }
}
