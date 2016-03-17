#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/log.hpp>
#include <ipp/loop/system.hpp>
#include "entity.hpp"
#include "entityfilter.hpp"

namespace ipp {
namespace entity {

/**
 * @brief Abstract interface for World Entity object observers.
 *
 * Allows implementation to receive Entity update (entity and component add/remove events)
 * notifications from World object.
 */
class WorldEntityObserver : public NonCopyable {
public:
    friend class World;

private:
    World& _world;

    /**
     * @brief Called by World to signal Entity components collection has been updated.
     *
     * Allows implementation to efficiently keep a list of group members only reevaluating
     * membership when components are added/removed.
     *
     * @note When Component is removed from entity it will not be in components list when this
     *       method is invoked but it will still be kept alive untill this function finishes so
     *       any references to removed component are still valid during this call.
     */
    virtual void onEntityComponentsModified(Entity& entity)
    {
    }

    /**
     * @brief Called by World to signal Entity was created in world.
     */
    virtual void onWorldEntityCreated(Entity& entity)
    {
    }

    /**
     * @brief Called by World to signal Entity will be removed from world.
     */
    virtual void onWorldEntityRemoving(Entity& entity)
    {
    }

public:
    WorldEntityObserver(World& world)
        : _world{world}
    {
    }

    /**
     * @brief Owning World instance.
     */
    World& getWorld() const
    {
        return _world;
    }
};
}
}
