#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/log.hpp>
#include <ipp/loop/system.hpp>
#include "entity.hpp"
#include "entityfilter.hpp"
#include "worldentityobserver.hpp"

namespace ipp {
namespace entity {

/**
 * @brief WorldEntityObserver implementation that uses EntityFilter to determine group membership.
 *
 * EntityFilter type with a static bool match(Entity&) method that will be invoked every time
 * Entity component collection gets updated.
 *
 * When an Entity matches EntityFilter getComponent is called for entity and stored in entity tuple.
 * Whenever Entity components change group entity component tuple is recreated.
 *
 * @note Removing a Entity from group involves O(N) entity tuple copies (vector.erase).
 */
template <typename EntityFilter, typename... ComponentTypes>
class EntityGroupWithFilter : public WorldEntityObserver {
private:
    std::vector<std::tuple<Entity*, ComponentTypes*...>> _entities;

    /**
     * @brief Called by onEntityComponentUpdate when entity gets added to entity group.
     */
    virtual void onGroupEntityAdded(Entity& entity)
    {
    }

    /**
     * @brief Called by onEntityComponentUpdate before entity gets removed from entity group.
     */
    virtual void onGroupEntityRemoved(Entity& entity)
    {
    }

protected:
    /**
     * @brief Override to apply component filtering and update components
     */
    virtual void onEntityComponentsModified(Entity& entity) override
    {
        auto it = std::find_if(_entities.begin(), _entities.end(), [&entity](auto& components) {
            return std::get<0>(components) == &entity;
        });

        auto matches = EntityFilter::match(entity);
        if (matches) {
            if (it == _entities.end()) {
                _entities.emplace_back(&entity, entity.findComponent<ComponentTypes>()...);
                onGroupEntityAdded(entity);
            }
            else {
                *it = std::make_tuple(&entity, entity.findComponent<ComponentTypes>()...);
            }
        }
        else {
            if (it != _entities.end()) {
                onGroupEntityRemoved(entity);
                _entities.erase(it);
            }
        }
    }

    /**
     * @brief Check if empty entity matches filters and dispatch onGroupEntityAdded if it does.
     */
    virtual void onWorldEntityCreated(Entity& entity) override
    {
    }

    /**
     * @brief Override to remove entity from group.
     */
    virtual void onWorldEntityRemoving(Entity& entity) override
    {
        auto it = std::find_if(_entities.begin(), _entities.end(), [&entity](auto& components) {
            return std::get<0>(components) == &entity;
        });
        if (it != _entities.end()) {
            _entities.erase(it);
        }
    }

public:
    EntityGroupWithFilter(World& world)
        : WorldEntityObserver(world)
    {
    }

    /**
     * @brief Group active entities.
     */
    const std::vector<std::tuple<Entity*, ComponentTypes*...>>& getEntities() const
    {
        return _entities;
    }

    /**
     * @brief Owning World object.
     */
    World& getWorld() const
    {
        return _world;
    }
};

/**
 * @brief Alias for EntityGroupWithFilter that must contain all Components and stores a reference.
 */
template <typename... Components>
using EntityGroupWithComponents =
    EntityGroupWithFilter<ContainsAllComponents<Components...>, Components...>;
}
}
